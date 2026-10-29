/* --------------------------------------------------------------------------
 * Yaoxin firmware entry (GCC / Keil)
 *---------------------------------------------------------------------------*/

#include <stdio.h>

#include "spec.h"
#include "DrvUART010.h"
#include "ipc.h"
#include "console_cli.h"
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

	yaoxin_init();	/* 内部会 register 遥信命令并 console_cli_init */
	console_cli_init("cm33> ");
	while (1) {
		yaoxin_queue_poll();
		console_cli_poll();
		keep_alive_debug_poll();
		__WFI();
	}
}
