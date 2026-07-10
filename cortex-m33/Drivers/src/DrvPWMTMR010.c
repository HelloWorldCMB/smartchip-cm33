/***************************************************************************
* Copyright  Faraday Technology Corp 2002-2003.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:timer.c                                                             *
* Description: Timer library routine                                       *
* Author:                                                        *
****************************************************************************/
//#include "flib.h"
#include "common_include.h"
#include "Driver_Common.h"
#include "leo_cm33.h"
#include "DrvPWMTMR010.h"
#include <stdlib.h>

UINT32 pwmTmrBaseIndex = -1;
UINT32 TimerBase[MAX_TIMER+1] ={0, PWM_FTPWMTMR010_PA_BASE+0x10, PWM_FTPWMTMR010_PA_BASE+0x20,PWM_FTPWMTMR010_PA_BASE+0x30,PWM_FTPWMTMR010_PA_BASE+0x40};
UINT32 CNTBBase[MAX_TIMER+1] ={0, PWM_FTPWMTMR010_PA_BASE+0x14 };
UINT32 CNPBBase[MAX_TIMER+1] ={0, PWM_FTPWMTMR010_PA_BASE+0x18 };

UINT32 TimerBase0[MAX_TIMER+1] ={0, PWM_FTPWMTMR010_PA_BASE+0x10, PWM_FTPWMTMR010_PA_BASE+0x20,PWM_FTPWMTMR010_PA_BASE+0x30,PWM_FTPWMTMR010_PA_BASE+0x40};
UINT32 CNTBBase0[MAX_TIMER+1] ={0, PWM_FTPWMTMR010_PA_BASE+0x14 };
UINT32 CNPBBase0[MAX_TIMER+1] ={0, PWM_FTPWMTMR010_PA_BASE+0x18 };

UINT32 TimerBase1[MAX_TIMER+1] ={0, PWM_FTPWMTMR010_1_PA_BASE+0x10, PWM_FTPWMTMR010_1_PA_BASE+0x20,PWM_FTPWMTMR010_1_PA_BASE+0x30,PWM_FTPWMTMR010_1_PA_BASE+0x40};
UINT32 CNTBBase1[MAX_TIMER+1] ={0, PWM_FTPWMTMR010_1_PA_BASE+0x14 };
UINT32 CNPBBase1[MAX_TIMER+1] ={0, PWM_FTPWMTMR010_1_PA_BASE+0x18 };

UINT32 TimerBase2[MAX_TIMER+1] ={0, PWM_FTPWMTMR010_2_PA_BASE+0x10, PWM_FTPWMTMR010_2_PA_BASE+0x20,PWM_FTPWMTMR010_2_PA_BASE+0x30,PWM_FTPWMTMR010_2_PA_BASE+0x40};
UINT32 CNTBBase2[MAX_TIMER+1] ={0, PWM_FTPWMTMR010_2_PA_BASE+0x14 };
UINT32 CNPBBase2[MAX_TIMER+1] ={0, PWM_FTPWMTMR010_2_PA_BASE+0x18 };

UINT32 TimerBase3[MAX_TIMER+1] ={0, PWM_FTPWMTMR010_3_PA_BASE+0x10, PWM_FTPWMTMR010_3_PA_BASE+0x20,PWM_FTPWMTMR010_3_PA_BASE+0x30,PWM_FTPWMTMR010_3_PA_BASE+0x40};
UINT32 CNTBBase3[MAX_TIMER+1] ={0, PWM_FTPWMTMR010_3_PA_BASE+0x14 };
UINT32 CNPBBase3[MAX_TIMER+1] ={0, PWM_FTPWMTMR010_3_PA_BASE+0x18 };


//#define timer_base  TIMER_FTPWMTMR010_PA_BASE
fLib_TimerControl *TimerControl[MAX_TIMER+1];
fLib_CNTB *PWMCNTB[MAX_TIMER+1];
fLib_CNPB *PWMCNPB[MAX_TIMER+1];

static fLib_TimerStruct ftimer[MAX_TIMER+1];
//funtion prototype
void fLib_Timer_AutoReloadValue(DRVTIMER timer, UINT32 value);
void  Timer_ResetAll(void);
INT32 GetTimerTick(DRVTIMER timer);
INT32 SetTimerTick(UINT32 timer,UINT32 clk_tick);
INT32 SetTimerClkSource(DRVTIMER timer,UINT32 clk);
//INT32 Timer_ConnectIsr(UINT32 timer,PrHandler handler);
volatile UINT32 T1_Tick = 0,T2_Tick = 0, T3_Tick = 0,T4_Tick = 0;
INT32 fLib_Timer_IntClear(int pwm, DRVTIMER timer);
INT32 fLib_Timer_AutoReloadEnable(DRVTIMER timer);
INT32 fLib_Timer_Counter(DRVTIMER timer);
//PWM
INT32 ftpwmtmr010_pwm_config(DRVTIMER timer, int duty_ns, int period_ns);
INT32 ftpwmtmr010_pwm_set_polarity(DRVTIMER timer, UINT8 polarity);
INT32 ftpwmtmr010_pwm_enable(DRVTIMER timer);
INT32 ftpwmtmr010_pwm_disable(DRVTIMER timer);

void fLib_Timer_Change_tmr_base(int index)
{
	switch(index){
		case 0:
			memcpy(TimerBase, TimerBase0, sizeof(TimerBase));
			memcpy(CNTBBase, CNTBBase0, sizeof(CNTBBase));
			memcpy(CNPBBase, CNPBBase0, sizeof(CNPBBase0));
			break;
		case 1:
			memcpy(TimerBase, TimerBase1, sizeof(TimerBase));
			memcpy(CNTBBase, CNTBBase1, sizeof(CNTBBase));
			memcpy(CNPBBase, CNPBBase1, sizeof(CNPBBase1));
			break;
		case 2:
			memcpy(TimerBase, TimerBase2, sizeof(TimerBase));
			memcpy(CNTBBase, CNTBBase2, sizeof(CNTBBase));
			memcpy(CNPBBase, CNPBBase2, sizeof(CNPBBase2));
			break;
		case 3:
			memcpy(TimerBase, TimerBase3, sizeof(TimerBase));
			memcpy(CNTBBase, CNTBBase3, sizeof(CNTBBase));
			memcpy(CNPBBase, CNPBBase3, sizeof(CNPBBase3));
			break;
	}
}

/* Routine to disable a timer and free-up the associated IRQ */
INT32 fLib_Timer_Close(DRVTIMER timer)
{
	UINT32 timer_irq;


    if (timer == 0 || timer > MAX_TIMER)
        return FALSE;

    if(!fLib_Timer_Disable(timer))
        return FALSE;   /* Stop the timer first */

	switch(timer)
	{
		case DRVPWMTMR1:
			timer_irq = PWM_FTPWMTMR010_1_IRQ;
		break;
		case DRVPWMTMR2:
			timer_irq = PWM_FTPWMTMR010_2_IRQ;
		break;
		case DRVPWMTMR3:
			timer_irq = PWM_FTPWMTMR010_3_IRQ;
		break;
		case DRVPWMTMR4:
			timer_irq = PWM_FTPWMTMR010_4_IRQ;
		break;
		default:
			return FALSE;
	}	
		
	NVIC_DisableIRQ((IRQn_Type)timer_irq);
		
    return TRUE;
}


INT32 fLib_Timer_IOCtrl(fLib_Timer_IoType IoType,DRVTIMER timer,UINT32 tick)
{

    switch(IoType)
    {
    case IO_TIMER_RESETALL:
        Timer_ResetAll();
        break;
    case IO_TIMER_GETTICK:
        return GetTimerTick(timer);
       // break;
    case IO_TIMER_SETTICK:
        return SetTimerTick(timer,tick);
       // break;
    case IO_TIMER_SETCLKSRC:
        return SetTimerClkSource(timer,tick);
       // break;
    default:
        return FALSE;
    }

    return TRUE;
}

/* Routine to start the specified timer & enable the interrupt */
//BOOL fLib_Timer_Init(DRVTIMER timer,UINT32 tick)
BOOL fLib_Timer_Init(DRVTIMER timer, UINT32 tick)

{
	  if(pwmTmrBaseIndex== -1) pwmTmrBaseIndex =0;
	
	  fLib_Timer_Change_tmr_base(pwmTmrBaseIndex);

    fLib_TimerStruct *ctimer=&ftimer[timer];
		UINT32 timer_irq;

    if (timer == 0 || timer > MAX_TIMER)
        return FALSE;

		switch(timer)
		{
			case DRVPWMTMR1:
				T1_Tick = 0;
				timer_irq = PWM_FTPWMTMR010_1_IRQ + pwmTmrBaseIndex*5;
			break;
			case DRVPWMTMR2:
				T2_Tick = 0;
				timer_irq = PWM_FTPWMTMR010_2_IRQ + pwmTmrBaseIndex*5;
			break;
			case DRVPWMTMR3:
				T3_Tick = 0;
				timer_irq = PWM_FTPWMTMR010_3_IRQ + pwmTmrBaseIndex*5;
			break;
			case DRVPWMTMR4:
				T4_Tick = 0;
				timer_irq = PWM_FTPWMTMR010_4_IRQ + pwmTmrBaseIndex*5;
			break;
			default:
				return FALSE;
		}

    TimerControl[timer]=(fLib_TimerControl *)(TimerBase[timer]);
		//CNTB, CNPB
		PWMCNTB[timer] = (fLib_CNTB *) (CNTBBase[timer]);
		PWMCNPB[timer] = (fLib_CNPB *) (CNPBBase[timer]);
		//
	 fLib_Timer_Close(timer);

    /* Set the timer tick */
    if(!fLib_Timer_IOCtrl(IO_TIMER_SETTICK,timer,tick))
        return FALSE;

    fLib_Timer_AutoReloadValue(timer,ctimer->Tick);
    /*enable auto and int bit */
    fLib_Timer_AutoReloadEnable(timer);

    if (!fLib_Timer_IntEnable(timer))
        return FALSE;

		NVIC_EnableIRQ((IRQn_Type)timer_irq);
		fLib_printf("Enable NVIC\n");

    /* Start the timer ticking */
    if(!fLib_Timer_Enable(timer))
        return FALSE;

    return TRUE;
}

INT32 fLib_Timer_AutoReloadEnable(DRVTIMER timer)
{
    //fLib_TimerStruct *ctimer=&ftimer[timer];

    if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;

    TimerControl[timer]->TmAutoLoad=1;
    return TRUE;
}


INT32 fLib_Timer_Counter(DRVTIMER timer)
{
    return inw(TimerBase[timer] + TIMER_CNTO);
}


INT32 fLib_Timer_IntEnable(DRVTIMER timer)
{
    if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;
	  //TimerControl[timer]->TmIntMode = 1; //kay 20191029 level -> pulse
    TimerControl[timer]->TmIntEn=1;
    return TRUE;
}

INT32 fLib_Timer_IntDisable(UINT32 timer)
{
    //fLib_TimerStruct *ctimer=&ftimer[timer];

    if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;


      TimerControl[timer]->TmIntEn=0;


    return TRUE;
}


INT32 fLib_Timer_IntModeEnable(UINT32 timer,UINT32 mode)
{
    //fLib_TimerStruct *ctimer=&ftimer[timer];

    if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;


      TimerControl[timer]->TmIntMode=mode;


    return TRUE;
}

INT32 fLib_Timer_DmaEnable(UINT32 timer)
{

    //fLib_TimerStruct *ctimer=&ftimer[timer];

    if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;


      TimerControl[timer]->TmDmaEn=1;


    return TRUE;
}


INT32 fLib_Timer_DeadZoneEnable(UINT32 timer,UINT32 offset)
{
    //fLib_TimerStruct *ctimer=&ftimer[timer];

    if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;


      TimerControl[timer]->TmDeadZone=offset;


    return TRUE;
}

/* This routine starts the specified timer hardware. */
INT32 fLib_Timer_Enable(DRVTIMER timer)
{
    fLib_TimerStruct *ctimer=&ftimer[timer];

    if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;

    if(ctimer->Running==TRUE)
        return FALSE;
    TimerControl[timer]->TmUpdate=1;
	  TimerControl[timer]->TmStart=1;
    //set the timer status =true
    ctimer->Running=TRUE;
    return TRUE;
}


/* This routine stops the specified timer hardware. */
INT32 fLib_Timer_Disable(DRVTIMER timer)
{
    fLib_TimerStruct *ctimer=&ftimer[timer];

    if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;

    /* Disable the Control register bit */
    TimerControl[timer]->TmStart=0;
    TimerControl[timer]->TmUpdate=0;
    TimerControl[timer]->TmOutInv=0;
    TimerControl[timer]->TmDmaEn=0;
    TimerControl[timer]->TmIntEn=0;
    TimerControl[timer]->TmDeadZone=0;
    TimerControl[timer]->TmSrc=0;
    TimerControl[timer]->TmAutoLoad=0;

     //set the timer status=false
    ctimer->Running=FALSE;

    return TRUE;
}

/* This routine starts the specified timer hardware. */
INT32 fLib_Timer_IntClear(int pwm, DRVTIMER timer)
{
    int value;
	  int base;
	
	switch(pwm){
		case 0:
			base = PWM_FTPWMTMR010_PA_BASE;
		  break;
		case 1:
			base = PWM_FTPWMTMR010_1_PA_BASE;
		  break;
		case 2:
			base = PWM_FTPWMTMR010_2_PA_BASE;
		  break;
		case 3:
			base = PWM_FTPWMTMR010_3_PA_BASE;
		  break;

		
	}

   if ((timer == 0) || (timer > MAX_TIMER))
       return FALSE;

    value=1<<(timer-1);
    outw(base + TIMER_INTSTAT, value);

    return TRUE;
}

void fLib_Timer_CmpValue(DRVTIMER timer, UINT32 value)
{
    outw(TimerBase[timer] + TIMER_COMPARE, value);
}

INT32 SetTimerClkSource(DRVTIMER timer,UINT32 clk)
{
	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	TimerControl[timer]->TmSrc=clk;

   	return TRUE;
}

#if 0
/* Routine to initialise install requested timer. Stops the timer. */
INT32 Timer_ConnectIsr(UINT32 timer,PrHandler handler)
{
    fLib_TimerStruct *ctimer=&ftimer[timer];
    UINT32 i;

    i = fLib_Timer_Vectors[timer];

	if (request_irq(i, handler, IRQF_SHARED | IRQF_DISABLED, "timer", 0) < 0)
	{
		return FALSE;
	}

    ctimer->Handler = handler;
    ctimer->IntNum = i;     /* INT number */

    return timer;
}
#endif

UINT32 fLib_CurrentT1Tick(void)
{
    return T1_Tick;
}

UINT32 fLib_CurrentT2Tick(void)
{
    return T2_Tick;
}


UINT32 fLib_CurrentT3Tick(void)
{
    return T3_Tick;
}

UINT32 fLib_CurrentT4Tick(void)
{
    return T4_Tick;
}

/////////////////////////////////////////////////////
//
//  Only for detail function call subroutine
//
/////////////////////////////////////////////////////


/* Start-up routine to initialise the timers to a known state */
void Timer_ResetAll(void)
{
    UINT32 i;

    //reset all timer to default value
    for (i = 1; i <= MAX_TIMER; i++)
        fLib_Timer_Disable((DRVTIMER)i);

}

INT32 GetTimerTick(DRVTIMER timer)
{
    UINT32 cur_tick;

    volatile fLib_TimerStruct *ctimer = &ftimer[timer];

    if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;

    cur_tick=ctimer->Tick;

    return cur_tick;
}

INT32 SetTimerTick(UINT32 timer,UINT32 clk_tick)
{
    volatile fLib_TimerStruct *ctimer = &ftimer[timer];

    if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;

    ctimer->Tick=clk_tick;

    return TRUE;
}

void fLib_Timer_AutoReloadValue(DRVTIMER timer, UINT32 value)
{
    outw(TimerBase[timer] + TIMER_LOAD, value);
}

// End of file - timer.c

void ftpwmtmr010_write_cnpb(int hw_index, DRVTIMER num, int val)
{
	int hw_addr;
	
	switch(hw_index)
	{
		case 0:
			hw_addr = PWM_FTPWMTMR010_PA_BASE;
			break;
		case 1:
			hw_addr = PWM_FTPWMTMR010_1_PA_BASE;
			break;
		case 2:
			hw_addr = PWM_FTPWMTMR010_2_PA_BASE;
			break;
		case 3:
			hw_addr = PWM_FTPWMTMR010_3_PA_BASE;
			break;
	}
	writel(val , hw_addr + num*0x10 + 0x08);
}

void ftpwmtmr010_write_cntb(int hw_index, DRVTIMER num, int val)
{
	int hw_addr;
	
	switch(hw_index)
	{
		case 0:
			hw_addr = PWM_FTPWMTMR010_PA_BASE;
			break;
		case 1:
			hw_addr = PWM_FTPWMTMR010_1_PA_BASE;
			break;
		case 2:
			hw_addr = PWM_FTPWMTMR010_2_PA_BASE;
			break;
		case 3:
			hw_addr = PWM_FTPWMTMR010_3_PA_BASE;
			break;
	}
	writel(val , hw_addr + num*0x10 + 0x04);
}

// Start of PWM function
INT32 ftpwmtmr010_pwm_config(DRVTIMER timer, int duty_ns, int period_ns)
{
	//struct ftpwmtmr010_pwm_chip *ftc_pwm_chip = to_ftpwmtmr010_pwm_chip(chip);
	//struct ftpwmtmr010_regs *regs = ftc_pwm_chip->mmio;
	
	unsigned int freq;
	unsigned int ratio_period = 0;
	unsigned int ratio_duty = 0;	
	unsigned int tmp;
			
	if(period_ns)                /*	ratio_period = 1000000000 / period_ns;*/
	{
		tmp = APB_CLK;
		//do_div(tmp , period_ns);
		tmp = tmp/period_ns;
		ratio_period = tmp;
	}
	
	if(duty_ns)                /*	ratio_duty = 1000000000 / duty_ns; */
	{	
		tmp = APB_CLK;
		//do_div(tmp , duty_ns);
		tmp = tmp/duty_ns;
		ratio_duty = tmp;
	}
	
	if(ratio_period)
	{	
		//freq = ftc_pwm_chip->hz /ratio_period;
		//ftc_pwm_chip->hz => source clock
		freq = APB_CLK / ratio_period;
		//writel(freq, &regs->timer[ftc_pwm_chip->id].cntb);
		//PWMCNTB[timer]->Counter = freq;
		ftpwmtmr010_write_cntb(pwmTmrBaseIndex, timer, freq);		
		//PWMCNTB[timer]->Counter = freq;
	}
	
	if(ratio_duty)
	{	
		//freq = ftc_pwm_chip->hz/ratio_duty;
		freq = APB_CLK / ratio_duty;
		//writel(freq, &regs->timer[ftc_pwm_chip->id].cmpb);	
		//PWMCNPB[timer]->CompareBuffer = freq;
		ftpwmtmr010_write_cnpb(pwmTmrBaseIndex, timer, freq);
	}

	return TRUE;
}

INT32 ftpwmtmr010_pwm_set_polarity(DRVTIMER timer, UINT8 polarity)
{
	fLib_printf("Polarity change\n");
	//struct ftpwmtmr010_pwm_chip *ftc_pwm_chip = chip;
	//struct ftpwmtmr010_regs *regs = (void*)PWM_FTPWMTMR010_PA_BASE;
	//unsigned int val;
		
	//val &= ~CTRL_OUT_INVT;	
	//writel(val, &regs->timer[ftc_pwm_chip->id].ctrl);
	TimerControl[timer]->TmOutInv=polarity;
	return TRUE;
}

INT32 ftpwmtmr010_pwm_enable(DRVTIMER timer)
{
	//struct ftpwmtmr010_pwm_chip *ftc_pwm_chip = to_ftpwmtmr010_pwm_chip(chip);
	//struct ftpwmtmr010_regs *regs = ftc_pwm_chip->mmio;
	//unsigned int val;

	fLib_printf("ftpwmtmr010_pwm_enable , timer %d TimerControl[1] = %x\n",timer, &TimerControl[timer]);
	TimerControl[timer]->TmAutoLoad=1;
	TimerControl[timer]->TmUpdate=1;
	TimerControl[timer]->TmStart=1;
	TimerControl[timer]->TmPwmEn=1;
	//writel(CTRL_AUTORELOAD | CTRL_UPDATE | CTRL_START |CTRL_PWMEN,&regs->timer[ftc_pwm_chip->id].ctrl);
	return TRUE;
}

INT32 ftpwmtmr010_pwm_disable(DRVTIMER timer)
{
	//struct ftpwmtmr010_pwm_chip *ftc_pwm_chip = to_ftpwmtmr010_pwm_chip(chip);
	//struct ftpwmtmr010_regs *regs = ftc_pwm_chip->mmio;
	//unsigned int val;

	fLib_printf("ftpwmtmr_pwm_disable(CTRL) : 0x%x\n",TimerControl[timer]);
	//val &= ~CTRL_PWMEN;
	//writel(val, &regs->timer[ftc_pwm_chip->id].ctrl);
	TimerControl[timer]->TmPwmEn=0;
	return TRUE;
}

// End of PWM function
