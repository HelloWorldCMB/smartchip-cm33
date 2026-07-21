/* --------------------------------------------------------------------------
 * Yaoxin firmware entry (GCC / Keil)
 *---------------------------------------------------------------------------*/

#include <stdio.h>

#include "spec.h"
#include "DrvUART010.h"
#include "ipc.h"
#include "DrvSemaphore.h"
#include "leo_cm33.h"

int main(void)
{
	SystemCoreClockUpdate();

	/*
	 * Init semaphore and enable semaphore interrupt to handle A7 core ioctl cmd
	 */
	semaphore_init(SEMAPHORE_PA_BASE);
	NVIC_EnableIRQ(SEMAPHORE_FTSEMAPHRE_IRQ);

	yaoxin_init();

	while (1) {
		yaoxin_debug_poll();
		__WFI();
	}
}
