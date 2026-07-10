#include <stdio.h>
#include <stdlib.h>
#include "Common_Include.h"
#include "time.h"
#include "WDT011_main.h"
#include "DrvWDT011.h"
#include "DrvUART010.h"	


uint8_t WD_ClkSrc = WdClkInt;
uint8_t WD_PreScaler = 4;
uint32_t WD_Clock = APB_CLK;
extern volatile UINT32 T4_Tick;
	
volatile UINT32 OverflowFlag, UnderflowFlag, ExtSignalFlag;
uint32_t WDT_BASE[]={WDT_FTWDT011_2_PA_BASE, WDT_FTWDT011_3_PA_BASE};
uint32_t WDT_BASE_IDX=0;

static void iwdt_test_init(void) {
	OverflowFlag  = 0;
	UnderflowFlag = 0;
	ExtSignalFlag = 0;
}

void ftwdt011_interrupt()
{
    UINT32 intSts = fLib_WatchDog_GetGlobalIntrStatus();
    UINT8 i, ovfSts, udfSts;
	
	  //fLib_WatchDog_ClearGlobalIntrStatus(intSts);
	  udfSts = (intSts & WdUDFIntrMask) >> UDFShiftBits;
	  ovfSts = (intSts & WdOVFIntrMask) >> OVFShiftBits;
	
    for (i = 0; i < DRVIWDT_MAX; i++) {
        if (intSts & (1<<i)) {
            fLib_printf("\n\rIWDT%d Interrupt ==> ", i);
					  if (udfSts & (1<<i)) {
						    fLib_printf("underflow !!!\n");
							  fLib_WatchDog_ClearUnderflowStatus((DRVIWDT) i);
							  UnderflowFlag |= (1<<i);						
						}
						else if (ovfSts & (1<<i)) {
						    fLib_printf("overflow !!!\n");
							  fLib_WatchDog_ClearOverflowStatus((DRVIWDT) i);
							  OverflowFlag |= (1<<i);
						}
						else {
						    fLib_printf("[Error] Interrupt Status is mismatched with over/under flow status !!!\n");
						}
        }
    }
}

void ftwdt011_3_interrupt()
{
    UINT32 intSts = fLib_WatchDog_GetGlobalIntrStatus();
    UINT8 i, ovfSts, udfSts;
	
	  //fLib_WatchDog_ClearGlobalIntrStatus(intSts);
	  udfSts = (intSts & WdUDFIntrMask) >> UDFShiftBits;
	  ovfSts = (intSts & WdOVFIntrMask) >> OVFShiftBits;
	
    for (i = 0; i < DRVIWDT_MAX; i++) {
        if (intSts & (1<<i)) {
            fLib_printf("\n\rIWDT%d Interrupt ==> ", i);
					  if (udfSts & (1<<i)) {
						    fLib_printf("underflow !!!\n");
							  fLib_WatchDog_ClearUnderflowStatus((DRVIWDT) i);
							  UnderflowFlag |= (1<<i);						
						}
						else if (ovfSts & (1<<i)) {
						    fLib_printf("overflow !!!\n");
							  fLib_WatchDog_ClearOverflowStatus((DRVIWDT) i);
							  OverflowFlag |= (1<<i);
						}
						else {
						    fLib_printf("[Error] Interrupt Status is mismatched with over/under flow status !!!\n");
						}
        }
    }
}

void WatchDogStart(DRVIWDT iwdt, UINT8 second)
{
    fLib_WatchDog_SetSignalLength(iwdt, 0xFF);            //signal asserting 256 clock cycles
    fLib_WatchDog_SetAutoReLoad(iwdt, WD_Clock/WD_PreScaler*second);
    fLib_WatchDog_Enable(iwdt);                           //Enable WD
}

void Reset_Board(DRVIWDT iwdt)
{
    fLib_WatchDog_SetSignalLength(iwdt, 0xFF);
    fLib_WatchDog_SetAutoReLoad(iwdt, 1);
    fLib_WatchDog_SysResetEnable(iwdt);
    fLib_WatchDog_Enable(iwdt);                           //Enable WD
}

// -----------------------------------------------------------------------------------------
// check whether watchdog will generate system interrupt (should within 2 sec) within 5 sec
// -----------------------------------------------------------------------------------------
int WatchDogOverflowSysIntTest(DRVIWDT iwdt)
{
    UINT32 nResultStatus;
    int result = 0;
    unsigned long start_jiffies;
	
    fLib_printf("\r%s start ...\n", __func__);
    OverflowFlag = 0;
    nResultStatus = FALSE;
    fLib_WatchDog_SysIntEnable(iwdt);

    #if defined(__MCU_N25F__)
    fLib_EnableInt(IWDT_FTWDT011_0_IRQ);		
    #else
    NVIC_EnableIRQ(WDT_FTWDT011_2_IRQ);
	  NVIC_EnableIRQ(WDT_FTWDT011_3_IRQ);
    #endif

    start_jiffies = T4_Tick;
    WatchDogStart(iwdt, 2);

    while (OverflowFlag == 0)
    {
        if ( (T4_Tick - start_jiffies) >= (5*1000) /*JIFF_TO_SEC(jiffies - start_jiffies) >= 3*/ )	// 5 sec
        {
            break;
        }
    }
		fLib_WatchDog_ReProg(iwdt); // stop wdt
    fLib_printf("iwdt%d overflow duration: %d ms ", iwdt, T4_Tick - start_jiffies);

    if ( (OverflowFlag & (1<<iwdt)) )
        nResultStatus = TRUE;

    fLib_WatchDog_SysIntDisable(iwdt);
    #if defined(__MCU_N25F__)
    fLib_DisableInt(IWDT_FTWDT011_0_IRQ);		
    #else
    NVIC_DisableIRQ(WDT_FTWDT011_2_IRQ);
		NVIC_DisableIRQ(WDT_FTWDT011_3_IRQ);
    #endif

    if (nResultStatus == TRUE)
    {
        fLib_printf("==> [Pass!]\n");
        result = 1;
    }
    else
    {
        fLib_printf("==> [Fail!]\n");
        result = 0;
    }

    fLib_printf("\n%s end ...\n", __func__);
    return result;
}

int WatchDogUnderflowSysIntTest(DRVIWDT iwdt)
{
    UINT32 nResultStatus;
    int result = 0;
    unsigned long start_jiffies;
	
    fLib_printf("\r%s start ...\n", __func__);
    UnderflowFlag = 0;
    nResultStatus = FALSE;
	  
	  fLib_WatchDog_SetUnderflowValue(iwdt, WD_Clock/WD_PreScaler*3); //3 Sec
    fLib_WatchDog_UnderflowIntEnable(iwdt);
    fLib_WatchDog_UnderflowEnable(iwdt);
	
	  fLib_WatchDog_SysIntEnable(iwdt);

    #if defined(__MCU_N25F__)
    fLib_EnableInt(IWDT_FTWDT011_0_IRQ);		
    #else
    NVIC_EnableIRQ(WDT_FTWDT011_2_IRQ);
	  NVIC_EnableIRQ(WDT_FTWDT011_3_IRQ);
    #endif

    WatchDogStart(iwdt, 5);
    fLib_printf("\rDelay 1s!\n");
	  delay_ms(1000);
	
    fLib_printf("\rKick(Tickle) Dog!\n");
    fLib_WatchDog_KickDog(iwdt);
	
	  start_jiffies = T4_Tick;
    while (UnderflowFlag == 0)
    {
        if ( (T4_Tick - start_jiffies) >= (1*1000) )	// 1 sec
        {
            break;
        }
    }
		fLib_WatchDog_ReProg(iwdt); // stop wdt
    fLib_printf("iwdt%d underflow duration: %d ms ", iwdt, T4_Tick - start_jiffies);

    if ( (UnderflowFlag & (1<<iwdt)) )
        nResultStatus = TRUE;

	  fLib_WatchDog_SysIntDisable(iwdt);
		
    fLib_WatchDog_UnderflowIntDisable(iwdt);
    fLib_WatchDog_UnderflowDisable(iwdt);
    #if defined(__MCU_N25F__)
    fLib_DisableInt(IWDT_FTWDT011_0_IRQ);		
    #else
    NVIC_DisableIRQ(WDT_FTWDT011_2_IRQ);
		NVIC_DisableIRQ(WDT_FTWDT011_3_IRQ);
    #endif

    if (nResultStatus == TRUE)
    {
        fLib_printf("==> [Pass!]\n");
        result = 1;
    }
    else
    {
        fLib_printf("==> [Fail!]\n");
        result = 0;
    }

    fLib_printf("\n%s end ...\n", __func__);
    return result;
}

#if 0
// ------------------------------------------------------------------------------------------
// check whether watchdog will generate external interrupt (should within 2 sec) within 5 sec
// ------------------------------------------------------------------------------------------
int WatchDogExtSignalTest(DRVIWDT iwdt)
{
    UINT32 nResultStatus;
    int result = 0;
    unsigned long start_jiffies;
	
    fLib_printf("\r%s...\n", __func__);
    ExtSignalFlag=0;
    nResultStatus = FALSE;
    fLib_WatchDog_ExtSignalEnable(iwdt);

	  NVIC_EnableIRQ(GPIO_FTGPIO010_0_IRQ); // Need to check gpio group ?

    WatchDogStart(iwdt, 2);

    start_jiffies = T4_Tick;
    fLib_printf("\rstart!\n");
    while (ExtSignalFlag == 0)
    {
        if ( (T4_Tick - start_jiffies) >= (5*1000) /*JIFF_TO_SEC(jiffies - start_jiffies) >= 3*/ )	// 5 sec
        {
            break;
        }
    }
		fLib_WatchDog_ReProg(iwdt); // stop wdt
    fLib_printf("Duration: %d ms ", T4_Tick - start_jiffies);

    if (ExtSignalFlag == 1)
        nResultStatus = TRUE;

    fLib_WatchDog_ExtSignalDisable(iwdt);
    NVIC_DisableIRQ(GPIO_FTGPIO010_0_IRQ);

    if (nResultStatus == TRUE)
    {
        fLib_printf("==> [Pass!]\n");
        result = 1;
    }
    else
    {
        fLib_printf("==> [Fail!]\n");
        result = 0;
    }

    fLib_printf("End %s !\n", __func__);
    return result;
}
#endif

#define LOOP_RESET_COUNTER          0x50000         /// program will continue to reset watch dog counter
                                                    /// when loop count is lower than this value
#define LOOP_END_TEST               0x1000000       /// if loop count reach this value and still not hw reset
                                                    /// then watch dog is fail

// --------------------------------------------------------------------
//	check whether watchDog restart is useful (can stop watch dog to reboot)
//	1. for loop to execute watchdog restart
//	2. after reach LOOP_RESET_COUNTER, wait for cpu reset
// --------------------------------------------------------------------
void WatchDogOverflowSysRstTest(DRVIWDT iwdt)
{
    int i;

    fLib_WatchDog_SysResetEnable(iwdt);
    WatchDogStart(iwdt, 2);

    fLib_printf("\r%s ...\n", __func__);

    for (i=0; i<LOOP_END_TEST; ++i)
	  {
        if (i==LOOP_RESET_COUNTER)
        {
            fLib_printf("\nThe test is PASS when CPU reset\n");
            fLib_printf("Waiting CPU reset\n");
        }
        else if (i<LOOP_RESET_COUNTER)
        {
            if (i&0xfff)
            {
                fLib_WatchDog_KickDog(iwdt);
            }
        }

        if (!(i&0xfff))
        {
            // There is some problem about
            fLib_printf(".");
        }
    }
    fLib_printf("\n\rIf you see this message \"The watchdog test is FAIL\", it presents WDT function fail\n");
}

// --------------------------------------------------------------------
// kick dog before underflow counter fired, wait for cpu reset
// --------------------------------------------------------------------
void WatchDogUnderflowSysRstTest(DRVIWDT iwdt)
{
    fLib_printf("\r%s ...\n", __func__);
	  fLib_WatchDog_SetUnderflowValue(iwdt, WD_Clock/WD_PreScaler*3); //3 Sec
    fLib_WatchDog_SysResetEnable(iwdt);
    fLib_WatchDog_UnderflowEnable(iwdt);

    WatchDogStart(iwdt, 5);
    fLib_printf("\rDelay 1s!\n");
	  delay_ms(1000);
	
    fLib_printf("\rKick(Tickle) Dog!\n");
    fLib_WatchDog_KickDog(iwdt);
    fLib_printf("\n\rIf you see this message \"The watchdog test is FAIL\", it presents WDT function fail\n");
}

// --------------------------------------------------------------------
//	check whether all watchDog is useful
//	1. for loop to execute watchdog restart
//	2. after reach LOOP_RESET_COUNTER, wait for cpu reset
// --------------------------------------------------------------------

void WatchDogAllKickedTest(void)
{
    UINT8 i, input, iwdt = 0xFF;

    fLib_printf("\r%s ...\n", __func__);
	
    for(i=0;i<DRVIWDT_MAX;i++)
    {
        fLib_WatchDog_SetUnderflowValue((DRVIWDT) i, WD_Clock/WD_PreScaler*1); //1 Sec
        fLib_WatchDog_UnderflowIntEnable((DRVIWDT) i);
        fLib_WatchDog_UnderflowEnable((DRVIWDT) i);
        fLib_WatchDog_SysIntEnable((DRVIWDT) i);
        WatchDogStart((DRVIWDT) i, 5+i);
    }
    #if defined(__MCU_N25F__)
    fLib_EnableInt(IWDT_FTWDT011_0_IRQ);		
    #else
    NVIC_EnableIRQ(WDT_FTWDT011_2_IRQ);
		NVIC_EnableIRQ(WDT_FTWDT011_3_IRQ);
    #endif
					
    do {
        delay_ms(2000);
        input -= '0';
        for (i=0; i<DRVIWDT_MAX; i++)
        {
            fLib_printf("[%d] counter: 0x%x\n", i, fLib_WatchDog_GetCurrentCounter((DRVIWDT) i));
            if ( input < DRVIWDT_MAX )
						{
							  iwdt = input;
							  fLib_printf("Stop kick iwdt%d\n", iwdt);						
						} 
						
						if (i != iwdt)
                fLib_WatchDog_KickDog((DRVIWDT) i);
        }
        input = fLib_getchar(DEBUG_CONSOLE);
    } while (input != ESC);

    for(i=0;i<DRVIWDT_MAX;i++)
    {
        fLib_WatchDog_ReProg((DRVIWDT) i); // stop wdt
        fLib_WatchDog_SysIntDisable((DRVIWDT) i);
    }
    #if defined(__MCU_N25F__)
    fLib_DisableInt(IWDT_FTWDT011_0_IRQ);		
    #else
    NVIC_DisableIRQ(WDT_FTWDT011_2_IRQ);
		NVIC_DisableIRQ(WDT_FTWDT011_3_IRQ);
    #endif
		
    fLib_printf("%s ...\n", __func__);
}

void WatchDogAllFiredTest(void)
{
    UINT8 i, input;

    fLib_printf("\r%s...\n", __func__);
	
    for(i=0;i<DRVIWDT_MAX;i++)
	  {
				fLib_WatchDog_SysIntEnable((DRVIWDT) i);
			
				if (WD_ClkSrc == WdClkInt)
					fLib_WatchDog_SetAutoReLoad((DRVIWDT) i, APB_CLK/WD_PreScaler);
				else
					fLib_WatchDog_SetAutoReLoad((DRVIWDT) i, EXT_CLK/WD_PreScaler);
			
				if ((i%2) == 0)
					fLib_WatchDog_SetPrescaler((DRVIWDT) i, (DIVIDE_16+i/2));
				else
					fLib_WatchDog_SetPrescaler((DRVIWDT) i, (DIVIDE_4+i/2));
  			
				fLib_WatchDog_SetSignalLength((DRVIWDT) i, 0xFF);
				fLib_WatchDog_Enable((DRVIWDT) i);
		}
		
    #if defined(__MCU_N25F__)
    fLib_EnableInt(IWDT_FTWDT011_0_IRQ);     
    #else
    NVIC_EnableIRQ(WDT_FTWDT011_2_IRQ);
		NVIC_EnableIRQ(WDT_FTWDT011_3_IRQ);
    #endif
					
    do {
        input = fLib_getchar(DEBUG_CONSOLE);
		} while (input != ESC);

    for(i=0;i<DRVIWDT_MAX;i++)
	  {
        fLib_WatchDog_ReProg((DRVIWDT) i); // stop wdt
        fLib_WatchDog_SysIntDisable((DRVIWDT) i);
        fLib_WatchDog_UnderflowIntDisable((DRVIWDT) i);
		}
    #if defined(__MCU_N25F__)
    fLib_DisableInt(IWDT_FTWDT011_0_IRQ);		
    #else
    NVIC_DisableIRQ(WDT_FTWDT011_2_IRQ);
		NVIC_DisableIRQ(WDT_FTWDT011_3_IRQ);
    #endif		
    fLib_printf("End %s !\n", __func__);
}
void FTWDT011_Test_Main()
{
    UINT8 item;
    char buf[80];
    unsigned int val;
    DRVIWDT iwdt = DRVIWDT_0;

    while (1)
    {
			  iwdt_test_init();
			
	      do {
   					fLib_printf("\nChoose which WDT (n=0~1) to test\n");
            fLib_printf("0: wdt011_2 (0x%x)\n", WDT_FTWDT011_2_PA_BASE);
            fLib_printf("1: wdt011_3 (0x%x)\n", WDT_FTWDT011_3_PA_BASE);
            fLib_printf("> ");
            fLib_gets(DEBUG_CONSOLE, buf);
            WDT_BASE_IDX = (DRVIWDT) atoi(buf);
				} while (WDT_BASE_IDX > ARRAY_SIZE(WDT_BASE));
				fLib_printf("choosed %d\n",WDT_BASE_IDX);
	      do {
            fLib_printf("\nChoose IWDTn (n=0~%d) or %d to start all wdt\n", DRVIWDT_MAX-1, DRVIWDT_MAX);
            fLib_printf("> ");
            fLib_gets(DEBUG_CONSOLE, buf);
            iwdt = (DRVIWDT) atoi(buf);
				} while (iwdt > DRVIWDT_MAX);

        do {
            fLib_printf("Choose clock source:\n");
            fLib_printf("1: Internal (%dHz)\n", WDT011_CLK);
            fLib_printf("2: External (EXT: %dHz)\n", EXT_CLK);
            fLib_printf("> ");
            fLib_gets(DEBUG_CONSOLE, buf);
            WD_ClkSrc = atoi(buf);
				} while (WD_ClkSrc > (WdClkExt+1));
				WD_ClkSrc -= 1;
				
				if (WD_ClkSrc == WdClkInt) {
						WD_Clock = APB_CLK;
						fLib_WatchDog_IntClockSrcEnable();
						
				}
				else {
						WD_Clock = EXT_CLK;
						fLib_WatchDog_ExtClockSrcEnable();
				}
				
        if (iwdt == DRVIWDT_MAX) // test all iwdt in the same time
        {
            fLib_printf("All IWDT0 ~ IWDT%d Test \n",DRVIWDT_MAX-1);
            fLib_printf("1: Normal Test (kick) \n");
            fLib_printf("2: Fired Test (without kick)\n");
            fLib_printf("> ");
            fLib_gets(DEBUG_CONSOLE, buf);
            item = atoi(buf);

            switch (item) {
            case 1:
                WatchDogAllKickedTest();
                break;
            case 2:
                WatchDogAllFiredTest();
                break;
						default:
							  break;
					}
        }
        else
        {				
            fLib_printf("Choose a test item:\n");
            fLib_printf("1: IWDT System Interrupt (overflow) Test \n");
            fLib_printf("2: IWDT System Interrupt (underflow) Test \n");
            fLib_printf("3: IWDT System Reset (overflow) Test \n");
            fLib_printf("4: IWDT System Reset (underflow) Test \n");
//            fLib_printf("5: IWDT External Signal Test \n");
            fLib_printf("5: Reset Board\n");
            fLib_printf("> ");
            fLib_gets(DEBUG_CONSOLE, buf);
            item = atoi(buf);
					
            switch (item) {
            case 1:
                WatchDogOverflowSysIntTest(iwdt);  // WdIntr (wd_ovf_intr)
                break;
            case 2:
                WatchDogUnderflowSysIntTest(iwdt); // WdUDFIntr (wd_udf_intr)
                break;
            case 3:
                WatchDogOverflowSysRstTest(iwdt);  // WdRst
                break;
            case 4:
                WatchDogUnderflowSysRstTest(iwdt); // WdRst
                break;
#if 0
            case 5:
                WatchDogExtSignalTest(iwdt);       // WdWxt
                break;
#endif
            case 5:
                Reset_Board(iwdt);                 // wd_rst directly
                break;
            default:
                fLib_printf("*** Non-existed item! ***\n");
                break;
            }
        }
    }
}

void WDT011_test_main(void)
{
    #if defined(__MCU_FA606TE__) || defined(__MCU_N25F__)
    fLib_Int_Init();

    if(!fLib_Timer_Init(DRVPWMTMR1, PWMTMR_1MSEC_PERIOD, (UINT32)PWMTMR1_IRQHandler, CLK_SRC_APB))//tick every 1ms
    {
        fLib_printf("Init timer%d Fail!\n",DRVPWMTMR1);
    }
    #else
    SysTick_Config(SystemCoreClock / 1000);
    #endif

//    fLib_SerialInit(DEBUG_CONSOLE, DEFAULT_CONSOLE_BAUD, PARITY_NONE, 0, 8);
    fLib_printf("\n\n");
    fLib_printf("+-----------------------------------------+\n");
    fLib_printf("|               IWDT011 Test              |\n");
    fLib_printf("+-----------------------------------------+\n");

    FTWDT011_Test_Main();
    while(1);
}
