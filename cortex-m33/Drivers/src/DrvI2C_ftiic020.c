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
 * $Date:        24. August 2015
 * $Revision:    V2.5
 *
 * Driver:       Driver_I2C0, Driver_I2C1
 * Configured:   via RTE_Device.h configuration file
 * Project:      I2C Driver for NXP LPC18xx
 * --------------------------------------------------------------------------
 * Use the following configuration settings in the middleware component
 * to connect to this driver.
 *
 *   Configuration Setting                 Value   I2C Interface
 *   ---------------------                 -----   -------------
 *   Connect to hardware via Driver_I2C# = 0       use I2C0
 *   Connect to hardware via Driver_I2C# = 1       use I2C1
 * -------------------------------------------------------------------------- */

/* History:
 *  Version 2.3
 *    - Pending IRQ flag cleared after aborted transfer
 *  Version 2.2
 *    - Updated initialization, uninitialization and power procedures
 *  Version 2.1
 *    - Updated to CMSIS Driver API V2.02
 *    - Added Multi-master support
 *  Version 2.0
 *    - Based on API V2.00
 *  Version 1.1
 *    - Based on API V1.10 (namespace prefix ARM_ added)
 *  Version 1.0
 *    - Initial release
 */

#include <string.h>
#include <Driver_Common.h>
#include "Drv_I2C_ftiic020.h"
#include "common_include.h"
#include "leo_cm33.h"

#define CONFIG_SYS_I2C_FTI2C010_V1_16_0

/* 7-bit dev address + 1-bit read/write */
#define I2C_RD(dev)             ((((dev) << 1) & 0xfe) | 1)
#define I2C_WR(dev)             (((dev) << 1) & 0xfe)
//#define Debug_IIC_Scan_001 1

#define ARM_I2C_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(2,3) /* driver version */
#define ARM_I2C_API_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(2,00)  /* API version */

extern void WaitForTick (void);
int I2C_RandomRead(I2C_RESOURCES *dev, unsigned char *buf, unsigned long address, unsigned long index, unsigned long num);

/* I2C core clock (system_LPC18xx.c) */
extern unsigned int GetClockFreq (unsigned int clk_src);


/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
  ARM_I2C_API_VERSION,
  ARM_I2C_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_I2C_CAPABILITIES DriverCapabilities = {
  0            /* supports 10-bit addressing */
};


/* I2C0 Resources */
I2C_RESOURCES I2C0_Resources = {
  IIC_FTIIC010_0_PA_BASE,
  IIC_FTIIC010_0_IRQ,
  0,
  0,
  ADDR_MODE_8BIT
};

/* I2C1 Resources */
I2C_RESOURCES I2C1_Resources = {
  IIC_FTIIC010_1_PA_BASE,
  IIC_FTIIC010_1_IRQ,
  0,
  0,
  ADDR_MODE_8BIT  
};

/* I2C2 Resources */
I2C_RESOURCES I2C2_Resources = {
  IIC_FTIIC010_2_PA_BASE,
  IIC_FTIIC010_2_IRQ,
  0,
  0,
  ADDR_MODE_8BIT  
};

/* I2C3 Resources */
I2C_RESOURCES I2C3_Resources = {
  IIC_FTIIC010_3_PA_BASE,
  IIC_FTIIC010_3_IRQ,
  0,
  0,
  ADDR_MODE_8BIT  
};


/**
  \fn          ARM_DRIVER_VERSION I2C_GetVersion (void)
  \brief       Get driver version.
  \return      \ref ARM_DRIVER_VERSION
*/
static ARM_DRIVER_VERSION I2C_GetVersion (void) {
  return DriverVersion;
}

/**
  \fn          ARM_I2C_CAPABILITIES I2C_GetCapabilities (void)
  \brief       Get driver capabilities.
  \return      \ref ARM_I2C_CAPABILITIES
*/
static ARM_I2C_CAPABILITIES I2C_GetCapabilities (void) {
  return DriverCapabilities;
}
UINT32 I2C_GetCR(I2C_RESOURCES *dev)
{
  	return readl(dev->io_base + I2C_Control);
}

void I2C_SetTGSR(I2C_RESOURCES *dev, UINT32 gsr,UINT32 tsr)
{
	writel((gsr << 10)|tsr, dev->io_base + I2C_TGSR);
}

/*
void I2C_SetSlaveAddress(I2C_RESOURCES *dev, UINT32 address)
{
 	writel(address, dev->io_base + I2C_SlaveAddr);
}
*/

UINT32 I2C_GetGSR(I2C_RESOURCES *dev)
{
 	UINT32 tmp;

	tmp = readl(dev->io_base + I2C_TGSR);

 	return ((tmp >> 10) & 0x7);
}

void I2C_SetClockdiv(I2C_RESOURCES *dev, UINT32 count)
{
 	writel(count, dev->io_base + I2C_ClockDiv);
}

UINT32 I2C_ReadStatus(I2C_RESOURCES *dev)
{
	return readl(dev->io_base + I2C_Status);
}

int fti2c010_wait(int regs, uint32_t mask)
{
	int ret = -1;
	uint32_t stat, ts;

	for (ts = 0; ts < 22000;ts++ ) {
		stat = readl(regs + I2C_Status);
		writel(stat, regs + I2C_Status);
		if ((stat & mask) == mask) {
			ret = 0;//it means ok
			break;
		}
		//delay_ms(1);
	}
	//if(ret == -1) fLib_printf("ERR: SR , val 0x%x\n", readl(regs+0x04));
  
	return ret;
}

int i2c_probe(ARM_DRIVER_I2C * I2Cdrv, int chip)
{
	int ret;
	I2C_RESOURCES *dev = I2Cdrv->dev;
	
	/* 1. Select slave device (7bits Address + 1bit R/W) */
	writel(I2C_WR(chip),dev->io_base + 0x0c );
	writel(CR_ENABLE | CR_TBEN | CR_START, dev->io_base +0);
	ret = fti2c010_wait(dev->io_base, SR_TD);
	
	if (ret!=0)
		return ret;
	
	/* 2. Select device register */
	writel(0, dev->io_base + 0x0c );
	writel(CR_ENABLE | CR_TBEN, dev->io_base + 0x0);
	ret = fti2c010_wait(dev->io_base, SR_TD);
	
	return ret;	
}

// --------------------------------------------------------------------
//			CSCLout means I2C bus clock value , *1000
// --------------------------------------------------------------------
int I2C_SetSCLout(I2C_RESOURCES *dev, unsigned int i_SCLout)
{
	int  i; 

	i_SCLout = i_SCLout *1000 ;
	if ( i_SCLout == 0 )
	{
		i_SCLout = I2C_Default_FREQ;		// default. clock is 45KHz
	}
	
	if (i_SCLout > I2C_MAX_FREQ) 
	{
		i_SCLout = I2C_MAX_FREQ;			// max. clock is 400KHz
	}
	i =((I2C_CLOCK-(i_SCLout*I2C_GetGSR(dev)))/(2*i_SCLout))-2;
	if (i >= BIT10) 
	{
		fLib_printf(" I2C_CLOCK=%d, i_SCLout=%d\n",I2C_CLOCK, i_SCLout);
		fLib_printf("Pclk is to fast to form i2c clock, fail \n");
		return -1;
	}
	I2C_SetClockdiv(dev, i);
	return 0;
}

static void to_i2c_addr(u8 *buf, uint32_t addr, int alen)
{
	int i, shift;

	if (!buf || alen <= 0)
		return;

	/* MSB first */
	i = 0;
	shift = (alen - 1) * 8;
	while (alen-- > 0) {
		buf[i] = (u8)(addr >> shift);
		shift -= 8;
	}
}

int fti2c010_read(int regs,
			u8 dev, unsigned int addr, int alen, uchar *buf, int len)
{
	int ret, pos;
	uchar paddr[4] = { 0 };
	
	fLib_printf("**int regs %x,u8 dev %x, unsigned int addr %x, int alen %x, int len %x\n",regs,
			dev, addr, alen, len);

	if(alen == 2) { //16bit length mode
		paddr[0]=(addr>>8)&0xff;
		paddr[1]=addr&0xff;
	}else{
		paddr[0]=addr&0xff;
	}
	
	/*
	 * Phase A. Set register address
	 */

	/* A.1 Select slave device (7bits Address + 1bit R/W) */
	writel(I2C_WR(dev), regs +0xc);
	writel(CR_ENABLE | CR_TBEN | CR_START, regs+0x0);
#ifndef CONFIG_SYS_I2C_FTI2C010_V1_16_0
	ret = fti2c010_wait(regs, SR_DT);
#else
	ret = fti2c010_wait(regs, SR_TD);
#endif
	if (ret)
		return ret;

	/* A.2 Select device register */
	for (pos = 0; pos < alen; ++pos) {
		uint32_t ctrl = CR_ENABLE | CR_TBEN;

		writel(paddr[pos], regs + +0x0c);
		writel(ctrl, regs +0x0);
#ifndef CONFIG_SYS_I2C_FTI2C010_V1_16_0
		ret = fti2c010_wait(regs, SR_DT);
#else
		ret = fti2c010_wait(regs, SR_TD);
#endif
		if (ret)
			return ret;
	}

	/*
	 * Phase B. Get register data
	 */

	/* B.1 Select slave device (7bits Address + 1bit R/W) */
	writel(I2C_RD(dev), regs+0x0c);
	writel(CR_ENABLE | CR_TBEN | CR_START, regs+0x0);
#ifndef CONFIG_SYS_I2C_FTI2C010_V1_16_0
	ret = fti2c010_wait(regs, SR_DT);
#else
	ret = fti2c010_wait(regs, SR_TD);
#endif
	if (ret)
		return ret;

	/* B.2 Get register data */
	for (pos = 0; pos < len; ++pos) {
		uint32_t ctrl = CR_ENABLE | CR_TBEN;
#ifndef CONFIG_SYS_I2C_FTI2C010_V1_16_0
		uint32_t stat = SR_DR;
#else
		uint32_t stat = SR_TD;
#endif

		if (pos == len - 1) {
			ctrl |= CR_NAK | CR_STOP;
#ifndef CONFIG_SYS_I2C_FTI2C010_V1_16_0
			stat |= SR_ACK;
#endif
		}
		writel(ctrl, regs+0x0);
		ret = fti2c010_wait(regs, stat);
		if (ret)
			break;
		buf[pos] = (uchar)(readl(regs+0x0c) & 0xFF);	
	}

	return ret;
}

int fti2c010_write(int regs,
			u8 dev, unsigned int addr, int alen, u8 *buf, int len)
{
	int ret, pos;
	uchar paddr[4] = { 0 };
	
	if(alen == 2) { //16bit length mode
		paddr[0]=(addr>>8)&0xff;
		paddr[1]=addr&0xff;
	}else{
		paddr[0]=addr&0xff;
	}

	/*
	 * Phase A. Set register address
	 *
	 * A.1 Select slave device (7bits Address + 1bit R/W)
	 */
	writel(I2C_WR(dev), regs+0xc);
	writel(CR_ENABLE | CR_TBEN | CR_START, regs+0x0);
#ifndef CONFIG_SYS_I2C_FTI2C010_V1_16_0
	ret = fti2c010_wait(regs, SR_DT);
#else
	ret = fti2c010_wait(regs, SR_TD);
#endif
	if (ret){
		fLib_printf("Set device address failed.\n");
		return ret;
	}

	/* A.2 Select device register */
	for (pos = 0; pos < alen; ++pos) {
		uint32_t ctrl = CR_ENABLE | CR_TBEN;

		writel(paddr[pos], regs+0xc);
		writel(ctrl, regs+0x0);
#ifndef CONFIG_SYS_I2C_FTI2C010_V1_16_0
	ret = fti2c010_wait(regs, SR_DT);
#else
	ret = fti2c010_wait(regs, SR_TD);
#endif
		if (ret){
			fLib_printf("Set device register failed.\n");
			return ret;
		}
	}

	/*
	 * Phase B. Set register data
	 */
	for (pos = 0; pos < len; ++pos) {
		uint32_t ctrl = CR_ENABLE | CR_TBEN;

		if (pos == len - 1){
			ctrl |= CR_STOP;
			delay_ms(5);
		}
		
		writel(buf[pos], regs+0xc);
		writel(ctrl, regs+0x0);
#ifndef CONFIG_SYS_I2C_FTI2C010_V1_16_0
		ret = fti2c010_wait(regs, SR_DT);
#else
		ret = fti2c010_wait(regs, SR_TD);
#endif
		if (ret){
			fLib_printf("Set register data failed.\n");
			break;
		}
	}
//disable
	writel( 0, regs+0x0);
	return ret;
}

void I2C_Init(I2C_RESOURCES *dev, UINT32 gsr, UINT32 tsr, UINT32 SCLout/*in unit of Khz */)
{
	dev->irq = -1;
	dev->address_mode = ADDR_MODE_8BIT;
	//setting PCLK clock  cycles
	I2C_SetTGSR(dev, gsr, tsr);
	I2C_SetSCLout(dev, SCLout);

	unsigned int ts;
	int ret = -1;
	fLib_printf("Reset i2c controller\n");
	writel(1, dev->io_base+ 0x0);
	for (ts = 0; ts < 50000; ts++) {
		if (!(readl(dev->io_base+0x0)&0x1)) {
			ret = 0;
			break;
		}
	}

	if (ret)
		fLib_printf("fti2c010: reset timeout\n");

}

/**
  \fn          int32_t I2Cx_Initialize (ARM_I2C_SignalEvent_t cb_event,
                                        I2C_RESOURCES         *i2c)
  \brief       Initialize I2C Interface.
  \param[in]   cb_event  Pointer to \ref ARM_I2C_SignalEvent
  \param[in]   i2c   Pointer to I2C resources
  \return      \ref execution_status
*/
static int32_t I2Cx_Initialize (ARM_I2C_SignalEvent_t cb_event, I2C_RESOURCES *i2c) {
  
  I2C_Init(i2c, I2C_GSR_Value, I2C_TSR_Value , I2C_SCLout);
	//  do_scan_dev(i2c, 1);
  i2c->cnt = 0;
  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I2Cx_Uninitialize (I2C_RESOURCES *i2c)
  \brief       De-initialize I2C Interface.
  \param[in]   i2c   Pointer to I2C resources
  \return      \ref execution_status
*/
static int32_t I2Cx_Uninitialize (I2C_RESOURCES *i2c) {

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I2Cx_PowerControl (ARM_POWER_STATE state,
                                          I2C_RESOURCES   *i2c)
  \brief       Control I2C Interface Power.
  \param[in]   state  Power state
  \param[in]   i2c    Pointer to I2C resources
  \return      \ref execution_status
*/
static int32_t I2Cx_PowerControl (ARM_POWER_STATE state, I2C_RESOURCES *i2c) {
  switch (state) {
    case ARM_POWER_OFF:
      break;

    case ARM_POWER_FULL:
      break;

    default:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
  }

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I2Cx_MasterTransmit (unsigned int       addr,
                                            const uint8_t *data,
                                            unsigned int       num,
                                            bool           xfer_pending,
                                            I2C_RESOURCES *i2c)
  \brief       Start transmitting data as I2C Master.
  \param[in]   addr          Slave address (7-bit or 10-bit)
  \param[in]   data          Pointer to buffer with data to transmit to I2C Slave
  \param[in]   num           Number of data bytes to transmit
  \param[in]   xfer_pending  Transfer operation is pending - Stop condition will not be generated
  \param[in]   i2c           Pointer to I2C resources
  \return      \ref execution_status
*/
static int32_t I2Cx_MasterTransmit (unsigned int       index,
                                    u8 *buf,
                                    unsigned int       num,
                                    bool           xfer_pending,
                                    I2C_RESOURCES *i2c) {

  int ret;																	
	I2C_RESOURCES *dev = i2c;
										
	if(i2c->address_mode == ADDR_MODE_16BIT){
		fLib_printf("addr:16bit\n");
		ret = fti2c010_write(dev->io_base, dev->dev_addr, index, 2, buf, num);
	}else
		ret = fti2c010_write(dev->io_base, dev->dev_addr, index, 1, buf, num);
	//everything seems fine
  dev->cnt = num; 
	if(ret ==0) return ARM_DRIVER_OK;
	return ARM_DRIVER_ERROR;
}

/**
  \fn          int32_t I2Cx_MasterReceive (unsigned int       addr,
                                           uint8_t       *data,
                                           unsigned int       num,
                                           bool           xfer_pending,
                                           I2C_RESOURCES *i2c)
  \brief       Start receiving data as I2C Master.
  \param[in]   addr          Slave address (7-bit or 10-bit)
  \param[out]  data          Pointer to buffer for data to receive from I2C Slave
  \param[in]   num           Number of data bytes to receive
  \param[in]   xfer_pending  Transfer operation is pending - Stop condition will not be generated
  \param[in]   i2c           Pointer to I2C resources
  \return      \ref execution_status
*/
static int32_t I2Cx_MasterReceive (unsigned int index,
                                   uchar        *data,
                                   unsigned int       num,
                                   bool           xfer_pending,
                                   I2C_RESOURCES *i2c) {
	int ret;
	I2C_RESOURCES *dev = i2c;

	dev->cnt = 0;
	if(i2c->address_mode == ADDR_MODE_16BIT){
		fLib_printf("addr:16bit\n");
		ret = fti2c010_read(dev->io_base,dev->dev_addr,index,2,data,num);
	}else
		ret = fti2c010_read(dev->io_base,dev->dev_addr,index,1,data,num);
	dev->cnt = num;
	if(ret == 0 )	return ARM_DRIVER_OK;
	return ARM_DRIVER_ERROR;
}

/**
  \fn          int32_t I2Cx_SlaveTransmit (const uint8_t *data,
                                           unsigned int       num,
                                           I2C_RESOURCES *i2c)
  \brief       Start transmitting data as I2C Slave.
  \param[in]   data  Pointer to buffer with data to transmit to I2C Master
  \param[in]   num   Number of data bytes to transmit
  \param[in]   i2c   Pointer to I2C resources
  \return      \ref execution_status
*/
static int32_t I2Cx_SlaveTransmit (const uint8_t *data,
                                   unsigned int       num,
                                   I2C_RESOURCES *i2c) {

  if (!data || !num) {
    /* Invalid parameters */
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I2Cx_SlaveReceive (uint8_t       *data,
                                          unsigned int       num,
                                          I2C_RESOURCES *i2c)
  \brief       Start receiving data as I2C Slave.
  \param[out]  data  Pointer to buffer for data to receive from I2C Master
  \param[in]   num   Number of data bytes to receive
  \param[in]   i2c   Pointer to I2C resources
  \return      \ref execution_status
*/
static int32_t I2Cx_SlaveReceive (uint8_t       *data,
                                  unsigned int       num,
                                  I2C_RESOURCES *i2c) {

  if (!data || !num) {
    /* Invalid parameters */ 
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I2Cx_GetDataCount (I2C_RESOURCES *i2c)
  \brief       Get transferred data count.
  \return      number of data bytes transferred; -1 when Slave is not addressed by Master
*/
static int32_t I2Cx_GetDataCount (I2C_RESOURCES *i2c) {
  return (i2c->cnt);
}

/**
  \fn          int32_t I2Cx_Control (unsigned int       control,
                                     unsigned int       arg,
                                     I2C_RESOURCES *i2c)
  \brief       Control I2C Interface.
  \param[in]   control  operation
  \param[in]   arg      argument of operation (optional)
  \param[in]   i2c      pointer to I2C resources
  \return      \ref execution_status
*/
static int32_t I2Cx_Control (unsigned int control, unsigned int arg, I2C_RESOURCES *i2c) {

   switch (control) {
    case ARM_I2C_OWN_ADDRESS:
   
      break;

    case ARM_I2C_BUS_SPEED:
      /* Set Bus Speed */
      switch (arg) {
        case ARM_I2C_BUS_SPEED_STANDARD:
          /* Standard Speed (100kHz) */
          break;
        case ARM_I2C_BUS_SPEED_FAST:
          /* Fast Speed     (400kHz) */
          break;
        case ARM_I2C_BUS_SPEED_FAST_PLUS:
          /* Fast+ Speed    (  1MHz) */
            break;
        default:
          return ARM_DRIVER_ERROR_UNSUPPORTED;
      }
      break;

    case ARM_I2C_BUS_CLEAR:
      /* Execute Bus clear */
      return ARM_DRIVER_ERROR_UNSUPPORTED;

    case ARM_I2C_ABORT_TRANSFER:
      break;
	#if 0
	case ARM_I2C_SCAN:
		fLib_printf("CONTROL: SCAN device\n");
	  do_scan_dev(i2c , 1);
	  break;
	
	case ARM_I2C_TEST_DEVICE:
		fLib_printf("CONTROL: test device\n");
	  do_test_dev(i2c , 0xa0);
	  break;
	#endif
    default:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  return ARM_DRIVER_OK;
}

/**
  \fn          ARM_I2C_STATUS I2Cx_GetStatus (I2C_RESOURCES *i2c)
  \brief       Get I2C status.
  \param[in]   i2c      pointer to I2C resources
  \return      I2C status \ref ARM_I2C_STATUS
*/
static ARM_I2C_STATUS I2Cx_GetStatus (I2C_RESOURCES *dev) {
	ARM_I2C_STATUS status;
	if((I2C_ReadStatus(dev) & SR_BUSY)== SR_BUSY)
		status.busy = 1;
	else
        	status.busy = 0;
	
	return status;
}

/**
  \fn          void I2Cx_IRQHandler (I2C_RESOURCES *i2c)
  \brief       I2C Event Interrupt handler.
  \param[in]   i2c  Pointer to I2C resources
*/
static void I2Cx_IRQHandler (I2C_RESOURCES *i2c) {
  
}

ARM_DRIVER_I2C Driver_I2C0;
ARM_DRIVER_I2C Driver_I2C1;
ARM_DRIVER_I2C Driver_I2C2;
ARM_DRIVER_I2C Driver_I2C3;

/* I2C0 Driver wrapper functions */
static int32_t I2C0_Initialize (ARM_I2C_SignalEvent_t cb_event) {
  return (I2Cx_Initialize (cb_event, Driver_I2C0.dev));
}
static int32_t I2C0_Uninitialize (void) {
  return (I2Cx_Uninitialize (Driver_I2C0.dev));
}
static int32_t I2C0_PowerControl (ARM_POWER_STATE state) {
  return (I2Cx_PowerControl (state, Driver_I2C0.dev));
}
static int32_t I2C0_MasterTransmit (unsigned int addr, uint8_t *data, unsigned int num, bool xfer_pending) {
  return (I2Cx_MasterTransmit (addr, data, num, xfer_pending, Driver_I2C0.dev));
}
static int32_t I2C0_MasterReceive (unsigned int addr, uint8_t *data, unsigned int num, bool xfer_pending) {
  return (I2Cx_MasterReceive (addr, data, num, xfer_pending, Driver_I2C0.dev));
}
static int32_t I2C0_SlaveTransmit (const uint8_t *data, unsigned int num) {
  return (I2Cx_SlaveTransmit (data, num, Driver_I2C0.dev));
}
static int32_t I2C0_SlaveReceive (uint8_t *data, unsigned int num) {
  return (I2Cx_SlaveReceive (data, num, Driver_I2C0.dev));
}
static int32_t I2C0_GetDataCount (void) {
  return (I2Cx_GetDataCount (Driver_I2C0.dev));
}
static int32_t I2C0_Control (unsigned int control, unsigned int arg) {
  return (I2Cx_Control (control, arg, Driver_I2C0.dev));
}
static ARM_I2C_STATUS I2C0_GetStatus (void) {
  return (I2Cx_GetStatus (Driver_I2C0.dev));
}
void I2C0_IRQHandler (void) {
  I2Cx_IRQHandler (Driver_I2C0.dev);
}

/* I2C0 Driver Control Block */
ARM_DRIVER_I2C Driver_I2C0 = {
  I2C_GetVersion,
  I2C_GetCapabilities,
  I2C0_Initialize,
  I2C0_Uninitialize,
  I2C0_PowerControl,
  I2C0_MasterTransmit,
  I2C0_MasterReceive,
  I2C0_SlaveTransmit,
  I2C0_SlaveReceive,
  I2C0_GetDataCount,
  I2C0_Control,
  I2C0_GetStatus,
};

/* I2C1 Driver wrapper functions */
static int32_t I2C1_Initialize (ARM_I2C_SignalEvent_t cb_event) {
  return (I2Cx_Initialize (cb_event, Driver_I2C1.dev));
}
static int32_t I2C1_Uninitialize (void) {
  return (I2Cx_Uninitialize (Driver_I2C1.dev));
}
static int32_t I2C1_PowerControl (ARM_POWER_STATE state) {
  return (I2Cx_PowerControl (state, Driver_I2C1.dev));
}
static int32_t I2C1_MasterTransmit (unsigned int addr, uint8_t *data, unsigned int num, bool xfer_pending) {
  return (I2Cx_MasterTransmit (addr, data, num, xfer_pending, Driver_I2C1.dev));
}
static int32_t I2C1_MasterReceive (unsigned int addr, uint8_t *data, unsigned int num, bool xfer_pending) {
  return (I2Cx_MasterReceive (addr, data, num, xfer_pending, Driver_I2C1.dev));
}
static int32_t I2C1_SlaveTransmit (const uint8_t *data, unsigned int num) {
  return (I2Cx_SlaveTransmit (data, num, Driver_I2C1.dev));
}
static int32_t I2C1_SlaveReceive (uint8_t *data, unsigned int num) {
  return (I2Cx_SlaveReceive (data, num, Driver_I2C1.dev));
}
static int32_t I2C1_GetDataCount (void) {
  return (I2Cx_GetDataCount (Driver_I2C1.dev));
}
static int32_t I2C1_Control (unsigned int control, unsigned int arg) {
  return (I2Cx_Control (control, arg, Driver_I2C1.dev));
}
static ARM_I2C_STATUS I2C1_GetStatus (void) {
  return (I2Cx_GetStatus (Driver_I2C1.dev));
}
void I2C1_IRQHandler (void) {
  I2Cx_IRQHandler (Driver_I2C1.dev);
}

/* I2C2 Driver wrapper functions */
static int32_t I2C2_Initialize (ARM_I2C_SignalEvent_t cb_event) {
  return (I2Cx_Initialize (cb_event, Driver_I2C2.dev));
}
static int32_t I2C2_Uninitialize (void) {
  return (I2Cx_Uninitialize (Driver_I2C2.dev));
}
static int32_t I2C2_PowerControl (ARM_POWER_STATE state) {
  return (I2Cx_PowerControl (state, Driver_I2C2.dev));
}
static int32_t I2C2_MasterTransmit (unsigned int addr, uint8_t *data, unsigned int num, bool xfer_pending) {
  return (I2Cx_MasterTransmit (addr, data, num, xfer_pending, Driver_I2C2.dev));
}
static int32_t I2C2_MasterReceive (unsigned int addr, uint8_t *data, unsigned int num, bool xfer_pending) {
  return (I2Cx_MasterReceive (addr, data, num, xfer_pending, Driver_I2C2.dev));
}
static int32_t I2C2_SlaveTransmit (const uint8_t *data, unsigned int num) {
  return (I2Cx_SlaveTransmit (data, num, Driver_I2C2.dev));
}
static int32_t I2C2_SlaveReceive (uint8_t *data, unsigned int num) {
  return (I2Cx_SlaveReceive (data, num, Driver_I2C2.dev));
}
static int32_t I2C2_GetDataCount (void) {
  return (I2Cx_GetDataCount (Driver_I2C2.dev));
}
static int32_t I2C2_Control (unsigned int control, unsigned int arg) {
  return (I2Cx_Control (control, arg, Driver_I2C2.dev));
}
static ARM_I2C_STATUS I2C2_GetStatus (void) {
  return (I2Cx_GetStatus (Driver_I2C2.dev));
}
void I2C2_IRQHandler (void) {
  I2Cx_IRQHandler (Driver_I2C2.dev);
}

/* I2C3 Driver wrapper functions */
static int32_t I2C3_Initialize (ARM_I2C_SignalEvent_t cb_event) {
  return (I2Cx_Initialize (cb_event, Driver_I2C3.dev));
}
static int32_t I2C3_Uninitialize (void) {
  return (I2Cx_Uninitialize (Driver_I2C3.dev));
}
static int32_t I2C3_PowerControl (ARM_POWER_STATE state) {
  return (I2Cx_PowerControl (state, Driver_I2C3.dev));
}
static int32_t I2C3_MasterTransmit (unsigned int addr, uint8_t *data, unsigned int num, bool xfer_pending) {
  return (I2Cx_MasterTransmit (addr, data, num, xfer_pending, Driver_I2C3.dev));
}
static int32_t I2C3_MasterReceive (unsigned int addr, uint8_t *data, unsigned int num, bool xfer_pending) {
  return (I2Cx_MasterReceive (addr, data, num, xfer_pending, Driver_I2C3.dev));
}
static int32_t I2C3_SlaveTransmit (const uint8_t *data, unsigned int num) {
  return (I2Cx_SlaveTransmit (data, num, Driver_I2C3.dev));
}
static int32_t I2C3_SlaveReceive (uint8_t *data, unsigned int num) {
  return (I2Cx_SlaveReceive (data, num, Driver_I2C3.dev));
}
static int32_t I2C3_GetDataCount (void) {
  return (I2Cx_GetDataCount (Driver_I2C3.dev));
}
static int32_t I2C3_Control (unsigned int control, unsigned int arg) {
  return (I2Cx_Control (control, arg, Driver_I2C3.dev));
}
static ARM_I2C_STATUS I2C3_GetStatus (void) {
  return (I2Cx_GetStatus (Driver_I2C3.dev));
}
void I2C3_IRQHandler (void) {
  I2Cx_IRQHandler (Driver_I2C3.dev);
}

/* I2C1 Driver Control Block */
ARM_DRIVER_I2C Driver_I2C1 = {
  I2C_GetVersion,
  I2C_GetCapabilities,
  I2C1_Initialize,
  I2C1_Uninitialize,
  I2C1_PowerControl,
  I2C1_MasterTransmit,
  I2C1_MasterReceive,
  I2C1_SlaveTransmit,
  I2C1_SlaveReceive,
  I2C1_GetDataCount,
  I2C1_Control,
  I2C1_GetStatus,
  &I2C1_Resources,
};

/* I2C1 Driver Control Block */
ARM_DRIVER_I2C Driver_I2C2 = {
  I2C_GetVersion,
  I2C_GetCapabilities,
  I2C2_Initialize,
  I2C2_Uninitialize,
  I2C2_PowerControl,
  I2C2_MasterTransmit,
  I2C2_MasterReceive,
  I2C2_SlaveTransmit,
  I2C2_SlaveReceive,
  I2C2_GetDataCount,
  I2C2_Control,
  I2C2_GetStatus,
  &I2C2_Resources,
};

/* I2C1 Driver Control Block */
ARM_DRIVER_I2C Driver_I2C3 = {
  I2C_GetVersion,
  I2C_GetCapabilities,
  I2C3_Initialize,
  I2C3_Uninitialize,
  I2C3_PowerControl,
  I2C3_MasterTransmit,
  I2C3_MasterReceive,
  I2C3_SlaveTransmit,
  I2C3_SlaveReceive,
  I2C3_GetDataCount,
  I2C3_Control,
  I2C3_GetStatus,
  &I2C3_Resources,
};
