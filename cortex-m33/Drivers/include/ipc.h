#ifndef __IPC_H
#define __IPC_H

#include "types.h"

#define YAOXIN_ENABLE

#define IPC_PER_CHANNEL_SRAM_SIZE	512
#define IPC_SHARE_SRAM_SIZE		0x8000

#define IPC_CFG_CHANNEL			(0)
#define IPC_YAOXIN_CHANNEL		(1)

typedef struct {
	unsigned int id;
	unsigned int len;
	unsigned char data[0];
} ipc_data_area_t;

/* 通道0 命令码 */
#define IPC_CMD_SET_TIME	0x01
#define IPC_CMD_SET_PARAM	0x02
#define IPC_CMD_ACK		0x80

typedef struct {
	uint32_t cmd;
	uint32_t seq;
} ipc_cmd_hdr_t;

typedef struct {
	uint32_t unix_sec;
	uint32_t unix_usec;
} ipc_set_time_t;

typedef struct {
	uint32_t debounce_ms;
	uint32_t reserved;
} ipc_set_param_t;

typedef struct {
	uint32_t cmd;
	uint32_t seq;
	uint32_t result;
} ipc_ack_t;

/* 通道1 遥信事件，总长 <= 64 字节 */
#define YAOXIN_TYPE_SIGNAL1	0
#define YAOXIN_TYPE_SIGNAL2	1
#define YAOXIN_TYPE_DOOR	2

#define YAOXIN_GPIO_BANK	0
#define YAOXIN_PIN_SIGNAL1	13
#define YAOXIN_PIN_SIGNAL2	12
#define YAOXIN_PIN_DOOR		11
#define YAOXIN_PIN_MASK		((1u << YAOXIN_PIN_DOOR) | \
				 (1u << YAOXIN_PIN_SIGNAL2) | \
				 (1u << YAOXIN_PIN_SIGNAL1))

#define YAOXIN_DEBOUNCE_MS_DEFAULT	30

typedef struct {
	uint32_t gpio_bank;
	uint32_t gpio_pin;
	uint32_t timestamp;
	uint32_t timestamp_us;
	uint32_t count;
	uint8_t  status;
	uint8_t  yaoxin_type;
	uint8_t  reserved[2];
} yaoxin_event_t;

void yaoxin_init(void);
void yaoxin_handle_cfg_cmd(void);
void yaoxin_send_event(uint32_t gpio_pin);

#endif
