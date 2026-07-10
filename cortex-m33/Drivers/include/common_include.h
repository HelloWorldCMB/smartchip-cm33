#ifndef _COMMON_INCLUDE_H
#define _COMMON_INCLUDE_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "leo_cm33.h"
//#include <system_kneron.h>
#include "types.h"
//#include "DrvUART010.h"
#include "io.h"
#include "Driver_USART.h"
#include "utility.h"
#include "DrvPWMTMR010.h"
//#include "ind_startup.h"
#define DISP_LINE_LEN 16
extern int print_buffer (UINT32 addr, void* data, UINT32 width, UINT32 count, UINT32 linelen);
extern void md(UINT32 addr, UINT32 length);
extern void delay_ms(unsigned int count);
#endif