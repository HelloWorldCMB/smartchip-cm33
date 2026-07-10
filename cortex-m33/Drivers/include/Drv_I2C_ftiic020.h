/* -----------------------------------------------------------------------------
 * Copyright (c) 2013-2014 ARM Ltd.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *
 * $Date:        21. May 2014
 * $Revision:    V2.1
 *
 * Project:      I2C Driver Definitions for FTIIC020
 * -------------------------------------------------------------------------- */

#ifndef __I2C_FTIIC020_H
#define __I2C_FTIIC020_H

#include "Driver_Common.h"

/* Clock Control Unit register */
#define CCU_CLK_CFG_RUN     (1 << 0)
#define CCU_CLK_CFG_AUTO    (1 << 1)
#define CCU_CLK_STAT_RUN    (1 << 0)

#define CLK_SRC_PLL1        0x09            // I2C clock source

/* I2C reset value for RGU */
#define RGU_RESET_I2C0      (1 << 16)       // I2C0 reset
#define RGU_RESET_I2C1      (1 << 17)       // I2C1 reset

/* I2C Driver state flags */
#define I2C_FLAG_INIT       (1 << 0)        // Driver initialized
#define I2C_FLAG_POWER      (1 << 1)        // Driver power on
#define I2C_FLAG_SETUP      (1 << 2)        // Master configured, clock set
#define I2C_FLAG_SLAVE_RX   (1 << 3)        // Slave receive registered

/* I2C Common Control flags */
#define I2C_CON_AA          (1 << 2)        // Assert acknowledge bit
#define I2C_CON_SI          (1 << 3)        // I2C interrupt bit
#define I2C_CON_STO         (1 << 4)        // STOP bit
#define I2C_CON_STA         (1 << 5)        // START bit
#define I2C_CON_I2EN        (1 << 6)        // I2C interface enable
#define I2C_CON_FLAGS       (I2C_CON_AA | I2C_CON_SI | I2C_CON_STO | I2C_CON_STA)

/* I2C Stalled Status flags */
#define I2C_MASTER          (1 << 0)        // Master stalled
#define I2C_SLAVE_TX        (1 << 1)        // Slave stalled on transmit
#define I2C_SLAVE_RX        (1 << 2)        // Slave stalled on receive
#define I2C_SLAVE           (I2C_SLAVE_TX | I2C_SLAVE_RX)

/* I2C Status Miscellaneous states */
#define I2C_STAT_BUSERR      0x00           // I2C Bus error

/* I2C Status Master mode */
#define I2C_STAT_MA_START    0x08           // START transmitted
#define I2C_STAT_MA_RSTART   0x10           // Repeated START transmitted
#define I2C_STAT_MA_SLAW_A   0x18           // SLA+W transmitted, ACK received
#define I2C_STAT_MA_SLAW_NA  0x20           // SLA+W transmitted, no ACK recvd
#define I2C_STAT_MA_DT_A     0x28           // Data transmitted, ACK received
#define I2C_STAT_MA_DT_NA    0x30           // Data transmitted, no ACK recvd
#define I2C_STAT_MA_ALOST    0x38           // Arbitration lost SLA+W or data
#define I2C_STAT_MA_SLAR_A   0x40           // SLA+R transmitted, ACK received
#define I2C_STAT_MA_SLAR_NA  0x48           // SLA+R transmitted, no ACK recvd
#define I2C_STAT_MA_DR_A     0x50           // Data received, ACK returned
#define I2C_STAT_MA_DR_NA    0x58           // Data received, no ACK returned

/* I2C Status Slave mode */
#define I2C_STAT_SL_SLAW_A   0x60           // SLA+W received, ACK returned
#define I2C_STAT_SL_ALOST_MW 0x68           // Arbitration lost SLA+W in Master mode
#define I2C_STAT_SL_GCA_A    0x70           // General address recvd, ACK returned
#define I2C_STAT_SL_ALOST_GC 0x78           // Arbitration lost in General call
#define I2C_STAT_SL_DR_A     0x80           // Data received, ACK returned
#define I2C_STAT_SL_DR_NA    0x88           // Data received, no ACK returned
#define I2C_STAT_SL_DRGC_A   0x90           // Data recvd General call, ACK returned
#define I2C_STAT_SL_DRGC_NA  0x98           // Data recvd General call, no ACK returned
#define I2C_STAT_SL_STOP     0xA0           // STOP received while addressed
#define I2C_STAT_SL_SLAR_A   0xA8           // SLA+R received, ACK returned
#define I2C_STAT_SL_ALOST_MR 0xB0           // Arbitration lost SLA+R in Master mode
#define I2C_STAT_SL_DT_A     0xB8           // Data transmitted, ACK received
#define I2C_STAT_SL_DT_NA    0xC0           // Data transmitted, no ACK received
#define I2C_STAT_SL_LDT_A    0xC8           // Last data transmitted, ACK received


/****** I2C Control Codes *****/

#define ARM_I2C_OWN_ADDRESS             (0x01)      ///< Set Own Slave Address; arg = address 
#define ARM_I2C_BUS_SPEED               (0x02)      ///< Set Bus Speed; arg = speed
#define ARM_I2C_BUS_CLEAR               (0x03)      ///< Execute Bus clear: send nine clock pulses
#define ARM_I2C_ABORT_TRANSFER          (0x04)      ///< Abort Master/Slave Transmit/Receive
#define ARM_I2C_SCAN        			(0x05)		///< Scan bus
#define ARM_I2C_TEST_DEVICE        			(0x06)		///< Test device

/*----- I2C Bus Speed -----*/
#define ARM_I2C_BUS_SPEED_STANDARD      (0x01)      ///< Standard Speed (100kHz)
#define ARM_I2C_BUS_SPEED_FAST          (0x02)      ///< Fast Speed     (400kHz)
#define ARM_I2C_BUS_SPEED_FAST_PLUS     (0x03)      ///< Fast+ Speed    (  1MHz)
#define ARM_I2C_BUS_SPEED_HIGH          (0x04)      ///< High Speed     (3.4MHz)

/* I2C clock divided register (I2C Clock Division Register (CDR) (Offset: 0x08))*/
#define I2C_CDR_Value		0x200   

/*For register I2C Set/Hold Time & Glitch Suppression Setting Register (TGSR) (Offset: 0x14)*/
#define I2C_GSR_Value		5//0x00  
#define I2C_TSR_Value		5//0x01
#define I2C_SCLout			200 //200Khz

/* I2C Register */

#define I2C_Control               	0x0
#define I2C_Status                	0x4
#define I2C_ClockDiv              	0x8
#define I2C_Data                   	0x0C
#define I2C_SlaveAddr              	0x10
#define I2C_TGSR                  	0x14  /* 2003/05/28 Correct by Peter */
#define I2C_BMR                   	0x18  /* 2003/05/28 Correct by Peter */


/* I2C Control register */


#define I2C_ALIEN                   0x2000  /* Arbitration lose */
#define I2C_SAMIEN                  0x1000  /* slave address match */
#define I2C_STOPIEN                 0x800   /* stop condition */
#define I2C_BERRIEN                 0x400   /* non ACK response */
#define I2C_DRIEN                   0x200   /* data receive */
#define I2C_DTIEN                   0x100   /* data transmit */
#define I2C_TBEN                    0x80    /* transfer byte enable */
#define I2C_ACKNAK                  0x40    /* ack sent */
#define I2C_STOP                    0x20    /* stop */
#define I2C_START                   0x10    /* start */
#define I2C_GCEN                    0x8     /* general call */
#define I2C_SCLEN                   0x4     /* enable clock */
#define I2C_I2CEN                   0x2     /* enable I2C */
#define I2C_I2CRST                  0x1     /* reset I2C */
#define I2C_ENABLE                  (I2C_ALIEN|I2C_SAMIEN|I2C_STOPIEN|I2C_BERRIEN|I2C_DRIEN|I2C_DTIEN|I2C_SCLEN|I2C_I2CEN)
#define I2C_SLAVE_ENABLE						(I2C_ALIEN|I2C_BERRIEN|I2C_DRIEN|I2C_DTIEN|I2C_I2CEN)

/*
 * control register
 */
#define CR_ALIRQ      0x2000  /* arbitration lost interrupt (master) */
#define CR_SAMIRQ     0x1000  /* slave address match interrupt (slave) */
#define CR_STOPIRQ    0x800   /* stop condition interrupt (slave) */
#define CR_NAKRIRQ    0x400   /* NACK response interrupt (master) */
#define CR_DRIRQ      0x200   /* rx interrupt (both) */
#define CR_DTIRQ      0x100   /* tx interrupt (both) */
#define CR_TBEN       0x80    /* tx enable (both) */
#define CR_NAK        0x40    /* NACK (both) */
#define CR_STOP       0x20    /* stop (master) */
#define CR_START      0x10    /* start (master) */
#define CR_GCEN       0x8     /* general call support (slave) */
#define CR_SCLEN      0x4     /* enable clock out (master) */
#define CR_I2CEN      0x2     /* enable I2C (both) */
#define CR_I2CRST     0x1     /* reset I2C (both) */
#define CR_ENABLE     \
	(CR_ALIRQ | CR_NAKRIRQ | CR_DRIRQ | CR_DTIRQ | CR_SCLEN | CR_I2CEN)

/*
 * status register
 */
#define SR_CLRAL      0x400    /* clear arbitration lost */
#define SR_CLRGC      0x200    /* clear general call */
#define SR_CLRSAM     0x100    /* clear slave address match */
#define SR_CLRSTOP    0x80     /* clear stop */
#define SR_CLRNAKR    0x40     /* clear NACK respond */
#define SR_DR         0x20     /* rx ready */
#define SR_DT         0x10     /* tx done */
#define SR_BB         0x8      /* bus busy */
#define SR_BUSY       0x4      /* chip busy */
#define SR_ACK        0x2      /* ACK/NACK received */
#define SR_RW         0x1      /* set when master-rx or slave-tx mode */


/* I2C Status Register */
#define SR_SBS        0x800000 /* start byte */
#define SR_START      0x800    /* start */
#define SR_AL         0x400    /* arbitration lost */
#define SR_GC         0x200    /* general call */
#define SR_SAM        0x100    /* slave address match */
#define SR_STOP       0x80     /* stop received */
#define SR_NACK       0x40     /* NACK received */
#define SR_TD         0x20     /* tx/rx finish */
#define SR_BB         0x8      /* bus busy */
#define SR_BUSY       0x4      /* chip busy */
#define SR_RW         0x1      /* set when master-rx or slave-tx mode */



/* I2C clock divided register (I2C Clock Division Register (CDR) (Offset: 0x08))*/
#define I2C_CDR_Value		0x200   

/*For register I2C Set/Hold Time & Glitch Suppression Setting Register (TGSR) (Offset: 0x14)*/
#define I2C_GSR_Value		5//0x00  
#define I2C_TSR_Value		5//0x01
#define I2C_SCLout			200 //200Khz

/* I2C slave address register */

#define I2C_EN10                    0x80000000  /* 10-bit address slave mode */
#define I2C_SARMSB                  0x380       /* mask for SAR msb when EN10=1 */
#define I2C_SARLSB                  0x7f	    /* mask for SAR lsb */

/* Bus Monitor Register */

#define I2C_SCL                     0x2
#define I2C_SDA                     0x1


// --------------------------------------------------------------------
//	config
// --------------------------------------------------------------------
#define I2C_Default_FREQ        	(45 * 1000)//(45 * 1024)
#define I2C_MAX_FREQ                (400 * 1000)//(400 * 1024)
#if 1	// org
#define WRITE_TIME_OUT				100 
#define READ_TIME_OUT				100 

#else
	#define WRITE_TIME_OUT				100		// 100000
	#define READ_TIME_OUT				100		// 100000
#endif

// --------------------------------------------------------------------
//		I2C_Dev->address_mode
// --------------------------------------------------------------------
#define ADDR_MODE_NONE			0			/// no internal address
#define ADDR_MODE_8BIT			1			/// internal address is 8 bit
#define ADDR_MODE_16BIT			2			/// internal address is 16 bit


/**
\brief I2C Status
*/
typedef struct _ARM_I2C_STATUS {
  unsigned int busy             : 1;        ///< Busy flag
  unsigned int mode             : 1;        ///< Mode: 0=Slave, 1=Master
  unsigned int direction        : 1;        ///< Direction: 0=Transmitter, 1=Receiver
  unsigned int general_call     : 1;        ///< General Call indication (cleared on start of next Slave operation)
  unsigned int arbitration_lost : 1;        ///< Master lost arbitration (cleared on start of next Master operation)
  unsigned int bus_error        : 1;        ///< Bus error detected (cleared on start of next Master/Slave operation)
} ARM_I2C_STATUS;

typedef void (*ARM_I2C_SignalEvent_t) (unsigned int event);  ///< Pointer to \ref ARM_I2C_SignalEvent : Signal I2C Event.

typedef struct _ARM_I2C_CAPABILITIES {
  unsigned int ten_bit          : 1;        ///< supports Simplex Mode (Master and Slave)
} ARM_I2C_CAPABILITIES;


/* I2C Resource Configuration */
typedef struct {
  int		        		io_base;                // I2C register interface
  int 	            irq;         // I2C Event IRQ Number
  unsigned int				cnt;
  unsigned int				dev_addr;  
  unsigned long				status;
  int 						address_mode;
} I2C_RESOURCES;

/**
\brief Access structure of the I2C Driver.
*/
typedef struct _ARM_DRIVER_I2C {
  ARM_DRIVER_VERSION   (*GetVersion)     (void);                                                                ///< Pointer to \ref ARM_I2C_GetVersion : Get driver version.
  ARM_I2C_CAPABILITIES (*GetCapabilities)(void);                                                                ///< Pointer to \ref ARM_I2C_GetCapabilities : Get driver capabilities.
  int              (*Initialize)     (ARM_I2C_SignalEvent_t cb_event);                                      ///< Pointer to \ref ARM_I2C_Initialize : Initialize I2C Interface.
  int              (*Uninitialize)   (void);                                                                ///< Pointer to \ref ARM_I2C_Uninitialize : De-initialize I2C Interface.
  int              (*PowerControl)   (ARM_POWER_STATE state);                                               ///< Pointer to \ref ARM_I2C_PowerControl : Control I2C Interface Power.
  int              (*MasterTransmit) (unsigned int addr,  uint8_t *data, unsigned int num, bool xfer_pending); ///< Pointer to \ref ARM_I2C_MasterTransmit : Start transmitting data as I2C Master.
  int              (*MasterReceive)  (unsigned int addr,  uint8_t *data, unsigned int num, bool xfer_pending); ///< Pointer to \ref ARM_I2C_MasterReceive : Start receiving data as I2C Master.
  int              (*SlaveTransmit)  (               const uint8_t *data, unsigned int num);                    ///< Pointer to \ref ARM_I2C_SlaveTransmit : Start transmitting data as I2C Slave.
  int              (*SlaveReceive)   (                     uint8_t *data, unsigned int num);                    ///< Pointer to \ref ARM_I2C_SlaveReceive : Start receiving data as I2C Slave.
  int              (*GetDataCount)   (void);                                                                ///< Pointer to \ref ARM_I2C_GetDataCount : Get transferred data count.
  int              (*Control)        (unsigned int control, unsigned int arg);                                      ///< Pointer to \ref ARM_I2C_Control : Control I2C Interface.
  ARM_I2C_STATUS       (*GetStatus)      (void);                                                                ///< Pointer to \ref ARM_I2C_GetStatus : Get I2C status.
  I2C_RESOURCES *dev;
} ARM_DRIVER_I2C;

extern int i2c_probe(ARM_DRIVER_I2C * I2Cdrv, int chip);
#endif /* __I2C_FTIIC020_H */
