#include "DrvPWMTMR010.h"
#include "DrvUART010.h"
#include "leo_cm33.h"
#include "io.h"
#include "types.h"
#include <stdio.h>
//#include "utility.h"
#include "cmsis_os2.h"
#include <stdlib.h>
#include "DrvGPIO010.h"
UINT8 hr,min,sec;
extern volatile UINT32 T1_Tick, T2_Tick, T3_Tick, T4_Tick;
extern UINT32 pwmTmrBaseIndex;

#define PWMTMR_TEST

void WaitForTick (void)  {
  UINT32 curTicks;

  curTicks = T4_Tick;                            // Save Current SysTick Value
  while (T4_Tick == curTicks)  {                 // Wait for next SysTick Interrupt
  #ifdef __MCU_FA606TE__
		__asm("SWI 0x0");
  #else
		__asm("SWI 0x0");
	  //__WFI (); 								   // Power-Down until next Event/Interrupt
  #endif
  }

}

void delay_ms(unsigned int count)
{
    unsigned i;
    for ( i = count; i!= 0; --i )
    {
        WaitForTick();
    }
	
}
/*void delay_ms(int msec)
{
    for (;msec != 0; --msec)
    {
        WaitForTick();
    }
}*/

void Delay(int sec)
{
    for (;sec != 0; --sec)
    {
        WaitForTick();
				//osDelay(sec);
    }
}

void PWM_Test(void)
{
	UINT8 key;
	char buf[32];
	int mode;
	DRVTIMER drvtmr;
	//pwmtmr_dma_0();
	//pwmtmr_dma_1();
	fLib_printf("Please Select which pwmtmr[0~3]: ");

	fLib_gets(DEBUG_CONSOLE, (char*)buf);
	mode = atoi((char*)buf);
	fLib_printf("\n");	
	
	pwmTmrBaseIndex = 0;//default
	switch(mode){
		case 0:
		case 1:
		case 2:
		case 3:
			pwmTmrBaseIndex = mode;
			break;
	}

	fLib_printf("Please Select which Timer[1~%d]: ",MAX_TIMER);

	fLib_gets(DEBUG_CONSOLE, (char*)buf);
	mode = atoi((char*)buf);
	fLib_printf("\n");	
	
	drvtmr = 1;//default
	switch(mode){
		case 1:
		case 2:
		case 3:
		case 4:
			if(pwmTmrBaseIndex==0 && mode==4){
				fLib_printf("hw0 tmr 4 is reserved for system timer\n");
				return;
			}
			drvtmr = (DRVTIMER) mode;
			break;
	}

 // fLib_Timer_Change_tmr_base(pwmTmrBaseIndex);
  fLib_Timer_Init(drvtmr, PWMTMR_1000MSEC_PERIOD);
	fLib_printf("PWM Function Test\n"); 
	fLib_printf("Press ESC to escape\n"); 
	ftpwmtmr010_pwm_config(drvtmr, 100000, 500000);
	ftpwmtmr010_pwm_set_polarity(0,0);
	ftpwmtmr010_pwm_enable(drvtmr);

	while(1){
		key = fLib_getchar(DEBUG_CONSOLE);
		if (key==0x1b)
			break;
/*		
		if (key=='q'){
			ftpwmtmr010_pwm_config(DRVPWMTMR1, 1000, 5000);
			ftpwmtmr010_pwm_set_polarity(0,0);
			ftpwmtmr010_pwm_enable(DRVPWMTMR1);
			break;
		}
*/		
	}
	fLib_Timer_Close(drvtmr);
fLib_printf("End\n"); 
}

// waiting to do: to arrangement the code
void pwmtmr010_main(void)
{
		UINT8 chr;
	  UINT8 uart_buf[16];
    //PinMux_UART010(0);
#if defined(__MCU_FA606TE__)
    fLib_Int_Init();
#endif	
	  fLib_printf("[1]pwm function output test\n");
	  fLib_printf("[2]timer function test\n");
	  fLib_gets(DEBUG_CONSOLE, (char*)uart_buf);
	  chr = atoi((char*)uart_buf);
	  fLib_printf("\n");		
		
	  if (chr==1){
			fLib_printf("\nPWM function Begin. Please measure at output pin...\n"); 
			PWM_Test();
			return;
		}
    //fLib_SerialInit(DEBUG_CONSOLE,DEFAULT_CONSOLE_BAUD, PARITY_NONE, 0, 8);
    fLib_printf("\n\n\nPWMTMR010 Test Begin...............\n"); 
		//Timer1
    fLib_printf("Timer1 Test...(As clock, Press \"ESC\" key to End Timer1 Test ...)\n");
    sec=min=hr=0;   
    #if defined(__MCU_FA606TE__)		
    if(!fLib_Timer_Init(DRVPWMTMR1, PWMTMR_1000MSEC_PERIOD, (UINT32)PWMTMR1_IRQHandler))//tick every 1 sec
    #else
	if(!fLib_Timer_Init(DRVPWMTMR1, PWMTMR_1MSEC_PERIOD))//tick every 1 sec
    #endif
    {
        fLib_printf("Init timer%d Fail!\n",DRVPWMTMR1);
    }
    else
    {
			while(fLib_getchar(DEBUG_CONSOLE) != 0x1b)
			{
				chr = fLib_getchar(DEBUG_CONSOLE);	//Press "ESC" key to terminate GPIO test					
				if (chr=='q')
				{
					break;
				}
			}
			if(inw(0xe000e300)!=0){
				fLib_printf("Interrupt!!\n");
			}
    }

    fLib_Timer_Close(DRVPWMTMR1);
#if 0
	if( MAX_TIMER >= 2)
	{
		//Timer2
	    fLib_printf("\nTimer2 Test...(Press \"ESC\" key to Timer3 Test ...)\n");
	    // sec=min=hr=0;   
	    #if defined(__MCU_FA606TE__)
	    if(!fLib_Timer_Init(DRVPWMTMR2, PWMTMR_1000MSEC_PERIOD, (UINT32)PWMTMR2_IRQHandler))//tick every 1 sec
	    #else
		if(!fLib_Timer_Init(DRVPWMTMR2, PWMTMR_10MSEC_PERIOD))//tick every 1 sec
	    #endif		
		{
		  fLib_printf("Init timer%d Fail!\n",DRVPWMTMR2);
		}
		else
		{
		  while(1)
		  {
			  chr = fLib_getchar(DEBUG_CONSOLE);  //Press "ESC" key to terminate GPIO test	  
			  if (chr==0x1b)
			  {
				  break;
			  }
		  }
		}
	    fLib_Timer_Close(DRVPWMTMR2);    
    }
#endif    
	if( MAX_TIMER >= 3)
	{
		//Timer3
		fLib_printf("\nTimer3 Test...(Press \"ESC\" key to Timer4 Test ...)\n");
	   // sec=min=hr=0;   
		#if defined(__MCU_FA606TE__)
	    if(!fLib_Timer_Init(DRVPWMTMR3, PWMTMR_1000MSEC_PERIOD, (UINT32)PWMTMR3_IRQHandler))//tick every 1 sec
	    #else
	  if(!fLib_Timer_Init(DRVPWMTMR3, PWMTMR_1000MSEC_PERIOD))//tick every 1 sec		
	    #endif
		{
			fLib_printf("Init timer%d Fail!\n",DRVPWMTMR3);
		}
		else
		{
			while(1)
			{				
			chr = fLib_getchar(DEBUG_CONSOLE);	//Press "ESC" key to terminate GPIO test	
			  if (chr==0x1b)
			  {
				  break;
			  }
			}
		}
		fLib_Timer_Close(DRVPWMTMR3);   
	}
	
	if( MAX_TIMER >= 4)
	{
		//Timer4
		//sec=min=hr=0;   
		fLib_printf("\nTimer4 Test...(Press \"ESC\" key to finish Test ...)\n");
		#if defined(__MCU_FA606TE__)
	    if(!fLib_Timer_Init(DRVPWMTMR4, PWMTMR_1000MSEC_PERIOD, (UINT32)PWMTMR4_IRQHandler))//tick every 1 sec
	    #else
			if(!fLib_Timer_Init(DRVPWMTMR4, PWMTMR_1000MSEC_PERIOD))//tick every 1 sec
	    #endif
		{
		  fLib_printf("Init timer%d Fail!\n",DRVPWMTMR4);
		}
		else
		{
		  while(1)
		  {
			  chr = fLib_getchar(DEBUG_CONSOLE);  //Press "ESC" key to terminate GPIO test	  
				if (chr==0x1b)
				{
					break;
				}
		  }
		}
	   fLib_Timer_Close(DRVPWMTMR4);
	}

    fLib_printf("End Timer Test!\n");
}

#if defined(__MCU_FA606TE__)
#if defined(__GHS__)
__interrupt
#else
__irq
#endif
#endif
void PWMTMR1_IRQHandler(void)
{	
	unsigned int reg_val;
 	fLib_Timer_IntClear(0, DRVPWMTMR1);
  T1_Tick++;
 	sec++;
	
	reg_val = fLib_Gpio_ReadData(GPIO_FTGPIO010_PA_BASE);
	if((reg_val >> 5) & 0x1) 
		reg_val &= ~(1<<5);
	else 
		reg_val |= (1<<5);
	
	fLib_Gpio_WriteData(GPIO_FTGPIO010_PA_BASE, reg_val);
	
#ifdef PWMTMR_TEST
	if( sec == 60 )
	{
		min++;
		sec=0;
		if( min == 60 )
		{
			hr++;
			min=0;
		}
	}
	
	fLib_printf("1: %02d: %02d: %02d\r",hr,min,sec);
#endif
}

#if defined(__MCU_FA606TE__)
#if defined(__GHS__)
__interrupt
#else
__irq
#endif
#endif
void PWMTMR2_IRQHandler(void)
{
 	fLib_Timer_IntClear(0, DRVPWMTMR2);
  T2_Tick++;
 	sec++;
#ifdef OTG210_HOST_TEST
	gwOTG_Timer_Counter++;
#endif
	
#ifdef PWMTMR_TEST
	if( sec == 60 )
	{
		min++;
		sec=0;
		if( min == 60 )
		{
			hr++;
			min=0;
		}
	}
	
	fLib_printf("2: %02d: %02d: %02d\r",hr,min,sec);
#endif
}

#if defined(__MCU_FA606TE__)
#if defined(__GHS__)
__interrupt
#else
__irq
#endif
#endif
void PWMTMR3_IRQHandler(void)
{
 	fLib_Timer_IntClear(0, DRVPWMTMR3);
  T3_Tick++;
 	sec++;
	if( sec == 60 )
	{
		min++;
		sec=0;
		if( min == 60 )
		{
			hr++;
			min=0;
		}
	}
	
	fLib_printf("3: %02d: %02d: %02d\r",hr,min,sec);    
}

#if defined(__MCU_FA606TE__)
#if defined(__GHS__)
__interrupt
#else
__irq
#endif
#endif
void PWMTMR4_IRQHandler(void)
{
 	fLib_Timer_IntClear(0, DRVPWMTMR4);

    T4_Tick++;
 	sec++;
	if( sec == 60 )
	{
		min++;
		sec=0;
		if( min == 60 )
		{
			hr++;
			min=0;
		}
	}
	
	//fLib_printf("4: %02d: %02d: %02d\r",hr,min,sec);    
}

void PWMTMR_IRQHandler(void)
{
 	fLib_Timer_IntClear(0, DRVPWMTMR1);

    T4_Tick++;
 	sec++;
	if( sec == 60 )
	{
		min++;
		sec=0;
		if( min == 60 )
		{
			hr++;
			min=0;
		}
	}
	
	//fLib_printf(" %02d: %02d: %02d\r",hr,min,sec);    	
}


void PWMTMR_1_1_IRQHandler(void)
{	
 	fLib_Timer_IntClear(1, DRVPWMTMR1);
  T1_Tick++;
 	sec++;
	
#ifdef PWMTMR_TEST
	if( sec == 60 )
	{
		min++;
		sec=0;
		if( min == 60 )
		{
			hr++;
			min=0;
		}
	}
	
	fLib_printf("1-1: %02d: %02d: %02d\r",hr,min,sec);
#endif
}

#if defined(__MCU_FA606TE__)
#if defined(__GHS__)
__interrupt
#else
__irq
#endif
#endif
void PWMTMR_1_2_IRQHandler(void)
{
 	fLib_Timer_IntClear(1, DRVPWMTMR2);
  T2_Tick++;
 	sec++;
#ifdef OTG210_HOST_TEST
	gwOTG_Timer_Counter++;
#endif
	
#ifdef PWMTMR_TEST
	if( sec == 60 )
	{
		min++;
		sec=0;
		if( min == 60 )
		{
			hr++;
			min=0;
		}
	}
	
	fLib_printf("1-2: %02d: %02d: %02d\r",hr,min,sec);
#endif
}

#if defined(__MCU_FA606TE__)
#if defined(__GHS__)
__interrupt
#else
__irq
#endif
#endif
void PWMTMR_1_3_IRQHandler(void)
{
 	fLib_Timer_IntClear(1 ,DRVPWMTMR3);
  T3_Tick++;
 	sec++;
	if( sec == 60 )
	{
		min++;
		sec=0;
		if( min == 60 )
		{
			hr++;
			min=0;
		}
	}
	
	fLib_printf("1-3: %02d: %02d: %02d\r",hr,min,sec);    
}

void PWMTMR_1_4_IRQHandler(void)
{
 	fLib_Timer_IntClear(1 ,DRVPWMTMR4);
  T3_Tick++;
 	sec++;
	if( sec == 60 )
	{
		min++;
		sec=0;
		if( min == 60 )
		{
			hr++;
			min=0;
		}
	}
	
	//fLib_printf("1-4: %02d: %02d: %02d\r",hr,min,sec);    
}


void PWMTMR_2_1_IRQHandler(void)
{	
 	fLib_Timer_IntClear(2, DRVPWMTMR1);
  T1_Tick++;
 	sec++;
	
#ifdef PWMTMR_TEST
	if( sec == 60 )
	{
		min++;
		sec=0;
		if( min == 60 )
		{
			hr++;
			min=0;
		}
	}
	
	fLib_printf("2-1: %02d: %02d: %02d\r",hr,min,sec);
#endif
}

#if defined(__MCU_FA606TE__)
#if defined(__GHS__)
__interrupt
#else
__irq
#endif
#endif
void PWMTMR_2_2_IRQHandler(void)
{
 	fLib_Timer_IntClear(2, DRVPWMTMR2);
  T2_Tick++;
 	sec++;
#ifdef OTG210_HOST_TEST
	gwOTG_Timer_Counter++;
#endif
	
#ifdef PWMTMR_TEST
	if( sec == 60 )
	{
		min++;
		sec=0;
		if( min == 60 )
		{
			hr++;
			min=0;
		}
	}
	
	fLib_printf("2-2: %02d: %02d: %02d\r",hr,min,sec);
#endif
}

#if defined(__MCU_FA606TE__)
#if defined(__GHS__)
__interrupt
#else
__irq
#endif
#endif
void PWMTMR_2_3_IRQHandler(void)
{
 	fLib_Timer_IntClear(2, DRVPWMTMR3);
  T3_Tick++;
 	sec++;
	if( sec == 60 )
	{
		min++;
		sec=0;
		if( min == 60 )
		{
			hr++;
			min=0;
		}
	}
	
	fLib_printf("2-3: %02d: %02d: %02d\r",hr,min,sec);    
}

void PWMTMR_2_4_IRQHandler(void)
{
 	fLib_Timer_IntClear(2 ,DRVPWMTMR4);
  T3_Tick++;
 	sec++;
	if( sec == 60 )
	{
		min++;
		sec=0;
		if( min == 60 )
		{
			hr++;
			min=0;
		}
	}
	
	fLib_printf("2-4: %02d: %02d: %02d\r",hr,min,sec);    
}


void PWMTMR_3_1_IRQHandler(void)
{	
 	fLib_Timer_IntClear(3, DRVPWMTMR1);
  T1_Tick++;
 	sec++;
	
#ifdef PWMTMR_TEST
	if( sec == 60 )
	{
		min++;
		sec=0;
		if( min == 60 )
		{
			hr++;
			min=0;
		}
	}
	
	fLib_printf("3-1: %02d: %02d: %02d\r",hr,min,sec);
#endif
}

#if defined(__MCU_FA606TE__)
#if defined(__GHS__)
__interrupt
#else
__irq
#endif
#endif
void PWMTMR_3_2_IRQHandler(void)
{
 	fLib_Timer_IntClear(3, DRVPWMTMR2);
  T2_Tick++;
 	sec++;
#ifdef OTG210_HOST_TEST
	gwOTG_Timer_Counter++;
#endif
	
#ifdef PWMTMR_TEST
	if( sec == 60 )
	{
		min++;
		sec=0;
		if( min == 60 )
		{
			hr++;
			min=0;
		}
	}
	
	fLib_printf("3-2: %02d: %02d: %02d\r",hr,min,sec);
#endif
}

#if defined(__MCU_FA606TE__)
#if defined(__GHS__)
__interrupt
#else
__irq
#endif
#endif
void PWMTMR_3_3_IRQHandler(void)
{
 	fLib_Timer_IntClear(3, DRVPWMTMR3);
  T3_Tick++;
 	sec++;
	if( sec == 60 )
	{
		min++;
		sec=0;
		if( min == 60 )
		{
			hr++;
			min=0;
		}
	}
	
	fLib_printf("3-3: %02d: %02d: %02d\r",hr,min,sec);    
}

void PWMTMR_3_4_IRQHandler(void)
{
 	fLib_Timer_IntClear(3 ,DRVPWMTMR4);
  T3_Tick++;
 	sec++;
	if( sec == 60 )
	{
		min++;
		sec=0;
		if( min == 60 )
		{
			hr++;
			min=0;
		}
	}
	
	//fLib_printf("3-4: %02d: %02d: %02d\r",hr,min,sec);    
}