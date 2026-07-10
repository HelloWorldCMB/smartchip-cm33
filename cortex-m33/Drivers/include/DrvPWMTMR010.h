#ifndef __TIMER_H
#define __TIMER_H

//#include "flib.h"
#include "types.h"
#include "leo_cm33.h"

#define HZ			100				/// how many tick each sec

#define MAX_TIMER					4

#define TIMER_INTSTAT              	0x0
#define TIMER_CR                	0x0
#define TIMER_LOAD					0x4
#define TIMER_COMPARE				0x8
#define TIMER_CNTO					0xc

typedef enum {
    DRVPWMTMR1=1,
    DRVPWMTMR2=2,
    DRVPWMTMR3=3, 
    DRVPWMTMR4=4,
    DRVPWMTMR5=5,
    DRVPWMTMR6=6,
    DRVPWMTMR7=7, 
    DRVPWMTMR8=8
} DRVTIMER;


typedef struct
{	
	UINT32 TmSrc:1;	/* bit 0 */
	UINT32 TmStart:1;
	UINT32 TmUpdate:1;
	UINT32 TmOutInv:1;
	UINT32 TmAutoLoad:1;
	UINT32 TmIntEn:1;
	UINT32 TmIntMode:1;
	UINT32 TmDmaEn:1;	
	UINT32 TmPwmEn:1;	/* bit 8 */
	UINT32 Reserved:15;	/* bit 9~23 */
	UINT32 TmDeadZone:8;	/* bit 24~31 */
}fLib_TimerControl;

typedef struct
{	
			UINT32 Counter;
}fLib_CNTB;

typedef struct
{
	UINT32 CompareBuffer;
}fLib_CNPB; //CMPB

typedef struct
{
	UINT32 Tm1Match1:1;
	UINT32 Tm1Match2:1;
	UINT32 Tm1Overflow:1;
	UINT32 Tm2Match1:1;
	UINT32 Tm2Match2:1;
	UINT32 Tm2Overflow:1;
	UINT32 Tm3Match1:1;
	UINT32 Tm3Match2:1;
	UINT32 Tm3Overflow:1;
	UINT32 Reserved;	
}fLib_TimerMask;


typedef struct 
{
    UINT32 IntNum;           /* interrupt number */   
    UINT32 Tick;        /* Tick Per Second */   
    UINT32 Running;       /* Is timer running */       
}fLib_TimerStruct;


//#define APB_CLK APB_CLOCK
//--------------------------------------------------
// Timer tick
//--------------------------------------------------
#define PWMTMR_1000MSEC_PERIOD			(UINT32)(APB_CLK)
#define PWMTMR_100MSEC_PERIOD			(UINT32)(APB_CLK/10)
#define PWMTMR_20MSEC_PERIOD			(UINT32)(APB_CLK/50)
#define PWMTMR_15MSEC_PERIOD			(UINT32)(((APB_CLK/100)*3)/2)
#define PWMTMR_10MSEC_PERIOD			(UINT32)(APB_CLK/100)
#define PWMTMR_1MSEC_PERIOD				(UINT32)(APB_CLK/1000)

typedef enum Timer_IoType
{
	IO_TIMER_RESETALL,
	IO_TIMER_GETTICK,
	IO_TIMER_SETTICK,
	IO_TIMER_SETCLKSRC
}fLib_Timer_IoType;


typedef struct
{
	UINT32 hour;
	UINT32 minute;
	UINT32 second;
}fLib_Time;

/*PWM function*/


/**
 * enum pwm_polarity - polarity of a PWM signal
 * @PWM_POLARITY_NORMAL: a high signal for the duration of the duty-
 * cycle, followed by a low signal for the remainder of the pulse
 * period
 * @PWM_POLARITY_INVERSED: a low signal for the duration of the duty-
 * cycle, followed by a high signal for the remainder of the pulse
 * period
 */
enum pwm_polarity {
	PWM_POLARITY_NORMAL,
	PWM_POLARITY_INVERSED,
};

typedef struct  {
	//struct pwm_chip chip;
	uint32_t id; /* timer id: range from 0 to 7 */
	uint32_t hz; /* source clock */
	uint32_t ctrl; /* control mask */
	uint32_t reload; /* reload value for periodic timer */
	//struct clk	*clk;
	//void __iomem *mmio;	
}ftpwmtmr010_pwm_chip;


/*PWM function*/

/*  -------------------------------------------------------------------------------
 *   API
 *  -------------------------------------------------------------------------------
 */
 
//this routines will export to upper ap or test program
//extern BOOL fLib_Timer_Init(DRVTIMER timer, UINT32 tick);

extern BOOL fLib_Timer_Init(DRVTIMER timer, UINT32 tick);

extern INT32 fLib_Timer_IOCtrl(fLib_Timer_IoType IoType,DRVTIMER timer,UINT32 tick);
extern INT32 fLib_Timer_Counter(DRVTIMER timer);
extern void fLib_Timer_AutoReloadValue(DRVTIMER timer, UINT32 value);
extern void fLib_Timer_CmpValue(DRVTIMER timer, UINT32 value);
extern INT32 fLib_Timer_Enable(DRVTIMER timer);
extern INT32 fLib_Timer_Disable(DRVTIMER timer);
extern INT32 fLib_Timer_Close(DRVTIMER timer);
extern INT32 fLib_Timer_IntEnable(DRVTIMER timer);
extern INT32 fLib_Timer_IntClear(int pwm, DRVTIMER timer);
extern void Timer_ResetAll(void);
extern UINT32 fLib_CurrentT1Tick(void);
extern UINT32 fLib_CurrentT2Tick(void);
extern UINT32 fLib_CurrentT3Tick(void);
extern UINT32 fLib_CurrentT4Tick(void);

extern void PWMTMR1_IRQHandler(void);
extern void PWMTMR2_IRQHandler(void);
extern void PWMTMR3_IRQHandler(void);
extern void PWMTMR4_IRQHandler(void);
INT32 ftpwmtmr010_pwm_config(DRVTIMER timer, int duty_ns, int period_ns);
INT32 ftpwmtmr010_pwm_set_polarity(DRVTIMER timer, UINT8 polarity);
INT32 ftpwmtmr010_pwm_enable(DRVTIMER timer);
INT32 ftpwmtmr010_pwm_disable(DRVTIMER timer);


#endif //__TIMER_H
