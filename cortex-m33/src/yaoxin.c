#include <string.h>

#include "ipc.h"
#include "spec.h"
#include "io.h"
#include "DrvGPIO010.h"
#include "DrvPWMTMR010.h"
#include "DrvUART010.h"
#include "leo_cm33.h"

#define YAOXIN_GPIO_POLARITY		0 /* 0:Active Low，1:Active High */

/*
 * 编译期调试打印开关，默认关闭。
 * 打开：make CFLAGS_EXTRA="-DYAOXIN_DEBUG_PRINT=1"
 */
#ifndef YAOXIN_DEBUG_PRINT
#define YAOXIN_DEBUG_PRINT		0
#endif

#if YAOXIN_DEBUG_PRINT
#define YAOXIN_PRINTF(...)		fLib_printf(__VA_ARGS__)
#else
#define YAOXIN_PRINTF(...)		do { } while (0)
#endif

/* 本地环形队列：ISR 入队固化时标，主循环间隔出队写共享内存 */
#define YAOXIN_LOCAL_QUEUE_DEPTH	32
/* 两次 notify 最小间隔（ms），给大核 IRQ 拷贝留时间；时标在入队时已固定 */
#define YAOXIN_IPC_GAP_MS		1

extern volatile UINT32 T4_Tick;

static UINT64 g_wall_base_sec;
static uint32 g_wall_base_ns;
static UINT64 g_assert_base;	/* 同步时刻大核启动时长 ns */
static uint32 g_time_base_ms;	/* 同步时刻本地 T4_Tick */
static uint32 g_time_synced;
static uint32 g_debounce_ms = YAOXIN_DEBOUNCE_MS_DEFAULT;
static uint32 g_trigger_count[32];
static uint32 g_last_trigger_ms[32];
static uint32 g_debug_last_ms;

/* SPSC：GPIO ISR 生产，主循环消费 */
static yaoxin_event_t g_event_q[YAOXIN_LOCAL_QUEUE_DEPTH];
static volatile uint32 g_q_head;	/* 消费端 */
static volatile uint32 g_q_tail;	/* 生产端 */
static uint32 g_q_drop;
static uint32 g_last_notify_ms;
static uint32 g_notify_pending;	/* 1=已 notify，需等 GAP 后再发下一条 */

static ipc_data_area_t *ipc_channel_addr(unsigned int channel)
{
	return (ipc_data_area_t *)(iRAM_DATA_BASE +
				   channel * IPC_PER_CHANNEL_SRAM_SIZE);
}

static void ipc_notify_a7(unsigned int channel)
{
	writel((1 << (channel + 1)) | 1, 0x57f00100);
}

static uint32 yaoxin_now_ms(void)
{
	return T4_Tick;
}

static const char *yaoxin_channel_str(uint32 channel)
{
	switch (channel) {
	case YAOXIN_CH_SIGNAL1:
		return "signal1";
	case YAOXIN_CH_SIGNAL2:
		return "signal2";
	case YAOXIN_CH_DOOR:
		return "door";
	default:
		return "unknown";
	}
}

/*
 * 时标换算（T4_Tick 须为真实 1ms）：
 *   wall / assert = 同步基准 + (当前 tick - 同步 tick)
 */
static void yaoxin_get_timestamp(uint32 now_ms,
				 UINT64 *wall_sec, uint32 *wall_ns,
				 UINT64 *assert_time)
{
	uint32 elapsed_ms = now_ms - g_time_base_ms;
	UINT64 add_ns = (UINT64)elapsed_ms * 1000000ULL;
	UINT64 total_ns;

	total_ns = (UINT64)g_wall_base_ns + add_ns;
	*wall_sec = g_wall_base_sec + total_ns / 1000000000ULL;
	*wall_ns = (uint32)(total_ns % 1000000000ULL);
	*assert_time = g_assert_base + add_ns;
}

static uint32 yaoxin_pin_to_channel(uint32 gpio_pin)
{
	switch (gpio_pin) {
	case YAOXIN_PIN_SIGNAL1:
		return YAOXIN_CH_SIGNAL1;
	case YAOXIN_PIN_SIGNAL2:
		return YAOXIN_CH_SIGNAL2;
	case YAOXIN_PIN_DOOR:
		return YAOXIN_CH_DOOR;
	default:
		return 0xFF;
	}
}

static uint32 read_pin_level(uint32 gpio_pin)
{
	return (fLib_Gpio_ReadData(GPIO_FTGPIO010_PA_BASE) >> gpio_pin) & 1U;
}

static uint32 yaoxin_state_get(uint32 gpio_pin)
{
	if (!YAOXIN_GPIO_POLARITY)
		return read_pin_level(gpio_pin) ? 0 : 1;
	else
		return read_pin_level(gpio_pin) ? 1 : 0;
}

static void yaoxin_gpio_configure(uint32 gpio_pin)
{
	fLib_SetGpioDir(GPIO_FTGPIO010_PA_BASE, gpio_pin, GPIO_DIR_INPUT);
	fLib_SetGpioEdgeMode(GPIO_FTGPIO010_PA_BASE, gpio_pin, BOTH);
	fLib_SetGpioTrigger(GPIO_FTGPIO010_PA_BASE, gpio_pin, GPIO_EDGE);
	/* 关闭硬件 debounce，避免挡分辨率/雪崩短间隔脉冲；软件消抖仍可用 */
	fLib_DisableGpioBounce(GPIO_FTGPIO010_PA_BASE, gpio_pin);
	fLib_ClearGpioInt(GPIO_FTGPIO010_PA_BASE, 1u << gpio_pin);
	fLib_SetGpioIntEnable(GPIO_FTGPIO010_PA_BASE, gpio_pin);
	fLib_SetGpioIntUnMask(GPIO_FTGPIO010_PA_BASE, gpio_pin);
}

static uint32 yaoxin_queue_full(void)
{
	return ((g_q_tail + 1U) % YAOXIN_LOCAL_QUEUE_DEPTH) == g_q_head;
}

static uint32 yaoxin_queue_empty(void)
{
	return g_q_head == g_q_tail;
}

/* ISR 入队：时标用 ISR 采样的 now_ms，保证同批双沿时标一致 */
static void yaoxin_enqueue_event(uint32 gpio_pin, uint32 now_ms)
{
	yaoxin_event_t *slot;
	uint32 next;
	uint32 value;

	if (yaoxin_queue_full()) {
		g_q_drop++;
		YAOXIN_PRINTF("[yaoxin] queue full, drop pin=%u total_drop=%u\n",
			      gpio_pin, g_q_drop);
		return;
	}

	value = yaoxin_state_get(gpio_pin);
	slot = &g_event_q[g_q_tail];
	memset(slot, 0, sizeof(*slot));
	slot->channel = yaoxin_pin_to_channel(gpio_pin);
	slot->count = g_trigger_count[gpio_pin];
	slot->value = value ? YAOXIN_VALUE_ASSERT : YAOXIN_VALUE_RESTORE;
	yaoxin_get_timestamp(now_ms, &slot->wall_time_sec, &slot->wall_time_ns,
			     &slot->assert_time);

	next = (g_q_tail + 1U) % YAOXIN_LOCAL_QUEUE_DEPTH;
	g_q_tail = next;

	YAOXIN_PRINTF("[yaoxin] enq %s ch=0x%x cnt=%u val=%u "
		      "wall=%llu.%09u assert=%llu\n",
		      yaoxin_channel_str(slot->channel),
		      (unsigned)slot->channel,
		      (unsigned)slot->count,
		      (unsigned)slot->value,
		      (unsigned long long)slot->wall_time_sec,
		      (unsigned)slot->wall_time_ns,
		      (unsigned long long)slot->assert_time);
}

static void yaoxin_ipc_post_event(const yaoxin_event_t *event)
{
	ipc_data_area_t *ipc_data = ipc_channel_addr(IPC_YAOXIN_CHANNEL);

	ipc_data->id = IPC_YAOXIN_CHANNEL;
	ipc_data->len = sizeof(yaoxin_event_t);
	memcpy(ipc_data->data, event, sizeof(yaoxin_event_t));
	ipc_notify_a7(IPC_YAOXIN_CHANNEL);
}

/*
 * 主循环调用：每次最多发送 1 条；两条之间至少隔 YAOXIN_IPC_GAP_MS，
 * 避免共享 SRAM 单槽被覆盖。
 */
void yaoxin_queue_poll(void)
{
	yaoxin_event_t event;
	uint32 now_ms;
	uint32 next;

	if (yaoxin_queue_empty())
		return;

	now_ms = yaoxin_now_ms();
	if (g_notify_pending &&
	    (now_ms - g_last_notify_ms) < YAOXIN_IPC_GAP_MS)
		return;

	memcpy(&event, &g_event_q[g_q_head], sizeof(event));
	next = (g_q_head + 1U) % YAOXIN_LOCAL_QUEUE_DEPTH;
	g_q_head = next;

	yaoxin_ipc_post_event(&event);
	g_last_notify_ms = now_ms;
	g_notify_pending = 1;

	YAOXIN_PRINTF("[yaoxin] tx ch=0x%x cnt=%u val=%u\n",
		      (unsigned)event.channel,
		      (unsigned)event.count,
		      (unsigned)event.value);
}

static void yaoxin_send_ack(uint32 seq, uint32 result)
{
	ipc_data_area_t *ipc_tx = ipc_channel_addr(IPC_CFG_CHANNEL);
	ipc_ack_t *ack = (ipc_ack_t *)ipc_tx->data;

	ipc_tx->id = IPC_CFG_CHANNEL;
	ipc_tx->len = sizeof(ipc_ack_t);
	ack->cmd = IPC_CMD_ACK;
	ack->seq = seq;
	ack->result = result;

	YAOXIN_PRINTF("[yaoxin] cfg ack seq=%u result=%u\n", seq, result);
	ipc_notify_a7(IPC_CFG_CHANNEL);
}

/* 兼容接口：入队（时标取调用时刻） */
void yaoxin_send_event(uint32 gpio_pin)
{
	yaoxin_enqueue_event(gpio_pin, yaoxin_now_ms());
}

void yaoxin_handle_cfg_cmd(void)
{
	ipc_data_area_t *ipc_rx = ipc_channel_addr(IPC_CFG_CHANNEL);
	ipc_cmd_hdr_t *hdr;
	uint32 result = 0;

	if (ipc_rx->len < sizeof(ipc_cmd_hdr_t)) {
		YAOXIN_PRINTF("[yaoxin] cfg cmd len error: %u\n", ipc_rx->len);
		return;
	}

	hdr = (ipc_cmd_hdr_t *)ipc_rx->data;

	YAOXIN_PRINTF("[yaoxin] cfg cmd=0x%02x seq=%u len=%u\n",
		      hdr->cmd, hdr->seq, ipc_rx->len);

	switch (hdr->cmd) {
	case IPC_CMD_SET_TIME: {
		ipc_set_time_t *time_data;

		if (ipc_rx->len < sizeof(ipc_cmd_hdr_t) + sizeof(ipc_set_time_t)) {
			result = 1;
			break;
		}

		time_data = (ipc_set_time_t *)(hdr + 1);
		g_wall_base_sec = time_data->wall_time_sec;
		g_wall_base_ns = time_data->wall_time_ns;
		g_assert_base = time_data->assert_time;
		g_time_base_ms = yaoxin_now_ms();
		g_time_synced = 1;

		YAOXIN_PRINTF("[yaoxin] time sync wall=%llu.%09u assert=%llu tick=%u\n",
			      (unsigned long long)g_wall_base_sec,
			      (unsigned)g_wall_base_ns,
			      (unsigned long long)g_assert_base,
			      (unsigned)g_time_base_ms);
		break;
	}
	case IPC_CMD_SET_PARAM: {
		ipc_set_param_t *param;

		if (ipc_rx->len < sizeof(ipc_cmd_hdr_t) + sizeof(ipc_set_param_t)) {
			result = 1;
			break;
		}

		param = (ipc_set_param_t *)(hdr + 1);
		if (param->debounce_ms > 0)
			g_debounce_ms = param->debounce_ms;

		YAOXIN_PRINTF("[yaoxin] debounce=%u ms\n", g_debounce_ms);
		break;
	}
	default:
		YAOXIN_PRINTF("[yaoxin] unknown cfg cmd 0x%02x\n", hdr->cmd);
		result = 1;
		break;
	}

	yaoxin_send_ack(hdr->seq, result);
}

void GPIO010_IRQHandler(void)
{
	uint32 status = fLib_GetGpioIntMaskStatus(GPIO_FTGPIO010_PA_BASE);
	uint32 now_ms = yaoxin_now_ms();
	uint32 pin;

	fLib_ClearGpioInt(GPIO_FTGPIO010_PA_BASE, status);

	for (pin = 0; pin < 32; pin++) {
		if (!(status & (1u << pin)))
			continue;
		if (!(YAOXIN_PIN_MASK & (1u << pin)))
			continue;

		if ((now_ms - g_last_trigger_ms[pin]) < g_debounce_ms) {
			YAOXIN_PRINTF("[yaoxin] pin %u debounce skip\n", pin);
			continue;
		}
		g_last_trigger_ms[pin] = now_ms;

		g_trigger_count[pin]++;
		yaoxin_enqueue_event(pin, now_ms);
	}
}

/* 调试使用：周期性打印 GPIO 电平与计数（需 YAOXIN_DEBUG_PRINT=1） */
void yaoxin_debug_poll(void)
{
#if YAOXIN_DEBUG_PRINT
	uint32 now_ms = yaoxin_now_ms();
	uint32 gpio_data;

	if ((now_ms - g_debug_last_ms) < 2000)
		return;
	g_debug_last_ms = now_ms;

	gpio_data = fLib_Gpio_ReadData(GPIO_FTGPIO010_PA_BASE);

	YAOXIN_PRINTF("[yaoxin] poll tick=%u synced=%u debounce=%ums qdrop=%u\n",
		      now_ms, g_time_synced, g_debounce_ms, g_q_drop);
	YAOXIN_PRINTF("[yaoxin] GPIO0_11=%u cnt=%u  GPIO0_12=%u cnt=%u  GPIO0_13=%u cnt=%u\n",
		      (gpio_data >> YAOXIN_PIN_DOOR) & 1U,
		      g_trigger_count[YAOXIN_PIN_DOOR],
		      (gpio_data >> YAOXIN_PIN_SIGNAL2) & 1U,
		      g_trigger_count[YAOXIN_PIN_SIGNAL2],
		      (gpio_data >> YAOXIN_PIN_SIGNAL1) & 1U,
		      g_trigger_count[YAOXIN_PIN_SIGNAL1]);
#else
	(void)g_debug_last_ms;
	(void)g_q_drop;
#endif
}

void yaoxin_init(void)
{
	uint32 gpio_data;

#if YAOXIN_DEBUG_PRINT
	fLib_SerialInit(DEBUG_CONSOLE, DEFAULT_CONSOLE_BAUD, PARITY_NONE, 0, 8);
#endif

	memset((void *)iRAM_DATA_BASE, 0x00, IPC_SHARE_SRAM_SIZE);
	memset(g_trigger_count, 0, sizeof(g_trigger_count));
	memset(g_last_trigger_ms, 0, sizeof(g_last_trigger_ms));
	memset(g_event_q, 0, sizeof(g_event_q));
	g_q_head = 0;
	g_q_tail = 0;
	g_q_drop = 0;
	g_last_notify_ms = 0;
	g_notify_pending = 0;

	/* PWMTMR_1MSEC_PERIOD == APB_CLK/1000，依赖 system_leo.h 中 APB_CLK 正确 */
	fLib_Timer_Init(DRVPWMTMR4, PWMTMR_1MSEC_PERIOD);

	yaoxin_gpio_configure(YAOXIN_PIN_SIGNAL1);
	yaoxin_gpio_configure(YAOXIN_PIN_SIGNAL2);
	yaoxin_gpio_configure(YAOXIN_PIN_DOOR);

	NVIC_EnableIRQ(GPIO_FTGPIO010_IRQ);

	gpio_data = fLib_Gpio_ReadData(GPIO_FTGPIO010_PA_BASE);
	YAOXIN_PRINTF("[yaoxin] gpio irq=%u debounce=%ums qdepth=%u gap=%ums\n",
		      GPIO_FTGPIO010_IRQ, g_debounce_ms,
		      YAOXIN_LOCAL_QUEUE_DEPTH, YAOXIN_IPC_GAP_MS);
	YAOXIN_PRINTF("[yaoxin] init done, GPIO0_11=%u GPIO0_12=%u GPIO0_13=%u\n",
		      (gpio_data >> YAOXIN_PIN_DOOR) & 1U,
		      (gpio_data >> YAOXIN_PIN_SIGNAL2) & 1U,
		      (gpio_data >> YAOXIN_PIN_SIGNAL1) & 1U);
}
