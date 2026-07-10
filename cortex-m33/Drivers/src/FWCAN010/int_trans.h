#include "types.h"

/*  Interrupt bit positions  */

#define LEVEL                           0
#define EDGE                            1
                         
#define H_ACTIVE                        0
#define L_ACTIVE                        1

/* A function with no argument returning pointer to a void function */

typedef void (*PrVoid) (void);
typedef void (*PrHandler) (UINT32, void *);  /* As PrVoid with one parameter */


typedef struct {
	PrHandler Handler;          /* Routine for specific interrupt */
	UINT32 IntNum; 
	void *private_arg;                //
}fLib_INT;



/*  -------------------------------------------------------------------------------
 *   define in intc.c
 *  -------------------------------------------------------------------------------
 */
#include "types.h"
 
extern UINT32 fLib_ReadIntVector(UINT32 *vect, PrVoid *oldHandler);
extern UINT32 fLib_WriteIntVector(UINT32 *vect, PrHandler newHandler);

extern void   fLib_Int_Init(void);


/* 
 * IRQ API
 */
extern int    fLib_ConnectIRQ( UINT32 IntNum,PrHandler Handler, ...);
extern void   fLib_EnableIRQ(UINT32 IRQ);
extern void   fLib_DisableIRQ(UINT32 IRQ);
extern void   fLib_ClearIRQ(UINT32 IRQ);
extern void   fLib_SetIRQmode(UINT32 IRQ,UINT32 mode);
extern void   fLib_RaiseSoftIRQ(UINT32 CPUID, UINT32 irq);

/* 
 * IRQ_priv API : for SGI, PPI in gic/intc030
 */
extern void fLib_DisableIRQ_priv(UINT32 irq_n);
extern void fLib_EnableIRQ_priv(UINT32 irq_n);
extern int fLib_ConnectIRQ_priv(UINT32 IntNum, PrHandler Handler, ...);

/* 
 * IRQ Legacy API
 */
extern INT32  fLib_CloseIRQ(UINT32 IntNum);
extern void   fLib_SetIRQmode(UINT32 IRQ,UINT32 edge);
extern void   fLib_SetIRQlevel(UINT32 IRQ,UINT32 low);
extern void   ClearIRQStatus(UINT32 IRQ);
