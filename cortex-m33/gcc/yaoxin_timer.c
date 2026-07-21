/**
 * Minimal timer IRQ / delay helpers for GCC yaoxin build.
 * Timer is configured as PWMTMR_1MSEC_PERIOD, so each IRQ == 1 ms.
 */
#include "DrvPWMTMR010.h"
#include "leo_cm33.h"

extern volatile UINT32 T4_Tick;

void PWMTMR4_IRQHandler(void)
{
	fLib_Timer_IntClear(0, DRVPWMTMR4);
	T4_Tick++;	/* 1 ms */
}

void WaitForTick(void)
{
	UINT32 curTicks = T4_Tick;
	while (T4_Tick == curTicks) {
		__WFI();
	}
}

void delay_ms(unsigned int count)
{
	unsigned int i;
	for (i = count; i != 0; --i)
		WaitForTick();
}
