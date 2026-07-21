/**************************************************************************//**
 * @file     core_cm33.h
 * @brief    Minimal CMSIS Cortex-M33 Core Peripheral Access Layer Header
 *           Sufficient for this project's NVIC / SCB / FPU usage.
 ******************************************************************************/
#ifndef __CORE_CM33_H_GENERIC
#define __CORE_CM33_H_GENERIC

#include <stdint.h>
#include "cmsis_version.h"

#ifdef __cplusplus
 extern "C" {
#endif

#define __CM33_CMSIS_VERSION_MAIN  (__CM_CMSIS_VERSION_MAIN)
#define __CM33_CMSIS_VERSION_SUB   (__CM_CMSIS_VERSION_SUB)
#define __CM33_CMSIS_VERSION       (__CM_CMSIS_VERSION)
#define __CORTEX_M                 (33U)

#if defined (__FPU_PRESENT) && (__FPU_PRESENT == 1U)
  #define __FPU_USED       1U
#else
  #define __FPU_USED       0U
#endif

#include "cmsis_compiler.h"

#ifdef __cplusplus
}
#endif

#endif /* __CORE_CM33_H_GENERIC */

#ifndef __CMSIS_GENERIC

#ifndef __CORE_CM33_H_DEPENDANT
#define __CORE_CM33_H_DEPENDANT

#ifdef __cplusplus
 extern "C" {
#endif

#if defined (__NVIC_PRIO_BITS)
#else
  #define __NVIC_PRIO_BITS          3U
  #warning "__NVIC_PRIO_BITS not defined, using default!"
#endif

#if defined (__Vendor_SysTickConfig)
#else
  #define __Vendor_SysTickConfig    0U
#endif

/* IO definitions */
#ifdef __cplusplus
  #define   __I     volatile
#else
  #define   __I     volatile const
#endif
#define     __O     volatile
#define     __IO    volatile
#define     __IM    volatile const
#define     __OM    volatile
#define     __IOM   volatile

/* NVIC */
typedef struct {
  __IOM uint32_t ISER[16U];
        uint32_t RESERVED0[16U];
  __IOM uint32_t ICER[16U];
        uint32_t RESERVED1[16U];
  __IOM uint32_t ISPR[16U];
        uint32_t RESERVED2[16U];
  __IOM uint32_t ICPR[16U];
        uint32_t RESERVED3[16U];
  __IOM uint32_t IABR[16U];
        uint32_t RESERVED4[16U];
  __IOM uint32_t ITNS[16U];
        uint32_t RESERVED5[16U];
  __IOM uint8_t  IPR[496U];
        uint32_t RESERVED6[580U];
  __OM  uint32_t STIR;
} NVIC_Type;

/* SCB */
typedef struct {
  __IM  uint32_t CPUID;
  __IOM uint32_t ICSR;
  __IOM uint32_t VTOR;
  __IOM uint32_t AIRCR;
  __IOM uint32_t SCR;
  __IOM uint32_t CCR;
  __IOM uint8_t  SHPR[12U];
  __IOM uint32_t SHCSR;
  __IOM uint32_t CFSR;
  __IOM uint32_t HFSR;
  __IOM uint32_t DFSR;
  __IOM uint32_t MMFAR;
  __IOM uint32_t BFAR;
  __IOM uint32_t AFSR;
  __IM  uint32_t ID_PFR[2U];
  __IM  uint32_t ID_DFR;
  __IM  uint32_t ID_AFR;
  __IM  uint32_t ID_MMFR[4U];
  __IM  uint32_t ID_ISAR[6U];
        uint32_t RESERVED0[1U];
  __IM  uint32_t CLIDR;
  __IM  uint32_t CTR;
  __IM  uint32_t CCSIDR;
  __IOM uint32_t CSSELR;
  __IOM uint32_t CPACR;
  __IOM uint32_t NSACR;
} SCB_Type;

#define SCB_AIRCR_VECTKEY_Pos              16U
#define SCB_AIRCR_VECTKEY_Msk              (0xFFFFUL << SCB_AIRCR_VECTKEY_Pos)
#define SCB_AIRCR_SYSRESETREQ_Pos           2U
#define SCB_AIRCR_SYSRESETREQ_Msk          (1UL << SCB_AIRCR_SYSRESETREQ_Pos)
#define SCB_AIRCR_PRIGROUP_Pos              8U
#define SCB_AIRCR_PRIGROUP_Msk             (7UL << SCB_AIRCR_PRIGROUP_Pos)

#define SCB_CCR_UNALIGN_TRP_Pos             3U
#define SCB_CCR_UNALIGN_TRP_Msk            (1UL << SCB_CCR_UNALIGN_TRP_Pos)

/* SysTick */
typedef struct {
  __IOM uint32_t CTRL;
  __IOM uint32_t LOAD;
  __IOM uint32_t VAL;
  __IM  uint32_t CALIB;
} SysTick_Type;

#define SysTick_CTRL_COUNTFLAG_Pos         16U
#define SysTick_CTRL_COUNTFLAG_Msk         (1UL << SysTick_CTRL_COUNTFLAG_Pos)
#define SysTick_CTRL_CLKSOURCE_Pos          2U
#define SysTick_CTRL_CLKSOURCE_Msk         (1UL << SysTick_CTRL_CLKSOURCE_Pos)
#define SysTick_CTRL_TICKINT_Pos            1U
#define SysTick_CTRL_TICKINT_Msk           (1UL << SysTick_CTRL_TICKINT_Pos)
#define SysTick_CTRL_ENABLE_Pos             0U
#define SysTick_CTRL_ENABLE_Msk            (1UL /*<< SysTick_CTRL_ENABLE_Pos*/)

/* Memory map */
#define SCS_BASE            (0xE000E000UL)
#define SysTick_BASE        (SCS_BASE + 0x0010UL)
#define NVIC_BASE           (SCS_BASE + 0x0100UL)
#define SCB_BASE            (SCS_BASE + 0x0D00UL)

#define SCB                 ((SCB_Type *)SCB_BASE)
#define SysTick             ((SysTick_Type *)SysTick_BASE)
#define NVIC                ((NVIC_Type *)NVIC_BASE)

#define _BIT_SHIFT(IRQn)    (((((uint32_t)(int32_t)(IRQn))) & 0x03UL) * 8UL)
#define _IP_IDX(IRQn)       ((((uint32_t)(int32_t)(IRQn)) >> 2UL))
#define _SHP_IDX(IRQn)      ((((((uint32_t)(int32_t)(IRQn)) & 0x0FUL) - 8UL) >> 2UL))

__STATIC_INLINE void __NVIC_EnableIRQ(IRQn_Type IRQn)
{
  if ((int32_t)(IRQn) >= 0) {
    __COMPILER_BARRIER();
    NVIC->ISER[(((uint32_t)IRQn) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)IRQn) & 0x1FUL));
    __COMPILER_BARRIER();
  }
}

__STATIC_INLINE void __NVIC_DisableIRQ(IRQn_Type IRQn)
{
  if ((int32_t)(IRQn) >= 0) {
    NVIC->ICER[(((uint32_t)IRQn) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)IRQn) & 0x1FUL));
    __DSB();
    __ISB();
  }
}

__STATIC_INLINE void __NVIC_SetPriority(IRQn_Type IRQn, uint32_t priority)
{
  if ((int32_t)(IRQn) >= 0) {
    NVIC->IPR[_IP_IDX(IRQn)] = ((uint32_t)(NVIC->IPR[_IP_IDX(IRQn)] & ~(0xFFUL << _BIT_SHIFT(IRQn))) |
                                (((priority << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL) << _BIT_SHIFT(IRQn)));
  } else {
    SCB->SHPR[_SHP_IDX(IRQn)] = ((uint32_t)(SCB->SHPR[_SHP_IDX(IRQn)] & ~(0xFFUL << _BIT_SHIFT(IRQn))) |
                                 (((priority << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL) << _BIT_SHIFT(IRQn)));
  }
}

__STATIC_INLINE uint32_t __NVIC_GetPriority(IRQn_Type IRQn)
{
  if ((int32_t)(IRQn) >= 0) {
    return ((uint32_t)(((NVIC->IPR[_IP_IDX(IRQn)] >> _BIT_SHIFT(IRQn)) & (uint32_t)0xFFUL) >> (8U - __NVIC_PRIO_BITS)));
  }
  return ((uint32_t)(((SCB->SHPR[_SHP_IDX(IRQn)] >> _BIT_SHIFT(IRQn)) & (uint32_t)0xFFUL) >> (8U - __NVIC_PRIO_BITS)));
}

#define NVIC_EnableIRQ      __NVIC_EnableIRQ
#define NVIC_DisableIRQ     __NVIC_DisableIRQ
#define NVIC_SetPriority    __NVIC_SetPriority
#define NVIC_GetPriority    __NVIC_GetPriority

__NO_RETURN __STATIC_INLINE void __NVIC_SystemReset(void)
{
  __DSB();
  SCB->AIRCR = (uint32_t)((0x5FAUL << SCB_AIRCR_VECTKEY_Pos) |
                          (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) |
                          SCB_AIRCR_SYSRESETREQ_Msk);
  __DSB();
  for (;;) {
    __ASM volatile ("nop");
  }
}

#define NVIC_SystemReset    __NVIC_SystemReset

#if defined (__FPU_USED) && (__FPU_USED == 1U)
/* CPACR is accessed via SCB->CPACR */
#endif

#ifdef __cplusplus
}
#endif

#endif /* __CORE_CM33_H_DEPENDANT */

#endif /* __CMSIS_GENERIC */
