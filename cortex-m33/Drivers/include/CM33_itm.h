/* ------------------------------------------------------------
 CM3_itm.h
 Header File

 Copyright (c) 2011 ARM Ltd.  All rights reserved.
------------------------------------------------------------
*/

#ifndef CM3_ITM_H
#define CM3_ITM_H

/*-----------------------------------------------------------------------*/
/*                           R E G I S T E R S                           */
/*-----------------------------------------------------------------------*/

/* Where in our address map the ITM register set is located */
#define ITM_BASE_ADDR            0xE0000000

/* Stimulus Ports 0:31 */
#define ITM_STIMULUS_PORTS        ((volatile unsigned int*)(ITM_BASE_ADDR+0x000))
/* Trace Enable */
#define ITM_TRACE_ENABLE        (*(volatile unsigned int*)(ITM_BASE_ADDR+0xE00))
/* Trace Control */
#define ITM_TRACE_CONTROL        (*(volatile unsigned int*)(ITM_BASE_ADDR+0xE80))
/* Integration Write */
#define ITM_INTEGRATION_WRITE    (*(volatile unsigned int*)(ITM_BASE_ADDR+0xEF8))
/* Integration Read */
#define ITM_INTEGRATION_READ    (*(volatile unsigned int*)(ITM_BASE_ADDR+0xEFC))
/* Integration Mode Control */
#define ITM_INTEGRATION_MODE    (*(volatile unsigned int*)(ITM_BASE_ADDR+0xF00))
/* Lock Access */
#define ITM_LOCK_ACCESS            (*(volatile unsigned int*)(ITM_BASE_ADDR+0xFB0))
/* Lock Status */
#define ITM_LOCK_STATUS            (*(volatile unsigned int*)(ITM_BASE_ADDR+0xFB4))

#endif
