#include "Driver_Common.h"
#include "io.h"
#include "leo_cm33.h"							// Device header
#include "Driver_USART.h"
bool UART0_Rx = false;
bool UART1_Rx = false;


void UART0_callback(uint32_t event)
{	
	if(event & ARM_USART_EVENT_RECEIVE_COMPLETE)
	{		
		UART0_Rx = true;			
	}
}
void UART1_callback(uint32_t event)
{	
	if(event & ARM_USART_EVENT_RECEIVE_COMPLETE)
	{		
		UART1_Rx = true;			
	}
}
static void PC_Loopback_Mode(void)
{
  char chr;	
	uint32_t len = 0;
	char uart_buff[64];
		
	fLib_printf("+-----------------------------------------------------------+\n");
	fLib_printf("|       Enter characters and finally press Enter keystroke  |\n");
	fLib_printf("+-----------------------------------------------------------+\n");	
		
	//uart_buff[0] = 0x50;
	//uart_buff[1] = 0x51;
	//Driver_USART0.Send(uart_buff, 2);

	while(1)
	{
		Driver_USART0.Receive(&chr, 1);		
		if(UART0_Rx)
		{
			UART0_Rx = false;		
			
			if(chr == 0x1B)
				break;			

			if(chr == 0x0D)
			{				
				//fLib_printf("Enter\n");
				Driver_USART0.Send(uart_buff, len);
				len = 0;
			}
			else
				uart_buff[len++] = chr;			
		}
		
	}	

	Driver_USART0.Control(ARM_USART_ABORT_TRANSFER, 0);
}

void UART_Pinmux(UINT8 UART_PORT)
{
	int data;
	if(UART_PORT == 0){} // default
	else if(UART_PORT == 1){
		data = inw(0xc238001c);
		outw(0xc238001c, data | 0x1000);
		data = inw(0xc2380144);
		outw(0xc2380144, data | 0x4);
		data = inw(0xc2380148);
		outw(0xc2380148, data | 0x4);
	}
	else if(UART_PORT == 2){
		data = inw(0xc238001c);
		outw(0xc238001c, data | 0x2000);
		data = inw(0xc238017c);
		outw(0xc238017c, data | 0x6);
		data = inw(0xc2380180);
		outw(0xc2380180, data | 0x6);
	}
	else if(UART_PORT == 3){
		data = inw(0xc238001c);
		outw(0xc238001c, data | 0x4000);
		data = inw(0xc2380184);
		outw(0xc2380184, data | 0x6);
		data = inw(0xc2380188);
		outw(0xc2380188, data | 0x6);
	}
	else if(UART_PORT == 4){
		data = inw(0xc238001c);
	outw(0xc238001c, data | 0x8000);
	data = inw(0xc238018c);
	outw(0xc238018c, data | 0x6);
	data = inw(0xc2380190);
	outw(0xc2380190, data | 0x6);
	}
	else 
		fLib_printf("ERR\n");
}

void UART_Init(void)
{
	if(inw(0xE00FF01C)==0x4e430000){
		//NCPU
		UART_Pinmux(NCPU_DEBUG_CONSOLE);
		Driver_USART1.Initialize(UART1_callback);	
		Driver_USART1.PowerControl(ARM_POWER_FULL);
		Driver_USART1.Control(ARM_USART_MODE_ASYNCHRONOUS | 
													ARM_USART_DATA_BITS_8 |
													ARM_USART_PARITY_NONE |
													ARM_USART_STOP_BITS_1 |
													ARM_USART_FLOW_CONTROL_NONE, DEFAULT_CONSOLE_BAUD/*115200*/);

	}
	if(inw(0xE00FF01C)==0x53430000){
		//SCPU
		UART_Pinmux(DEBUG_CONSOLE);
		Driver_USART0.Initialize(UART0_callback);	
		Driver_USART0.PowerControl(ARM_POWER_FULL);
#ifdef UART_FLOWCONTROL
		Driver_USART0.Control(ARM_USART_MODE_ASYNCHRONOUS | 
												ARM_USART_DATA_BITS_8 |
	                      ARM_USART_PARITY_NONE |
	                      ARM_USART_STOP_BITS_1 |
												ARM_USART_FLOW_CONTROL_RTS_CTS, DEFAULT_CONSOLE_BAUD/*115200*/);

#else
		Driver_USART0.Control(ARM_USART_MODE_ASYNCHRONOUS | 
													ARM_USART_DATA_BITS_8 |
													ARM_USART_PARITY_NONE |
													ARM_USART_STOP_BITS_1 |
													ARM_USART_FLOW_CONTROL_NONE, DEFAULT_CONSOLE_BAUD/*115200*/);
#endif
	}
	NVIC_EnableIRQ(UART_FTUART010_0_IRQ);
}
