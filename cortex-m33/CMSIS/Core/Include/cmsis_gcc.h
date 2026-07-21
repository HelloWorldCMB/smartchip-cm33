/**************************************************************************//**
 * @file     cmsis_gcc.h
 * @brief    CMSIS compiler GCC header file (minimal subset for this project)
 ******************************************************************************/
#ifndef __CMSIS_GCC_H
#define __CMSIS_GCC_H

#ifndef   __ASM
  #define __ASM                                  __asm
#endif
#ifndef   __INLINE
  #define __INLINE                               inline
#endif
#ifndef   __STATIC_INLINE
  #define __STATIC_INLINE                        static inline
#endif
#ifndef   __STATIC_FORCEINLINE
  #define __STATIC_FORCEINLINE                   __attribute__((always_inline)) static inline
#endif
#ifndef   __NO_RETURN
  #define __NO_RETURN                            __attribute__((__noreturn__))
#endif
#ifndef   __USED
  #define __USED                                 __attribute__((used))
#endif
#ifndef   __WEAK
  #define __WEAK                                 __attribute__((weak))
#endif
#ifndef   __PACKED
  #define __PACKED                               __attribute__((packed, aligned(1)))
#endif
#ifndef   __PACKED_STRUCT
  #define __PACKED_STRUCT                        struct __attribute__((packed, aligned(1)))
#endif
#ifndef   __ALIGNED
  #define __ALIGNED(x)                           __attribute__((aligned(x)))
#endif
#ifndef   __RESTRICT
  #define __RESTRICT                             __restrict
#endif

#define __COMPILER_BARRIER()                     __ASM volatile ("":::"memory")

__STATIC_FORCEINLINE void __disable_irq(void)
{
  __ASM volatile ("cpsid i" : : : "memory");
}

__STATIC_FORCEINLINE void __enable_irq(void)
{
  __ASM volatile ("cpsie i" : : : "memory");
}

__STATIC_FORCEINLINE void __WFI(void)
{
  __ASM volatile ("wfi":::);
}

__STATIC_FORCEINLINE void __WFE(void)
{
  __ASM volatile ("wfe":::);
}

__STATIC_FORCEINLINE void __DSB(void)
{
  __ASM volatile ("dsb 0xF":::"memory");
}

__STATIC_FORCEINLINE void __ISB(void)
{
  __ASM volatile ("isb 0xF":::"memory");
}

__STATIC_FORCEINLINE void __DMB(void)
{
  __ASM volatile ("dmb 0xF":::"memory");
}

__STATIC_FORCEINLINE uint32_t __get_MSP(void)
{
  uint32_t result;
  __ASM volatile ("MRS %0, msp" : "=r" (result));
  return result;
}

__STATIC_FORCEINLINE void __set_MSP(uint32_t topOfMainStack)
{
  __ASM volatile ("MSR msp, %0" : : "r" (topOfMainStack) : );
}

__STATIC_FORCEINLINE uint32_t __get_PSP(void)
{
  uint32_t result;
  __ASM volatile ("MRS %0, psp" : "=r" (result));
  return result;
}

__STATIC_FORCEINLINE void __set_PSP(uint32_t topOfProcStack)
{
  __ASM volatile ("MSR psp, %0" : : "r" (topOfProcStack) : );
}

__STATIC_FORCEINLINE uint32_t __get_PRIMASK(void)
{
  uint32_t result;
  __ASM volatile ("MRS %0, primask" : "=r" (result));
  return result;
}

__STATIC_FORCEINLINE void __set_PRIMASK(uint32_t priMask)
{
  __ASM volatile ("MSR primask, %0" : : "r" (priMask) : "memory");
}

__STATIC_FORCEINLINE uint32_t __get_BASEPRI(void)
{
  uint32_t result;
  __ASM volatile ("MRS %0, basepri" : "=r" (result));
  return result;
}

__STATIC_FORCEINLINE void __set_BASEPRI(uint32_t basePri)
{
  __ASM volatile ("MSR basepri, %0" : : "r" (basePri) : "memory");
}

__STATIC_FORCEINLINE uint32_t __get_CONTROL(void)
{
  uint32_t result;
  __ASM volatile ("MRS %0, control" : "=r" (result));
  return result;
}

__STATIC_FORCEINLINE void __set_CONTROL(uint32_t control)
{
  __ASM volatile ("MSR control, %0" : : "r" (control) : "memory");
  __ISB();
}

#endif /* __CMSIS_GCC_H */
