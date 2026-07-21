/**************************************************************************//**
 * @file     cmsis_compiler.h
 * @brief    CMSIS compiler generic header file (minimal for GCC build)
 ******************************************************************************/
#ifndef __CMSIS_COMPILER_H
#define __CMSIS_COMPILER_H

#include <stdint.h>

#if defined ( __GNUC__ )
  #include "cmsis_gcc.h"
#else
  #error Unknown compiler.
#endif

#endif
