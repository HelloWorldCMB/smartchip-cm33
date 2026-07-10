
#ifndef __CPE_AHB_DMA_H
#define __CPE_AHB_DMA_H

#include "types.h"

/* registers */
#define DMA_INT						0x0
#define DMA_INT_TC					0x4
#define DMA_INT_TC_CLR				0x8
#define DMA_INT_ERRABT				0xC
#define DMA_INT_ERRABT_CLR			0x10
#define DMA_TC						0x14
#define DMA_ERRABT					0x18
#define DMA_CH_EN					0x1C
#define DMA_CH_BUSY					0x20
#define DMA_CSR						0x24
#define DMA_SYNC					0x28

#define DMA_FEATURE					0x34

#define DMA_C0_DevDtBase			0x40
#define DMA_C0_DevRegBase			0x80


#define DMA_CHANNEL_OFFSET			0x20
#define DMA_CHANNEL0_BASE			0x100
#define DMA_CHANNEL1_BASE			0x120
#define DMA_CHANNEL2_BASE			0x140
#define DMA_CHANNEL3_BASE			0x160
#define DMA_CHANNEL4_BASE			0x180
#define DMA_CHANNEL5_BASE			0x1a0
#define DMA_CHANNEL6_BASE			0x1c0
#define DMA_CHANNEL7_BASE			0x1e0

#define DMA_CHANNEL_CSR_OFFSET		0x0
#define DMA_CHANNEL_CFG_OFFSET		0x4
#define DMA_CHANNEL_SRCADDR_OFFSET	0x8
#define DMA_CHANNEL_DSTADDR_OFFSET	0xc
#define DMA_CHANNEL_LLP_OFFSET		0x10
#define DMA_CHANNEL_SIZE_OFFSET		0x14


/* bit mapping of main configuration status register(CSR) */
#define DMA_CSR_M1ENDIAN			0x00000004
#define DMA_CSR_M0ENDIAN			0x00000002
#define DMA_CSR_DMACEN				0x00000001

/* bit mapping of channel control register */
#define DMA_CSR_TC_MSK				0x80000000
#define DMA_CSR_CHPRJ_HIGHEST		0x00C00000
#define DMA_CSR_CHPRJ_2ND			0x00800000
#define DMA_CSR_CHPRJ_3RD			0x00400000
#define DMA_CSR_PRTO3				0x00200000
#define DMA_CSR_PRTO2				0x00100000
#define DMA_CSR_PRTO1				0x00080000
#define DMA_CSR_SRC_BURST_SIZE_1	0x00000000
#define DMA_CSR_SRC_BURST_SIZE_4	0x00010000
#define DMA_CSR_SRC_BURST_SIZE_8	0x00020000
#define DMA_CSR_SRC_BURST_SIZE_16	0x00030000
#define DMA_CSR_SRC_BURST_SIZE_32	0x00040000
#define DMA_CSR_SRC_BURST_SIZE_64	0x00050000
#define DMA_CSR_SRC_BURST_SIZE_128	0x00060000
#define DMA_CSR_SRC_BURST_SIZE_256	0x00070000

#define DMA_CSR_ABT					0x00008000
#define DMA_CSR_SRC_WIDTH_8			0x00000000
#define DMA_CSR_SRC_WIDTH_16		0x00000800
#define DMA_CSR_SRC_WIDTH_32		0x00001000

#define DMA_CSR_DST_WIDTH_8			0x00000000
#define DMA_CSR_DST_WIDTH_16		0x00000100
#define DMA_CSR_DST_WIDTH_32		0x00000200

#define DMA_CSR_MODE_NORMAL			0x00000000
#define DMA_CSR_MODE_HANDSHAKE		0x00000080

#define DMA_CSR_SRC_INCREMENT		0x00000000
#define DMA_CSR_SRC_DECREMENT		0x00000020
#define DMA_CSR_SRC_FIX				0x00000040

#define DMA_CSR_DST_INCREMENT		0x00000000
#define DMA_CSR_DST_DECREMENT		0x00000008
#define DMA_CSR_DST_FIX				0x00000010

#define DMA_CSR_SRC_SEL				0x00000004
#define DMA_CSR_DST_SEL				0x00000002
#define DMA_CSR_CH_ENABLE			0x00000001	

#define DMA_CSR_CHPR1				0x00C00000
#define DMA_CSR_SRC_SIZE			0x00070000
#define DMA_CSR_SRC_WIDTH			0x00003800
#define DMA_CSR_DST_WIDTH			0x00000700	
#define DMA_CSR_SRCAD_CTL			0x00000060
#define DMA_CSR_DSTAD_CTL			0x00000018


#define DMA_MAX_SIZE				0x10000
#define DAM_CHANNEL_NUMBER			8

/* bit mapping of channel configuration register */
#define DMA_CFG_INT_ERR_MSK_Disable	0x00000000
#define DMA_CFG_INT_TC_MSK_Disable	0x00000000

/* bit mapping of Linked List Control Descriptor */
#define DMA_LLP_TC_MSK				0x10000000

#define DMA_LLP_SRC_WIDTH_8			0x00000000
#define DMA_LLP_SRC_WIDTH_16		0x02000000
#define DMA_LLP_SRC_WIDTH_32		0x04000000

#define DMA_LLP_DST_WIDTH_8			0x00000000
#define DMA_LLP_DST_WIDTH_16		0x00400000
#define DMA_LLP_DST_WIDTH_32		0x00800000

#define DMA_LLP_SRC_INCREMENT		0x00000000
#define DMA_LLP_SRC_DECREMENT		0x00100000
#define DMA_LLP_SRC_FIX				0x00200000

#define DMA_LLP_DST_INCREMENT		0x00000000
#define DMA_LLP_DST_DECREMENT		0x00040000
#define DMA_LLP_DST_FIX				0x00080000

#define DMA_LLP_SRC_SEL				0x00020000
#define DMA_LLP_DST_SEL				0x00010000

/////////////////////////// AHB DMA Define //////////////////////////////////
#define AHBDMA_Channel0					0
#define AHBDMA_Channel1					1
#define AHBDMA_Channel2					2	
#define AHBDMA_Channel3					3	
#define AHBDMA_Channel4					4		
#define AHBDMA_Channel5					5			
#define AHBDMA_Channel6					6				
#define AHBDMA_Channel7					7	


#define AHBDMA_CH_SD					0
#define AHBDMA_CH_I2S_TX				1
#define AHBDMA_CH_I2S_RX				2
#define AHBDMA_CH_ADC					3
#define AHBDMA_CH_IDE_TX				4
#define AHBDMA_CH_IDE_RX				5
#define AHBDMA_CH_SPI2_TX				6
#define AHBDMA_CH_SPI2_RX				7



#define AHBDMA_SrcWidth_Byte				0					
#define AHBDMA_SrcWidth_Word				1					
#define AHBDMA_SrcWidth_DWord				2		

#define AHBDMA_DstWidth_Byte				0					
#define AHBDMA_DstWidth_Word				1					
#define AHBDMA_DstWidth_DWord				2					

#define AHBDMA_Burst1						0					
#define AHBDMA_Burst4						1					
#define AHBDMA_Burst8						2					
#define AHBDMA_Burst16						3					
#define AHBDMA_Burst32						4					
#define AHBDMA_Burst64						5					
#define AHBDMA_Burst128					6					
#define AHBDMA_Burst256					7					

#define AHBDMA_NormalMode					0					
#define AHBDMA_HwHandShakeMode			1					

#define AHBDMA_SrcInc						0					
#define AHBDMA_SrcDec						1					
#define AHBDMA_SrcFix						2	

#define AHBDMA_DstInc						0					
#define AHBDMA_DstDec						1					
#define AHBDMA_DstFix						2	

#define AHBDMA_PriorityLow					0					
#define AHBDMA_Priority3rd					1					
#define AHBDMA_Priority2nd					2	
#define AHBDMA_PriorityHigh					3	


// --------------------------------------------------------------------
//	3.21.1 Channel Control Register (Cn_CSR) (0x100, 0x120, ....)
// --------------------------------------------------------------------
typedef struct
{
	UINT32 enable:1;
	UINT32 dst_sel:1;
	UINT32 src_sel:1;
	UINT32 dst_ctrl:2;
	UINT32 src_ctrl:2;
	UINT32 mode:1;
	UINT32 dst_width:3;		
	UINT32 src_width:3;
	UINT32 reserved1:1;
	UINT32 abt:1;
	UINT32 src_size:3;
	UINT32 prot:3;
	UINT32 priority:2;
	UINT32 ff_th:3;			//FIE7021
	UINT32 reserved0:4;		//FIE7021
	UINT32 tc_msk:1;	
}fLib_DMA_CH_CSR_t;

// Jerry has modfied by jerry
typedef struct
{
	UINT32 int_tc_msk:1;
	UINT32 int_err_msk:1;
	UINT32 int_abt_msk:1;		
//FIE7020	UINT32 reserved0:5;
	UINT32 src_rs:4;		//FIE7021
	UINT32 src_he:1;		//FIE7021
	UINT32 busy:1;
//FIE7020	UINT32 reserved1:7;	
	UINT32 dst_rs:4;		//FIE7021
	UINT32 dst_he:1;		//FIE7021
	UINT32 reserved1:2;		//FIE7021
	UINT32 llp_cnt:4;	
	UINT32 reserved2:12;
}fLib_DMA_CH_CFG_t;
	
typedef struct
{
	UINT32 master_id:1;
	UINT32 reserved:1;
	UINT32 link_list_addr:30;
}fLib_DMA_CH_LLP_t;


// --------------------------------------------------------------------
// Table 3-2. Control Field Definition in Linked List Descriptor
// --------------------------------------------------------------------
typedef struct
{
	UINT32 reserved:16;		//FIE7021
	
	UINT32 dst_sel:1;
	UINT32 src_sel:1;
	UINT32 dst_ctrl:2;
	UINT32 src_ctrl:2;
	UINT32 dst_width:3;
	UINT32 src_width:3;
	UINT32 tc_msk:1;
	UINT32 ff_th:3;			//FIE7021
}fLib_DMA_LLP_CTRL_t;


// channel n registers (0x100 ~ ...)
typedef struct
{
	fLib_DMA_CH_CSR_t csr;			// 0x0
	fLib_DMA_CH_CFG_t cfg;			// 0x4
	UINT32 src_addr;				// 0x8
	UINT32 dst_addr;				// 0xc
	fLib_DMA_CH_LLP_t llp;			// 0x10
	UINT32 size;					// 0x14
	UINT32 dummy[2];
}fLib_DMA_CH_t;


// --------------------------------------------------------------------
//	Table 3-1. Address Map for Linked List Descriptor (Base Address: Cn_LLP[31:2])
// --------------------------------------------------------------------
typedef struct
{
	UINT32 src_addr;
	UINT32 dst_addr;
	fLib_DMA_CH_LLP_t llp;
	fLib_DMA_LLP_CTRL_t llp_ctrl;
	UINT32 size;			//FIE7021
}fLib_DMA_LLD_t;


typedef struct
{
	UINT32 dma_int;							// 0x00
	UINT32 dma_int_tc;						// 0x04
	UINT32 dma_int_tc_clr;					// 0x08
	UINT32 dma_int_err;						// 0x0c
	UINT32 dma_int_err_clr;					// 0x10
	UINT32 dma_tc;							// 0x14
	UINT32 dma_err;							// 0x18
	UINT32 dma_ch_enable;					// 0x1c
	UINT32 dma_ch_busy;						// 0x20
	UINT32 dma_csr;							// 0x24
	UINT32 dma_sync;						// 0x28
	UINT32 dummy0[5];						
	UINT32 dma_ch_dev_dt_base[8];			// 0x40
	
	UINT32 dummy1[8];						// 0x60
	
	UINT32 dma_ch_dev_reg_base[8];			// 0x80
	
	UINT32 dummy2[24];
	
	fLib_DMA_CH_t dma_ch[8];				// 0x100 ~ ...
}fLib_DMA_Reg_t;


/*  -------------------------------------------------------------------------------
 *   API
 *  -------------------------------------------------------------------------------
 */
 
extern int    fLib_IsDMAChannelBusy(INT32 Channel);
extern int    fLib_IsDMAChannelEnable(INT32 Channel);
extern UINT32 fLib_GetDMAIntStatus(void);
extern UINT32 fLib_GetDMAChannelIntStatus(INT32 Channel);
extern int    fLib_GetDMABusyStatus(void);
extern int    fLib_GetDMAEnableStatus(void);

extern void   fLib_InitDMA(UINT32 M0_BigEndian, UINT32 M1_BigEndian, UINT32 Sync);
extern void   fLib_EnableDMAChannel(INT32 Channel);

extern void   fLib_DisableDMAChannel(INT32 Channel);  // add by jerry

extern void   fLib_ClearDMAChannelIntStatus(INT32 Channel);

extern void   fLib_SetDMAChannelCfg(INT32 Channel, fLib_DMA_CH_CSR_t Csr);
extern void   fLib_SetDMAChannelCnCfg(INT32 Channel, fLib_DMA_CH_CFG_t CnCfg);
extern fLib_DMA_CH_CSR_t fLib_GetDMAChannelCfg(INT32 Channel);
extern void   fLib_DMA_CHIntMask(INT32 Channel, fLib_DMA_CH_CFG_t Mask);
extern void   fLib_DMA_CHLinkList(INT32 Channel, fLib_DMA_CH_LLP_t LLP);
extern void   fLib_DMA_CHDataCtrl(INT32 Channel, UINT32 SrcAddr, UINT32 DstAddr, UINT32 Size);

extern void fLib_DMA_NormalMode(
UINT32 Channel,   // use which channel for AHB DMA, 0..7
UINT32 SrcAddr,   // source begin address
UINT32 DstAddr,   // dest begin address
UINT32 Size,      // total bytes
UINT32 SrcWidth,  // source width 8/16/32 bits -> 0/1/2
UINT32 DstWidth,  // dest width 8/16/32 bits -> 0/1/2
UINT32 SrcSize,   // source burst size, How many "SrcWidth" will be transmmited at one times ?
UINT32 SrcCtrl,   // source address change : Inc/dec/fixed --> 0/1/2
UINT32 DstCtrl,   // dest address change : Inc/dec/fixed --> 0/1/2
UINT32 Priority,  // priority for this chaanel 0(low)/1/2/3(high)
UINT32 Mode,      // Normal/Hardwire,   0/1
int    req	  	  // DMA request select (ps. ???????? ram vs device, ?????? req)
				  // ?? device vs device, ?????? api
);


#ifdef not_complete_yet
extern void fLib_DMA_NormalModeEx(
UINT32 Channel,   // use which channel for AHB DMA, 0..7
UINT32 tcmask,	  // Terminal count mask
UINT32 SrcAddr,   // source begin address
UINT32 DstAddr,   // dest begin address
UINT32 Size,      // total bytes
UINT32 SrcWidth,  // source width 8/16/32 bits -> 0/1/2
UINT32 DstWidth,  // dest width 8/16/32 bits -> 0/1/2
UINT32 SrcSize,   // source burst size, How many "SrcWidth" will be transmmited at one times ?
UINT32 SrcCtrl,   // source address change : Inc/dec/fixed --> 0/1/2
UINT32 DstCtrl,   // dest address change : Inc/dec/fixed --> 0/1/2
UINT32 Priority,  // priority for this chaanel 0(low)/1/2/3(high)
UINT32 Mode       // Normal/Hardwire,   0/1
);
#endif /* end_of_not */


extern void fLib_DMA_LinkMode(
UINT32 Channel,   // use which channel for AHB DMA, 0..7
UINT32 LinkAddr,  // Link-List address
UINT32 LLPCount,  // total link-list node
UINT32 SrcAddr,   // source begin address
UINT32 DstAddr,   // dest begin address
UINT32 Size,      // total bytes
UINT32 SrcWidth,  // source width 8/16/32 bits -> 0/1/2
UINT32 DstWidth,  // dest width 8/16/32 bits -> 0/1/2
UINT32 SrcSize,   // source burst size, How many "SrcWidth" will be transmmited at one times ?
UINT32 SrcCtrl,   // source address change : Inc/dec/fixed --> 0/1/2
UINT32 DstCtrl,   // dest address change : Inc/dec/fixed --> 0/1/2
UINT32 Priority,  // priority for this chaanel 0(low)/1/2/3(high)
UINT32 Mode,      // Normal/Hardwire,   0/1
int    req	  	  // DMA request select (ps. ???????? ram vs device, ?????? req)
				  // ?? device vs device, ?????? api
);


#ifdef not_complete_yet
extern void fLib_DMA_InitLinkMode(
UINT32 Channel,   // use which channel for AHB DMA, 0..7
UINT32 LinkAddr,  // Link-List address
UINT32 SrcAddr,   // source begin address
UINT32 DstAddr,   // dest begin address
UINT32 Size,      // total bytes
UINT32 SrcWidth,  // source width 8/16/32 bits -> 0/1/2
UINT32 DstWidth,  // dest width 8/16/32 bits -> 0/1/2
UINT32 SrcSize,   // source burst size, How many "SrcWidth" will be transmmited at one times ?
UINT32 SrcCtrl,   // source address change : Inc/dec/fixed --> 0/1/2
UINT32 DstCtrl,   // dest address change : Inc/dec/fixed --> 0/1/2
UINT32 Priority,  // priority for this chaanel 0(low)/1/2/3(high)
UINT32 Mode,      // Normal/Hardwire,   0/1
UINT32 tcmask
);
#endif /* end_of_not */


extern void fLib_SetupDescriptor(
UINT32 LinkAddr,		// address of this descriptor
UINT32 LLPCount,		// number of link list node
UINT32 SrcAddr,			// source begin address
UINT32 DstAddr,			// destination begin address
UINT32 nextllpaddr,		// next llp address
UINT32 tcmask,
UINT32 SrcWidth,
UINT32 DstWidth,
UINT32 SrcCtrl,
UINT32 DstCtrl
);

extern void fLib_SetupDescriptor_new(
UINT32 LinkAddr,		// address of this descriptor
UINT32 LLPCount,		// number of link list node
UINT32 SrcAddr,			// source begin address
UINT32 DstAddr,			// destination begin address
UINT32 nextllpaddr,		// next llp address
UINT32 tcmask,
UINT32 SrcWidth,
UINT32 DstWidth,
UINT32 SrcCtrl,
UINT32 DstCtrl,
UINT32 Count
);

extern void fLib_DMA_WaitIntStatus(UINT32 Channel);
extern void fLib_DMA_SetInterrupt(UINT32 channel, UINT32 tcintr, UINT32 errintr, UINT32 abtintr);
extern void fLib_DMA_ResetChannel(UINT8 channel);
extern void fLib_DMA_ClearAllInterrupt(void);
extern void fLib_DMA_EnableDMAInt(void);
extern void fLib_DMA_DisableDMAInt(void);
extern void fLib_DMA_EnableDMATCInt(void);
extern void fLib_DMA_DisableDMATCInt(void);
extern void fLib_DMA_WaitDMAInt(UINT32 channel);
extern void fLib_DMA_WaitDMATCInt(UINT32 channel);
int fLib_DMA_ChannelNum(void);		// DMA maximum channel number

#endif
