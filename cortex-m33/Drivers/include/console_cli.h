#ifndef CONSOLE_CLI_H
#define CONSOLE_CLI_H

/*
 * 轻量非阻塞串口命令行框架。
 * 业务模块通过 console_cli_register() 挂命令，main 只调 console_cli_poll()。
 */

typedef void (*console_cmd_handler_t)(int argc, char **argv);

typedef struct {
	const char *name;			/* 命令名，如 "s" / "status" */
	const char *help;			/* 一行帮助 */
	console_cmd_handler_t handler;
} console_cmd_t;

void console_cli_init(const char *prompt);
/* 注册一张命令表；可多次调用以叠加不同模块的命令 */
int console_cli_register(const console_cmd_t *table, unsigned int count);
void console_cli_poll(void);
void console_cli_prompt(void);

/* 工具：解析无符号十进制，成功返回 1 */
unsigned int console_cli_parse_u32(const char *s, unsigned int *out);

#endif
