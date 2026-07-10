#include "DrvPWMTMR010.h"
#include "DrvUART010.h"
#include "leo_cm33.h"
#include "io.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include "DrvDMAC020.h"


#define BUF_SIZE 4
extern void delay_ms(unsigned int count);

extern volatile UINT32 T4_Tick;

typedef struct uart_dma_req_mode{   
	int dma_txreq;
	int dma_rxreq;
	int mode;
} UART_DMA_REQ_MODE;


void prepare_buf(char *buf)
{
	int i;
	
	for(i=0;i< BUF_SIZE;i++){
		*(buf+i) =0x34+i;//(UINT8)(buf+i); 
	}		
}

void switch_mode_register(int mode)
{
	int val;
	int base = SCU_MODE_REG;
	return;
	//bit 1:0 to 0
	val = readl(base);
	val &=~0xc0000000;
	val |= (mode<<30);
	writel(val, base);
}

void enable_uart_dma(int base)
{
	//enable fcr dma_en;
	int val;
	
	val = readl(base+0x10);
	val |= (0x1<<5); //enable dma
	writel(val,base+0x10);
}

void UART_Find_DMA_REQ(UINT32 base, struct uart_dma_req_mode *ptr)
{
	int choice;
	char buf[64];
	
	switch(base)
	{
		case UART_FTUART010_0_PA_BASE:
			ptr->dma_txreq = UART_FTUART010_0_DMA_TXREQ;
		  ptr->dma_rxreq = UART_FTUART010_0_DMA_RXREQ;
		  ptr->mode = UART_FTUART010_0_DMA_MODE_MUX;
			break;
		
		case UART_FTUART010_1_PA_BASE:
			ptr->dma_txreq = UART_FTUART010_1_DMA_TXREQ;
		  ptr->dma_rxreq = UART_FTUART010_1_DMA_RXREQ;
		  ptr->mode = UART_FTUART010_1_DMA_MODE_MUX;
			break;
		
		case UART_FTUART010_2_PA_BASE:
			ptr->dma_txreq = UART_FTUART010_2_DMA_TXREQ;
		  ptr->dma_rxreq = UART_FTUART010_2_DMA_RXREQ;
		  ptr->mode = UART_FTUART010_2_DMA_MODE_MUX;		
			break;
		
		case UART_FTUART010_3_PA_BASE:
			ptr->dma_txreq = UART_FTUART010_3_DMA_TXREQ;
		  ptr->dma_rxreq = UART_FTUART010_3_DMA_RXREQ;
		  ptr->mode = UART_FTUART010_3_DMA_MODE_MUX;
			break;
		
		case UART_FTUART010_4_PA_BASE:
			fLib_printf("Please Select mode[1/3]: ");
			fLib_gets(DEBUG_CONSOLE, (char*)buf);
			choice = atoi((char*)buf);
			fLib_printf("\n");				
		  switch(choice){
				case 1:
					ptr->dma_txreq = UART_FTUART010_4_DMA_TXREQ;
					ptr->dma_rxreq = UART_FTUART010_4_DMA_RXREQ;
					ptr->mode = UART_FTUART010_4_DMA_MODE_MUX_1;
				break;
				default:
					ptr->dma_txreq = UART_FTUART010_4_MODE_3_DMA_TXREQ;
					ptr->dma_rxreq = UART_FTUART010_4_MODE_3_DMA_RXREQ;
					ptr->mode = UART_FTUART010_4_DMA_MODE_MUX_3;
			}
			break;
		
		case UART_FTUART010_5_PA_BASE:
			fLib_printf("Please Select mode[1/3]: ");
			fLib_gets(DEBUG_CONSOLE, (char*)buf);
			choice = atoi((char*)buf);
			fLib_printf("\n");				
		  switch(choice){
				case 1:
					ptr->dma_txreq = UART_FTUART010_5_DMA_TXREQ;
					ptr->dma_rxreq = UART_FTUART010_5_DMA_RXREQ;
					ptr->mode = UART_FTUART010_5_DMA_MODE_MUX;					
				break;
				default:
					ptr->dma_txreq = UART_FTUART010_5_DMA_TXREQ_3;
					ptr->dma_rxreq = UART_FTUART010_5_DMA_RXREQ_3;
					ptr->mode = UART_FTUART010_5_DMA_MODE_MUX_3;					
				break;		
			}
			break;
		
		case UART_FTUART010_6_PA_BASE:
			ptr->dma_txreq = UART_FTUART010_6_DMA_TXREQ;
		  ptr->dma_rxreq = UART_FTUART010_6_DMA_RXREQ;
		  ptr->mode = UART_FTUART010_6_DMA_MODE_MUX;
			break;				

		case UART_FTUART010_7_PA_BASE:
			ptr->dma_txreq = UART_FTUART010_7_DMA_TXREQ;
		  ptr->dma_rxreq = UART_FTUART010_7_DMA_RXREQ;
		  ptr->mode = UART_FTUART010_7_DMA_MODE_MUX;
			break;				
		
		case UART_FTUART010_8_PA_BASE:
			ptr->dma_txreq = UART_FTUART010_8_DMA_TXREQ;
		  ptr->dma_rxreq = UART_FTUART010_8_DMA_RXREQ;
		  ptr->mode = UART_FTUART010_8_DMA_MODE_MUX;
			break;				
		
		case UART_FTUART010_9_PA_BASE:
			ptr->dma_txreq = UART_FTUART010_9_DMA_TXREQ;
		  ptr->dma_rxreq = UART_FTUART010_9_DMA_RXREQ;
		  ptr->mode = UART_FTUART010_9_DMA_MODE_MUX;
			break;				
		
		
	}
	fLib_printf("%s txreq %x rxreq %x mode %x\n",__func__,ptr->dma_txreq,ptr->dma_rxreq,ptr->mode);
}

int uart_id_to_base(int id)
{
	switch(id)
	{
		case 0:
			return UART_FTUART010_0_PA_BASE;
		case 1:
			return UART_FTUART010_1_PA_BASE;
		case 2:
			return UART_FTUART010_2_PA_BASE;
		case 3:
			return UART_FTUART010_3_PA_BASE;
		case 4:
			return UART_FTUART010_4_PA_BASE;
		case 5:
			return UART_FTUART010_5_PA_BASE;
		case 6:
			return UART_FTUART010_6_PA_BASE;
		case 7:
			return UART_FTUART010_7_PA_BASE;
		case 8:
			return UART_FTUART010_8_PA_BASE;
		case 9:
			return UART_FTUART010_9_PA_BASE;
		default:
			return UART_FTUART010_0_PA_BASE;
		
	}
}

void uart_reset_fifo(int base)
{
	int val;
	
	val = readl(base+8);
	val |=6;
	writel(val, base+8);	
}

void uart_enable_loopback(int base)
{
	int val;
	
	val = readl(base+0x10);
	val |= (0x1<<4); //loopback
	writel(val, base+0x10);
}

void uart_set_mode(int base)
{
	int val;
	
	val = readl(base+0x20);
	val &= ~0x13;
	val |= (0x1);//sir
	writel(val, base+0x20);
  //enable TX/RX of IRD	
	val = readl(base+0x24);
	val &= ~0x3;
	val |= (0x3);
	writel(val, base+0x24);
	
}


void uart_dma_test_main(void)
{
	int i;
	char chr;
	char buf[BUF_SIZE];
	char uart_buf[64];
	int src_width = 0;//source width 8/16/32 bits -> 0/1/2
	int dst_width = 0;//0;//dst width 8/16/32 bits -> 0/1/2
	int burst_size = 0;
	int src_ctrl = 0 ;// Inc/dec/fixed --> 0/1/2
	int dst_ctrl = 2 ;// Inc/dec/fixed --> 0/1/2
	int priority = 0;
	int hw = 1;	
	int choice;
	//int mode = 0; //this is scu hardware handshake mode switch(0x50001000)
	struct uart_dma_req_mode req_mode;
	int base;
	
	for(i=0;i< DRVUART_MAX_UART;i++)
	{
		if(i==2) continue;		//skip main console output
//	  fLib_SerialInit((DRVUART_PORT)i ,DEFAULT_CONSOLE_BAUD, PARITY_NONE, 0, 8);	
	}
	
	fLib_printf("Please Select which uart[0~9]: ");

	fLib_gets(DEBUG_CONSOLE, (char*)uart_buf);
	choice = atoi((char*)uart_buf);
	fLib_printf("\n");		
	
	base = uart_id_to_base(choice);
	UART_Find_DMA_REQ(base, &req_mode);
	readl(base);
	//prepare data bufer for dmac020	
	prepare_buf(buf);
	
	if((base == UART_FTUART010_5_PA_BASE) || (base == UART_FTUART010_8_PA_BASE))
	{ 
		uart_reset_fifo(UART_FTUART010_5_PA_BASE);
		uart_reset_fifo(UART_FTUART010_8_PA_BASE);
		uart_set_mode(UART_FTUART010_5_PA_BASE);
		uart_set_mode(UART_FTUART010_8_PA_BASE);
		//uart_enable_loopback(base);
	}
	/*****now let's verify tx ***/
	
	//switch system mode register
	switch_mode_register(req_mode.mode);
	//since uart is already initialized, just enable DMA 
	enable_uart_dma(base);
	//send some data by using dma020 API;
  fLib_InitDMA(FALSE, FALSE, 0x0);	
	fLib_DMA_ClearAllInterrupt(); 
	fLib_DMA_EnableDMAInt(); //Enable DMA Interrupt			
	fLib_DMA_NormalMode(0,(UINT32)buf, base,4, src_width , dst_width, burst_size, src_ctrl,dst_ctrl ,priority, hw,req_mode.dma_txreq);
	fLib_EnableDMAChannel(0);
	fLib_DMA_WaitDMAInt(0);
	fLib_DisableDMAChannel(0);	
	//check if uart5 / 8 
	if((base == UART_FTUART010_5_PA_BASE) || (base == UART_FTUART010_8_PA_BASE))
	{		  
		  delay_ms(5);
		  int ch;

			//read data from other side
		  if(base == UART_FTUART010_5_PA_BASE)
				ch = readb(UART_FTUART010_8_PA_BASE);
			else
				ch = readb(UART_FTUART010_5_PA_BASE);
				
			if((buf[0]) != ch)
			{
				
				fLib_printf("UART test fail, buf[0] %x read %x\n",buf[0],ch);
				fLib_printf("read again %x %x\n",readb(base),readb(base));
				return;
			}
			fLib_printf("IRDA tx test :[success]\n");
	}
  //fLib_printf("Read uart fifo back , buf0 %x buf1 %x\n",readl(base),readl(base));	
  //verify rx 
	fLib_printf("Press a input from other UART(except uart5,8)\n");
	if((base == UART_FTUART010_5_PA_BASE) || (base == UART_FTUART010_8_PA_BASE))
	{			
		choice = T4_Tick&0xff;
		fLib_printf("Write tick  %x into data port\n",choice);
		uart_reset_fifo(base);
		if(base == UART_FTUART010_5_PA_BASE)
			writeb(choice, UART_FTUART010_8_PA_BASE);
		else
			writeb(choice, UART_FTUART010_5_PA_BASE);
		
	}
	delay_ms(2);
	fLib_DMA_ClearAllInterrupt(); 
	fLib_DMA_EnableDMAInt(); //Enable DMA Interrupt			
	src_ctrl = 2 ;// Inc/dec/fixed --> 0/1/2
	dst_ctrl = 0 ;// Inc/dec/fixed --> 0/1/2	

	//just 1 byte
	fLib_DMA_NormalMode(1, base, (UINT32)buf, 1, src_width , dst_width, burst_size, src_ctrl,dst_ctrl ,priority, hw,req_mode.dma_rxreq);
	fLib_EnableDMAChannel(1);
	fLib_DMA_WaitDMAInt(1);
	fLib_DisableDMAChannel(1);
	fLib_printf("\n received %x\n",buf[0]);
	if((base == UART_FTUART010_5_PA_BASE) || (base == UART_FTUART010_8_PA_BASE))
	{
			if(choice != buf[0])
				fLib_printf("UART rx test fail, choice %x , buf0 %x\n",choice, buf[0]);
			else
				fLib_printf("IRDA rx test :[success]\n");
	}
}