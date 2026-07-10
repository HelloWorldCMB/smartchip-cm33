#include "string.h"
#include "io.h"
#include "leo_cm33.h"
#include "Driver_USART.h"
#include "Drv_I2C_ftiic020.h"

/* I2C Driver */
extern ARM_DRIVER_I2C Driver_I2C2;
static ARM_DRIVER_I2C * I2Cdrv = &Driver_I2C2;
extern I2C_RESOURCES I2C0_Resources;
extern I2C_RESOURCES I2C1_Resources;
extern I2C_RESOURCES I2C2_Resources;
extern I2C_RESOURCES I2C3_Resources;
extern void delay_ms(unsigned int count);
extern int fti2c010_read(int regs, u8 dev, unsigned int addr, int alen, uchar *buf, int len);

int32_t EEPROM_WriteBuf (uint16_t offset, uint8_t *buf, uint32_t len) {
 /*
  I2Cdrv->MasterTransmit (DeviceAddr, a, 2, true);
  //fLib_printf("Master Trasmit over\n");
  while (I2Cdrv->GetStatus().busy);
  */
	//fLib_printf("MastReceive start, dev_addr %x\n",I2Cdrv->dev->dev_addr);
	//i2c_probe(I2Cdrv,I2Cdrv->dev->dev_addr);
  I2Cdrv->MasterTransmit (offset, buf, len, false);
  //fLib_printf("MastReceive over\n");
  while (I2Cdrv->GetStatus().busy);
  //fLib_printf("Readbuf: datacnt %d\n",I2Cdrv->GetDataCount());	
  if (I2Cdrv->GetDataCount () != len) return -1;

	delay_ms(10);
  return 0;
}

int32_t EEPROM_ReadBuf (uint16_t offset, uint8_t *buf, uint32_t len)
{
  I2Cdrv->MasterReceive (offset, buf, len, false);
  if (I2Cdrv->GetDataCount () != len) return -1; 
  return 0;
}

void i2C_scan(void)
{	
	//I2Cdrv->dev = &I2C0_Resources;	
	int i , i2c_h, dev_num;
	fLib_printf("Scanning I2C[0]~[3]...\n");
		
	for(i2c_h=0;i2c_h < 4;i2c_h++){
		dev_num = 0;
		fLib_printf("Probing i2c[%d]\n",i2c_h);
		if(i2c_h!=3)
			I2Cdrv->dev->io_base = IIC_FTIIC010_0_PA_BASE +i2c_h*0x00100000;
		else
			I2Cdrv->dev->io_base = 0x56f00000;
		fLib_printf("i2c base[%x]\n",I2Cdrv->dev->io_base);
		I2Cdrv->Initialize(NULL);	
				
		for(i=0;i<0xff;i+=1)
		{
			if(i2c_probe(I2Cdrv,i) == 0){
				fLib_printf("i2c[%d] Got %x\n",i2c_h,i);
				dev_num++;
			}
		}
		fLib_printf("i2c[%d] Got %d Device.\n",i2c_h,dev_num);
	}
}

void dump_buf(UINT8 *buf , uint32_t len)
{
	int i;

	fLib_printf("\n------------------------------\n");
	for(i=0 ; i < len ; i++){
		if((i!=0) && (i%16==0)) fLib_printf("\n");
		fLib_printf("%02x ",buf[i]);
		
	}
	fLib_printf("\n");
}

void I2C010_main(void){
	
	UINT8 buf[256];
	int i, ret;
	int data;
	UINT8 wr_data[128];
	
	i2C_scan();
return;
	for(i = 0; i < 128; i++)
		wr_data[i] = 0x80-i;

#if 1 //this is used on i2c0 eeprom
	// EEPROM R/W test
	I2Cdrv->dev = &I2C0_Resources;
	I2Cdrv->dev->dev_addr = 0x50;
	I2Cdrv->Initialize(NULL);	

	EEPROM_WriteBuf(32,wr_data, 16);
	//delay_ms(10);
	EEPROM_WriteBuf(32+16,wr_data+16, 16);
	memset(buf,0,32);
	EEPROM_ReadBuf(32, buf, 32);
	//EEPROM_ReadBuf(32, buf, 16);
	dump_buf(buf,32);
	if(memcmp(buf, wr_data, 16)==0){
		fLib_printf("I2C eeprom write/read compare PASS\n");
	}else{
		fLib_printf("I2C eeprom write/read compare failed\n");
	}
	
#endif
#if 0 //this is used on i2c2 lcd edid
	I2Cdrv->dev = &I2C2_Resources;
	I2Cdrv->dev->dev_addr = 0x58;
	I2Cdrv->Initialize(NULL);	
	//EEPROM_ReadBuf(0, buf, 8);
	//EEPROM_ReadBuf(8, buf+8, 8);
	EEPROM_ReadBuf(0, buf, 8);
	EEPROM_ReadBuf(8, buf+8, 8);
	EEPROM_ReadBuf(16, buf+16, 8);
	EEPROM_ReadBuf(24, buf+24, 8);
	EEPROM_ReadBuf(32, buf+32, 8);
	
	dump_buf(buf,128);
#endif
	
	
}