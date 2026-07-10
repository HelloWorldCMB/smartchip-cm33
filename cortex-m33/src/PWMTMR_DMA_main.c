#include "DrvPWMTMR010.h"
#include "DrvUART010.h"
#include "leo_cm33.h"
#include "io.h"
#include "types.h"
#include <stdio.h>
#include "DrvDMAC020.h"
#include <string.h>
#include "Common_include.h"

#define BUF_SIZE 64
#define PWM010_0_DMA_REQ 0 //0x50001000 bit 1:0 = 0
#define PWM010_1_DMA_REQ 1 
#define PWM010_2_DMA_REQ 2 
#define PWM010_3_DMA_REQ 3

#define TMR_TICK_BASE 0x5530004c

extern void switch_mode_register(int mode);
extern void prepare_buf(char *buf);
extern INT32 fLib_Timer_DmaEnable(UINT32 timer);
extern UINT32 pwmTmrBaseIndex;
extern UINT32 TimerBase[];

void pwmtmr_dma_by_tmrnum(DRVTIMER tmr_num, int dma_req)
{
	int i;
	char chr;
	char buf[BUF_SIZE];
	int src_width = 2;//source width 8/16/32 bits -> 0/1/2
	int dst_width = 2;//dst width 8/16/32 bits -> 0/1/2
	int burst_size = 0;
	int src_ctrl = 2 ;// Inc/dec/fixed --> 0/1/2
	int dst_ctrl = 0 ;// Inc/dec/fixed --> 0/1/2
	int priority = 0;
	int hw = 1;
	int mode = pwmTmrBaseIndex; //this is scu hardware handshake mode switch(0x50001000)
	
	if(!fLib_Timer_Init(tmr_num, PWMTMR_1000MSEC_PERIOD))//tick every 1 sec
  {
      fLib_printf("Init timer%d Fail!\n",tmr_num);
		  return;
  }	
	fLib_Timer_DmaEnable(tmr_num);
	//prepare data bufer for dmac020
	memset(buf,0,BUF_SIZE);
	
	//switch system mode register
	switch_mode_register(mode);
	//send some data by using dma020 API;
	
  fLib_InitDMA(FALSE, FALSE, 0x0);	
	fLib_DMA_ClearAllInterrupt(); 
	fLib_DMA_EnableDMAInt(); //Enable DMA Interrupt			
	fLib_DMA_NormalMode(0,TMR_TICK_BASE,(UINT32)buf, BUF_SIZE, src_width , dst_width, burst_size, src_ctrl,dst_ctrl ,priority, hw,dma_req);
	fLib_EnableDMAChannel(0);
	fLib_DMA_WaitDMAInt(0);
	fLib_DisableDMAChannel(0);  
	fLib_Timer_Close(tmr_num);
  md((UINT32)buf,BUFSIZ);	
}

void pwmtmr_dma_test_main(void)
{
	char buf[32];
	int mode;
	//pwmtmr_dma_0();
	//pwmtmr_dma_1();
	fLib_printf("Please Select which pwmtmr[0~3]: ");

	fLib_gets(DEBUG_CONSOLE, (char*)buf);
	mode = atoi((char*)buf);
	fLib_printf("\n");	
	switch(mode){
		case 0:
		case 1:
		case 2:
		case 3:
			pwmTmrBaseIndex = mode;
			break;
	}
	pwmtmr_dma_by_tmrnum(DRVPWMTMR1,PWM010_0_DMA_REQ);
	pwmtmr_dma_by_tmrnum(DRVPWMTMR2,PWM010_1_DMA_REQ);
	pwmtmr_dma_by_tmrnum(DRVPWMTMR3,PWM010_2_DMA_REQ);
	pwmtmr_dma_by_tmrnum(DRVPWMTMR4,PWM010_3_DMA_REQ);
	
}