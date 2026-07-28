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
	uint32 cmd;
	uint32 seq;
} ipc_cmd_hdr_t;

/*
 * 时间同步载荷（通道0）：
 *   wall_time_*  : 墙钟时间
 *   assert_time  : 大核启动至今时长（ns，CLOCK_BOOTTIME）
 * 小核以本地 T4_Tick 为锚点换算后续事件时标。
 */
typedef struct {
	UINT64 wall_time_sec;
	uint32 wall_time_ns;
	uint32 reserved;
	UINT64 assert_time;
} ipc_set_time_t;

typedef struct {
	uint32 debounce_ms;
	uint32 reserved;
} ipc_set_param_t;

typedef struct {
	uint32 cmd;
	uint32 seq;
	uint32 result;
} ipc_ack_t;

/* 通道1 遥信事件，总长 32 字节（<= 64） */
#define YAOXIN_CH_DOOR		0x20
#define YAOXIN_CH_SIGNAL1	0x21
#define YAOXIN_CH_SIGNAL2	0x22

#define YAOXIN_GPIO_BANK	0
#define YAOXIN_PIN_SIGNAL1	13
#define YAOXIN_PIN_SIGNAL2	12
#define YAOXIN_PIN_DOOR		11
#define YAOXIN_PIN_MASK		((1u << YAOXIN_PIN_DOOR) | \
				 (1u << YAOXIN_PIN_SIGNAL2) | \
				 (1u << YAOXIN_PIN_SIGNAL1))

#define YAOXIN_DEBOUNCE_MS_DEFAULT	30

#define YAOXIN_VALUE_RESTORE	0	/* 恢复 */
#define YAOXIN_VALUE_ASSERT	1	/* 变位 */

typedef struct {
	uint32 channel;		/* 门节点0x20 / 1路0x21 / 2路0x22 */
	uint32 count;		/* 变位总数 */
	uint32 value;		/* 1-变位 0-恢复 */
	uint32 wall_time_ns;	/* 变位时间 ns（秒内余数） */
	UINT64 wall_time_sec;	/* 变位时间 sec */
	UINT64 assert_time;	/* 大核启动至今时长 ns */
} yaoxin_event_t;

void yaoxin_init(void);
void yaoxin_handle_cfg_cmd(void);
void yaoxin_send_event(uint32 gpio_pin);
void yaoxin_queue_poll(void);	/* 主循环调用：出队并间隔 notify 大核 */
void keep_alive_debug_poll(void);

#endif
