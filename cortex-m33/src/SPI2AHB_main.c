#include "Common_Include.h"
#include "ftssp010.h"

extern void SPI2AHB_Read_Dma(UINT32 base, UINT32 *buf, UINT32 byte_addr);
extern void delay_ms(unsigned int count);
extern int spi_choice;


void DumpData(INT32U *pp, INT32U start_addr, INT32U size)
{
    INT32U i=0;
	do
	{                        
		if((i%4)==0)
		{
			fLib_printf("\n");
			fLib_printf("[0x%08x]:",start_addr);
			start_addr+=16;
		}
		fLib_printf(" 0x%08x", *(pp++));
		i++;
	}while((pp!=NULL)&&(i<size));
	fLib_printf("\n");
}

int spi_id_to_base(int id)
{
	switch(id)
	{
		case 0:
			return SPI_FTSSP010_0_PA_BASE;
		case 1:
			return SPI_FTSSP010_1_PA_BASE;
		case 2:
			return SPI_FTSSP010_2_PA_BASE;
		case 3:
			return SPI_FTSSP010_3_PA_BASE;
		case 4:
			return SPI_FTSSP010_4_PA_BASE;
		case 5:
			return SPI_FTSSP010_5_PA_BASE;
		case 6:
			return SPI_FTSSP010_6_PA_BASE;
		default:
			return SPI_FTSSP010_0_PA_BASE;
		
	}
}
void SPI2AHB_test_main(void)
{
	UINT32 i = 0;
	int byte_addr, spim, data;
	int num;
	UINT32 buf[128];
	int argc;
	char *argv[40];
	UINT32 base_addr;
	UINT32 rd_buf[256];
	UINT32 wr_buf;
	int choice;
	
	fLib_printf("Please Select which spi[0~6]: ");

	fLib_gets(DEBUG_CONSOLE, (char*)buf);
	choice = atoi((char*)buf);
	fLib_printf("\n");		
	
	base_addr = spi_id_to_base(choice);
	FTSSP010_REG_BASE_M = base_addr;
	
	if((choice == 3) || (choice==4)){
			fLib_printf("Please Select mode[1/3]: ");
			fLib_gets(DEBUG_CONSOLE, (char*)buf);
			spi_choice = atoi((char*)buf);
	}
	
	fLib_printf("+----------------------------------------------------------+\n");
  fLib_printf("|                   SPI2AHB Test                           |\n");
  fLib_printf("+----------------------------------------------------------+\n");
	fLib_printf("Revision:%x\n", inw(base_addr+0x60));
	 
	//Initialize SSP010 contorller
	ftssp010_SPI2AHB_master_init(0);

	for(;;)
	{
	  fLib_printf("+----------------------------------------------------------+\n");
		fLib_printf("usage(hexadecimal value must have prefix 0x)\n");
		fLib_printf("    r [word address] (Ex: r 0x10100000)\n");
		fLib_printf("    w [word address] [data](Ex: w 0x10100000 0x12345678)\n");			
		fLib_printf("    exit\n");
		fLib_printf("please input command: ");
		fLib_gets(DEBUG_CONSOLE, (char *)buf);
		fLib_printf("\n");
		argc = substring(argv, (char *)buf, " \r\n\t");
		
		if (strcmp(argv[0], "r")==0)
		{
			memset(rd_buf, 0, sizeof(rd_buf));
			byte_addr = atonum(argv[1]);
			if(argc < 2) 
			{
				fLib_printf("invalid parameters!\n");
				continue;
			}

			if(argc == 2){ 
				num = 1;// single read
				SPI2AHB_Read_Dma(base_addr, rd_buf, byte_addr);
				delay_ms(20);
				memset(rd_buf, 0, sizeof(rd_buf));
				SPI2AHB_Read_Dma(base_addr, rd_buf, byte_addr);
			}
			/*else{
				num = atonum(argv[2]);
				SPI2AHB_Multi_Read(base_addr, rd_buf, byte_addr, num);
			}*/
			
			DumpData((UINT32*)rd_buf, byte_addr, num);
		}

		else if (strcmp(argv[0], "w")==0)
		{
			if(argc < 3) 
			{
				fLib_printf("invalid parameters!\n");
				continue;
			}
			
			byte_addr = atonum(argv[1]);

			wr_buf = atonum(argv[2]);
			num = 1; // single write
			
			SPI2AHB_Write(base_addr, wr_buf, byte_addr, num);
			fLib_printf("==write end==\n");
		}
		
		else if (strcmp(argv[0], "exit")==0)
		{
			return;
		}
	}	
}
