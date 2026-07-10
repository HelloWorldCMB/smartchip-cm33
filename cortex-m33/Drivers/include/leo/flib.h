#ifndef FLIB_H
#define FLIB_H

//#include "platform.h"
#include "FIE3100.h"
//#include "plat-FIE3100.h"
#include "spec.h"

#include "types.h"
#include "io.h"
#include "fdebug.h"
#include "pin_mux.h"
//#include "spec_macro.h"
#if defined(__MCU_CM0__) || defined(__MCU_CM3__)
//#include "nvic.h"
#elif defined(__MCU_FA606TE__)
#include "interrupt.h"
#else
#error "unknown platform"
#endif
#endif //FLIB_H
