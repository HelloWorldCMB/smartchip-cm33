#include "Common_Include.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h> // for va_list va_start()
#include "int_trans.h"
#include "CAN_test.h"
fLib_INT IRQVector[PLATFORM_IRQ_TOTALCOUNT];
fLib_INT IRQVector1[PLATFORM_IRQ_TOTALCOUNT];

/* define IRQ trap and handler routine */
PrVoid fLib_NewIRQTrap = (PrVoid) 0;	
PrHandler fLib_IRQHandle = (PrHandler) 0;

void CAN010_0_interrupt(void)
{
    int irq = IRQ_CAN1;
    fLib_INT *action;
    action = &IRQVector[irq];

    if (action->Handler != NULL)
    {
        action->Handler(irq, NULL);
    }else{
            fLib_printf("ftintc030: No handler installed: %d\n", irq);        // can't call printf in int handler
    }
}

void CAN010_1_interrupt(void)
{
    int irq = IRQ_CAN2;
    fLib_INT *action;
    action = &IRQVector1[irq];

    if (action->Handler != NULL)
    {
        action->Handler(irq, NULL);
    }else{
            fLib_printf("ftintc030: No handler installed: %d\n", irq);        // can't call printf in int handler
    }
}

void fLib_SetIRQmode(UINT32 IRQ,UINT32 mode)
{
	  return;
}

void fLib_DisableIRQ(UINT32 IRQ)
{
	  NVIC_DisableIRQ(IRQ);
}

void fLib_EnableIRQ(UINT32 IRQ)
{
	  NVIC_EnableIRQ(IRQ);
}

int fLib_ConnectInt(fLib_INT * IntVector,UINT32 IntNum, PrHandler Handler, void *args)
{
    fLib_INT *interrupt;


    if (IntNum >= PLATFORM_IRQ_TOTALCOUNT)
        return FALSE;

    if (!Handler)
        return FALSE;
		
    interrupt = &IntVector[IntNum];
		

    if (interrupt->Handler)
        return FALSE;//Interrupt handler has assigned

    interrupt->Handler = Handler;   
    interrupt->IntNum=IntNum;
    interrupt->private_arg= args;

    return TRUE;
}

/*
 * Routine to check and install requested interrupt. Just sets up logical
 * structures, doesn't alter interrupts at all.
 * 081202: Richard Lin: Adding appending args examples ~~
 	usage 1: fLib_ConnectInt(IRQ_IAmIRQ, i, handler, (void *)private_point)
 	usage 2: fLib_ConnectInt(IRQ_IAmIRQ, i, handler, NULL)
 	usage 3: fLib_ConnectInt(IRQ_IAmIRQ, i, handler)
 */
int fLib_ConnectIRQ(UINT32 IntNum, PrHandler Handler, ...)
{
    fLib_INT *Irq;

    va_list arg;
    void *string;

    va_start(arg, Handler);
    string = va_arg(arg, char *);
    va_end(arg);
    if(IntNum == IRQ_CAN1)
			return fLib_ConnectInt(IRQVector, IntNum, Handler, string);
		return fLib_ConnectInt(IRQVector1, IntNum, Handler, string);
}

/* Call this routine before trying to change the routine attached to an IRQ */
INT32 fLib_CloseIRQ(UINT32 IntNum)
{   
    if (IntNum >= PLATFORM_IRQ_TOTALCOUNT)
    {
        printf("Can't close Interrupt %d\n", IntNum);
        return FALSE;
    }

    // Disable the interrupt & then remove the handler
    fLib_DisableIRQ(IntNum);
		if(IntNum == IRQ_CAN1){
			IRQVector[IntNum].Handler = (PrHandler) 0;
			IRQVector[IntNum].IntNum =  0;
		}else{
			IRQVector1[IntNum].Handler = (PrHandler) 0;
			IRQVector1[IntNum].IntNum =  0;
			
		}

    return TRUE;

}


/* Initialise the IRQ environment without installing interrupt handlers. */
void fLib_ResetIRQ()
{
    int i;     

    /* Mask out all interrupt sources. */
    for (i = 0; i < PLATFORM_IRQ_TOTALCOUNT; i++)
    {	
	    fLib_DisableIRQ(i);
        IRQVector[i].IntNum = 0;
        IRQVector[i].Handler = (PrHandler) 0;      
    }	

}