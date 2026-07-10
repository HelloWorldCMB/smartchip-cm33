#ifndef __CAN_TYPE_H
#define __CAN_TYPE_H

#define __IO volatile

//typedef unsigned int UINT32;
//typedef unsigned char UINT8;
//typedef unsigned short UINT16;
//typedef unsigned int uintptr_t;
//typedef unsigned char BOOL;

#ifndef ENABLE
#define ENABLE	1
#endif

#ifndef DISABLE
#define	DISABLE	0
#endif


typedef enum {RESET = 0, SET = !RESET} FlagStatus, ITStatus;

#endif