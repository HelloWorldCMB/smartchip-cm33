#include <string.h>

#include "console_cli.h"
#include "DrvUART010.h"

#define CLI_BUF_LEN		48
#define CLI_MAX_ARGS		6
#define CLI_MAX_CMDS		24

static char g_buf[CLI_BUF_LEN];
static unsigned int g_len;
static const char *g_prompt = "> ";

static console_cmd_t g_cmds[CLI_MAX_CMDS];
static unsigned int g_cmd_count;

void console_cli_prompt(void)
{
	fLib_printf("%s", g_prompt);
}

void console_cli_init(const char *prompt)
{
	if (prompt && prompt[0])
		g_prompt = prompt;
	g_len = 0;
	g_buf[0] = '\0';
	fLib_printf("[cli] ready, input h to list commands\n");
	console_cli_prompt();
}

int console_cli_register(const console_cmd_t *table, unsigned int count)
{
	unsigned int i;

	if (!table || count == 0)
		return -1;

	for (i = 0; i < count; i++) {
		if (g_cmd_count >= CLI_MAX_CMDS)
			return -1;
		if (!table[i].name || !table[i].handler)
			continue;
		g_cmds[g_cmd_count++] = table[i];
	}
	return 0;
}

unsigned int console_cli_parse_u32(const char *s, unsigned int *out)
{
	unsigned int v = 0;
	unsigned int n = 0;

	if (!s || !out)
		return 0;
	while (*s == ' ' || *s == '\t')
		s++;
	if (*s < '0' || *s > '9')
		return 0;
	while (*s >= '0' && *s <= '9') {
		v = v * 10U + (unsigned int)(*s - '0');
		s++;
		n++;
	}
	if (!n)
		return 0;
	*out = v;
	return 1;
}

static void cli_cmd_help(int argc, char **argv)
{
	unsigned int i;

	(void)argc;
	(void)argv;

	fLib_printf("cmds:\n");
	fLib_printf("  h / help / ?     show this help\n");
	for (i = 0; i < g_cmd_count; i++) {
		fLib_printf("  %-14s %s\n",
			    g_cmds[i].name,
			    g_cmds[i].help ? g_cmds[i].help : "");
	}
}

static int cli_name_eq(const char *a, const char *b)
{
	char ca, cb;

	while (*a && *b) {
		ca = *a;
		cb = *b;
		if (ca >= 'A' && ca <= 'Z')
			ca = (char)(ca - 'A' + 'a');
		if (cb >= 'A' && cb <= 'Z')
			cb = (char)(cb - 'A' + 'a');
		if (ca != cb)
			return 0;
		a++;
		b++;
	}
	return (*a == '\0' && *b == '\0');
}

static void cli_exec_line(char *line)
{
	char *argv[CLI_MAX_ARGS];
	int argc = 0;
	char *p = line;
	unsigned int i;

	while (*p == ' ' || *p == '\t')
		p++;
	if (*p == '\0')
		return;

	while (*p && argc < CLI_MAX_ARGS) {
		argv[argc++] = p;
		while (*p && *p != ' ' && *p != '\t')
			p++;
		if (*p) {
			*p++ = '\0';
			while (*p == ' ' || *p == '\t')
				p++;
		}
	}

	if (cli_name_eq(argv[0], "h") ||
	    cli_name_eq(argv[0], "help") ||
	    cli_name_eq(argv[0], "?")) {
		cli_cmd_help(argc, argv);
		return;
	}

	for (i = 0; i < g_cmd_count; i++) {
		if (cli_name_eq(argv[0], g_cmds[i].name)) {
			g_cmds[i].handler(argc, argv);
			return;
		}
	}

	fLib_printf("unknown '%s', try h\n", argv[0]);
}

void console_cli_poll(void)
{
	char ch;

	while (fLib_kbhit(DEBUG_CONSOLE)) {
		ch = fLib_getch(DEBUG_CONSOLE);
		if (ch == 0)
			break;

		if (ch == '\r' || ch == '\n') {
			fLib_printf("\n");
			g_buf[g_len] = '\0';
			cli_exec_line(g_buf);
			g_len = 0;
			console_cli_prompt();
			continue;
		}

		if (ch == 0x08 || ch == 0x7F) {
			if (g_len > 0) {
				g_len--;
				fLib_printf("\b \b");
			}
			continue;
		}

		if (ch >= 0x20 && ch < 0x7F && g_len + 1U < CLI_BUF_LEN) {
			g_buf[g_len++] = ch;
			fLib_putchar(DEBUG_CONSOLE, ch);
		}
	}
}
