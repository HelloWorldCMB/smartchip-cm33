/**************************************************************************//**
 * @file     system_LEO.h
 * @brief    CMSIS Cortex-M# Device Peripheral Access Layer Header File
 *           for the Device Series ...
 * @version  V2.10
 * @date     20. July 2011
 *
 * @note
 * Copyright (C) 2010-2011 ARM Limited. All rights reserved.
 *
 * @par
 * ARM Limited (ARM) is supplying this software for use with Cortex-M
 * processor based microcontrollers.  This file can be freely distributed
 * within development tools that are supporting such ARM based processors.
 *
 * @par
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 ******************************************************************************/


#ifndef SYSTEM_LEO_H
#define SYSTEM_LEO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// --------------------------------------------------------------------
//	apb dma req number
// --------------------------------------------------------------------

#define AHB2APB_DMA_REQ_SSP1_TX			0x3
#define AHB2APB_DMA_REQ_SSP1_RX			0x4
#define AHB2APB_DMA_REQ_UART2_TX		0x9
#define AHB2APB_DMA_REQ_UART2_RX		0xa
#define AHB2APB_DMA_REQ_SSP2_TX			0xb
#define AHB2APB_DMA_REQ_SSP2_RX			0xc
#define AHB2APB_DMA_REQ_UART3_TX		0xe
#define AHB2APB_DMA_REQ_UART3_RX		0xf


#define AHB2APB_DMA_REQ_AUDIO_TX		AHB2APB_DMA_REQ_SSP2_TX
#define AHB2APB_DMA_REQ_AUDIO_RX		AHB2APB_DMA_REQ_SSP2_RX


// --------------------------------------------------------------------
//	ahb dma request number
// --------------------------------------------------------------------
#define TMR1_DMA_REQ						0
#define TMR2_DMA_REQ						1
#define TMR3_DMA_REQ						2
#define TMR4_DMA_REQ						3
#define IRDA_UART_0_TX_DMA_REQ		4
#define IRDA_UART_0_RX_DMA_REQ		5
#define SSP_TX_DMA_REQ					6
#define SSP_RX_DMA_REQ					7
#define SPI_DMA_REQ							8
#define SLCD_DMA_REQ							9
#define IRDA_UART_1_TX_DMA_REQ		10
#define IRDA_UART_1_RX_DMA_REQ		11
#define RSV1_DMA_REQ							12
#define RSV2_DMA_REQ							13
#define RSV3_DMA_REQ							14
#define	SDC_DMA_REQ								15

#define SSP_CLK					12288000			// for all SSPs
#define UART_CLOCK				58982400
#define CPU_CLK					150000000			// 48Mhz

#define AHB_CLK					150000000
#define APB_CLK					100000000
#define APB_CLOCK				APB_CLK
#define I2C_CLOCK				100000000//25000000
#define UART_CLOCK_2			(UART_CLOCK)
#define NR_IRQS				64
#define WDT011_CLK     100000000//PCLK

#define I2S_TOTAL_SIZE		0x400//0x4000//0x2000

//#define MEMTEST
/* define which items need to be tested */
#define DMAC020_TEST
#define FPU_TEST
#define GPIO_TEST
#define I2C_TEST
//#define I2S_TEST
#define ITM_TEST
#define PWMTMR_TEST
#define SEMAPHORE_TEST
#define SSPSPI_TEST
#define KBC_TEST
#define UART_DMA_TEST
#define PWMTMR_DMA_TEST
#define SPI2AHB_TEST
//#define CAN010_TEST
#define WDT011_TEST
//#define EFUSE_TEST

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_LEO_H */
