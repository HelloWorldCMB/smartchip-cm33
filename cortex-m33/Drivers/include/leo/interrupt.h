#ifndef __INTERRUPT_H
#define __INTERRUPT_H

//#include "i606.h"
#include "Types.h"
//#include "common.h"

#if defined(__MCU_FA606TE__) || defined(__MCU_CM0__)
//typedef int irqreturn_t;
typedef int (*IrqHandler)(int irq, void *arg);//typedef irqreturn_t (*IrqHandler)(int irq, void *arg);

#if defined(__MCU_CM0__)
extern void  DispatchIRQ(void);
#else
#if defined(__GHS__)
extern __interrupt void  DispatchIRQ(void);
#else
extern __irq void  DispatchIRQ(void);
#endif
#endif
#if defined(__MCU_FA606TE__)
#if defined(__GHS__)
extern __interrupt void  DispatchFIQ(void);
#else
extern __irq void  DispatchFIQ(void);
#endif
#endif

#define IRQ_NONE        (0)
#define IRQ_HANDLED     (1)
#define IRQ_RETVAL(x)   ((x) != 0)

#define IRQ_SOURCE                      0
#define IRQ_MASK                        0x04
#define IRQ_CLEAR                       0x08
#define IRQ_MODE                        0x0c
#define IRQ_LEVEL                       0x10
#define IRQ_STATUS                      0x14

#if defined(__MCU_FA606TE__)
#define FIQ_SOURCE                      0x20
#define FIQ_MASK                        0x24
#define FIQ_CLEAR                       0x28
#define FIQ_MODE                        0x2c
#define FIQ_LEVEL                       0x30
#define FIQ_STATUS                      0x34
#endif

#define EXT_IRQ_SOURCE                  0x60
#define EXT_IRQ_MASK                    0x64
#define EXT_IRQ_CLEAR                   0x68
#define EXT_IRQ_MODE                    0x6c
#define EXT_IRQ_LEVEL                   0x70
#define EXT_IRQ_STATUS                  0x74

#if defined(__MCU_FA606TE__)
#define EXT_FIQ_SOURCE                  0x80
#define EXT_FIQ_MASK                    0x84
#define EXT_FIQ_CLEAR                   0x88
#define EXT_FIQ_MODE                    0x8c
#define EXT_FIQ_LEVEL                   0x90
#define EXT_FIQ_STATUS                  0x94

#define FIQ_OFFSET                      32
#endif

#define VECT_ADDR0						0x100
#define VECT_ADDRCTRL0					0x180
#define DEF_VECTADDR					0x200
#define SEL_VECTADDR					0x204
#define INT_PRIARBCTRL					0x208

#define SW_INT_ENABLE					0x20c
#define SW_INT_CLE						0x210


 
/*  Interrupt bit positions  */

//#define MAXIRQNUM                       63 
//#define MAXFIQNUM                       31  
//#define NR_IRQS                         (MAXIRQNUM + 1)
//#define NR_FIQS                         (MAXFIQNUM + 1)

#define NR_VECT													16 
#define VECT_IRQ_SRC_EN									(1<<6)
#define VECT_IRQ_EN			(1<<19)
#define	NEW_IRQSTATUS_EN	(1<<20)
#define	ROUND_ROBIN		(1<<18)
#define	FIX_PRIOR		0
#define	FIX_ORDER		(1<<17)
#define ROUND_LV			0x00FF //lowest 7 vect number set to high level

#define LEVEL                           0
#define EDGE                            1
                         
#define H_ACTIVE                        0
#define L_ACTIVE                        1

/* A function with no argument returning pointer to a void function */
typedef void (*PrVoid) (void);


typedef struct 
{
	PrVoid Handler;//PrHandler Handler;          /* Routine for specific interrupt */
  UINT32 IntNum;           
}fLib_INT;


/*  -------------------------------------------------------------------------------
 *   API
 *  -------------------------------------------------------------------------------
 */
 
/*  -------------------------------------------------------------------------------
 *   define in interrupt.c
 *  -------------------------------------------------------------------------------
 */
  
 
//extern UINT32 fLib_ReadIntVector(UINT32 *vect, PrVoid *oldHandler);
//extern UINT32 fLib_WriteIntVector(UINT32 *vect, PrHandler newHandler);
extern void   fLib_Int_Init(void);
extern void   fLib_SetIntTrig(UINT32 IntNum,int intMode,int intLevel);
extern int    fLib_ConnectInt(UINT32 IntNum,PrVoid Handler);
extern void   fLib_EnableInt(UINT32 IntNum);
extern void   fLib_DisableInt(UINT32 IntNum);
extern void   fLib_ClearInt(UINT32 IntNum);
extern int  fLib_CloseInt(UINT32 IntNum);
extern void fLib_EnableVectorInt(void);
extern void fLib_DisableVectorInt(void);
extern int fLib_ConnectVectorInt(UINT32 IntNum, UINT32 Handler);
extern int fLib_CloseVectorInt(UINT32 IntNum);

/*  -------------------------------------------------------------------------------
 *   define in fiqtrap.s
 *  -------------------------------------------------------------------------------
 */
 
//extern void fLib_FIQTrap(void);

/*  -------------------------------------------------------------------------------
 *   define in irqtrap.s
 *  -------------------------------------------------------------------------------
 */
 
//extern void fLib_IRQTrap(void);

#if defined(__MCU_FA606TE__)
/*  -------------------------------------------------------------------------------
 *   define in fiq.c
 *  -------------------------------------------------------------------------------
 */
extern fLib_INT  FIQVector[PLATFORM_FIQ_TOTALCOUNT];
//extern PrVoid    fLib_NewFIQTrap;  
extern PrVoid fLib_FIQHandle ;
//extern PrVoid    fLib_OldFIQTrap;
extern UINT64    FIQSources;
extern void   MaskFIQ(UINT32 FIQ);
extern void   UnmaskFIQ(UINT32 FIQ);
extern void   ClearFIQStatus(UINT32 FIQ);
//extern PrVoid NewFIQ(PrHandler handler, PrVoid trap);

extern void   SetFIQmode(UINT32 FIQ,UINT32 edge);
extern void   SetFIQlevel(UINT32 FIQ,UINT32 low); 
#endif
/*  -------------------------------------------------------------------------------
 *   define in irq.c
 *  -------------------------------------------------------------------------------
 */

extern fLib_INT  IRQVector[PLATFORM_IRQ_TOTALCOUNT];

//extern PrVoid    fLib_NewIRQTrap;  
extern PrVoid fLib_IRQHandle ;
//extern PrVoid    fLib_OldIRQTrap;	
extern UINT64    IRQSources;	
extern void   MaskIRQ(UINT32 IRQ);
extern void   UnmaskIRQ(UINT32 IRQ);
extern void   ClearIRQStatus(UINT32 IRQ);
//extern PrVoid NewIRQ(PrHandler handler, PrVoid trap);
//extern PrVoid NewIRQ2(PrHandler handler, PrVoid trap);
//extern void   DispatchIRQ(UINT32 flags);
extern void   SetIRQmode(UINT32 IRQ,UINT32 edge);
extern void   SetIRQlevel(UINT32 IRQ,UINT32 low);
extern void EnableVectInt(void);
extern void DisableVectInt(void);
extern void ClearPrioMask(UINT32 VIC);
extern void SetVectAddr(UINT32 vectnum,UINT32 addr);
extern void SetVectAddrCtrl(UINT32 vectnum, UINT32 val);
extern void SetDefVectAddr(UINT32 addr);	
extern UINT32 GetVectAddr(UINT32 vectnum);
extern UINT32 GetVectAddrCtrl(UINT32 vectnum);
#endif //End of defined(__MCU_FA606TE__) || defined(__MCU_CM0__)
#endif //End of __INTERRUPT_H
