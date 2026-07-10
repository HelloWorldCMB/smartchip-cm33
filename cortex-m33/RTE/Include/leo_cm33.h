/**************************************************************************//**
 * @file     leo_cm33.h
 * @brief    CMSIS Core Peripheral Access Layer Header File for
 *           ARMCM4 Device (configured for CM4 without FPU)
 * @version  V5.3.1
 * @date     09. July 2018
 ******************************************************************************/
/*
 * Copyright (c) 2009-2018 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
 #ifndef Vango_Leo
#define Vango_Leo
#ifdef __cplusplus
extern "C" {
#endif

#include <system_LEO.h>
	
/* -------  Start of section using anonymous unions and disabling warnings  ------- */
#if   defined (__CC_ARM)
  #pragma push
  #pragma anon_unions
#elif defined (__ICCARM__)
  #pragma language=extended
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wc11-extensions"
  #pragma clang diagnostic ignored "-Wreserved-id-macro"
#elif defined (__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined (__TMS470__)
  /* anonymous unions are enabled by default */
#elif defined (__TASKING__)
  #pragma warning 586
#elif defined (__CSMC__)
  /* anonymous unions are enabled by default */
#else
  #warning Not supported compiler type
#endif	

/* -------------------------  Interrupt Number Definition  ------------------------ */
typedef enum {
/* -------------------  Cortex-M4 Processor Exceptions Numbers  ------------------- */
  Reset_IRQn                    = -15,              /*!<   1  Reset Vector, invoked on Power up and warm reset                 */
  NonMaskableInt_IRQn           = -14,              /*!<   2  Non maskable Interrupt, cannot be stopped or preempted           */
  HardFault_IRQn                = -13,              /*!<   3  Hard Fault, all classes of Fault                                 */
  MemoryManagement_IRQn         = -12,              /*!<   4  Memory Management, MPU mismatch, including Access Violation
																															and No Match                                                     */
  BusFault_IRQn                 = -11,              /*!<   5  Bus Fault, Pre-Fetch-, Memory Access Fault, other address/memory
																															related Fault                                                    */
  UsageFault_IRQn               = -10,              /*!<   6  Usage Fault, i.e. Undef Instruction, Illegal State Transition    */
  SVCall_IRQn                   =  -5,              /*!<  11  System Service Call via SVC instruction                          */
  DebugMonitor_IRQn             =  -4,              /*!<  12  Debug Monitor                                                    */
  PendSV_IRQn                   =  -2,              /*!<  14  Pendable request for system service                              */
  SysTick_IRQn                  =  -1,              /*!<  15  System Tick Timer                                                */
/* -------------------  Kneron_Mozart Specific Interrupt Numbers  ------------------- */
  GPIO_FTGPIO010_IRQ            =  0,
  GPIO_FTGPIO010_1_I            =  1,
	WDT_FTWTD011_2_IRQ            =  2,
	WDT_FTWTD011_3_IRQ            =  3,
	SSP_FTSSP010_0_I2S_IRQ        =  4,
	SSP_FTSSP010_1_I2S_IRQ				=  5,
	SSP_FTSSP010_0_SPI_IRQ				=  6,
	SSP_FTSSP010_1_SPI_IRQ				=  7,
	SSP_FTSSP010_2_SPI_IRQ        =  8,
  SSP_FTSSP010_3_SPI_IRQ        =  9,
	SSP_FTSSP010_4_SPI_IRQ        =  10,
	SSP_FTSSP010_5_SPI_IRQ        =  11,
	SSP_FTSSP010_6_SPI_IRQ        =  12,
	PWM_FTPWMTMR010_0_IRQ					=  13,
	PWM_FTPWMTMR010_1_IRQ					=  14,
	PWM_FTPWMTMR010_2_IRQ					=  15,
	PWM_FTPWMTMR010_3_IRQ         =  16,
  PWM_FTPWMTMR010_4_IRQ         =  17,
	PWM_FTPWMTMR010_1_0IRQ        =  18,
	PWM_FTPWMTMR010_1_1_IRQ       =  19,
	PWM_FTPWMTMR010_1_2_IRQ       =  20,
	PWM_FTPWMTMR010_1_3_IRQ				=  21,
	PWM_FTPWMTMR010_1_4_IRQ				=  22,
	PWM_FTPWMTMR010_2_0_IRQ				=  23,
	PWM_FTPWMTMR010_2_1_IRQ       =  24,
	PWM_FTPWMTMR010_2_2_IRQ       =  25,
	PWM_FTPWMTMR010_2_3_IRQ       =  26,
	PWM_FTPWMTMR010_2_4_IRQ				=  27,
	PWM_FTPWMTMR010_3_0_IRQ				=  28,
	PWM_FTPWMTMR010_3_1_IRQ				=  30,
	PWM_FTPWMTMR010_3_2_IRQ       =  31,
  PWM_FTPWMTMR010_3_3_IRQ       =  29,
	PWM_FTPWMTMR010_3_4_IRQ       =  32,
	IIC_FTIIC010_0_IRQ            =  33,
	IIC_FTIIC010_1_IRQ            =  34,
	IIC_FTIIC010_2_IRQ						=  35,
	IIC_FTIIC010_3_IRQ					  =  36,
	IIC_FTIIC010_4_IRQ					  =  37,
	UART_FTUART010_0_IRQ          =  38,
  UART_FTUART010_0_1_IRQ        =  39,
	UART_FTUART010_1_IRQ          =  40,
	UART_FTUART010_1_1_IRQ        =  41,
	UART_FTUART010_2_IRQ					=  42,
	UART_FTUART010_2_1_IRQ				=  43,
	UART_FTUART010_3_IRQ					=  44,
	UART_FTUART010_3_1_IRQ        =  45,
  UART_FTUART010_4_IRQ          =  46,
	UART_FTUART010_4_1_IRQ        =  47,
	UART_FTUART010_5_IRQ          =  48,
	UART_FTUART010_5_1_IRQ        =  49,
	UART_FTUART010_6_IRQ				  =  50,
	UART_FTUART010_6_1_IRQ				=  51,
	UART_FTUART010_7_IRQ					=  52,
	UART_FTUART010_7_1_IRQ        =  53,
  UART_FTUART010_8_IRQ          =  54,
	UART_FTUART010_8_1_IRQ				=  55,
	UART_FTUART010_9_IRQ				  =  56,
	UART_FTUART010_9_1_IRQ        =  57,
	CAN_FTCAN010_0_IRQ						=  58,
	CAN_FTCAN010_1_IRQ						=  59,
	KBC_FTKBCC010_IRQ							=  60,
	KBC_FTKBCC010_PAD_IRQ							=  61,
	KBC_FTKBCC010_RX_IRQ							=  62,
	KBC_FTKBCC010_TX_IRQ							=  63,
	ADC_FTADCC010_IRQ							=  64,
	DDC_FTDDCC010_IRQ							=  65,
	DMA_FTDMAC020_0_IRQ							=  66,
	DMA_FTDMAC020_0_TC_IRQ					=  67,
	DMA_FTDMAC020_0_ERR_IRQ					=  68,
	SEMAPHORE_FTSEMAPHRE_IRQ			=  69,
	SEMAPHORE_FTSEMAPHRE_CTI_IRQ	=  70,
	
} IRQn_Type;

/* --------  Configuration of Core Peripherals  ----------------------------------- */
#define __MPU_PRESENT             1U        /* MPU present */
#define __VTOR_PRESENT            1U        /* VTOR present */
#define __NVIC_PRIO_BITS          3U        /* Number of Bits used for Priority Levels */
#define __Vendor_SysTickConfig    0U        /* Set to 1 if different SysTick Config is used */
#define __FPU_PRESENT             1U        /* with FPU present */
#ifdef __ARM_FEATURE_DSP
#undef __ARM_FEATURE_DSP
#define __ARM_FEATURE_DSP					0U
#endif

#include "core_cm33.h"                       /* Processor and core peripherals */                   
#include "system_ARMCM33.h"

#endif
