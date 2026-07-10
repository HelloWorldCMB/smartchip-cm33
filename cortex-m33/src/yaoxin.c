#include <string.h>

#include "ipc.h"
#include "spec.h"
#include "io.h"
#include "DrvGPIO010.h"
#include "DrvPWMTMR010.h"
#include "DrvUART010.h"
#include "leo_cm33.h"

extern volatile UINT32 T4_Tick;

static uint32_t g_time_base_sec;
static uint32_t g_time_base_us;
static uint32_t g_time_base_ms;
static uint32_t g_debounce_ms = YAOXIN_DEBOUNCE_MS_DEFAULT;
static uint32_t g_trigger_count[32];
static uint32_t g_last_trigger_ms[32];

static ipc_data_area_t *ipc_channel_addr(unsigned int channel)
{
	return (ipc_data_area_t *)(iRAM_DATA_BASE +
				   channel * IPC_PER_CHANNEL_SRAM_SIZE);
}

static void ipc_notify_a7(unsigned int channel)
{
	writel((1 << (channel + 1)) | 1, 0x57f00100);
}

static uint32_t yaoxin_now_ms(void)
{
	return T4_Tick;
}

static void yaoxin_get_timestamp(uint32_t now_ms,
				 uint32_t *sec, uint32_t *usec)
{
	uint32_t elapsed_ms = now_ms - g_time_base_ms;
	uint32_t elapsed_us = elapsed_ms * 1000U + g_time_base_us;

	*sec = g_time_base_sec + elapsed_us / 1000000U;
	*usec = elapsed_us % 1000000U;
}

static uint8_t yaoxin_pin_to_type(uint32_t gpio_pin)
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

static void yaoxin_gpio_configure(uint32_t gpio_pin)
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

static void yaoxin_send_ack(uint32_t seq, uint32_t result)
{
	ipc_data_area_t *ipc_tx = ipc_channel_addr(IPC_CFG_CHANNEL);
	ipc_ack_t *ack = (ipc_ack_t *)ipc_tx->data;

	ipc_tx->id = IPC_CFG_CHANNEL;
	ipc_tx->len = sizeof(ipc_ack_t);
	ack->cmd = IPC_CMD_ACK;
	ack->seq = seq;
	ack->result = result;

	ipc_notify_a7(IPC_CFG_CHANNEL);
}

void yaoxin_send_event(uint32_t gpio_pin)
{
	ipc_data_area_t *ipc_data = ipc_channel_addr(IPC_YAOXIN_CHANNEL);
	yaoxin_event_t *event = (yaoxin_event_t *)ipc_data->data;
	uint32_t now_ms = yaoxin_now_ms();

	ipc_data->id = IPC_YAOXIN_CHANNEL;
	ipc_data->len = sizeof(yaoxin_event_t);

	memset(event, 0, sizeof(yaoxin_event_t));
	event->gpio_bank = YAOXIN_GPIO_BANK;
	event->gpio_pin = gpio_pin;
	event->yaoxin_type = yaoxin_pin_to_type(gpio_pin);
	event->count = g_trigger_count[gpio_pin];
	event->status = (fLib_Gpio_ReadData(GPIO_FTGPIO010_PA_BASE) >>
			 gpio_pin) & 1U;
	yaoxin_get_timestamp(now_ms, &event->timestamp, &event->timestamp_us);

	ipc_notify_a7(IPC_YAOXIN_CHANNEL);
}

void yaoxin_handle_cfg_cmd(void)
{
	ipc_data_area_t *ipc_rx = ipc_channel_addr(IPC_CFG_CHANNEL);
	ipc_cmd_hdr_t *hdr;
	uint32_t result = 0;

	if (ipc_rx->len < sizeof(ipc_cmd_hdr_t))
		return;

	hdr = (ipc_cmd_hdr_t *)ipc_rx->data;

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
		break;
	}
	default:
		result = 1;
		break;
	}

	yaoxin_send_ack(hdr->seq, result);
}

void GPIO010_IRQHandler(void)
{
	uint32_t status = fLib_GetGpioIntMaskStatus(GPIO_FTGPIO010_PA_BASE);
	uint32_t now_ms = yaoxin_now_ms();
	uint32_t pin;

	fLib_ClearGpioInt(GPIO_FTGPIO010_PA_BASE, status);

	for (pin = 0; pin < 32; pin++) {
		if (!(status & (1u << pin)))
			continue;
		if (!(YAOXIN_PIN_MASK & (1u << pin)))
			continue;

		if ((now_ms - g_last_trigger_ms[pin]) < g_debounce_ms)
			continue;
		g_last_trigger_ms[pin] = now_ms;

		g_trigger_count[pin]++;
		yaoxin_send_event(pin);
	}
}

void yaoxin_init(void)
{
	memset((void *)iRAM_DATA_BASE, 0x00, IPC_SHARE_SRAM_SIZE);
	memset(g_trigger_count, 0, sizeof(g_trigger_count));
	memset(g_last_trigger_ms, 0, sizeof(g_last_trigger_ms));

	fLib_Timer_Init(DRVPWMTMR4, PWMTMR_1MSEC_PERIOD);

	yaoxin_gpio_configure(YAOXIN_PIN_SIGNAL1);
	yaoxin_gpio_configure(YAOXIN_PIN_SIGNAL2);
	yaoxin_gpio_configure(YAOXIN_PIN_DOOR);

	NVIC_EnableIRQ(GPIO_FTGPIO010_IRQ);
}
