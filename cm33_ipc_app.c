/*
 * cm33_ipc_app.c - 大核侧 IPC 通信 Demo
 *
 * 与小核 ipc.h / yaoxin.c 协议保持一致：
 *   通道0 (IPC_CFG_CHANNEL)   : 大核 -> 小核，下发时间同步、消抖参数，小核回 ACK
 *   通道1 (IPC_YAOXIN_CHANNEL): 小核 -> 大核，上报遥信事件（时间 + 状态 + 次数）
 *
 * 遥信引脚映射（大核 GPIO 命名）：
 *   遥信1  GPIO0_13
 *   遥信2  GPIO0_12
 *   门节点 GPIO0_11
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>

#define DEV_NAME    "/dev/cm33_ipc"

/* 驱动侧单通道共享内存大小（与小核 IPC_PER_CHANNEL_SRAM_SIZE 逻辑对应） */
#define IPC_PER_CHANNEL_SRAM_SIZE	(4096 + 4)
#define IPC_CHANNEL_MAX			2

/* 通道号，须与小核 ipc.h 一致 */
#define IPC_CFG_CHANNEL			0
#define IPC_YAOXIN_CHANNEL		1

typedef struct {
	unsigned int id;
	size_t len;
	unsigned char data[0];
} ipc_data_area_t;

#define CM33_MAGIC					'I'
#define CM33_IPC_IOCTRL_SEND_DATA		_IOW(CM33_MAGIC, 1, ipc_data_area_t)
#define CM33_IPC_IOCTRL_REV_DATA		_IOR(CM33_MAGIC, 2, ipc_data_area_t)
#define CM33_IPC_IOCTRL_SPECIFIED_CHANNAL	_IOR(CM33_MAGIC, 3, ipc_data_area_t)

/* ---------- 通道0 命令协议（与小核 ipc.h 同步） ---------- */

#define IPC_CMD_SET_TIME	0x01	/* 下发时间基准，供小核打时标 */
#define IPC_CMD_SET_PARAM	0x02	/* 下发消抖等参数 */
#define IPC_CMD_ACK		0x80	/* 小核确认帧 */

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
	uint32_t result;		/* 0=成功，非0=失败 */
} ipc_ack_t;

/* ---------- 通道1 遥信事件（与小核 ipc.h 同步） ---------- */

#define YAOXIN_TYPE_SIGNAL1	0
#define YAOXIN_TYPE_SIGNAL2	1
#define YAOXIN_TYPE_DOOR	2

#define YAOXIN_DEBOUNCE_MS_DEFAULT	30

typedef struct {
	uint32_t gpio_bank;		/* 固定为 0，对应 GPIO0_xx */
	uint32_t gpio_pin;		/* 11/12/13 */
	uint32_t timestamp;		/* Unix 秒 */
	uint32_t timestamp_us;		/* 微秒 */
	uint32_t count;			/* 该路累计触发次数 */
	uint8_t  status;		/* 当前电平：0/1 */
	uint8_t  yaoxin_type;		/* 0遥信1 / 1遥信2 / 2门节点 */
	uint8_t  reserved[2];
} yaoxin_event_t;

static uint32_t g_cmd_seq;

static void print_hex(const unsigned char *data, size_t len)
{
	size_t i;

	for (i = 0; i < len; i++) {
		printf("%02x ", data[i]);
		if ((i + 1) % 16 == 0)
			printf("\n");
	}
	if (len % 16 != 0)
		printf("\n");
}

static const char *yaoxin_type_name(uint8_t type)
{
	switch (type) {
	case YAOXIN_TYPE_SIGNAL1:
		return "遥信1";
	case YAOXIN_TYPE_SIGNAL2:
		return "遥信2";
	case YAOXIN_TYPE_DOOR:
		return "门节点";
	default:
		return "未知";
	}
}

static void yaoxin_print_event(const yaoxin_event_t *event)
{
	printf("  GPIO%d_%u  %s\n",
	       event->gpio_bank, event->gpio_pin,
	       yaoxin_type_name(event->yaoxin_type));
	printf("  时间: %u.%06u\n", event->timestamp, event->timestamp_us);
	printf("  次数: %u  电平: %u\n", event->count, event->status);
}

/*
 * 通道0 发送命令并等待小核 ACK。
 * 通道0 为双向通道：SEND 后通过 RECV 阻塞/重试读取确认帧。
 */
static int ipc_cfg_send_and_wait_ack(int fd,
				     ipc_data_area_t *send_buf,
				     ipc_data_area_t *recv_buf,
				     size_t payload_len)
{
	ipc_ack_t *ack;
	int ret, retry;

	send_buf->id = IPC_CFG_CHANNEL;
	send_buf->len = sizeof(ipc_cmd_hdr_t) + payload_len;

	ret = ioctl(fd, CM33_IPC_IOCTRL_SEND_DATA, send_buf);
	if (ret < 0) {
		perror("CM33_IPC_IOCTRL_SEND_DATA");
		return -1;
	}

	/* 非阻塞 open 时，ACK 可能稍后到达，做有限次重试 */
	for (retry = 0; retry < 50; retry++) {
		memset(recv_buf->data, 0,
		       IPC_PER_CHANNEL_SRAM_SIZE - sizeof(ipc_data_area_t));
		recv_buf->id = IPC_CFG_CHANNEL;

		ret = ioctl(fd, CM33_IPC_IOCTRL_REV_DATA, recv_buf);
		if (ret == 0)
			break;
		if (errno != EAGAIN) {
			perror("CM33_IPC_IOCTRL_REV_DATA");
			return -1;
		}
		usleep(10000);	/* 10ms */
	}

	if (ret < 0) {
		printf("等待 ACK 超时\n");
		return -1;
	}

	if (recv_buf->len < sizeof(ipc_ack_t)) {
		printf("ACK 长度异常: %zu\n", recv_buf->len);
		return -1;
	}

	ack = (ipc_ack_t *)recv_buf->data;
	if (ack->cmd != IPC_CMD_ACK) {
		printf("ACK 命令码错误: 0x%02x\n", ack->cmd);
		return -1;
	}
	if (ack->result != 0) {
		printf("小核处理失败, seq=%u, result=%u\n", ack->seq, ack->result);
		return -1;
	}

	printf("ACK OK, seq=%u\n", ack->seq);
	return 0;
}

/* 向小核同步当前系统时间，作为遥信时标基准 */
static int yaoxin_sync_time(int fd,
			    ipc_data_area_t *send_buf,
			    ipc_data_area_t *recv_buf)
{
	struct timeval tv;
	ipc_cmd_hdr_t *hdr;
	ipc_set_time_t *time_data;

	gettimeofday(&tv, NULL);

	hdr = (ipc_cmd_hdr_t *)send_buf->data;
	time_data = (ipc_set_time_t *)(hdr + 1);

	hdr->cmd = IPC_CMD_SET_TIME;
	hdr->seq = ++g_cmd_seq;
	time_data->unix_sec = (uint32_t)tv.tv_sec;
	time_data->unix_usec = (uint32_t)tv.tv_usec;

	printf("同步时间: %u.%06u, seq=%u\n",
	       time_data->unix_sec, time_data->unix_usec, hdr->seq);

	return ipc_cfg_send_and_wait_ack(fd, send_buf, recv_buf,
					 sizeof(ipc_set_time_t));
}

/* 向小核下发消抖时间（毫秒） */
static int yaoxin_set_debounce(int fd,
			       ipc_data_area_t *send_buf,
			       ipc_data_area_t *recv_buf,
			       uint32_t debounce_ms)
{
	ipc_cmd_hdr_t *hdr;
	ipc_set_param_t *param;

	hdr = (ipc_cmd_hdr_t *)send_buf->data;
	param = (ipc_set_param_t *)(hdr + 1);

	hdr->cmd = IPC_CMD_SET_PARAM;
	hdr->seq = ++g_cmd_seq;
	param->debounce_ms = debounce_ms;
	param->reserved = 0;

	printf("设置消抖: %u ms, seq=%u\n", debounce_ms, hdr->seq);

	return ipc_cfg_send_and_wait_ack(fd, send_buf, recv_buf,
					 sizeof(ipc_set_param_t));
}

/* 遥信初始化：同步时间 + 设置默认消抖 */
static int yaoxin_init_cfg(int fd,
			   ipc_data_area_t *send_buf,
			   ipc_data_area_t *recv_buf)
{
	if (yaoxin_sync_time(fd, send_buf, recv_buf) < 0)
		return -1;

	return yaoxin_set_debounce(fd, send_buf, recv_buf,
				   YAOXIN_DEBOUNCE_MS_DEFAULT);
}

/*
 * 持续读取通道1 遥信事件。
 * 通道1 为小核单向发送，每条最大 64 字节，驱动侧有 64 条缓存。
 */
static void yaoxin_poll_events(int fd, ipc_data_area_t *recv_buf)
{
	yaoxin_event_t *event;
	int ret;

	printf("监听遥信事件（通道1），Ctrl+C 退出...\n");

	while (1) {
		memset(recv_buf->data, 0,
		       IPC_PER_CHANNEL_SRAM_SIZE - sizeof(ipc_data_area_t));
		recv_buf->id = IPC_YAOXIN_CHANNEL;

		ret = ioctl(fd, CM33_IPC_IOCTRL_REV_DATA, recv_buf);
		if (ret < 0) {
			if (errno == EAGAIN) {
				usleep(100000);	/* 100ms 轮询间隔 */
				continue;
			}
			perror("CM33_IPC_IOCTRL_REV_DATA");
			break;
		}

		if (recv_buf->len < sizeof(yaoxin_event_t)) {
			printf("事件长度异常: %zu, hex:\n", recv_buf->len);
			print_hex(recv_buf->data, recv_buf->len);
			continue;
		}

		event = (yaoxin_event_t *)recv_buf->data;
		printf("\n--- 遥信事件 ---\n");
		yaoxin_print_event(event);
	}
}

/* 原始数据发送（调试用，保留原 Demo 行为） */
static int ipc_raw_send(int fd, ipc_data_area_t *send_buf,
			ipc_data_area_t *recv_buf)
{
	int channel, len, data, ret, i;

	printf("channel id:\n");
	if (scanf("%d", &channel) != 1)
		return 0;
	if (channel > (IPC_CHANNEL_MAX - 1)) {
		printf("channel error\n");
		return 0;
	}

	printf("len:\n");
	if (scanf("%d", &len) != 1)
		return 0;
	if (len > (int)(IPC_PER_CHANNEL_SRAM_SIZE - sizeof(ipc_data_area_t))) {
		printf("len error\n");
		return 0;
	}

	send_buf->id = channel;
	send_buf->len = len;
	memset(send_buf->data, 0x00,
	       IPC_PER_CHANNEL_SRAM_SIZE - sizeof(ipc_data_area_t));

	printf("data (hex):\n");
	if (scanf("%x", &data) != 1)
		return 0;
	memset(send_buf->data, data, len);

	printf("send channel: %d, len: %d\n", send_buf->id, send_buf->len);
	print_hex(send_buf->data, send_buf->len);

	ret = ioctl(fd, CM33_IPC_IOCTRL_SEND_DATA, send_buf);
	printf("CM33_IPC_IOCTRL_SEND_DATA ret: %d\n", ret);
	if (ret < 0 || channel != IPC_CFG_CHANNEL)
		return ret;

	/* 通道0 发送后可尝试读取回复 */
	recv_buf->id = channel;
	ret = ioctl(fd, CM33_IPC_IOCTRL_REV_DATA, recv_buf);
	printf("CM33_IPC_IOCTRL_REV_DATA ret: %d\n", ret);
	if (ret < 0)
		return ret;

	printf("recv channel: %d, len: %zu\n", recv_buf->id, recv_buf->len);
	print_hex(recv_buf->data, recv_buf->len);
	return 0;
}

/* 原始数据接收（调试用） */
static int ipc_raw_recv(int fd, ipc_data_area_t *recv_buf)
{
	int channel, ret;

	printf("channel id:\n");
	if (scanf("%d", &channel) != 1)
		return 0;
	if (channel > (IPC_CHANNEL_MAX - 1)) {
		printf("channel error\n");
		return 0;
	}

	recv_buf->id = channel;
	ret = ioctl(fd, CM33_IPC_IOCTRL_REV_DATA, recv_buf);
	printf("CM33_IPC_IOCTRL_REV_DATA ret: %d\n", ret);
	if (ret < 0)
		return ret;

	printf("recv channel: %d, len: %zu\n", recv_buf->id, recv_buf->len);
	print_hex(recv_buf->data, recv_buf->len);
	return 0;
}

int main(int argc, char **argv)
{
	int fd = -1;
	int select;
	uint32_t debounce_ms;
	ipc_data_area_t *send_ipc_data;
	ipc_data_area_t *recv_ipc_data;

	(void)argc;
	(void)argv;

	fd = open(DEV_NAME, O_RDWR | O_NONBLOCK);
	if (fd < 0) {
		perror("open failed");
		return -1;
	}

	send_ipc_data = malloc(IPC_PER_CHANNEL_SRAM_SIZE);
	recv_ipc_data = malloc(IPC_PER_CHANNEL_SRAM_SIZE);
	if (!send_ipc_data || !recv_ipc_data) {
		perror("malloc failed");
		close(fd);
		return -1;
	}

	while (1) {
		printf("\n======== CM33 IPC Demo ========\n");
		printf("0: 遥信初始化（同步时间 + 消抖%dms）\n",
		       YAOXIN_DEBOUNCE_MS_DEFAULT);
		printf("1: 设置消抖时间（通道0）\n");
		printf("2: 监听遥信事件（通道1）\n");
		printf("3: 发送原始数据（调试）\n");
		printf("4: 接收原始数据（调试）\n");
		printf("================================\n");
		printf("请选择: ");

		if (scanf("%d", &select) != 1)
			break;

		switch (select) {
		case 0:
			yaoxin_init_cfg(fd, send_ipc_data, recv_ipc_data);
			break;
		case 1:
			printf("消抖时间(ms): ");
			if (scanf("%u", &debounce_ms) == 1)
				yaoxin_set_debounce(fd, send_ipc_data,
						    recv_ipc_data, debounce_ms);
			break;
		case 2:
			yaoxin_poll_events(fd, recv_ipc_data);
			break;
		case 3:
			ipc_raw_send(fd, send_ipc_data, recv_ipc_data);
			break;
		case 4:
			ipc_raw_recv(fd, recv_ipc_data);
			break;
		default:
			printf("无效选项\n");
			break;
		}
	}

	free(send_ipc_data);
	free(recv_ipc_data);
	close(fd);
	return 0;
}
