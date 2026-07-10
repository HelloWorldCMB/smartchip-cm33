/***************************************************************************
* Copyright  Faraday Technology Corp 2002-2003.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:DMA.c                                                               *
* Description: DMA C Library routine                                       *
* Author:                                                        *
****************************************************************************/

#include "leo_cm33.h"
#include "utility.h"
#include "io.h"
#include "DrvDMAC020.h"
#include "DrvUART010.h"
#include "ftssp010.h"

//#define CN_SIZE			(0x400000-1)
#define CN_SIZE			1024//4080		//==> for test only

#define CPU_TO_AHB_ADDRSPACE 0x23
//#define DBG_DMA 1

int burst_size_selection[] = {1, 4, 8, 16, 32, 64, 128, 256};

volatile fLib_DMA_Reg_t *DMAReg = (fLib_DMA_Reg_t *)DMAC_FTDMAC020_PA_BASE;
static volatile UINT32	DMA_INT_OCCURRED = 0, DMA_TC_INT_OCCURRED = 0;




void AHB_DMA_IRQHandler(void)
{
	UINT32 status;
	
	status = DMAReg->dma_int;
	
	fLib_printf("%s %x\n", __func__, status);
	
	if(status)
		DMA_INT_OCCURRED |= status;
  else
		return;
	
	outw(DMAC_FTDMAC020_PA_BASE+DMA_INT_TC_CLR, status);
	outw(DMAC_FTDMAC020_PA_BASE+DMA_INT_ERRABT_CLR, status);	
	
	ftssp010_spi_slave_dma_read(FTSSP010_REG_BASE_S, (void *)0x50120000, 5386);

}


void AHB_DMA_TC_IRQHandler(void)
{
	UINT32 status;
	
	fLib_printf("%s\n", __func__);
	status = DMAReg->dma_int_tc;
	
	if(status)
		DMA_TC_INT_OCCURRED |= status;
	else
		return;
	
	outw(DMAC_FTDMAC020_PA_BASE+DMA_INT_TC_CLR, status);	
}

int fLib_IsDMAChannelBusy(INT32 Channel)
{
	return ((DMAReg->dma_ch_busy >> Channel) & 0x1);
}

int fLib_IsDMAChannelEnable(INT32 Channel)
{
	return ((DMAReg->dma_ch_enable >> Channel) & 0x1);
}

int fLib_GetDMABusyStatus(void)
{
	return DMAReg->dma_ch_busy;
}

int fLib_GetDMAEnableStatus(void)
{
	return DMAReg->dma_ch_enable;
}

UINT32 fLib_GetDMAIntStatus(void)
{
	return DMAReg->dma_int;
}



UINT32 fLib_GetDMAChannelIntStatus(INT32 Channel)
{
	volatile UINT32 IntStatus = 0;
	
	if((DMAReg->dma_int >> Channel) & 0x01)
	{
		if((DMAReg->dma_int_tc >> Channel) & 0x01)
			IntStatus |= 1;
		if((DMAReg->dma_int_err >> Channel) & 0x01)
			IntStatus |= 2;
	}
		
	return IntStatus;
}


void fLib_InitDMA(UINT32 M0_BigEndian, UINT32 M1_BigEndian, UINT32 Sync)
{		
	DMAReg->dma_csr = (M0_BigEndian ? DMA_CSR_M0ENDIAN : 0) | 
	(M1_BigEndian ? DMA_CSR_M1ENDIAN : 0) | DMA_CSR_DMACEN;
	
	DMAReg->dma_sync = Sync; 
}


void fLib_EnableDMAChannel(INT32 Channel)
{
	UINT32 reg;
		
	reg = *(UINT32 *)&DMAReg->dma_ch[Channel].csr;
	reg |= DMA_CSR_CH_ENABLE;
	*(UINT32 *)&DMAReg->dma_ch[Channel].csr = reg;
}

void fLib_DisableDMAChannel(INT32 Channel)
{
	UINT32 reg;
	
	reg = *(UINT32 *)&DMAReg->dma_ch[Channel].csr;
	reg &= ~DMA_CSR_CH_ENABLE;
	*(UINT32 *)&DMAReg->dma_ch[Channel].csr = reg;
}

// added by Shawn 2005.5.18
void fLib_EnableDMA(unsigned int value)
{
	unsigned int reg;
	
	value &= 0x1;	// check input
	
	reg = DMAReg->dma_csr;
	
	reg &= ~(0x1);
	reg |= value;
	
	DMAReg->dma_csr = reg;
}

// This function was modified by jerry
void fLib_ClearDMAChannelIntStatus(INT32 Channel)
{
	DMAReg->dma_int_tc_clr = 1 << Channel;
	DMAReg->dma_int_err_clr = 1 << Channel;
}


void fLib_SetDMAChannelCfg(INT32 Channel, fLib_DMA_CH_CSR_t Csr)
{
	DMAReg->dma_ch[Channel].csr = Csr;
}

fLib_DMA_CH_CSR_t fLib_GetDMAChannelCfg(INT32 Channel)
{
	return DMAReg->dma_ch[Channel].csr;
}


void fLib_SetDMAChannelCnCfg(INT32 Channel, fLib_DMA_CH_CFG_t CnCfg)
{
	DMAReg->dma_ch[Channel].cfg = CnCfg;
}

fLib_DMA_CH_CFG_t fLib_GetDMAChannelCnCfg(INT32 Channel)
{
	return DMAReg->dma_ch[Channel].cfg;
}

void fLib_DMA_CHIntMask(INT32 Channel, fLib_DMA_CH_CFG_t Mask)
{
	DMAReg->dma_ch[Channel].cfg = Mask;
}

void fLib_DMA_CHLinkList(INT32 Channel, fLib_DMA_CH_LLP_t LLP)
{
	DMAReg->dma_ch[Channel].llp = LLP;
}

void fLib_DMA_CHDataCtrl(INT32 Channel, UINT32 SrcAddr, UINT32 DstAddr, UINT32 Size)
{
	DMAReg->dma_ch[Channel].src_addr = SrcAddr/*CPU_TO_AHB_ADDRSPACE(SrcAddr)*/;
	DMAReg->dma_ch[Channel].dst_addr = DstAddr/*CPU_TO_AHB_ADDRSPACE(DstAddr)*/;
	DMAReg->dma_ch[Channel].size = Size;
}


void fLib_DMA_LinkMode(
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
int    req
)
{
	fLib_DMA_CH_t DMAChannel;
	UINT32 LLPSize, i, offset;
	fLib_DMA_LLD_t *LLP;
	UINT32 Count = 0;
	UINT32 AHB_SrcAddr = /*CPU_TO_AHB_ADDRSPACE*/(SrcAddr);
	UINT32 AHB_DstAddr = /*CPU_TO_AHB_ADDRSPACE*/(DstAddr);
	int burst_size = burst_size_selection[SrcSize];			// ref: [18:16] in Cn_CSR
		
	LLP = (fLib_DMA_LLD_t *)LinkAddr;
	*((unsigned int *)&DMAChannel.csr) = 0;		// clear value of csr;
	
#ifdef DBG_DMA
	fLib_printf("Ch%d, Src=%08X, Dst=%08X, Size=%08X SrcWidth=%db, DstWidth=%db, SrcSize=%dB\n"
           "SrcCtrl=%s, DstCtrl=%s, Priority=%d, Mode=%s, LLPCnt = %d\n",
	       Channel, SrcAddr, DstAddr, Size, 1 << (SrcWidth + 3), 1 << (DstWidth + 3), 
	       ((SrcSize == 0) ? 1 : 1 << (SrcSize+1)), 
	       ((SrcCtrl == 0) ? "Inc" : ((SrcCtrl == 1) ? "Dec" : "Fix")),
	       ((DstCtrl == 0) ? "Inc" : ((DstCtrl == 1) ? "Dec" : "Fix")),
	       Priority, ((Mode == 0) ? "Normal" : "HW"), LLPCount);
#endif	
	fLib_printf("size: %x \n", Size);
	Size = Size / (1 << SrcWidth);		// how many unit want to transfer
	
	if(LLPCount && LinkAddr)
	{
	LLPSize = CN_SIZE;
	if (req != 0)		// memory to memory does not have this restriction
	{
		LLPSize = RoundDown(CN_SIZE, burst_size);			// how many cycle a descriptor can transfer
		if((Size%burst_size)==0){
			fLib_printf("size: %x burst_size: %x ????\n", Size, burst_size);
			while(1);
		}
	}
	Count = divRoundDown(Size, LLPSize);		// how many link-list structure need to fill
	fLib_printf("ASSERT: Count(%d)<=LLPCount(%d) size %x llpsize %x\n",Count, LLPCount,Size,LLPSize);
	if(Count<=LLPCount){
		fLib_printf("2 ????\n");
		while(1);

	}

    // At last, 2 part
    if (Count > 0)
    {
			offset = LLPSize << SrcWidth;						
	   	for(i = 0; i < Count ;i++)
	   	{
		  	if (SrcCtrl == 0)  // increase
			 	LLP[i].src_addr = (UINT32)AHB_SrcAddr + ((i+1) * offset);
		  	else if(SrcCtrl==1) // decrease
			 	LLP[i].src_addr = (UINT32)AHB_SrcAddr - ((i+1) * offset);
		  	else if(SrcCtrl==2)	// fixed
			 	LLP[i].src_addr = (UINT32)AHB_SrcAddr;
		
		  	if(DstCtrl == 0)  
			 	LLP[i].dst_addr = (UINT32)AHB_DstAddr + ((i+1) * offset);
		  	else if(DstCtrl == 1)	// Decrease
			 	LLP[i].dst_addr = (UINT32)AHB_DstAddr - ((i+1) * offset);
		  	else if(DstCtrl == 2)
			 	LLP[i].dst_addr = (UINT32)AHB_DstAddr;
			
		  	*((UINT32 *)&(LLP[i].llp)) = 0;
		  	LLP[i].llp.link_list_addr = /*CPU_TO_AHB_ADDRSPACE*/((UINT32)&LLP[i+1]) >> 2;
		
		  	*((UINT32 *)&(LLP[i].llp_ctrl)) = 0;
		  	LLP[i].llp_ctrl.tc_msk = 1;
		  	LLP[i].llp_ctrl.src_width = SrcWidth; /* source transfer size */
		  	LLP[i].llp_ctrl.dst_width = DstWidth; /* destination transfer size */
		  	LLP[i].llp_ctrl.src_ctrl = SrcCtrl; /* source increment, decrement or fix */
		  	LLP[i].llp_ctrl.dst_ctrl = DstCtrl; /* destination increment, decrement or fix */
		  	LLP[i].llp_ctrl.src_sel = 0; /* source AHB master id */
		  	LLP[i].llp_ctrl.dst_sel = 0; /* destination AHB master id */
		  	LLP[i].llp_ctrl.ff_th = 2; //FIE7021 fifo threshold value = 4
			LLP[i].size = LLPSize;
			Size -= LLPSize;
		}
	   	LLP[i-1].llp.link_list_addr = 0;
	   	LLP[i-1].llp_ctrl.tc_msk = 0;	// Enable tc status
       	LLP[i-1].size = Size;
       	Size = LLPSize;
	}
	
#ifdef DBG_DMA
	for(i = 0; i < LLPCount;i++)
	{
		fLib_printf("src=%0.8X, dst=%0.8X, link=%0.8X, ctrl=%.8X\n", LLP[i].src_addr, LLP[i].dst_addr,
			*(UINT32 *)&(LLP[i].llp), *(UINT32 *)&(LLP[i].llp_ctrl));

	}
#endif
}
	/* program channel */
	fLib_ClearDMAChannelIntStatus(Channel);

	/* program channel CSR */
   	DMAChannel.csr.ff_th = 2; //FIE7021 fifo threshold value = 4
   	DMAChannel.csr.priority = Priority; /* priority */
   	DMAChannel.csr.prot = 0; /* PROT 1-3 bits */
   	DMAChannel.csr.src_size = SrcSize; /* source burst size */
	//FIE7021 this bit should be 0 when change other bits
   	DMAChannel.csr.abt = 0; /* NOT transaction abort */
   	DMAChannel.csr.src_width = SrcWidth; /* source transfer size */
   	DMAChannel.csr.dst_width = DstWidth; /* destination transfer size */
   	DMAChannel.csr.mode = Mode; /* Normal mode or Hardware handshake mode */
   	DMAChannel.csr.src_ctrl = SrcCtrl; /* source increment, decrement or fix */
   	DMAChannel.csr.dst_ctrl = DstCtrl; /* destination increment, decrement or fix */
   	DMAChannel.csr.src_sel = 0; /* source AHB master id */
   	DMAChannel.csr.dst_sel = 0; /* destination AHB master id */

	DMAChannel.csr.reserved1 = 0;
	DMAChannel.csr.reserved0 = 0;
	
	/* program channel CFG */
	DMAChannel.cfg.int_tc_msk = 0;				// Enable tc status
	DMAChannel.cfg.int_err_msk = 0;
	DMAChannel.cfg.src_rs = req;
	DMAChannel.cfg.dst_rs = req;
	DMAChannel.cfg.src_he = (SrcCtrl == 2);		// SrcCtrl==2 means fix source address ==> peripheral
	DMAChannel.cfg.dst_he = (DstCtrl == 2);
	DMAChannel.cfg.busy = 0;			
	DMAChannel.cfg.reserved1 = 0;		
	DMAChannel.cfg.llp_cnt = 0;			
	DMAChannel.cfg.reserved2 = 0;
	
   /* program channel llp */
   *((UINT32 *)&(DMAChannel.llp)) = 0;

	if (Count > 0)
	{
		DMAChannel.csr.tc_msk = 1; /* enable terminal count */
		DMAChannel.llp.link_list_addr = /*CPU_TO_AHB_ADDRSPACE*/((UINT32)&LLP[0]) >> 2;
	}
	else
	{
		DMAChannel.csr.tc_msk = 0; /* no LLP */
	}
	
	fLib_SetDMAChannelCfg(Channel, DMAChannel.csr);
	fLib_DMA_CHIntMask(Channel, DMAChannel.cfg);
	fLib_DMA_CHLinkList(Channel, DMAChannel.llp);
   	
   	/* porgram address and size */
   	fLib_DMA_CHDataCtrl(Channel, SrcAddr, DstAddr, Size);
}


void fLib_DMA_NormalMode(
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
int    req
)
{	    
	fLib_DMA_LinkMode(
			Channel,   // use which channel for AHB DMA, 0..7
			NULL,
			0,  // total link-list node
			SrcAddr,   // source begin address
			DstAddr,   // dest begin address
			Size,      // total bytes
			SrcWidth,  // source width 8/16/32 bits -> 0/1/2
			DstWidth,  // dest width 8/16/32 bits -> 0/1/2
			SrcSize,   // source burst size, How many "SrcWidth" will be transmmited at one times ?
			SrcCtrl,   // source address change : Inc/dec/fixed --> 0/1/2
			DstCtrl,   // dest address change : Inc/dec/fixed --> 0/1/2
			Priority,  // priority for this chaanel 0(low)/1/2/3(high)
			Mode,      // Normal/Hardwire,   0/1
			req);
}


void fLib_DMA_SetInterrupt(UINT32 channel, UINT32 tcintr, UINT32 errintr, UINT32 abtintr)
{
	fLib_DMA_CH_CFG_t cfg;
//	int i;
	
	cfg =  fLib_GetDMAChannelCnCfg(channel); //ycmo091007 add
	
	if(tcintr)
		cfg.int_tc_msk = 0;	// Enable terminal count interrupt
	else
		cfg.int_tc_msk = 1;	// Disable terminal count interrupt
		
	if(errintr)
		cfg.int_err_msk = 0;	// Enable error interrupt
	else
		cfg.int_err_msk = 1;	// Disable error interrupt
		
	if(abtintr)
		cfg.int_abt_msk = 0;	// Enable abort interrupt	
	else
		cfg.int_abt_msk = 1;	// Disable abort interrupt	
	
	fLib_DMA_CHIntMask(channel, cfg);	
}

void fLib_DMA_ResetChannel(UINT8 channel)
{
	UINT32 base = DMAC_FTDMAC020_PA_BASE+DMA_CHANNEL0_BASE+channel*DMA_CHANNEL_OFFSET;	
	
	outw(base+DMA_CHANNEL_CSR_OFFSET,0);
	outw(base+DMA_CHANNEL_CFG_OFFSET,7);
	outw(base+DMA_CHANNEL_SRCADDR_OFFSET,0);
	outw(base+DMA_CHANNEL_DSTADDR_OFFSET,0);
	outw(base+DMA_CHANNEL_LLP_OFFSET,0);
	outw(base+DMA_CHANNEL_SIZE_OFFSET,0);
}

void fLib_DMA_ClearAllInterrupt()
{
	// Clear all interrupt source
	outw(DMAC_FTDMAC020_PA_BASE+DMA_INT_TC_CLR,0xFF);
	outw(DMAC_FTDMAC020_PA_BASE+DMA_INT_ERRABT_CLR,0xFF00FF);	
}

void fLib_DMA_WaitIntStatus(UINT32 Channel)
{
	UINT32 choffset;
	volatile UINT32 status;
	
	choffset = 1 << Channel;

	while((inw(DMAC_FTDMAC020_PA_BASE+DMA_TC)&choffset)==0)
    	;

	
	fLib_DisableDMAChannel(Channel);
}

int fLib_DMA_ChannelNum()
{
	return (readl(DMAC_FTDMAC020_PA_BASE+DMA_FEATURE)>>12)&0xf;
}

void fLib_DMA_EnableDMAInt(void)
{
	DMA_INT_OCCURRED = 0;
	NVIC_EnableIRQ(DMA_FTDMAC020_0_IRQ);	 
}

void fLib_DMA_DisableDMAInt(void)
{
	NVIC_DisableIRQ(DMA_FTDMAC020_0_IRQ);
}

void fLib_DMA_EnableDMATCInt(void)
{
	DMA_TC_INT_OCCURRED = 0;
	NVIC_EnableIRQ(DMA_FTDMAC020_0_TC_IRQ);	  
}

void fLib_DMA_DisableDMATCInt(void)
{
	NVIC_DisableIRQ(DMA_FTDMAC020_0_TC_IRQ);	
}

void fLib_DMA_WaitDMAInt(UINT32 channel)
{	
	while(!(DMA_INT_OCCURRED & (1 << channel)))
	{
	  __WFE (); 								   // Power-Down until next Event/Interrupt				
	}	
	
	DMA_INT_OCCURRED &= ~(1 << channel);
}

void fLib_DMA_WaitDMATCInt(UINT32 channel)
{	
	while(!(DMA_TC_INT_OCCURRED & (1 << channel)))
	{
	  __WFE (); 								   // Power-Down until next Event/Interrupt
	}	
	
	DMA_TC_INT_OCCURRED &= ~(1 << channel);
}
