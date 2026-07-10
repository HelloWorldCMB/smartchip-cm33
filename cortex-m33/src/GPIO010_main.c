// --------------------------------------------------------------------
//	todo list:
//		1. connect int 時, 是否為 IRQ_IAmIRQ 的問題
// --------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cmsis_os2.h"
#include "DrvGPIO010.h"	
#include "DrvUART010.h"
#include "leo_cm33.h"
#include "io.h"
#include "types.h"
#include "DrvPWMTMR010.h"
#include "ipc.h"


#define board_LED_FIRST_BIT				( 0 )				
#define board_LEDS_NUM						( 8 ) 
#define board_LED_INITIALISER			{(1 << board_LED_FIRST_BIT), (1 << (board_LED_FIRST_BIT + 1)), (1 << (board_LED_FIRST_BIT + 2)),\
																									 (1 << (board_LED_FIRST_BIT + 3)), (1 << (board_LED_FIRST_BIT + 4)), (1 << (board_LED_FIRST_BIT + 5)),(1 << (board_LED_FIRST_BIT + 6)),(1 << (board_LED_FIRST_BIT + 7))}
#define boardLED_ON					( 1 )
#define boardLED_OFF				( 0 )

#define board_Button_Start 0
#define board_Button_End 7
																									 
extern void DelayTime1(UINT32);
extern void Output2DebugPort(UINT32);
UINT32 GPIOGetS(void);

extern unsigned int volatile msTicks;                       // Counter for millisecond Interval
/* An array that contains the bit numbers of each used LED. */
uint32_t ulLEDBits[ board_LEDS_NUM ] = board_LED_INITIALISER;
int button_status=0;

extern void WaitForTick (void);				
extern void delay_ms(unsigned int count);
																									
#ifndef YAOXIN_ENABLE
void GPIO010_IRQHandler(void)
{
	UINT32 IntrMaskedStat;
	UINT8 i;
		
	IntrMaskedStat=fLib_GetGpioIntMaskStatus(GPIO_FTGPIO010_PA_BASE);
	//clear CPIO interrupt
	fLib_ClearGpioInt(GPIO_FTGPIO010_PA_BASE, IntrMaskedStat);
	for (i=0; i<32; ++i)
	{
		if((IntrMaskedStat&(1<<i))!= 0)
		{			
			fLib_printf("GPIO %d has been triggered!\n", i);
		}
	}
}
#endif


static void GPIO_InputTest(UINT32 io_base, UINT32 irq, UINT32 pattern)
{
    UINT32 i;
    UINT32 trigger_level, trigger_mode, single_both_edge=0;
    volatile unsigned char chr;
    UINT8 buf[2];

	fLib_printf("+---------------------------------------+\n");
	fLib_printf("|       Trigger by edge or level        |\n");
	fLib_printf("+---------------------------------------+\n");
	fLib_printf("| 0: Edge trigger                       |\n");
	fLib_printf("| 1: Level trigger                      |\n");
	fLib_printf("+---------------------------------------+\n");
	fLib_printf("Please Select keystroke 0 or 1: ");

	fLib_gets(DEBUG_CONSOLE, (char*)buf);
	trigger_level = atoi((char*)buf);
	
	fLib_printf("\n");
	
	//trigger_level = GPIOGetS();
	if (trigger_level == 0)
	{
	    trigger_level = GPIO_EDGE;
	// Because some GPIO are pull low, so must use edge trigger!!
	//trigger_level = GPIO_EDGE;
		fLib_printf("+---------------------------------------+\n");
		fLib_printf("|       Trigger by single or both edge  |\n");
		fLib_printf("+---------------------------------------+\n");
		fLib_printf("| 0: Single edge                        |\n");
		fLib_printf("| 1: Both edge                          |\n");
		fLib_printf("+---------------------------------------+\n");
		fLib_printf("Please Select keystroke 0 or 1: ");

		fLib_gets(DEBUG_CONSOLE, (char*)buf);
		single_both_edge = atoi((char*)buf);
	fLib_printf("\n");
	//	single_both_edge = GPIOGetS();
	if (single_both_edge == 0)
	{
	    single_both_edge = SINGLE;
	}
	else
	{
	    single_both_edge = BOTH;	
	}
		fLib_printf("+------------------------------------------------+\n");
		fLib_printf("|       Trigger by Rising or Failing edage       |\n");
		fLib_printf("+------------------------------------------------+\n");
		fLib_printf("| 0: Rising edge                                 |\n");
		fLib_printf("| 1: Falling edge                                |\n");
		fLib_printf("+------------------------------------------------+\n");
		fLib_printf("Please Select keystroke 0 or 1: ");

		fLib_gets(DEBUG_CONSOLE, (char*)buf);
		trigger_mode = atoi((char*)buf);
		fLib_printf("\n");
		if (trigger_mode == 0)
		{
			trigger_mode = GPIO_Rising;
		}
		else
		{
			trigger_mode = GPIO_Falling;	
		}
	}
	else
	{
	    trigger_level = GPIO_LEVEL;	
		fLib_printf("+------------------------------------------------+\n");
		fLib_printf("|       Trigger by High or Low level             |\n");
		fLib_printf("+------------------------------------------------+\n");
		fLib_printf("| 0: High level                                  |\n");
		fLib_printf("| 1: Low level                                   |\n");
		fLib_printf("+------------------------------------------------+\n");
		fLib_printf("Please  Select keystroke 0 or 1: ");

		fLib_gets(DEBUG_CONSOLE, (char*)buf);
		trigger_mode = atoi((char*)buf);
		fLib_printf("\n");
		if (trigger_mode == 0)
		{
			trigger_mode = GPIO_High;
		}
		else
		{
			trigger_mode = GPIO_LOW;	
		}
	}
	// In the normal case, GPIO will pull high, when push button, GPIO will pull low
	// So set interrupt to be low active
	//trigger_mode = GPIO_LOW;			/// edge trigger ==> failing edge, level trigger ==> low level

#if defined(__PLATFORM_FIE3101F__)
		// In FIE3101, PB4 and PB5 are mapped into GPIO_FTGPIO010_0_14 and 15, respectively
		for (i=14; i<=15; ++i)
#else	
    for (i=board_Button_Start; i<board_Button_End; ++i)
#endif	
    {
    	if ( (pattern>>i)&0x1 )
    	{
        	fLib_DisableGpioInt(io_base, i);
        }
    }

	// Set GPIO 0 ~ 31 as Input mode and interrupt trigger mode
#if defined(__PLATFORM_FIE3101F__)
		// In FIE3101, PB4 and PB5 are mapped into GPIO_FTGPIO010_0_14 and 15, respectively
	for (i=14; i<=15; ++i)
#else			
	for (i=board_Button_Start; i<board_Button_End + 1; ++i)
#endif		
	{
		if ( (pattern>>i)&0x1 )
		{
			fLib_SetGpioDir(io_base, i,GPIO_DIR_INPUT);
			fLib_SetGpioEdgeMode(io_base, i, single_both_edge);
			fLib_SetGpioTrigger(io_base, i, trigger_level);
			fLib_SetGpioActiveMode(io_base, i, trigger_mode);
			//clear CPIO interrupt
			fLib_ClearGpioInt(io_base, 1<<i);

			fLib_EnableGpioBounce(io_base, i, APB_CLOCK /12000); // 
				
			//Enable GPIO interrupt
			fLib_SetGpioIntEnable(io_base, i);
			fLib_SetGpioIntUnMask(io_base, i);
		}
	}
	
	#if defined(__MCU_FA606TE__)
	fLib_DisableInt(GPIO_FTGPIO010_0_IRQ);
	fLib_SetIntTrig(GPIO_FTGPIO010_0_IRQ, LEVEL, H_ACTIVE);
  
	if(fLib_ConnectVectorInt(GPIO_FTGPIO010_0_IRQ, (UINT32)GPIO010_IRQHandler) == FALSE)
	{
		fLib_printf("Fail to register vector irq\n");		
	}
	
	fLib_EnableInt(GPIO_FTGPIO010_0_IRQ);
	#else
	NVIC_EnableIRQ(GPIO_FTGPIO010_IRQ);//fLib_UnmaskInt(GPIO_FTGPIO010_0_IRQ);//Enable External 3 of NVIC
	#endif
	fLib_printf("Press any GPIO button...or press \"ESC\" keystroke to Quit\n");

	while(1)
	{
		//GPIO010_IRQHandler();//polling
		chr = fLib_getch(DEBUG_CONSOLE);	//Press "ESC" key to terminate GPIO test	
        if (chr==0x1b)
		{
			break;
		}
	}

	#if defined(__MCU_FA606TE__)
	fLib_DisableInt(GPIO_FTGPIO010_0_IRQ);		
	
	if(fLib_CloseVectorInt(GPIO_FTGPIO010_0_IRQ) == FALSE)
		{
		fLib_printf("Fail to unregister vector irq\n");		
	}
	#else
	NVIC_DisableIRQ(GPIO_FTGPIO010_IRQ);
	#endif	
	
	fLib_printf("GPIO Input Test Finish!\n");
}

static void GPIO_OutputTest(UINT32 io_base)
{
	UINT32 i;
	static UINT8 ucLED = 0, ucState = boardLED_ON;	
	
	/* Set the LED outputs to output. */
	/* Configure GPIO4~7 as outputs to control LEDs */
	for (i = 0; i < board_LEDS_NUM; i++)	
	{
		fLib_SetGpioDir(GPIO_FTGPIO010_PA_BASE, (i + board_LED_FIRST_BIT), GPIO_DIR_OUTPUT);	
	}
		
	fLib_printf("Now an array of LEDs are periodically toggled. Press \"ESC\" keystroke to Quit\n");

	while(1)
	{
		fLib_printf("start ucLED %d\n",ucLED);
		delay_ms(50);
		if( ucState == boardLED_ON )
		{
			/* Turn the next LED on. */
			fLib_Gpio_SetData(GPIO_FTGPIO010_PA_BASE, ulLEDBits[ucLED]);					
		}
		else
		{
			/* Turn the next LED off. */
			fLib_Gpio_ClearData(GPIO_FTGPIO010_PA_BASE, ulLEDBits[ucLED]);				
		}

		ucLED++;
	
		if( ucLED >= board_LEDS_NUM )
		{
			/* All the LEDs have now either been turned on, or off.  Go back to the
			first LED and invert the setting. */
			ucLED = 0U;

			if( ucState == boardLED_ON )
			{
				ucState = boardLED_OFF;
			}
			else
			{
				ucState = boardLED_ON;
			}
		}		

		if (fLib_getch(DEBUG_CONSOLE) == 0x1b)
		{
			/* Turn off all LEDs when exiting */
			for (i = 0; i < board_LEDS_NUM; i++)	
			{
				fLib_Gpio_ClearData(GPIO_FTGPIO010_PA_BASE, ulLEDBits[i]);				
			}
			
			ucLED = 0;
			ucState = boardLED_ON;
			break;
		}
	}

	fLib_printf("GPIO Output Test Finish!\n");
}

UINT32 GPIOGetS(void)
{
	char Buffer[256];
	char seps[]  = " ";
	char *token;
	char Cmd[256];
	UINT32 i;

	fLib_gets(DEBUG_CONSOLE, Buffer);
	token = strtok( Buffer, seps );
	if( token == NULL )
		return (0);
       
	strcpy(Cmd, token);
	
	i = atoi( Cmd );

	return(i);
}
extern void delay_ms(unsigned int count);
void GPIO010_main(void)
{
	volatile UINT8 key;

	fLib_printf("GPIO\n");
	
	do
  { 
  
		fLib_printf("\n");
	fLib_printf("+---------------------------------------+\n");
	fLib_printf("|             GPIO010 Test              |\n");
	fLib_printf("+---------------------------------------+\n");
		fLib_printf("| 0: GPIO Input Test                    |\n");
		fLib_printf("| 1: GPIO Output Test                   |\n");
		fLib_printf("+---------------------------------------+\n");
		fLib_printf("| ESC : Quit                            |\n");
		fLib_printf("+---------------------------------------+\n");
		fLib_printf("Please Select keystroke 0-1 or ESC to exit: ");			

	  key = fLib_getchar(DEBUG_CONSOLE);
	  fLib_putchar(DEBUG_CONSOLE, key);
	  fLib_printf("\n\n");
	
		switch(key)
	  {
	   	case '0':
	GPIO_InputTest(GPIO_FTGPIO010_PA_BASE, GPIO_FTGPIO010_IRQ, 0xffffffff);
				break;
	    case '1':  
				GPIO_OutputTest(GPIO_FTGPIO010_PA_BASE);
				break;
			default:
				break;
	  }
	}while(key != 0x1b);
	
	fLib_printf("GPIO010 Test Finish!\n");	
}

