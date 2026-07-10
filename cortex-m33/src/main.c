/* --------------------------------------------------------------------------

 * Copyright (c) 2013-2017 ARM Limited. All rights reserved.

 *

 * SPDX-License-Identifier: Apache-2.0

 *

 * Licensed under the Apache License, Version 2.0 (the License); you may

 * not use this file except in compliance with the License.

 * You may obtain a copy of the License at

 *

 * www.apache.org/licenses/LICENSE-2.0

 *

 * Unless required by applicable law or agreed to in writing, software

 * distributed under the License is distributed on an AS IS BASIS, WITHOUT

 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

 * See the License for the specific language governing permissions and

 * limitations under the License.

 *

 *      Name:    Blinky.c

 *      Purpose: RTOS2 example program

 *

 *---------------------------------------------------------------------------*/



#include <stdio.h>

#include "RTE_Components.h"
#include "cmsis_os2.h"
#include "spec.h"
#include "DrvUART010.h"
#include "ipc.h"
#include "DrvSemaphore.h"

int main(void)

{
	SystemCoreClockUpdate();

	semaphore_init(SEMAPHORE_PA_BASE);

	NVIC_EnableIRQ(SEMAPHORE_FTSEMAPHRE_IRQ);

	fLib_printf("yaoxin firmware %s\n", __TIME__);

	yaoxin_init();

	while (1) {

		__WFI();

	}

}
