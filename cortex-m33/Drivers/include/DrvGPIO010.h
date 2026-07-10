#ifndef __GPIO_H
#define __GPIO_H

#include "types.h"
 
#define GPIO_DOUT_OFFSET            0x0
#define GPIO_DIN_OFFSET             0x4
#define GPIO_PINOUT_OFFSET          0x8
#define GPIO_PIN_BYPASS             0xC
#define GPIO_DATASET                0x10
#define GPIO_DATACLR                0x14
#define GPIO_PULLENABLE             0x18
#define GPIO_PULLType               0x1C
#define GPIO_INT_ENABLE             0x20
#define GPIO_INT_RAWSTATE           0x24
#define GPIO_INT_MASKSTATE          0x28
#define GPIO_INT_MASK               0x2C
#define GPIO_INT_CLEAR              0x30
#define GPIO_INT_TRIGGER            0x34
#define GPIO_INT_BOTH               0x38
#define GPIO_INT_RISENEG            0x3C
#define GPIO_INT_BOUNCEENABLE       0x40
#define GPIO_INT_PRESCALE           0x44	


#define GPIO_NUM               		32
#define GPIO_EDGE             		0
#define GPIO_LEVEL               	1	
#define SINGLE                    	0
#define BOTH                      	1


#define GPIO_DIR_OUTPUT			1
#define GPIO_DIR_INPUT			0

#define GPIO_INPUT                  0
#define GPIO_OUTPUT                 1

#define LOW							0
#define HIGH						1

#define GPIO_0                      0
#define GPIO_1                      1
#define GPIO_2                      2
#define GPIO_3                      3
#define GPIO_4                      4
#define GPIO_5                      5
#define GPIO_6                      6
#define GPIO_7                      7
#define GPIO_8                      8
#define GPIO_9                      9
#define GPIO_10                     10
#define GPIO_11                     11
#define GPIO_12                     12
#define GPIO_13                     13
#define GPIO_14                     14
#define GPIO_15                     15
#define GPIO_16                     16 	
#define GPIO_17                     17	
#define GPIO_18                     18	
#define GPIO_19                     19	
#define GPIO_20                     20
#define GPIO_21                     21
#define GPIO_22                     22
#define GPIO_23                     23
#define GPIO_24                     24
#define GPIO_25                     25
#define GPIO_26                     26
#define GPIO_27                     27
#define GPIO_28                     28
#define GPIO_29                     29
#define GPIO_30                     30
#define GPIO_31                     31

#define BUTTON_0	GPIO_16
#define BUTTON_1	GPIO_17
#define BUTTON_2	GPIO_18
#define BUTTON_3	GPIO_19


#ifndef GPIO_TEST_H
#define GPIO_TEST_H

#define GPIO_KEYPAD_SWITCH	0x98100024
#define GPIO_INPUT		0
#define GPIO_OUTPUT		1
#define GPIO_Rising		0
#define GPIO_Falling		1
#define GPIO_High		0
#define GPIO_LOW		1
#define GPIO_LED_NUM		16
#define GPIO_LED_BASE		16


#endif


/*  -------------------------------------------------------------------------------
 *   API
 *  -------------------------------------------------------------------------------
 */

extern UINT32 fLib_Gpio_ReadData(UINT32 io_base);
extern void fLib_Gpio_WriteData(UINT32 io_base, UINT32 data);
extern void fLib_Gpio_SetData(UINT32 io_base, UINT32 data);
extern void fLib_Gpio_ClearData(UINT32 io_base, UINT32 data);

extern void  fLib_Gpio_PullEnable(UINT32 io_base, UINT32 pin);  
extern void  fLib_Gpio_PullDisable(UINT32 io_base, UINT32 pin);  
extern void  fLib_Gpio_PullHigh(UINT32 io_base, UINT32 pin);  
extern void  fLib_Gpio_PullLow(UINT32 io_base, UINT32 pin);
extern BOOL  fLib_Gpio_IsRawInt(UINT32 io_base, UINT32 pin);

extern void fLib_EnableGpioInt(UINT32 io_base, UINT32 pin, UINT32 trigger, UINT32 active);
extern void fLib_DisableGpioInt(UINT32 io_base, UINT32 pin);
extern void fLib_EnableGpioBounce(UINT32 io_base, UINT32 pin, UINT32 clkdiv);
extern void fLib_DisableGpioBounce(UINT32 io_base, UINT32 pin);

extern void fLib_SetGpioDir(UINT32 io_base, UINT32 pin, UINT32 dir);
extern void fLib_SetGpioBypass(UINT32 io_base, UINT32 pin);
extern void fLib_SetGpioEdgeMode(UINT32 io_base, UINT32 pin, UINT32 both);

extern UINT32 fLib_GetGpioIntMaskStatus(UINT32 io_base);
extern void fLib_ClearGpioInt(UINT32 io_base, UINT32 data);

extern void  fLib_SetGpioIntMask(UINT32 io_base, UINT32 pin);
extern void  fLib_SetGpioIntUnMask(UINT32 io_base, UINT32 pin);
extern void  fLib_SetGpioIntEnable(UINT32 io_base, UINT32 pin);
extern void  fLib_SetGpioIntDisable(UINT32 io_base, UINT32 pin);
extern void fLib_SetGpioTrigger(UINT32 io_base, UINT32 pin, UINT32 trigger);
extern void fLib_SetGpioActiveMode(UINT32 io_base, UINT32 pin, UINT32 Active);


#endif /* __GPIO_H */
