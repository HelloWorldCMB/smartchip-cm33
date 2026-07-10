#include "io.h"
#include "leo_cm33.h"
#include "Driver_USART.h"
#include "Driver_SAI.h"
#include "wm8731.h"
#include "DrvSSP010.h"
#include "Drv_I2C_ftiic020.h"
#include "stdlib.h"

extern ARM_DRIVER_SAI Driver_SAI0;
ARM_DRIVER_SAI * SAIdrv = &Driver_SAI0;
extern I2S_RESOURCES I2S0_Resources;
extern void dump_buf(uint8_t *buf, int len);
extern void delay_ms(unsigned int count);
/* I2C Driver */
extern ARM_DRIVER_I2C Driver_I2C1;
static ARM_DRIVER_I2C * I2Cdrv = &Driver_I2C1;

void I2S_test_main(void)
{
	char *buf;
	fLib_printf("[I2S test]\n");
	int data, i, base_addr;
	char str_buf[16];
	int opt;
	int i2c_slave_addr=0;
  
	fLib_printf("Select i2s 0/1 to tset [0/1]\n");
	fLib_gets(DEBUG_CONSOLE, (char*)str_buf);
	opt = atoi((char*)str_buf);
	
	if (opt == 0)
	{
	    base_addr =  I2S_FTSSP010_0_PA_BASE;
		  fLib_printf("Selected i2s0\n");
		  i2c_slave_addr = 0x1a;
	}else
	{
		  base_addr =  I2S_FTSSP010_1_PA_BASE;
		  i2c_slave_addr = 0x1b;
		  fLib_printf("Selected i2s1\n");
	}
	
	
	SAIdrv->dev = &I2S0_Resources;
	SAIdrv->dev->iobase = base_addr;
	SAIdrv->Initialize(NULL);
	I2Cdrv->Initialize(NULL);											
	wm8510_init(CODEC_AS_MASTER, i2c_slave_addr);

	fLib_SSPClearTxFIFO(base_addr);
	fLib_SSPClearRxFIFO(base_addr);	
	
	char key;	
	#if 1
	fLib_printf("!!Set jumper to RX side!!\n");
	while(1){
		fLib_printf("Enter \"r\" to recording (%d bytes)\n",I2S_TOTAL_SIZE);
		key = fLib_getchar(DEBUG_CONSOLE);
		if(key=='r'){
			fLib_printf("Recording...\n");
			SAIdrv->Receive((unsigned int *)I2S_MEM_BASE, I2S_TOTAL_SIZE);
			break;
			
		}
		if(key==0x1b)
			break;
	}
	fLib_printf("!!Set jumper to TX side!!\n");
	#endif
	while(1){
		fLib_printf("Enter \"p\" to playback (%d bytes)\n",I2S_TOTAL_SIZE);
		key = fLib_getchar(DEBUG_CONSOLE);
		if(key=='p'){
			fLib_printf("Playing...(\"ESC\" to exit)\n");
			SAIdrv->Send((unsigned int*)I2S_MEM_BASE, I2S_TOTAL_SIZE);
		}
		if(key==0x1b)
			break;
	}
	
	fLib_printf("I2S test end\n");
}