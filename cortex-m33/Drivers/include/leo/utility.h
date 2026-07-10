#ifndef UTILITY_H
#define UTILITY_H


#ifdef __cplusplus
extern "C" {
#endif


#include "types.h"

#define	CMDLEN			50
#define	MAXARGS			20
	
#define divRoundDown(n,s)   ((n) / (s))
#define divRoundUp(n,s)     ((n+s-1)/(s))

#define ARRAY_SIZE(x) 		(sizeof(x) / sizeof((x)[0]))

#define RoundUp(val, units) \
		((((unsigned long)(val) + ((units) - 1)) / (units)) * (units))
#define RoundDown(val, units) \
		(((unsigned long)(val)/(units))*(units))
		

#define REG32(adr)             *(volatile UINT32 *)(adr)
#define Min(a,b)  (((a) < (b)) ? (a) : (b))
#define Max(a,b)  (((a) > (b)) ? (a) : (b))

#ifndef	isblank
#define	isblank(ch)	(((ch) == ' ') || ((ch) == '\t'))
#endif

int substring(char **ptr, char *string, char *pattern);
unsigned int atonum(char *val_str);


struct burnin_cmd
{
	char    *string;					/* command name */	
	void    (*burnin_routine)();		/* implementing routine */
};

typedef struct cmd { //bessel:move from Drvftsdc021.h
	INT8 *name;
	INT8 *usage;
	//bessel:For IAR, it's a error "a value of type "UINT16 (*)(UINT32, INT8 **)" cannot be used to initialize an entity of type "INT32 (*)(INT32, INT8 **)"
	 UINT16(*func) (UINT32 argc, INT8 ** argv);// INT32(*func) (INT32 argc, INT8 ** argv);
} cmd_t;

extern int substring(char **ptr, char *string, char *pattern);
extern unsigned int atonum(char *val_str);
extern u32 get_dex(void);
extern void PrintWelcomeMsg(struct burnin_cmd * cmd, int col_width);
extern void ManualTesting(struct burnin_cmd * cmd, int col_width, int have_back);
extern void mem_dump(unsigned int addr, int size);
extern void DumpData(INT8U *pp, INT16U start_addr, INT32U size);
extern UINT32 makeargs(INT8 * cmd, INT32 * argcptr, INT8 *** argvptr);
extern INT32 do_help(INT32 argc, INT8 ** argv,cmd_t *input_CmdTbl);
extern UINT32 ExecCmd(INT8 * cmdline,cmd_t *input_CmdTbl);
extern unsigned int atonum(char *val_str);

#ifdef __cplusplus
}
#endif



#endif
