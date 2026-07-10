#include "cmsis_os2.h"                  // ARM::CMSIS:RTOS2:Keil RTX5
#include "Common_Include.h"
#include <stdio.h>
#include "ctype.h"

#define SCREEN_WIDTH 26
#define DISP_LINE_LEN 16
#define MAX_LINE_LENGTH_BYTES (64)
#define DEFAULT_LINE_LENGTH_BYTES (16)

int print_buffer (UINT32 addr, void* data, UINT32 width, UINT32 count, UINT32 linelen)
{
	/* linebuf as a union causes proper alignment */
	union linebuf {
		uint32_t ui[MAX_LINE_LENGTH_BYTES/sizeof(uint32_t) + 1];
		uint16_t us[MAX_LINE_LENGTH_BYTES/sizeof(uint16_t) + 1];
		uint8_t  uc[MAX_LINE_LENGTH_BYTES/sizeof(uint8_t) + 1];
	} lb;
	int i;

	if (linelen*width > MAX_LINE_LENGTH_BYTES)
		linelen = MAX_LINE_LENGTH_BYTES / width;
	if (linelen < 1)
		linelen = DEFAULT_LINE_LENGTH_BYTES / width;
	while (count) {
		fLib_printf("%08x:", addr);

		/* check for overflow condition */
		if (count < linelen)
			linelen = count;

		/* Copy from memory into linebuf and print hex values */
		for (i = 0; i < linelen; i++) {
			uint32_t x;
			if (width == 4)
				x = lb.ui[i] = *(volatile uint32_t *)data;
			else if (width == 2)
				x = lb.us[i] = *(volatile uint16_t *)data;
			else
				x = lb.uc[i] = *(volatile uint8_t *)data;
			fLib_printf(" %08x", x);
			data += width;
		}

		/* Print data in ASCII characters */
		for (i = 0; i < linelen * width; i++) {
			if (!isprint(lb.uc[i]) || lb.uc[i] >= 0x80)
				lb.uc[i] = '.';
		}
		lb.uc[i] = '\0';
		fLib_printf("    %s\n", lb.uc);

		/* update references */
		addr += linelen * width;
		count -= linelen;

	}

	return 0;
}

void md(UINT32 addr, UINT32 length)
{  
	int size;
	
	size=4;	
	print_buffer(addr, (void*)addr, size, length>>3, DISP_LINE_LEN/size);
}

struct burnin_cmd
{
	char    *string;								/* command name */	
	void    (*burnin_routine)();		/* implementing routine */
};

extern void DMAC020_test_main(void);
extern void FPU_test_main(void);
extern void GPIO010_main(void);
extern void I2C010_main(void);
extern void itm_test(void);
extern void pwmtmr010_main(void);
extern void semaphore_test_main(void);
extern void I2S_test_main(void);
extern void ftssp010_main(void);
extern void kbc_test_main(void);
extern void uart_dma_test_main(void);
extern void pwmtmr_dma_test_main(void);
extern void SPI2AHB_test_main(void);
extern void CAN_test_menu(void);
extern int ddr_sram_rw_verify(void);
extern void WDT011_test_main(void);
extern void eFuse_main(void);

void cti_main(void)
{
	NVIC_EnableIRQ(SEMAPHORE_FTSEMAPHRE_CTI_IRQ);
	printf("Wait CTI interrupt\n");
	while(1);
}

void ipc_main(void)
{
	fLib_printf("trigger ca7 interrupt\n");
	writel(0x5,0x57f00100);
	delay_ms(1000);
	fLib_printf("Enable IPC interrupt, wait ca7 to trigger\n");
	NVIC_EnableIRQ(SEMAPHORE_FTSEMAPHRE_IRQ);	
}


struct burnin_cmd Legacy_cmd_value[] = {
#ifdef MEMTEST	
{"sram test",ddr_sram_rw_verify},
#endif
#ifdef I2C_TEST
	{"I2C010 Test", I2C010_main},
#endif
	
#ifdef GPIO_TEST
	{"GPIO010 Test", GPIO010_main},
#endif
	
#ifdef PWMTMR_TEST
	{"PWMTMR Test", pwmtmr010_main},
#endif
#ifdef SEMAPHORE_TEST
	{"Semaphore Test", semaphore_test_main},
#endif
	
#ifdef ITM_TEST
	{"ITM Test", itm_test},
#endif
	
#ifdef FPU_TEST
	{"FPU Test", FPU_test_main},
#endif	
	
#ifdef DMAC020_TEST
	{"DMAC020 Test", DMAC020_test_main},
#endif		

#ifdef SSPSPI_TEST
	{"SSP SPI Test", ftssp010_main},
#endif

#ifdef I2S_TEST
	{"I2S Test", I2S_test_main},
#endif
	
#ifdef KBC_TEST
	{"KBC Test", kbc_test_main},
#endif	
#ifdef UART_DMA_TEST	
	{"uart2 dmac test",uart_dma_test_main},
#endif	
#ifdef PWMTMR_DMA_TEST	
		{"pwmtmr dmac test",pwmtmr_dma_test_main},
#endif
#ifdef SPI2AHB_TEST	
		{"SPI2AHB test",SPI2AHB_test_main},
#endif		
#ifdef CAN010_TEST	
		{"CAN010 test",CAN_test_menu},
#endif			
#ifdef WDT011_TEST	
		{"WDT011 test",WDT011_test_main},
#endif			
#ifdef EFUSE_TEST	
		{"Efuse test",eFuse_main},
#endif			
		{"CTI recv test",cti_main},
		{"IPC test",ipc_main},

	{"", NULL}
};

void PrintWelcomeMsg(struct burnin_cmd * cmd, int col_width)
{
	int i=0;
    char buf[256];
    char buf1[256];
    int len;
    int str_len;
  
    fLib_printf("  ----------------------------------------------------------------------------\n");
    memset(buf, ' ', sizeof(buf));
    len=0;
    for (i=0; cmd[i].burnin_routine != NULL; ++i)
    {
     	str_len = 5 + strlen(cmd[i].string);
     	if (len+str_len>SCREEN_WIDTH)
     	{
      		buf[len]='\0';
      		fLib_printf("%s\n", buf);
      		memset(buf, ' ', sizeof(buf));
      		len=0;
     	}
     	sprintf(buf1, "(%2d) %s", i+1, cmd[i].string);
     	memcpy(buf+len, buf1, str_len);
     	len += RoundUp(str_len, col_width);
		}
		
    if (len>0)
    {
     	buf[len]='\0';
     	fLib_printf("%s\n", buf);
    }
    fLib_printf("( 0) Enter RTX Kernel\n");
}

void ManualTesting(struct burnin_cmd * cmd, int col_width)
{
	unsigned int id;
	char Buffer[256];
	int argc;
	char *argv[64];
 	int cmd_size=0;

#if 1
 	for (; cmd[cmd_size].burnin_routine; cmd_size++) ;
  
	while(1)
	{
		PrintWelcomeMsg(cmd, col_width);
		
		fLib_printf("\nCommand>>");
		fLib_gets(DEBUG_CONSOLE,Buffer);
		fLib_printf("\r");

		argc = substring(argv, Buffer, " \r\n\t");
		if (argc==1)
		{
			id = atoi(argv[0]);
 
			if (id>0 && id<=cmd_size)
			{
				if ((int)cmd[id-1].burnin_routine != 0x1)
				cmd[id-1].burnin_routine();
			}
			if (id==0)
			{
				return;
			}
		}
	}
	#endif
}

void ip_test_main(void){
	
	char infobuf[100];
	/*
	osVersion_t osv;
  osStatus_t status;
  status = osKernelGetInfo(&osv, infobuf, sizeof(infobuf));
  if(status == osOK) {
		fLib_printf("+-------------------Keil freeRTOS----------------------+\n");
    fLib_printf("Kernel Information: %s\r\n", infobuf);
    fLib_printf("Kernel Version    : %d\r\n", osv.kernel);
    fLib_printf("Kernel API Version: %d\r\n", osv.api);
  }
	*/
	
	//common timer for delay
	fLib_Timer_Init(DRVPWMTMR4, PWMTMR_10MSEC_PERIOD);
	ManualTesting(Legacy_cmd_value, SCREEN_WIDTH);
}
