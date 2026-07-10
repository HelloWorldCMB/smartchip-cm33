#include <string.h>
#include <Driver_Common.h>
#include "Drv_I2C_ftiic020.h"
#include "common_include.h"
#include "leo_cm33.h"

#include "wm8731.h"

/* I2C Driver */
extern ARM_DRIVER_I2C Driver_I2C1;
static ARM_DRIVER_I2C * I2Cdrv = &Driver_I2C1;

/*
 * WM8731 is a write only device in I2C bus.
 * A input pin, named CSB, is used to select one of two slave address
 *    CSB	Address
 *    ---	-------
 *    0		0011010
 *    1		0011011
 *
 * The faraday WM8731 codec module ties this CSB pin to low by R21, so
 * the write address of WM8731 will be 0x34

 WM8731 : 0x34/0x35, 0x36/0x37(default)


 */
#define WM8731WA	0x36
#define VOL_SIZE(S)          (S==1) ? 0 : ((0x8*(S-1))-1)

UINT32 wm8510_input = 0;

/*
 * WM8731 I2C control interface
 *
 * START<7-bit ADDR><R/W><ACK><DATA B15-8><ACK><DATA B7-0><ACK>STOP
 *
 * B[15..9] are Register Address Bits
 * B[8:0] are Register Data Bits
 * 
 */
static int i2c_write (unsigned short data)
{
#if 0	
    int i;
		i = I2C_ByteWrite (wm_i2c_dev, WM8510WA, (data>>8), (data&0xff));
    if ( -1 == i )
    {
        fLib_printf("i2c write fail\n");
    }
		return i;
#else
		fLib_printf("i2c write data:0x%x\n", data);
		uint8_t wr_buf = data&0xff;
		uint16_t index = data >>8;
		int ret;


		//I2Cdrv->dev->dev_addr = (I2Cdrv->dev->dev_addr)&(~0x1);
		fLib_printf("I2C io_base %x, addr=%x, index=%x\n", I2Cdrv->dev->io_base,I2Cdrv->dev->dev_addr, index);
		ret = I2Cdrv->MasterTransmit (index, &wr_buf, 1 , false);
		while (I2Cdrv->GetStatus().busy);
		if(ret != ARM_DRIVER_OK) return -1;
		return 1;
#endif		
	
}


/*
 * WM8731 I2C control interface
 *
 * START<7-bit ADDR><R/W><ACK><DATA B15-8><ACK><DATA B7-0><ACK>STOP
 *
 * B[15..9] are Register Address Bits
 * B[8:0] are Register Data Bits
 * 
 */
static unsigned int i2c_read (unsigned short addr)
{
	u8 buf;

	//return I2C_Read(wm_i2c_dev, WM8510WA, addr);
	fLib_printf("MastReceive start, dev_addr %x\n",I2Cdrv->dev->dev_addr);
	//i2c_probe(I2Cdrv,I2Cdrv->dev->dev_addr);
	I2Cdrv->dev->dev_addr = (I2Cdrv->dev->dev_addr)|0x1;
  I2Cdrv->MasterReceive (addr, &buf, 1, false);
	
	return buf;
}


/* ----------------------------------------------------------------------
 * Codec register control functions
 * ----------------------------------------------------------------------
 */
int wm8510SetSamplingCtrl(unsigned int rate)
{	
	static unsigned short regMW8510R7ctrl = KR7VALUE;
	
	if(rate == 8000)
	{
		regMW8510R7ctrl |= 0xA;
	}
	
	return i2c_write (regMW8510R7ctrl);
}


/*ioctl to set volume by config file vol field*/
void wm8510_init_volume(unsigned int level)
{
    unsigned int value;

    if (level<1 || level>32)
    {
		fLib_printf("volume level wrong\n");
      	return;
    }
    else
		value=VOL_SIZE(level);

    value=((KR11<<KAddrShift) | value);
    i2c_write((unsigned short)value);
}

/*keypad pressed to adjust volume*/
void wm8510_set_volume(unsigned int level)
{
    unsigned int value;

    value=i2c_read(0xb); 
    value=value&0xff;

    if(value==0 || value==255)
		return;
		
    if(level == 1)
    {
        value=((KR11<<KAddrShift) | (value+1));
        i2c_write ((unsigned short)value);
    }
    else
    {
        value=((KR11<<KAddrShift) | (value-1));
        i2c_write ((unsigned short)value);
    }
}

#define uint16_t	unsigned short


void wm8510_init(CODEC_MODE_T mode, int i2c_slave_addr)
{
	UINT32 srcin;
	
	I2Cdrv->dev->io_base = IIC_FTIIC010_1_PA_BASE;
	I2Cdrv->dev->dev_addr = i2c_slave_addr;
/* Reset*/
	i2c_write ((uint16_t)KR15VALUE);
		;

	i2c_write ((uint16_t)KR6VALUE); 

	/* Mode control and bit length = 16 bits */
	if(mode == CODEC_AS_MASTER)
		i2c_write ((uint16_t)(KR7VALUE|0x40));
	else
	i2c_write ((uint16_t)KR7VALUE);
	
	/* MCLK = 12.288MHz and FS = 48KHz */
	i2c_write ((uint16_t)KR8VALUE); 
	i2c_write ((uint16_t)KR9VALUE);
re1:
	if (!wm8510_input)
	{
		fLib_printf("Select :1) MIC-in    2) Line-in    3) Other\r\n");
	}
	else
	{
		srcin=wm8510_input;
		goto select_done;
	}
	
	srcin = 1;

select_done:
	if(srcin == 1)
	{
		fLib_printf("Source =  MIC\r\n");
		i2c_write ((uint16_t)KR4VALUEM);  //Choice LIne-in or MIC-in
	}
	else if(srcin == 2)
	{
		fLib_printf("Source =  Line\r\n");
		i2c_write ((uint16_t)KR4VALUEL);
	}
	else if(srcin == 3)
	{
		fLib_printf("Source =  Other\r\n");
		i2c_write ((uint16_t)KR4VALUEO);	
	}
	else
	{
		fLib_printf("Keyin error!! again\r\n");
		goto re1;
	}
	i2c_write ((uint16_t)KR0VALUE);
	i2c_write ((uint16_t)KR1VALUE);
	i2c_write ((uint16_t)KR5VALUE);
	fLib_printf("wm8731 init done\n");
	//i2c_write ((uint16_t)KR2VALUE);
	//i2c_write ((uint16_t)KR3VALUE); 
	//fLib_printf("R0 status = %x\n",i2c_read(0x0));
	//fLib_printf("R6 status = %x\n",i2c_read(0x6));
	
  /*      i2c_write ((uint16_t)KR2VALUE);  
        i2c_write ((uint16_t)KR3VALUE);  
        i2c_write ((uint16_t)KR4VALUE);  
        i2c_write ((uint16_t)KR5VALUE); 
        i2c_write ((uint16_t)KR6VALUE);  
        i2c_write ((uint16_t)KR7VALUE);  
        i2c_write ((uint16_t)KR8VALUE); 
	*/	 
	
	
		/*
        i2c_write ((uint16_t)KR10VALUE); 
        i2c_write ((uint16_t)KR11VALUE); 
        i2c_write ((uint16_t)KR14VALUE); 
        i2c_write ((uint16_t)KR15VALUE);
        i2c_write ((uint16_t)KR18VALUE); 
        i2c_write ((uint16_t)KR19VALUE); 
        i2c_write ((uint16_t)KR20VALUE);
        i2c_write ((uint16_t)KR21VALUE); 
        i2c_write ((uint16_t)KR22VALUE); 
        i2c_write ((uint16_t)KR24VALUE); 
        i2c_write ((uint16_t)KR25VALUE); 
        i2c_write ((uint16_t)KR27VALUE); 
        i2c_write ((uint16_t)KR28VALUE); 
        i2c_write ((uint16_t)KR29VALUE); 
        i2c_write ((uint16_t)KR30VALUE); 
        i2c_write ((uint16_t)KR32VALUE); 
        i2c_write ((uint16_t)KR33VALUE); 
        i2c_write ((uint16_t)KR34VALUE); 
        i2c_write ((uint16_t)KR35VALUE); 
        i2c_write ((uint16_t)KR36VALUE); 
        i2c_write ((uint16_t)KR37VALUE); 
        i2c_write ((uint16_t)KR38VALUE); 
        i2c_write ((uint16_t)KR39VALUE); 
        i2c_write ((uint16_t)KR44VALUE); 
        i2c_write ((uint16_t)KR45VALUE); 
        i2c_write ((uint16_t)KR47VALUE); 
        i2c_write ((uint16_t)KR49VALUE); 
        i2c_write ((uint16_t)KR50VALUE); 
        i2c_write ((uint16_t)KR54VALUE); 
        i2c_write ((uint16_t)KR56VALUE); 
        */
}


