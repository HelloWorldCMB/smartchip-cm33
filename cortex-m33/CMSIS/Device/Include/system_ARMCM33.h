/**************************************************************************//**
 * @file     system_ARMCM33.h
 ******************************************************************************/
#ifndef SYSTEM_ARMCM33_H
#define SYSTEM_ARMCM33_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

extern uint32_t SystemCoreClock;

extern void SystemInit(void);
extern void SystemCoreClockUpdate(void);

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_ARMCM33_H */
