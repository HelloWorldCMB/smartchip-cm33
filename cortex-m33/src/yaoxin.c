#include <string.h>

#include "ipc.h"
#include "spec.h"
#include "io.h"
#include "DrvGPIO010.h"
#include "DrvPWMTMR010.h"
#include "DrvUART010.h"
#include "leo_cm33.h"

extern volatile UINT32 T4_Tick;

static uint32 g_time_base_sec;
static uint32 g_time_base_us;
static uint32 g_time_base_ms;
static uint32 g_time_synced;
static uint32 g_debounce_ms = YAOXIN_DEBOUNCE_MS_DEFAULT;
static uint32 g_trigger_count[32];
static uint32 g_last_trigger_ms[32];
static uint32 g_debug_last_ms;

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

static const char *yaoxin_type_str(uint8 type)
{
	switch (type) {
	case YAOXIN_TYPE_SIGNAL1:
		return "signal1";
	case YAOXIN_TYPE_SIGNAL2:
		return "signal2";
	case YAOXIN_TYPE_DOOR:
		return "door";
	default:
		return "unknown";
	}
}

/*
 * 时标换算（T4_Tick 必须为真实 1ms）：
 *   绝对时间 = 同步时的 Unix 时间 + (当前 tick - 同步时 tick)
 *
 * 注意：不要写成 sec += elapsed_ms（会把毫秒当成秒，快 1000 倍）。
 */
static void yaoxin_get_timestamp(uint32 now_ms,
				 uint32 *sec, uint32 *usec)
{
	uint32 elapsed_ms = now_ms - g_time_base_ms;
	uint32 add_sec;
	uint32 add_us;
	uint32 total_us;

	add_sec = elapsed_ms / 1000U;
	add_us = (elapsed_ms % 1000U) * 1000U;

	total_us = g_time_base_us + add_us;
	*sec = g_time_base_sec + add_sec + total_us / 1000000U;
	*usec = total_us % 1000000U;
}

static uint8 yaoxin_pin_to_type(uint32 gpio_pin)
{
	switch (gpio_pin) {
	case YAOXIN_PIN_SIGNAL1:
		return YAOXIN_TYPE_SIGNAL1;
	case YAOXIN_PIN_SIGNAL2:
		return YAOXIN_TYPE_SIGNAL2;
	case YAOXIN_PIN_DOOR:
		return YAOXIN_TYPE_DOOR;
	default:
		return 0xFF;
	}
}

static uint32 yaoxin_read_pin_level(uint32 gpio_pin)
{
	return (fLib_Gpio_ReadData(GPIO_FTGPIO010_PA_BASE) >> gpio_pin) & 1U;
}

static void yaoxin_gpio_configure(uint32 gpio_pin)
{
	fLib_SetGpioDir(GPIO_FTGPIO010_PA_BASE, gpio_pin, GPIO_DIR_INPUT);
	fLib_SetGpioEdgeMode(GPIO_FTGPIO010_PA_BASE, gpio_pin, BOTH);
	fLib_SetGpioTrigger(GPIO_FTGPIO010_PA_BASE, gpio_pin, GPIO_EDGE);
	fLib_SetGpioActiveMode(GPIO_FTGPIO010_PA_BASE, gpio_pin, GPIO_Rising);
	fLib_EnableGpioBounce(GPIO_FTGPIO010_PA_BASE, gpio_pin,
			      APB_CLOCK / 12000);
	fLib_ClearGpioInt(GPIO_FTGPIO010_PA_BASE, 1u << gpio_pin);
	fLib_SetGpioIntEnable(GPIO_FTGPIO010_PA_BASE, gpio_pin);
	fLib_SetGpioIntUnMask(GPIO_FTGPIO010_PA_BASE, gpio_pin);
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

void yaoxin_send_event(uint32 gpio_pin)
{
	ipc_data_area_t *ipc_data = ipc_channel_addr(IPC_YAOXIN_CHANNEL);
	yaoxin_event_t *event = (yaoxin_event_t *)ipc_data->data;
	uint32 now_ms = yaoxin_now_ms();

	ipc_data->id = IPC_YAOXIN_CHANNEL;
	ipc_data->len = sizeof(yaoxin_event_t);

	memset(event, 0, sizeof(yaoxin_event_t));
	event->gpio_bank = YAOXIN_GPIO_BANK;
	event->gpio_pin = gpio_pin;
	event->yaoxin_type = yaoxin_pin_to_type(gpio_pin);
	event->count = g_trigger_count[gpio_pin];
	event->status = yaoxin_read_pin_level(gpio_pin);
	yaoxin_get_timestamp(now_ms, &event->timestamp, &event->timestamp_us);

	fLib_printf("[yaoxin] event %s GPIO0_%u cnt=%u lvl=%u t=%u.%06u\n",
		    yaoxin_type_str(event->yaoxin_type), gpio_pin,
		    event->count, event->status,
		    event->timestamp, event->timestamp_us);

	ipc_notify_a7(IPC_YAOXIN_CHANNEL);
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
	fLib_printf("[yaoxin] cfg cmd=0x%02x seq=%u len=%u\n",
		    hdr->cmd, hdr->seq, ipc_rx->len);

	switch (hdr->cmd) {
	case IPC_CMD_SET_TIME: {
		ipc_set_time_t *time_data;

		if (ipc_rx->len < sizeof(ipc_cmd_hdr_t) + sizeof(ipc_set_time_t)) {
			result = 1;
			break;
		}

		time_data = (ipc_set_time_t *)(hdr + 1);
		g_time_base_sec = time_data->unix_sec;
		g_time_base_us = time_data->unix_usec;
		g_time_base_ms = yaoxin_now_ms();
		g_time_synced = 1;
		fLib_printf("[yaoxin] time sync %u.%06u tick=%u\n",
			    g_time_base_sec, g_time_base_us, g_time_base_ms);
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
		fLib_printf("[yaoxin] debounce=%u ms\n", g_debounce_ms);
		break;
	}
	default:
		fLib_printf("[yaoxin] unknown cfg cmd 0x%02x\n", hdr->cmd);
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
	fLib_printf("[yaoxin] gpio irq status=0x%08x tick=%u\n", status, now_ms);

	for (pin = 0; pin < 32; pin++) {
		if (!(status & (1u << pin)))
			continue;
		if (!(YAOXIN_PIN_MASK & (1u << pin)))
			continue;

		if ((now_ms - g_last_trigger_ms[pin]) < g_debounce_ms) {
			fLib_printf("[yaoxin] pin %u debounce skip\n", pin);
			continue;
		}
		g_last_trigger_ms[pin] = now_ms;

		g_trigger_count[pin]++;
		yaoxin_send_event(pin);
	}
}

/* 周期性打印 GPIO 电平与计数，便于确认小核侧状态 */
void yaoxin_debug_poll(void)
{
	uint32 now_ms = yaoxin_now_ms();
	uint32 gpio_data;
	int i;

	if ((now_ms - g_debug_last_ms) < 2000)
		return;
	g_debug_last_ms = now_ms;

	gpio_data = fLib_Gpio_ReadData(GPIO_FTGPIO010_PA_BASE);

	fLib_printf("[yaoxin] poll tick=%u synced=%u debounce=%ums\n",
		    now_ms, g_time_synced, g_debounce_ms);
	fLib_printf("[yaoxin] (poll every 2000 ticks ~= 2s if 1ms/tick)\n");
	fLib_printf("[yaoxin] GPIO0_11=%u cnt=%u  GPIO0_12=%u cnt=%u  GPIO0_13=%u cnt=%u\n",
		    (gpio_data >> YAOXIN_PIN_DOOR) & 1U,
		    g_trigger_count[YAOXIN_PIN_DOOR],
		    (gpio_data >> YAOXIN_PIN_SIGNAL2) & 1U,
		    g_trigger_count[YAOXIN_PIN_SIGNAL2],
		    (gpio_data >> YAOXIN_PIN_SIGNAL1) & 1U,
		    g_trigger_count[YAOXIN_PIN_SIGNAL1]);
}

void yaoxin_init(void)
{
	uint32 gpio_data;

	/* 初始化调试串口 115200 8N1 */
	fLib_SerialInit(DEBUG_CONSOLE, DEFAULT_CONSOLE_BAUD, PARITY_NONE, 0, 8);

	memset((void *)iRAM_DATA_BASE, 0x00, IPC_SHARE_SRAM_SIZE);
	memset(g_trigger_count, 0, sizeof(g_trigger_count));
	memset(g_last_trigger_ms, 0, sizeof(g_last_trigger_ms));

	/* PWMTMR_1MSEC_PERIOD == APB_CLK/1000，依赖 system_leo.h 中 APB_CLK 正确 */
	fLib_Timer_Init(DRVPWMTMR4, PWMTMR_1MSEC_PERIOD);
	fLib_printf("[yaoxin] timer reload=%u (APB_CLK=%u), expect 1ms/tick\n",
		    (unsigned)PWMTMR_1MSEC_PERIOD, (unsigned)APB_CLK);

	yaoxin_gpio_configure(YAOXIN_PIN_SIGNAL1);
	yaoxin_gpio_configure(YAOXIN_PIN_SIGNAL2);
	yaoxin_gpio_configure(YAOXIN_PIN_DOOR);

	NVIC_EnableIRQ(GPIO_FTGPIO010_IRQ);

	gpio_data = fLib_Gpio_ReadData(GPIO_FTGPIO010_PA_BASE);
	fLib_printf("[yaoxin] gpio irq=%u debounce=%ums\n",
		    GPIO_FTGPIO010_IRQ, g_debounce_ms);
	fLib_printf("[yaoxin] init done, GPIO0_11=%u GPIO0_12=%u GPIO0_13=%u\n",
		    (gpio_data >> YAOXIN_PIN_DOOR) & 1U,
		    (gpio_data >> YAOXIN_PIN_SIGNAL2) & 1U,
		    (gpio_data >> YAOXIN_PIN_SIGNAL1) & 1U);
}
