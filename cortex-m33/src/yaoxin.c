#include <string.h>

#include "ipc.h"
#include "spec.h"
#include "io.h"
#include "DrvGPIO010.h"
#include "DrvPWMTMR010.h"
#include "DrvUART010.h"
#include "leo_cm33.h"

#define YAOXIN_VERSION_MAJOR	1
#define YAOXIN_VERSION_MINOR	0
#define YAOXIN_VERSION_PATCH	0

#define YAOXIN_GPIO_POLARITY		0 /* 0:Active Low，1:Active High */

/*
 * 编译期调试打印开关，默认关闭。
 * 打开：make EXTRA_CFLAGS="-DYAOXIN_DEBUG_PRINT=1"
 */
#ifndef YAOXIN_DEBUG_PRINT
#define YAOXIN_DEBUG_PRINT		0
#endif

#if YAOXIN_DEBUG_PRINT
#define YAOXIN_PRINTF(...)		fLib_printf(__VA_ARGS__)
#else
#define YAOXIN_PRINTF(...)		do { } while (0)
#endif

/* 本地环形队列：确认后入队，主循环间隔出队写共享内存 */
#define YAOXIN_LOCAL_QUEUE_DEPTH	32
/* 两次 notify 最小间隔（ms），给大核 IRQ 拷贝留时间；时标在入队时已固定 */
#define YAOXIN_IPC_GAP_MS		1

/* 低有效：idle→pending_assert→asserted→pending_restore→idle */
#define YX_ST_IDLE		0
#define YX_ST_PENDING_ASSERT	1
#define YX_ST_ASSERTED		2
#define YX_ST_PENDING_RESTORE	3

extern volatile UINT32 T4_Tick;

static UINT64 g_wall_base_sec;
static uint32 g_wall_base_ns;
static UINT64 g_assert_base;	/* 同步时刻大核启动时长 ns */
static uint32 g_time_base_ms;	/* 同步时刻本地 T4_Tick */
static uint32 g_time_synced;
static uint32 g_debounce_ms = YAOXIN_DEBOUNCE_MS_DEFAULT;
static uint32 g_trigger_count[32];
static uint32 g_debug_last_ms;

static uint8 g_pin_state[32];
static uint32 g_pin_edge_ms[32];	/* 进入 pending 时的边沿时刻（时标） */

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

/* 1=当前处于有效（变位）电平 */
static uint32 yaoxin_pin_is_active(uint32 gpio_pin)
{
	uint32 high = read_pin_level(gpio_pin);

	if (!YAOXIN_GPIO_POLARITY)
		return high ? 0U : 1U;	/* Active Low */

	return high ? 1U : 0U;		/* Active High */
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

/* 确认后入队：edge_ms 为边沿时刻，value 为变位/恢复 */
static void yaoxin_enqueue_confirmed(uint32 gpio_pin, uint32 edge_ms,
				     uint32 value)
{
	yaoxin_event_t *slot;
	uint32 next;

	if (yaoxin_queue_full()) {
		g_q_drop++;
		YAOXIN_PRINTF("[yaoxin] queue full, drop pin=%u total_drop=%u\n",
			      gpio_pin, g_q_drop);
		return;
	}

	g_trigger_count[gpio_pin]++;

	slot = &g_event_q[g_q_tail];
	memset(slot, 0, sizeof(*slot));
	slot->channel = yaoxin_pin_to_channel(gpio_pin);
	slot->count = g_trigger_count[gpio_pin];
	slot->value = value;
	yaoxin_get_timestamp(edge_ms, &slot->wall_time_sec, &slot->wall_time_ns,
			     &slot->assert_time);

	next = (g_q_tail + 1U) % YAOXIN_LOCAL_QUEUE_DEPTH;
	g_q_tail = next;

	YAOXIN_PRINTF("[yaoxin] enq %s ch=0x%x cnt=%u val=%u edge_ms=%u\n",
		      yaoxin_channel_str(slot->channel),
		      (unsigned)slot->channel,
		      (unsigned)slot->count,
		      (unsigned)slot->value,
		      (unsigned)edge_ms);
}

/* 确认变位，直接入队 */
static void yaoxin_accept_assert(uint32 pin, uint32 edge_ms)
{
	yaoxin_enqueue_confirmed(pin, edge_ms, YAOXIN_VALUE_ASSERT);
	g_pin_state[pin] = YX_ST_ASSERTED;
}

/* 确认恢复，直接入队 */
static void yaoxin_accept_restore(uint32 pin, uint32 edge_ms)
{
	yaoxin_enqueue_confirmed(pin, edge_ms, YAOXIN_VALUE_RESTORE);
	g_pin_state[pin] = YX_ST_IDLE;
}

/*
 * GPIO ISR：根据当前电平推进状态机；时标记在进入 pending 的边沿时刻。
 * 真正上报在主循环确认之后。
 */
static void yaoxin_pin_on_edge(uint32 pin, uint32 now_ms)
{
	uint32 active = yaoxin_pin_is_active(pin);

	switch (g_pin_state[pin]) {
	case YX_ST_IDLE:
		if (!active)
			break;
		if (g_debounce_ms == 0) {
			yaoxin_accept_assert(pin, now_ms);
		} else {
			g_pin_state[pin] = YX_ST_PENDING_ASSERT;
			g_pin_edge_ms[pin] = now_ms;
			YAOXIN_PRINTF("[yaoxin] pin %u pending assert @%u\n",
				      pin, now_ms);
		}
		break;

	case YX_ST_PENDING_ASSERT:
		if (!active) {
			/* 待定期间回到空闲电平 → 噪声 */
			g_pin_state[pin] = YX_ST_IDLE;
			YAOXIN_PRINTF("[yaoxin] pin %u assert noise\n", pin);
		}
		break;

	case YX_ST_ASSERTED: /* 变位已确认，事件已上报，等待恢复 */
		if (active)
			break;
		if (g_debounce_ms == 0) {
			yaoxin_accept_restore(pin, now_ms);
		} else {
			g_pin_state[pin] = YX_ST_PENDING_RESTORE;
			g_pin_edge_ms[pin] = now_ms;
			YAOXIN_PRINTF("[yaoxin] pin %u pending restore @%u\n",
				      pin, now_ms);
		}
		break;

	case YX_ST_PENDING_RESTORE:
		if (active) {
			/* 待定恢复期间又变有效 → 噪声，仍保持变位 */
			g_pin_state[pin] = YX_ST_ASSERTED;
			YAOXIN_PRINTF("[yaoxin] pin %u restore noise\n", pin);
		}
		break;

	default:
		g_pin_state[pin] = YX_ST_IDLE;
		break;
	}
}

/* 主循环：到 debounce_ms 后复检电平，确认则入队（时标仍用边沿时刻） */
static void yaoxin_debounce_confirm(void)
{
	uint32 now_ms = yaoxin_now_ms();
	uint32 pin;

	if (g_debounce_ms == 0)
		return;

	for (pin = 0; pin < 32; pin++) {
		if (!(YAOXIN_PIN_MASK & (1u << pin)))
			continue;

		if (g_pin_state[pin] == YX_ST_PENDING_ASSERT) {
			if ((now_ms - g_pin_edge_ms[pin]) < g_debounce_ms)
				continue;
			if (yaoxin_pin_is_active(pin)) {
				yaoxin_accept_assert(pin, g_pin_edge_ms[pin]);
				YAOXIN_PRINTF("[yaoxin] pin %u assert ok\n", pin);
			} else {
				g_pin_state[pin] = YX_ST_IDLE;
				YAOXIN_PRINTF("[yaoxin] pin %u assert timeout noise\n",
					      pin);
			}
		} else if (g_pin_state[pin] == YX_ST_PENDING_RESTORE) {
			if ((now_ms - g_pin_edge_ms[pin]) < g_debounce_ms)
				continue;
			if (!yaoxin_pin_is_active(pin)) {
				yaoxin_accept_restore(pin, g_pin_edge_ms[pin]);
				YAOXIN_PRINTF("[yaoxin] pin %u restore ok\n", pin);
			} else {
				g_pin_state[pin] = YX_ST_ASSERTED;
				YAOXIN_PRINTF("[yaoxin] pin %u restore timeout noise\n",
					      pin);
			}
		}
	}
}

/* 共享内存禁止直接 64 位访问（LDRD/STRD 可能挂总线） */
static void yaoxin_store_u32(void *dst, uint32 val)
{
	*(volatile uint32 *)dst = val;
}

static void yaoxin_store_u64_le(void *dst, UINT64 val)
{
	volatile uint32 *w = (volatile uint32 *)dst;

	w[0] = (uint32)val;
	w[1] = (uint32)(val >> 32);
}

static void yaoxin_ipc_post_event(const yaoxin_event_t *event)
{
	ipc_data_area_t *ipc_data = ipc_channel_addr(IPC_YAOXIN_CHANNEL);
	uint8 *p = ipc_data->data;

	ipc_data->id = IPC_YAOXIN_CHANNEL;
	ipc_data->len = sizeof(yaoxin_event_t);

	/* 与 yaoxin_event_t 布局一致：4x u32 + 2x u64 */
	yaoxin_store_u32(p + 0, event->channel);
	yaoxin_store_u32(p + 4, event->count);
	yaoxin_store_u32(p + 8, event->value);
	yaoxin_store_u32(p + 12, event->wall_time_ns);
	yaoxin_store_u64_le(p + 16, event->wall_time_sec);
	yaoxin_store_u64_le(p + 24, event->assert_time);

	ipc_notify_a7(IPC_YAOXIN_CHANNEL);
}

void yaoxin_queue_poll(void)
{
	yaoxin_event_t event;
	uint32 now_ms;
	uint32 next;

	yaoxin_debounce_confirm();

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

	// YAOXIN_PRINTF("[yaoxin] tx ch=0x%x cnt=%u val=%u\n",
	// 	      (unsigned)event.channel,
	// 	      (unsigned)event.count,
	// 	      (unsigned)event.value);
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

	fLib_printf("[yaoxin] cfg ack seq=%u result=%u\n", seq, result);
	ipc_notify_a7(IPC_CFG_CHANNEL);
}

static void yaoxin_send_version(uint32 seq, uint32 result)
{
	ipc_data_area_t *ipc_tx = ipc_channel_addr(IPC_CFG_CHANNEL);
	ipc_reply_t *reply = (ipc_reply_t *)ipc_tx->data;

	ipc_tx->id = IPC_CFG_CHANNEL;
	ipc_tx->len = sizeof(ipc_reply_t);

	reply->ack.cmd = IPC_CMD_ACK;
	reply->ack.seq = seq;
	reply->ack.result = result;
	reply->version.major = YAOXIN_VERSION_MAJOR;
	reply->version.minor = YAOXIN_VERSION_MINOR;
	reply->version.patch = YAOXIN_VERSION_PATCH;

	fLib_printf("[yaoxin] version=V%u.%u.%u\n",
			    YAOXIN_VERSION_MAJOR,
			    YAOXIN_VERSION_MINOR,
			    YAOXIN_VERSION_PATCH);

	ipc_notify_a7(IPC_CFG_CHANNEL);
}

/* 共享内存上避免直接 LDRD/64 位访问，部分总线会挂死 */
static UINT64 yaoxin_load_u64_le(const void *src)
{
	const volatile uint32 *w = (const volatile uint32 *)src;
	uint32 lo = w[0];
	uint32 hi = w[1];

	return ((UINT64)hi << 32) | (UINT64)lo;
}

static uint32 yaoxin_load_u32(const void *src)
{
	return *(const volatile uint32 *)src;
}

static void yaoxin_copy_set_time(ipc_set_time_t *dst, const void *src)
{
	const uint8 *p = (const uint8 *)src;

	/*
	 * 勿对共享区直接解引用 UINT64（编译器可能生成 LDRD）。
	 * 部分共享 SRAM/总线对 64 位访问会挂死；
	 * 统一按 32 位小端拼装到本地副本。
	 */
	dst->wall_time_sec = yaoxin_load_u64_le(p + 0);
	dst->wall_time_ns = yaoxin_load_u32(p + 8);
	dst->reserved = yaoxin_load_u32(p + 12);
	dst->assert_time = yaoxin_load_u64_le(p + 16);
}

void yaoxin_handle_cfg_cmd(void)
{
	ipc_data_area_t *ipc_rx = ipc_channel_addr(IPC_CFG_CHANNEL);
	ipc_cmd_hdr_t *hdr;
	uint32 result = 0;

	if (ipc_rx->len < sizeof(ipc_cmd_hdr_t)) {
		fLib_printf("[yaoxin] cfg cmd len error: %u\n", ipc_rx->len);
		return;
	}

	hdr = (ipc_cmd_hdr_t *)ipc_rx->data;

	/* 若本核 notify 再次进中断，共享区已是 ACK，勿当命令处理 */
	if (hdr->cmd == IPC_CMD_ACK)
		return;

	fLib_printf("[yaoxin] cfg cmd=0x%02x seq=%u len=%u\n",
		      hdr->cmd, hdr->seq, ipc_rx->len);

	switch (hdr->cmd) {
	case IPC_CMD_SET_TIME: {
		ipc_set_time_t time_data;

		if (ipc_rx->len < sizeof(ipc_cmd_hdr_t) + sizeof(ipc_set_time_t)) {
			fLib_printf("[yaoxin] SET_TIME len=%u need=%u\n",
				      ipc_rx->len,
				      (unsigned)(sizeof(ipc_cmd_hdr_t) +
						  sizeof(ipc_set_time_t)));
			result = 1;
			break;
		}

		yaoxin_copy_set_time(&time_data, hdr + 1);
		g_wall_base_sec = time_data.wall_time_sec;
		g_wall_base_ns = time_data.wall_time_ns;
		g_assert_base = time_data.assert_time;
		g_time_base_ms = yaoxin_now_ms();
		g_time_synced = 1;

		fLib_printf("[yaoxin] time sync wall=%08x%08x.%09u assert=%08x%08x tick=%u\n",
			      (uint32)(g_wall_base_sec >> 32),
			      (uint32)(g_wall_base_sec),
			      (unsigned)g_wall_base_ns,
			      (uint32)(g_assert_base >> 32),
			      (uint32)g_assert_base,
			      (unsigned)g_time_base_ms);
		break;
	}
	case IPC_CMD_SET_PARAM: {
		ipc_set_param_t param;
		const uint8 *p;

		if (ipc_rx->len < sizeof(ipc_cmd_hdr_t) + sizeof(ipc_set_param_t)) {
			result = 1;
			break;
		}

		p = (const uint8 *)(hdr + 1);
		param.debounce_ms = yaoxin_load_u32(p + 0);
		param.reserved = yaoxin_load_u32(p + 4);
		g_debounce_ms = param.debounce_ms;

		fLib_printf("[yaoxin] debounce=%u ms\n", g_debounce_ms);
		break;
	}
	case IPC_CMD_GET_VERSION: {
		yaoxin_send_version(hdr->seq, 0);
		goto done;
	}
	default:
		fLib_printf("[yaoxin] unknown cfg cmd 0x%02x\n", hdr->cmd);
		result = 1;
		break;
	}

	yaoxin_send_ack(hdr->seq, result);
done:
	return;
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

		yaoxin_pin_on_edge(pin, now_ms);
	}
}

void keep_alive_debug_poll(void)
{
	uint32 now_ms = yaoxin_now_ms();

	if ((now_ms - g_debug_last_ms) < 2000)
		return;
	g_debug_last_ms = now_ms;

	fLib_printf("[%d]:Still alive\n", now_ms / 1000);
}

static void yaoxin_pin_state_init(uint32 gpio_pin)
{
	/* 按当前电平对齐状态，上电不主动上报 */
	if (yaoxin_pin_is_active(gpio_pin))
		g_pin_state[gpio_pin] = YX_ST_ASSERTED;
	else
		g_pin_state[gpio_pin] = YX_ST_IDLE;
	g_pin_edge_ms[gpio_pin] = 0;
}

void yaoxin_init(void)
{
	uint32 gpio_data;

	fLib_SerialInit(DEBUG_CONSOLE, DEFAULT_CONSOLE_BAUD, PARITY_NONE, 0, 8);

	memset((void *)iRAM_DATA_BASE, 0x00, IPC_SHARE_SRAM_SIZE);
	memset(g_trigger_count, 0, sizeof(g_trigger_count));
	memset(g_pin_state, 0, sizeof(g_pin_state));
	memset(g_pin_edge_ms, 0, sizeof(g_pin_edge_ms));
	memset(g_event_q, 0, sizeof(g_event_q));
	g_q_head = 0;
	g_q_tail = 0;
	g_q_drop = 0;
	g_last_notify_ms = 0;
	g_notify_pending = 0;

	fLib_Timer_Init(DRVPWMTMR4, PWMTMR_1MSEC_PERIOD);

	yaoxin_gpio_configure(YAOXIN_PIN_SIGNAL1);
	yaoxin_gpio_configure(YAOXIN_PIN_SIGNAL2);
	yaoxin_gpio_configure(YAOXIN_PIN_DOOR);

	yaoxin_pin_state_init(YAOXIN_PIN_SIGNAL1);
	yaoxin_pin_state_init(YAOXIN_PIN_SIGNAL2);
	yaoxin_pin_state_init(YAOXIN_PIN_DOOR);

	NVIC_EnableIRQ(GPIO_FTGPIO010_IRQ);

	gpio_data = fLib_Gpio_ReadData(GPIO_FTGPIO010_PA_BASE);
	fLib_printf("[yaoxin] gpio irq=%u debounce=%ums qdepth=%u gap=%ums\n",
		      GPIO_FTGPIO010_IRQ, g_debounce_ms,
		      YAOXIN_LOCAL_QUEUE_DEPTH, YAOXIN_IPC_GAP_MS);
	fLib_printf("[yaoxin] init done, GPIO0_11=%u GPIO0_12=%u GPIO0_13=%u\n",
		      (gpio_data >> YAOXIN_PIN_DOOR) & 1U,
		      (gpio_data >> YAOXIN_PIN_SIGNAL2) & 1U,
		      (gpio_data >> YAOXIN_PIN_SIGNAL1) & 1U);
}
