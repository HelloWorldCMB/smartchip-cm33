/***************************************************************************
* Copyright  Faraday Technology Corp 2002-2003.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:serial.c                                                            *
* Description: serial library routine                                      *
* Author:                                                        *
****************************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

#include "io.h"
//#include "DrvUART010.h"
#include "leo_cm33.h"
#include "Driver_USART.h"

FT_UART0_Type *UART_PORT[DRVUART_MAX_UART] = {FT_UART0, FT_UART1, FT_UART2, FT_UART3, FT_UART4, FT_UART5,FT_UART6,FT_UART7,FT_UART8,FT_UART9};

void fLib_SetSerialMode(DRVUART_PORT port_no, uint32_t mode)
{
	UART_PORT[port_no]->MDR &= ~SERIAL_MDR_MODE_SEL;
  UART_PORT[port_no]->MDR |= mode;
}


void fLib_EnableIRMode(DRVUART_PORT port_no, uint32_t TxEnable, uint32_t RxEnable)
{
	UART_PORT[port_no]->ACR &= ~(SERIAL_ACR_TXENABLE | SERIAL_ACR_RXENABLE);

	if(TxEnable)
      UART_PORT[port_no]->ACR |= SERIAL_ACR_TXENABLE;

	if(RxEnable)
      UART_PORT[port_no]->ACR |= SERIAL_ACR_RXENABLE;
}

/*-----------------------------------------------------------------------------
  Function:		fLib_SerialInit

  Parameter:
	        	None
  Returns:
               	None
  Description:
               	Initialize UART0, 38400bps, 8N1.
 *-----------------------------------------------------------------------------*/

void fLib_SerialInit (DRVUART_PORT port_no, uint32_t baudrate, uint32_t parity,uint32_t num,uint32_t len)
{
	uint32_t lcr;


	lcr = UART_PORT[port_no]->LCR & ~SERIAL_LCR_DLAB;
	/* Set DLAB=1 */
	UART_PORT[port_no]->LCR = SERIAL_LCR_DLAB;

	/* Set baud rate */
	baudrate = (port_no == DRVUART_PORT0 || port_no == DRVUART_PORT3)?(UART_CLOCK/baudrate):(UART_CLOCK_2/baudrate);
	baudrate >>= 4;	//divided by 16
	UART_PORT[port_no]->DLM = ((baudrate & 0xff00) >> 8);
	UART_PORT[port_no]->DLL = (baudrate & 0xff);

	//clear orignal parity setting
	lcr &= 0xc0;
	#ifdef UART_FLOWCONTROL
	UART_PORT[port_no]->IER = 0x30; //rts flow control enable
	//UART_PORT[port_no]->MCR = 0x1;
	//outw(0xc140010,0x1);
	#endif
	switch (parity) {
	case PARITY_NONE:
		//do nothing
		break;
	case PARITY_ODD:
		lcr |= SERIAL_LCR_ODD;
		break;
	case PARITY_EVEN:
		lcr |= SERIAL_LCR_EVEN;
		break;
	case PARITY_MARK:
		lcr |= (SERIAL_LCR_STICKPARITY | SERIAL_LCR_ODD);
		break;
	case PARITY_SPACE:
		lcr |= (SERIAL_LCR_STICKPARITY | SERIAL_LCR_EVEN);
		break;
	default:
		break;
	}

	if(num==2)
		lcr|=SERIAL_LCR_STOP;

	len-=5;
	lcr|=len;
	UART_PORT[port_no]->LCR = lcr;
}

void fLib_SetSerialLoopback(DRVUART_PORT port_no, uint32_t onoff)
{
	if(onoff==ON)
		UART_PORT[port_no]->MCR |= SERIAL_MCR_LPBK;
	else
		UART_PORT[port_no]->MCR &= ~(SERIAL_MCR_LPBK);
}

void fLib_SetSerialFifoCtrl(DRVUART_PORT port_no, uint32_t level_tx, uint32_t level_rx, uint32_t resettx, uint32_t resetrx)  //V1.20//ADA10022002
{
	uint8_t fcr = 0;


 	fcr |= SERIAL_FCR_FE;

 	switch(level_rx)  //V1.20//ADA10022002//Start
 	{
 		case 4:
 			fcr|=0x40;
 			break;
 		case 8:
 			fcr|=0x80;
 			break;
 		case 14:
 			fcr|=0xc0;
 			break;
 		default:
 			break;
 	}
  //V1.20//ADA10022002//Start
 	switch(level_tx)
 	{
 		case 3:
 			fcr|=0x01<<4;
 			break;
 		case 9:
 			fcr|=0x02<<4;
 			break;
 		case 13:
 			fcr|=0x03<<4;
 			break;
 		default:
 			break;
 	}
  //V1.20//ADA10022002//End
	if(resettx)
		fcr|=SERIAL_FCR_TXFR;

	if(resetrx)
		fcr|=SERIAL_FCR_RXFR;

	UART_PORT[port_no]->FCR = fcr;
}


void fLib_DisableSerialFifo(DRVUART_PORT port_no)
{
	UART_PORT[port_no]->FCR = 0;
}


void fLib_SetSerialInt(DRVUART_PORT port_no, uint32_t IntMask)
{
	UART_PORT[port_no]->IER = IntMask;
}

char fLib_GetSerialChar(DRVUART_PORT port_no)
{
	while(!((UART_PORT[port_no]->LSR & SERIAL_LSR_DR)==SERIAL_LSR_DR))
		;	// wait until Rx ready

	return (UART_PORT[port_no]->RBR);
}

void fLib_PutSerialChar(DRVUART_PORT port_no, char Ch)
{
	while (!((UART_PORT[port_no]->LSR & SERIAL_LSR_THRE)==SERIAL_LSR_THRE));	// wait until Tx ready

	UART_PORT[port_no]->THR = Ch;
}

void fLib_PutSerialStr(DRVUART_PORT port_no, char *Str)
{
  char *cp;

 	for(cp = Str; *cp != 0; cp++)
   		fLib_PutSerialChar(port_no, *cp);
}

void fLib_Modem_waitcall(DRVUART_PORT port_no)
{
	fLib_PutSerialStr(port_no, "ATS0=2\r");
}

void fLib_Modem_call(DRVUART_PORT port_no, char *tel)
{
	fLib_PutSerialStr(port_no, "ATDT");
	fLib_PutSerialStr(port_no,  tel);
	fLib_PutSerialStr(port_no, "\r");
}

void fLib_EnableSerialInt(DRVUART_PORT port_no, uint32_t mode)
{
	UART_PORT[port_no]->IER |= mode;
}


void fLib_DisableSerialInt(DRVUART_PORT port_no, uint32_t mode)
{
	UART_PORT[port_no]->IER &= (~mode);
}

uint32_t fLib_ReadSerialIER(DRVUART_PORT port_no)
{
	return UART_PORT[port_no]->IER;
}

uint32_t fLib_SerialIntIdentification(DRVUART_PORT port_no)
{
	return UART_PORT[port_no]->IIR;
}

void fLib_SetSerialLineBreak(DRVUART_PORT port_no)
{
	UART_PORT[port_no]->LCR |= (SERIAL_LCR_SETBREAK);
}

void fLib_SerialRequestToSend(DRVUART_PORT port_no)
{
	UART_PORT[port_no]->MCR |= (SERIAL_MCR_RTS);
}

void fLib_SerialStopToSend(DRVUART_PORT port_no)
{
	UART_PORT[port_no]->MCR &= ~(SERIAL_MCR_RTS);
}

void fLib_SerialDataTerminalReady(DRVUART_PORT port_no)
{
	UART_PORT[port_no]->MCR |= (SERIAL_MCR_DTR);
}

void fLib_SerialDataTerminalNotReady(DRVUART_PORT port_no)
{
	UART_PORT[port_no]->MCR &= ~(SERIAL_MCR_DTR);
}

uint32_t fLib_ReadSerialLineStatus(DRVUART_PORT port_no)
{
	return UART_PORT[port_no]->LSR;
}

uint32_t fLib_ReadSerialModemStatus(DRVUART_PORT port_no)
{
	return UART_PORT[port_no]->MSR;
}
// End of file - serial.c

//used for CLI
//#define CLI_PORT 	DebugSerialPort

uint32_t GetUartStatus(DRVUART_PORT port_no)
{
    return UART_PORT[port_no]->LSR;
}


uint32_t IsThrEmpty(uint32_t status)
{
    if((status & SERIAL_LSR_THRE)==SERIAL_LSR_THRE)
        return true;
    else
        return false;
}

uint32_t IsDataReady(uint32_t status)
{
    if((status & SERIAL_LSR_DR)==SERIAL_LSR_DR)
        return true;
    else
        return false;
}

void CheckRxStatus(DRVUART_PORT port_no)
{
  uint32_t Status;


  do
  {
		Status = GetUartStatus(port_no);
	}
	while (!IsDataReady(Status));	// wait until Rx ready
}

void CheckTxStatus(DRVUART_PORT port_no)
{
  uint32_t Status;

	do
  {
	  Status = GetUartStatus(port_no);
  }while (!IsThrEmpty(Status));	// wait until Tx ready
}
uint32_t fLib_kbhit(DRVUART_PORT port_no)
{
  uint32_t Status;


	Status = GetUartStatus(port_no);

	if(IsDataReady(Status))
		return 1;
	else
		return 0;
}

char fLib_getch(DRVUART_PORT port_no)
{
  char ch;


	if(fLib_kbhit(port_no))
		ch = UART_PORT[port_no]->RBR;
  else
  	ch=0;

  return ch;
}

char fLib_getchar(DRVUART_PORT port_no)
{

	CheckRxStatus(port_no);

  return (UART_PORT[port_no]->RBR);
}

int fLib_getchar2(DRVUART_PORT port_no)
{
   int ch = -1;
	 int Status;

	 Status = GetUartStatus(port_no);
	 if(IsDataReady(Status))
		  return (int)(UART_PORT[port_no]->RBR);

	 return ch;
}


char fLib_getchar_timeout(DRVUART_PORT port_no, unsigned long timeout)
{
	/* wait until rx status ready */
	while((GetUartStatus(port_no) & SERIAL_IER_DR) == 0x0)
	{
		if(timeout != 0)/* 0 means never timeout */
		{
			timeout --;/* count down timeout value */

			if(timeout == 0)/* 0 means timeout */
			{
				return (0xFF);/* return timeout value */
			}
		}
	}

	/* return rx character */
	return (UART_PORT[port_no]->RBR);
}


void fLib_putchar(DRVUART_PORT port_no, char Ch)
{
    if(Ch!='\0')
    {
      CheckTxStatus(port_no);
      UART_PORT[port_no]->THR = Ch;
    }

    if (Ch == '\n')
    {
	    CheckTxStatus(port_no);
      UART_PORT[port_no]->THR = '\r';
    }
}

void fLib_putc(DRVUART_PORT port_no, char Ch)
{
  CheckTxStatus(port_no);
	UART_PORT[port_no]->THR = Ch;

  if (Ch == '\n')
  {
	  CheckTxStatus(port_no);
    UART_PORT[port_no]->THR = '\r';
  }
}


void fLib_putc2(DRVUART_PORT port_no, char Ch)
{
  //CheckTxStatus(port_no);
	UART_PORT[port_no]->THR = Ch;

  if (Ch == '\n')
  {
	  //CheckTxStatus(port_no);
    UART_PORT[port_no]->THR = '\r';
  }
}


void fLib_putstr(DRVUART_PORT port_no, char *str)
{
  char *cp;


	for(cp = str; *cp != 0; cp++)
		fLib_putchar(port_no, *cp);
}


void fLib_printf(const char *f, ...)	/* variable arguments */
{
    va_list arg_ptr;
    char buffer[256];
    int32_t i;

   	//put the character to buffer
   	va_start(arg_ptr, f);
   	vsprintf(&buffer[0], f, arg_ptr);
   	va_end(arg_ptr);

   	//output the buffer
    i=0;
    //while((buffer[i])&&(i<255))

    while(buffer[i])
    {
    	fLib_putchar(DEBUG_CONSOLE, buffer[i]);
    	i++;
    }
   /* restore the previous mode */
   //spin_unlock_irqrestore(NULL, flags);

   // return i;
}


//int fLib_scanf(char *buf)
int fLib_gets(DRVUART_PORT port_no, char *buf)
{
    char    *cp;
    char    data;
    uint32_t  count;
    count = 0;
    cp = buf;

    do
    {
        data = fLib_getchar(port_no);

        switch(data)
        {
            case RETURN_KEY:
                if(count < 256)
                {
                    *cp = '\0';
                    fLib_putchar(port_no, '\n');
                }
                break;
            case BACKSP_KEY:
            case DELETE_KEY:
                if(count)
                {
                    count--;
                    *(--cp) = '\0';
                    fLib_putstr(port_no, "\b \b");
                }
                break;
            default:
                if( data > 0x1F && data < 0x7F && count < 256)
                {
                    *cp = (char)data;
                    cp++;
                    count++;
                    fLib_putchar(port_no, data);
                }
                break;
        }
    } while(data != RETURN_KEY);

  return (count);
}


//for SWI call
void fLib_DebugPrintChar(DRVUART_PORT port_no, char ch)
{
    if(ch != '\0' && ch != '\n')
    {
		  fLib_PutSerialChar(port_no, ch);
    }
    else if(ch == '\n')
    {
  	  fLib_PutSerialChar(port_no, '\r');//CR
      fLib_PutSerialChar(port_no, '\n');//LF
    }
}

void fLib_DebugPrintString(DRVUART_PORT port_no, char *str)
{
	while(*str)
	{
		fLib_DebugPrintChar(port_no, *str);
		str++;
	}
}

char fLib_DebugGetChar(DRVUART_PORT port_no)
{
	return fLib_GetSerialChar(port_no);
}

uint32_t fLib_DebugGetUserCommand(DRVUART_PORT port_no, uint8_t * buffer, uint32_t Len)
{
    int offset = 0, c;

    buffer[0] = '\0';
    while (offset < (Len - 1)) {
	c = fLib_GetSerialChar(port_no);

	if (c == '\b')		//backspace
	{
	    if (offset > 0) {
		// Rub out the old character & update the console output
		offset--;
		buffer[offset] = 0;

		fLib_DebugPrintString(port_no, "\b \b");
	    }
	} else if (c == DELETE_KEY)	//backspace
	{
	    if (offset > 0) {
		// Rub out the old character & update the console output
		offset--;
		buffer[offset] = 0;

		fLib_DebugPrintString(port_no, "\b \b");
	    }
	}

	else {
	    if (c == '\r')
		c = '\n';	// treat \r as \n

	    fLib_PutSerialChar(port_no, c);

	    buffer[offset++] = c;

	    if (c == '\n')
		break;
	}
    }

    buffer[offset] = '\0';

    return offset;
}
