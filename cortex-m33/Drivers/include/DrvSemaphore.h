#ifndef __SEMAPHORE_H
#define __SEMAPHORE_H

//#include "flib.h"
#include "types.h"
#include "leo_cm33.h"

#define SEMAPHORE_SLAVE_UNAVAILABLE 0x1
#define SEMAPHORE_SLAVE_AVAILABLE 0x2
#define SEMAPHORE_CLEAR_STATUS 0x1
#define MAX_SEPHAMORE_REG 64
#define SEMAPHORE_CTRL 0x120

extern void semaphore_init(UINT32 io_base);
extern UINT32 semaphore_slave_is_available(UINT32 io_base, UINT32 num);
extern UINT32 semaphore_write_reg(UINT32 io_base , UINT32 num);
extern UINT32 semaphore_slave_set_occupy(UINT32 io_base, UINT32 num);

#endif //__SEMAPHORE_H