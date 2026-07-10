#include <string.h>
#include <stdlib.h>

#include "leo_cm33.h"
#include "io.h"
#include "types.h"

#include "DrvDMAC020.h"
#include "DrvUART010.h"
//#include "DrvSPI020.h"

#define DMA_CHANNEL_COUNT 8 
#define min_t(x,y) ( x < y ? x: y )

#define DATA_BUF_SIZE		0x80//0x1000//0x200000//0x2000  0x100000:1M 0x1000:4K
//#define SPI020_DMA_DATAPORT	(SPI_FTSPI020_PA_BASE+0x100)

extern UINT32 rd_bl_len, wr_bl_len;

int DMA_MemToMemTest0(UINT32 *SrcAddr, UINT32 *DstAddr, UINT32 Size);
int DMA_MemToMemTest1(UINT32 *SrcAddr, UINT32 *DstAddr, UINT32 Size, UINT32 LLPCount);
int DMA_MemToMemTest2(UINT32 *Addr0, UINT32 *Addr1, UINT32 *Addr2, UINT32 *Addr3, UINT32 Size, UINT32 LLPCount);

UINT32 AHBDMA_Channel = 0;

//void spin_lock_cachesave(char);
//void spin_unlock_cache(void);

extern void fLib_DisableCache(void);
#if 0
static char dma_buf1[DATA_BUF_SIZE/*+0x3*/];
static char dma_buf2[DATA_BUF_SIZE/*+0x3*/];
static char dma_buf3[DATA_BUF_SIZE/*+0x3*/];
static char dma_buf4[DATA_BUF_SIZE/*+0x3*/];
#endif
uint32_t volatile msTicks;                       // Counter for millisecond Interval


#if 0
void SysTick_Handler (void) {                    // SysTick Interrupt Handler
	  msTicks++;                                     // Increment Counter
}
#endif
extern void WaitForTick (void);
extern void delay_ms(int msec);
#if 0
void WaitForTick (void)  {
  uint32_t curTicks;

  curTicks = msTicks;                            // Save Current SysTick Value
  while (msTicks == curTicks)  {                 // Wait for next SysTick Interrupt
	  __WFE (); 								   // Power-Down until next Event/Interrupt
  }

}

void delay_ms(int msec)
{
    for (;msec != 0; --msec)
    {
        WaitForTick();
    }
}
#endif
#if 0
static void DMA_Memory_To_SPI_Flash(UINT8 *SrcAddr, UINT32 DstAddr, UINT32 Size)
{
	UINT32 i;

	UINT32 write_len=Size;
	UINT32 access_byte;
	UINT32 offset=0;
	fLib_DMA_CH_CSR_t csr;
	fLib_DMA_CH_CFG_t	cfg;
	
	
  memset(&csr, 0x0, sizeof(fLib_DMA_CH_CSR_t));
	memset(&cfg, 0x0, sizeof(fLib_DMA_CH_CFG_t));
	
  for(i=0;i<write_len;i++)//Prepare written data
  {
	  SrcAddr[i]=i;
  }

  spi020_flash_64kErase(offset);
	fLib_InitDMA(FALSE, FALSE, 0x0);				
  fLib_DMA_ClearAllInterrupt();
	fLib_DMA_EnableDMAInt(); //Enable DMA Interrupt		
	fLib_DMA_ResetChannel(0);	
	csr.dst_ctrl = AHBDMA_DstFix;
	csr.src_ctrl = AHBDMA_SrcInc;
	csr.mode = AHBDMA_HwHandShakeMode;
	csr.dst_width = AHBDMA_DstWidth_DWord;
	csr.src_width = AHBDMA_SrcWidth_DWord;		
	cfg.dst_rs = SPI_DMA_REQ;
	cfg.dst_he = 1;
  cfg.src_rs = 0;
  cfg.src_he = 0;
  cfg.int_abt_msk = 0;	
	cfg.int_err_msk = 0;
	cfg.int_tc_msk = 0;
		
	fLib_SetDMAChannelCfg(0, csr);
	fLib_SetDMAChannelCnCfg(0, cfg);
	
	while(write_len>0)
	{		
		access_byte=min_t(write_len, spi020_txfifo_depth());
		spi020_flash_write(FLASH_DMA_WRITE, offset, access_byte, NULL);		
		fLib_DMA_CHDataCtrl(0, ((UINT32)SrcAddr)+offset, DstAddr, access_byte>>2);		
		fLib_EnableDMAChannel(0);
		fLib_DMA_WaitDMAInt(0);		
		fLib_DisableDMAChannel(0);  
		spi020_dma_write_stop();
		write_len-=access_byte;
		offset+=access_byte;
	}	
	
	fLib_DMA_DisableDMAInt(); //Disable DMA Interrupt				
}

static void DMA_SPI_Flash_To_Memory(UINT32 SrcAddr, UINT8 *DstAddr, UINT32 Size)
{	
	UINT32 read_len=Size;	
	fLib_DMA_CH_CSR_t csr;
	fLib_DMA_CH_CFG_t	cfg;
	
	
	memset(&csr, 0x0, sizeof(fLib_DMA_CH_CSR_t));
	memset(&cfg, 0x0, sizeof(fLib_DMA_CH_CFG_t));
  memset(DstAddr,0,sizeof(DstAddr));			
	spi020_flash_read(FLASH_DMA_READ, 0, read_len, NULL);
	fLib_InitDMA(FALSE, FALSE, 0x0);				
  fLib_DMA_ClearAllInterrupt();
	fLib_DMA_EnableDMAInt(); //Enable DMA Interrupt					
	fLib_DMA_ResetChannel(0);
	csr.dst_ctrl = AHBDMA_DstInc;
	csr.src_ctrl = AHBDMA_SrcFix;
	csr.mode = AHBDMA_HwHandShakeMode;
	csr.dst_width = AHBDMA_DstWidth_DWord;
	csr.src_width = AHBDMA_SrcWidth_DWord;		
	cfg.dst_rs = 0;
	cfg.dst_he = 0;
	cfg.src_rs = SPI_DMA_REQ;
	cfg.src_he = 1;
	cfg.int_abt_msk = 0;	
	cfg.int_err_msk = 0;
	cfg.int_tc_msk = 0;
		
	fLib_SetDMAChannelCfg(0, csr);
	fLib_SetDMAChannelCnCfg(0, cfg);
	fLib_DMA_CHDataCtrl(0, SrcAddr, (UINT32)DstAddr, read_len>>2);		
	fLib_EnableDMAChannel(0);
	fLib_DMA_WaitDMAInt(0);		
	fLib_DisableDMAChannel(0);  
	spi020_dma_read_stop();			
	
	fLib_DMA_DisableDMAInt(); //Disable DMA Interrupt	
}

void DMA_Mem_SPI_Test(void)
{
	unsigned int *ptr1 = (unsigned int *)dma_buf1;
	unsigned int *ptr2 = (unsigned int *)dma_buf2;
	int i;
	

	fLib_printf("\rMemory <-> SPI Test\n");				
  
	DMA_Memory_To_SPI_Flash((UINT8 *)ptr1, SPI020_DMA_DATAPORT, DATA_BUF_SIZE);	
	DMA_SPI_Flash_To_Memory(SPI020_DMA_DATAPORT, (UINT8 *)ptr2, DATA_BUF_SIZE);	
    
  if(memcmp(ptr1, ptr2, DATA_BUF_SIZE) != 0)
  {
     for(i = 0; i < (DATA_BUF_SIZE/4); i++)
     {
        if(ptr1[i] != ptr2[i])
        {
            //fail("========>DstAddr[%0.8X] = %0.8X, should be %0.8X\n", i, ptr2[i], ptr1[i]);
						fLib_printf("[FAIL]========>DstAddr[%0.8X] = %0.8X, should be %0.8X\n", i, ptr2[i], ptr1[i]);	
            return;
        }
    }
  }   
  
	fLib_printf("\rPASS!\n");  	
}
#endif

#if 0
void DMA_Interrupt_Test(void)
{
	unsigned int *ptr1 = (unsigned int *)dma_buf1;
	unsigned int *ptr2 = (unsigned int *)dma_buf2;
	
	
	fLib_InitDMA(FALSE, FALSE, 0x0);			
	fLib_DMA_ClearAllInterrupt();  
	
	AHBDMA_Channel = 2;	
	fLib_DMA_ResetChannel(AHBDMA_Channel);	
	fLib_DMA_EnableDMAInt(); //Enable DMA Interrupt			
	fLib_DMA_NormalMode(AHBDMA_Channel, (UINT32)ptr1, (UINT32)ptr2, 0x100, 0, 0, 0, 0, 0, 0, 0, 0);
	fLib_EnableDMAChannel(AHBDMA_Channel);  
	fLib_DMA_WaitDMAInt(AHBDMA_Channel);
	fLib_printf("DMA Intr occurred!\n");	
  fLib_DisableDMAChannel(AHBDMA_Channel);  
	fLib_DMA_DisableDMAInt(); //Disable DMA Interrupt

	AHBDMA_Channel = 1;		
	fLib_DMA_ResetChannel(AHBDMA_Channel);	
	fLib_DMA_EnableDMATCInt(); //Enable DMA Terminal Count Interrupt	
	fLib_DMA_NormalMode(AHBDMA_Channel, (UINT32)ptr1, (UINT32)ptr2, 0x100, 0, 0, 0, 0, 0, 0, 0, 0);
	fLib_EnableDMAChannel(AHBDMA_Channel);  
	fLib_DMA_WaitDMATCInt(AHBDMA_Channel);	
  fLib_printf("DMA Intr TC occurred!\n");			
  fLib_DisableDMAChannel(AHBDMA_Channel);  
	fLib_DMA_DisableDMATCInt(); //Disable DMA Terminal Count Interrupt
}
#endif
UINT32 DMA_Mem_Mem_Test(void)
{
	int i;
#if 0
	unsigned int *ptr1 = (unsigned int *)dma_buf1;
	unsigned int *ptr2 = (unsigned int *)dma_buf2;
	unsigned int *ptr3 = (unsigned int *)dma_buf3;
	unsigned int *ptr4 = (unsigned int *)dma_buf4; 		
#else
	unsigned int *ptr1 = (unsigned int *)(FREE_MEM_BASE );
	unsigned int *ptr2 = (unsigned int *)(FREE_MEM_BASE + 0x100);
	unsigned int *ptr3 = (unsigned int *)(FREE_MEM_BASE + 0x180);
	unsigned int *ptr4 = (unsigned int *)(FREE_MEM_BASE + 0x200); 		
#endif
	
	int channel_num;
	
  fLib_printf("\rMemory <-> Memory Test\n");
	fLib_printf("\rptr1=%x, ptr2=%x,ptr3=%x, ptr4=%x\n",ptr1, ptr2, ptr3, ptr4);

  fLib_InitDMA(FALSE, FALSE, 0x0);	
	fLib_DMA_ClearAllInterrupt(); 
	fLib_DMA_EnableDMAInt(); //Enable DMA Interrupt					
	channel_num = fLib_DMA_ChannelNum();

#if 0
  for (i = 0; i < channel_num; i++)
  {
     AHBDMA_Channel = i;
  
		if(!DMA_MemToMemTest0(ptr1, ptr2, DATA_BUF_SIZE))
     {
        //fail("\rFail!\n");
			  fLib_printf("\rFail!\n"); //legend
        return 0;
     }
		 
     fLib_DMA_ResetChannel(AHBDMA_Channel);     
 }
#endif
#if 1
 for (i = 0; i < channel_num; i++)
 {
   AHBDMA_Channel = i;

	 if (!DMA_MemToMemTest1(ptr1, ptr2, 0x1000, 7))		//0x1000, 7))
	 //if (!DMA_MemToMemTest1(ptr1, ptr2, 0x1000, 7))		//0x1000, 7))
   {
      //fail("\rFail!\n");
			fLib_printf("\rFail!\n");
      return 0;
   }

   fLib_DMA_ResetChannel(AHBDMA_Channel);   
 }
 #endif
#if 0
  if (!DMA_MemToMemTest2(ptr1, ptr2, ptr3, ptr4, 0x1000, 10))		// 0xA000/* 0x600000 */, 10))
  {
     //fail("\rFail!\n");
		fLib_printf("\rFail!\n");
     return 0;
  }		
#endif
	fLib_DMA_DisableDMAInt(); //Disable DMA Interrupt	
	fLib_printf("\rPASS!\n");  
	return 1;
}


 int DMA_MemToMemTest0(UINT32 *SrcAddr, UINT32 *DstAddr, UINT32 Size)
{
    UINT32 i, SrcWidth, DstWidth, SrcSize;
    UINT32 *TempAddr;

		//fLib_printf("1\n");
		fLib_printf("DMA_MemToMemTest0 for DMA Channel %d\n", AHBDMA_Channel);  
    /* init source */
    for(i = 0; i < (Size/4); i++)
        *(SrcAddr+i) =(UINT32)( SrcAddr+i);   
#if 1
    for(SrcWidth = 0; SrcWidth <= 2; SrcWidth++) 
    {

        for(DstWidth = 0; DstWidth <= 2; DstWidth++)
        {
    
            for(SrcSize = 0; SrcSize <= 7; SrcSize++)
            {
    
            if((SrcWidth < DstWidth) && (SrcSize == 0))
                {
#if 0                   
                    fLib_printf("Can not use this configure!!! SrcWidth=%d. DstWidth=%d, SrcSize=%d\n", 
                        1 << (SrcWidth + 3), 1 << (DstWidth + 3), ((SrcSize == 0) ? 1 : 1 << (SrcSize+1)));
#endif                      
                    break;
                }
                /*
				if (dma_test_cnt==0x36)
				{
					fLib_printf("dma_test_cnt==36\n");
				}*/
			//	display_process(dma_test_cnt++);
                //fLib_printf("************************************************************************\n");
                /////////////////////////////////////////////////////////////// 
                // clear destination, SRC(increment) ==> DST(increment)
                memset(DstAddr, 0, Size);

                fLib_DMA_NormalMode(AHBDMA_Channel, (UINT32)SrcAddr, (UINT32)DstAddr, Size, SrcWidth, DstWidth, SrcSize, 0, 0, 0, 0, 0);
              //fLib_printf("Phase 1:Src=0x%x, Dst=0x%x\r\n",SrcAddr,DstAddr);                
								fLib_EnableDMAChannel(AHBDMA_Channel);
                fLib_DMA_WaitDMAInt(AHBDMA_Channel);
                fLib_DisableDMAChannel(AHBDMA_Channel);  

                if(memcmp(SrcAddr, DstAddr, Size) != 0)
                {
                    for(i = 0; i < (Size/4); i++)
                    {
                      
                        if(DstAddr[i] != SrcAddr[i])
                        {
                            //fail("========>DstAddr[%0.8X] = %0.8X, should be %0.8X\n", i, DstAddr[i], SrcAddr[i]);
													fLib_printf("========>1DstAddr:= %0.8X,SrcAddr= %0.8X\n", DstAddr+i, SrcAddr+i);
														fLib_printf("========>DstAddr[%0.8X] = %0.8X, should be %0.8X\n", i, DstAddr[i], SrcAddr[i]);
                            return FALSE;
                        }
                    }
                }
                
//              fLib_printf("--------------------\n");
                /////////////////////////////////////////////////////////////// 
                // clear destination, SRC+size(descrement) <== DST+size(descrement)
                TempAddr = DstAddr; DstAddr = SrcAddr; SrcAddr = TempAddr;
                memset(DstAddr, 0, Size);

                fLib_DMA_NormalMode(AHBDMA_Channel, (UINT32)(SrcAddr)+Size-1, (UINT32)(DstAddr)+Size-1, Size, SrcWidth, DstWidth, SrcSize, 1, 1, 0, 0, 0);
        //      fLib_printf("Phase 2:Src=0x%x, Dst=0x%x\r\n",(SrcAddr+Size-1),(DstAddr+Size-1));
                fLib_EnableDMAChannel(AHBDMA_Channel);
                fLib_DMA_WaitDMAInt(AHBDMA_Channel);
                fLib_DisableDMAChannel(AHBDMA_Channel);  

                if(memcmp(SrcAddr, DstAddr, Size) != 0)
                {
                    for(i = 0; i < (Size/4); i++)
                    {
                        if(DstAddr[i] != SrcAddr[i])
                        {
                            //fail("========>DstAddr[%0.8X] = %0.8X, should be %0.8X\n", i, DstAddr[i], SrcAddr[i]);
													fLib_printf("========>2DstAddr:= %0.8X,SrcAddr= %0.8X\n", DstAddr+i, SrcAddr+i);
													fLib_printf("========>DstAddr[%0.8X] = %0.8X, should be %0.8X\n", i, DstAddr[i], SrcAddr[i]);
                            return FALSE;
                        }
                    }
                }
//              fLib_printf("--------------------\n");
                /////////////////////////////////////////////////////////////// 
                // clear destination, SRC+size(descrement) ==> DST(increment)
                TempAddr = DstAddr; DstAddr = SrcAddr; SrcAddr = TempAddr;
                memset(DstAddr, 0, Size);

                fLib_DMA_NormalMode(AHBDMA_Channel, (UINT32)(SrcAddr)+Size-1, (UINT32)DstAddr, Size, SrcWidth, DstWidth, SrcSize, 1, 0, 0, 0, 0);
        //      fLib_printf("Phase 3:Src=0x%x, Dst=0x%x\r\n",(SrcAddr+Size-1),DstAddr);
                fLib_EnableDMAChannel(AHBDMA_Channel);
                fLib_DMA_WaitDMAInt(AHBDMA_Channel);
                fLib_DisableDMAChannel(AHBDMA_Channel);  

                /////////////////////////////////////////////////////////////// 
                // clear destination, SRC+size(increment) ==> DST(descrement)
                TempAddr = DstAddr; DstAddr = SrcAddr; SrcAddr = TempAddr;
                memset(DstAddr, 0, Size);

                fLib_DMA_NormalMode(AHBDMA_Channel, (UINT32)SrcAddr, (UINT32)(DstAddr)+Size-1, Size, SrcWidth, DstWidth, SrcSize, 0, 1, 0, 0, 0);
        //      fLib_printf("Phase 4:Src=0x%x, Dst=0x%x\r\n",SrcAddr,(DstAddr+Size-1));
                fLib_EnableDMAChannel(AHBDMA_Channel);
                fLib_DMA_WaitDMAInt(AHBDMA_Channel);
                fLib_DisableDMAChannel(AHBDMA_Channel);  

                /////////////////////////////////////////////////////////////// 
                // clear destination, SRC+size(descrement) ==> DST(increment)
                TempAddr = DstAddr; DstAddr = SrcAddr; SrcAddr = TempAddr;
                memset(DstAddr, 0, Size);

                fLib_DMA_NormalMode(AHBDMA_Channel, (UINT32)(SrcAddr)+Size-1, (UINT32)DstAddr, Size, SrcWidth, DstWidth, SrcSize, 1, 0, 0, 0, 0);
        //      fLib_printf("Phase 5:Src=0x%x, Dst=0x%x\r\n",(SrcAddr+Size-1),DstAddr);
                fLib_EnableDMAChannel(AHBDMA_Channel);
                fLib_DMA_WaitDMAInt(AHBDMA_Channel);
                fLib_DisableDMAChannel(AHBDMA_Channel);  

                /////////////////////////////////////////////////////////////// 
                // clear destination, SRC+size(increment) ==> DST(decrement)
                TempAddr = DstAddr; DstAddr = SrcAddr; SrcAddr = TempAddr;
                memset(DstAddr, 0, Size);

                fLib_DMA_NormalMode(AHBDMA_Channel, (UINT32)SrcAddr, (UINT32)(DstAddr)+Size-1, Size, SrcWidth, DstWidth, SrcSize, 0, 1, 0, 0, 0);
        //      fLib_printf("Phase 6:Src=0x%x, Dst=0x%x\r\n",SrcAddr,(DstAddr+Size-1));
                fLib_EnableDMAChannel(AHBDMA_Channel);
                fLib_DMA_WaitDMAInt(AHBDMA_Channel);
                fLib_DisableDMAChannel(AHBDMA_Channel);  
                
                
                    for(i = 0; i < (Size/4); i++)
                    {   
                                    
                        if(DstAddr[i] != (UINT32)(DstAddr+i))
                        {
                            //fail("========>DstAddr[%0.8X] = %0.8X, should be %0.8X\n", i, (UINT32)DstAddr[i], (UINT32)DstAddr+i);
													fLib_printf("========>3DstAddr:= %0.8X,SrcAddr= %0.8X\n", DstAddr+i, SrcAddr+i);
														fLib_printf("========>DstAddr[%0.8X] = %0.8X, should be %0.8X\n", i*4, (UINT32)DstAddr[i], (UINT32)DstAddr+(i*4));
                            return FALSE;
                        }
                    }
            
                
                TempAddr = DstAddr; DstAddr = SrcAddr; SrcAddr = TempAddr;
                
            //  fLib_printf("DMA configure: SrcWidth=%d. DstWidth=%d, SrcSize=%d test pass...\n", 
            //      1 << (SrcWidth + 3), 1 << (DstWidth + 3), ((SrcSize == 0) ? 1 : 1 << (SrcSize+1)));             
            //  fLib_printf("OKOKOKOK!!!\n");
            }
        }
    }
    #endif
    return TRUE;
}


int DMA_MemToMemTest1(UINT32 *SrcAddr, UINT32 *DstAddr, UINT32 Size, UINT32 LLPCount)
{
    UINT32 i, SrcWidth, DstWidth, SrcSize;
    UINT32 *TempAddr;
    UINT32 *LinkAddr;
    fLib_printf("DMA_MemToMemTest1 link mode for DMA Channel %d src %x dst %x Size %x\n", AHBDMA_Channel,SrcAddr,DstAddr,Size);
    
//  LinkAddr = (UINT32 *)malloc(sizeof(fLib_DMA_LLD_t) * LLPCount);
 //LinkAddr = (UINT32 *)LINK_ADDR;
    if ( (LinkAddr = (UINT32 *)malloc(sizeof(fLib_DMA_LLD_t) * LLPCount))==0 )
    {
    	fLib_printf("memory allocate fail\n");
    	for (;;) ;
    }
    fLib_printf("##### LinkAddr is 0x%x #####\r\n",LinkAddr);
    /* init source */
    for(i = 0; i < (Size/4); i++){
			  SrcAddr[i] = (UINT32)(SrcAddr+i);
		}
    
    for(SrcWidth = 0; SrcWidth <= 2; SrcWidth++)
    {
//  SrcWidth=2;
        for(DstWidth = 0; DstWidth <= 2; DstWidth++)
        {
    //  DstWidth=2;
            for(SrcSize = 0; SrcSize <= 7; SrcSize++)
            {
            
                if((SrcWidth < DstWidth) && (SrcSize == 0))
                {
#if 0                   
                    fLib_printf("Can not use this configure!!! SrcWidth=%d. DstWidth=%d, SrcSize=%d\n", 
                        1 << (SrcWidth + 3), 1 << (DstWidth + 3), ((SrcSize == 0) ? 1 : 1 << (SrcSize+1)));
#endif                      
                    break;
                }
            
            	//display_process(dma_test_cnt++);
                //fLib_printf("************************************************************************\n");
                /////////////////////////////////////////////////////////////// 
                /* clear destination, SRC(increment) ==> DST(increment) */
                memset(DstAddr, 0, Size);
                
                fLib_DMA_LinkMode(AHBDMA_Channel, (UINT32)LinkAddr, LLPCount, (UINT32)SrcAddr, (UINT32)DstAddr, Size, SrcWidth, DstWidth, SrcSize, 0, 0, 0, 0, 0);
                fLib_EnableDMAChannel(AHBDMA_Channel);
                fLib_DMA_WaitDMAInt(AHBDMA_Channel);
                fLib_DisableDMAChannel(AHBDMA_Channel);  

                if(memcmp(SrcAddr, DstAddr, Size) != 0)
                {
                    for(i = 0; i < (Size/4); i++)
                    {
                        if(DstAddr[i] != SrcAddr[i])
                        {
                            fLib_printf("========>1DstAddr:= %0.8X,SrcAddr= %0.8X\n", DstAddr+i, SrcAddr+i);
														fLib_printf("========>DstAddr[%0.8X] = %0.8X, should be %0.8X\n", i, DstAddr[i], SrcAddr[i]);
                            free(LinkAddr);
                            return FALSE;
                        }
                    }
                }
                //fLib_printf("--------------------\n");
                /////////////////////////////////////////////////////////////// 
                /* clear destination, SRC+size(descrement) <== DST+size(descrement) */
                TempAddr = DstAddr; DstAddr = SrcAddr; SrcAddr = TempAddr;
                memset(DstAddr, 0, Size);
                
                fLib_DMA_LinkMode(AHBDMA_Channel, (UINT32)LinkAddr, LLPCount, (UINT32)(SrcAddr)+Size-1, (UINT32)(DstAddr)+Size-1, Size, SrcWidth, DstWidth, SrcSize, 1, 1, 0, 0, 0);
                fLib_EnableDMAChannel(AHBDMA_Channel);
                fLib_DMA_WaitDMAInt(AHBDMA_Channel);
                fLib_DisableDMAChannel(AHBDMA_Channel);  

                if(memcmp(SrcAddr, DstAddr, Size) != 0)
                {
                    for(i = 0; i < (Size/4); i++)
                    {
                        if(DstAddr[i] != SrcAddr[i])
                        {
                            //fail("========>DstAddr[%0.8X] = %0.8X, should be %0.8X\n", i, DstAddr[i], SrcAddr[i]);
													  fLib_printf("========>1DstAddr:= %0.8X,SrcAddr= %0.8X\n", DstAddr+i, SrcAddr+i);
														fLib_printf("========>DstAddr[%0.8X] = %0.8X, should be %0.8X\n", i, DstAddr[i], SrcAddr[i]);
                            free(LinkAddr);
                            return FALSE;
                        }
                    }
                }
                //fLib_printf("--------------------\n");
                /////////////////////////////////////////////////////////////// 
                /* clear destination, SRC+size(descrement) ==> DST(increment) */
                TempAddr = DstAddr; DstAddr = SrcAddr; SrcAddr = TempAddr;
                memset(DstAddr, 0, Size);
            
                fLib_DMA_LinkMode(AHBDMA_Channel, (UINT32)LinkAddr, LLPCount, (UINT32)(SrcAddr)+Size-1, (UINT32)DstAddr, Size, SrcWidth, DstWidth, SrcSize, 1, 0, 0, 0, 0);
                fLib_EnableDMAChannel(AHBDMA_Channel);
                fLib_DMA_WaitDMAInt(AHBDMA_Channel);
                fLib_DisableDMAChannel(AHBDMA_Channel);  
    
                /////////////////////////////////////////////////////////////// 
                /* clear destination, SRC+size(increment) ==> DST(descrement) */
                TempAddr = DstAddr; DstAddr = SrcAddr; SrcAddr = TempAddr;
                memset(DstAddr, 0, Size);

                fLib_DMA_LinkMode(AHBDMA_Channel, (UINT32)LinkAddr, LLPCount, (UINT32)SrcAddr, (UINT32)(DstAddr)+Size-1, Size, SrcWidth, DstWidth, SrcSize, 0, 1, 0, 0, 0);
                fLib_EnableDMAChannel(AHBDMA_Channel);
                fLib_DMA_WaitDMAInt(AHBDMA_Channel);
                fLib_DisableDMAChannel(AHBDMA_Channel);  
            
                /////////////////////////////////////////////////////////////// 
                /* clear destination, SRC+size(descrement) ==> DST(increment) */
                TempAddr = DstAddr; DstAddr = SrcAddr; SrcAddr = TempAddr;
                memset(DstAddr, 0, Size);

                
                fLib_DMA_LinkMode(AHBDMA_Channel, (UINT32)LinkAddr, LLPCount, (UINT32)(SrcAddr)+Size-1, (UINT32)DstAddr, Size, SrcWidth, DstWidth, SrcSize, 1, 0, 0, 0, 0);
                fLib_EnableDMAChannel(AHBDMA_Channel);
                fLib_DMA_WaitDMAInt(AHBDMA_Channel);
                fLib_DisableDMAChannel(AHBDMA_Channel);  

                /////////////////////////////////////////////////////////////// 
                /* clear destination, SRC+size(increment) ==> DST(decrement) */
                TempAddr = DstAddr; DstAddr = SrcAddr; SrcAddr = TempAddr;
                memset(DstAddr, 0, Size);

                
                fLib_DMA_LinkMode(AHBDMA_Channel, (UINT32)LinkAddr, LLPCount, (UINT32)SrcAddr, (UINT32)(DstAddr)+Size-1, Size, SrcWidth, DstWidth, SrcSize, 0, 1, 0, 0, 0);
                fLib_EnableDMAChannel(AHBDMA_Channel);
                fLib_DMA_WaitDMAInt(AHBDMA_Channel);
                fLib_DisableDMAChannel(AHBDMA_Channel);  
            
        //      if(memcmp(SrcAddr, DstAddr, Size) != 0)
                {
                    for(i = 0; i < (Size/4); i++)
                    {
                        if(DstAddr[i] != (UINT32)(DstAddr+i))
                        {
                            //fail("DstAddr[%0.8x] = %0.8x, should be %0.8x\n", i, (UINT32)DstAddr[i], (UINT32)DstAddr+i);
														fLib_printf("DstAddr[%0.8x] = %0.8x, should be %0.8x\n", i, (UINT32)DstAddr[i], (UINT32)DstAddr+i);
                            free(LinkAddr);
                            return FALSE;
                        }
                    }
                }
                
                
                TempAddr = DstAddr; DstAddr = SrcAddr; SrcAddr = TempAddr;              
            //  fLib_printf("OKOKOKOK!!!\n");
            }
        }
    }
    
    free(LinkAddr);
    return TRUE;
}


int DMA_MemToMemTest2(UINT32 *Addr0, UINT32 *Addr1, UINT32 *Addr2, UINT32 *Addr3, UINT32 Size, UINT32 LLPCount)
{
    UINT32 i, SrcWidth, DstWidth, SrcSize;
    UINT32 *LinkAddr0, *LinkAddr1, *LinkAddr2, *LinkAddr3;
    //UINT32 IntStatus, PreIntStatus;

    fLib_printf("DMA_MemToMemTest2 Priority Checking...\n");
    
    for(i = 0; i < (Size/4); i++)
    {
      
        Addr0[i] = (UINT32)(Addr0+i);
    }   
    for(i = 0; i < (Size/4); i++)
    {
       
        Addr1[i] = (UINT32)(Addr1+i);
    }
    
    for(i = 0; i < (Size/4); i++)
    {
        
        Addr2[i] = (UINT32)(Addr2+i);
    }
        
    for(i = 0; i < (Size/4); i++)
    {
        
        Addr3[i] = (UINT32)(Addr3+i);
    }
        
    LinkAddr0 = (UINT32 *)malloc(sizeof(fLib_DMA_LLD_t) * LLPCount);
    LinkAddr1 = (UINT32 *)malloc(sizeof(fLib_DMA_LLD_t) * LLPCount);
    LinkAddr2 = (UINT32 *)malloc(sizeof(fLib_DMA_LLD_t) * LLPCount);
    LinkAddr3 = (UINT32 *)malloc(sizeof(fLib_DMA_LLD_t) * LLPCount);
    
    if ( LinkAddr0==NULL || LinkAddr1==NULL || LinkAddr2==NULL 
    		|| LinkAddr3==NULL )
    {
    	fLib_printf("memory allocate fail\n");
    	for (;;) ;
    }
    
    SrcWidth = 2;
    DstWidth = 2;
    SrcSize = 7;
            
    /* addr0 -> addr1 & addr2 -> addr3 */
    fLib_DMA_LinkMode(0, (UINT32)LinkAddr0, LLPCount, (UINT32)Addr0, (UINT32)Addr1, Size, SrcWidth, DstWidth, SrcSize, 0, 0, 3, 0, 0);
    fLib_DMA_LinkMode(1, (UINT32)LinkAddr2, LLPCount, (UINT32)Addr2, (UINT32)Addr3, Size, SrcWidth, DstWidth, SrcSize, 0, 0, 2, 0, 0);
    
    /* addr1 -> addr2 & addr3 -> add0 */
    fLib_DMA_LinkMode(2, (UINT32)LinkAddr1, LLPCount, (UINT32)Addr1, (UINT32)Addr2, Size, SrcWidth, DstWidth, SrcSize, 0, 0, 1, 0, 0);
    fLib_DMA_LinkMode(3, (UINT32)LinkAddr3, LLPCount, (UINT32)Addr3, (UINT32)Addr0, Size, SrcWidth, DstWidth, SrcSize, 0, 0, 0, 0, 0);
    
    fLib_EnableDMAChannel(0);		
    fLib_EnableDMAChannel(1);
    fLib_EnableDMAChannel(2);
    fLib_EnableDMAChannel(3);

#if 1
		fLib_DMA_WaitDMAInt(0);		
		fLib_DMA_WaitDMAInt(1);		
		fLib_DMA_WaitDMAInt(2);
		fLib_DMA_WaitDMAInt(3);
		fLib_DisableDMAChannel(0);				
    fLib_DisableDMAChannel(1);
    fLib_DisableDMAChannel(2);
    fLib_DisableDMAChannel(3);
#else    
    i = 1;
    PreIntStatus = 0;
	
    do
    {
        IntStatus = fLib_GetDMAIntStatus();
        
        if(IntStatus < PreIntStatus)
        {
            fLib_printf("******** IntStatus Error:%0.2X\n", IntStatus);
            free(LinkAddr0);
            free(LinkAddr1);
            free(LinkAddr2);
            free(LinkAddr3);
            return FALSE;
        }
        else if(IntStatus > PreIntStatus)
        {
//          fLib_printf("IntStatus OK:%0.2X\n", IntStatus);
            i <<= 1;
        }
        
        if ( (IntStatus == 0x0) || (IntStatus == 0x1) || (IntStatus == 0x3) || (IntStatus == 0x7) || (IntStatus == 0xf)){
            PreIntStatus = IntStatus;
        }
        else 
        {
            fLib_printf("DMA priority error\n");
            free(LinkAddr0);
            free(LinkAddr1);
            free(LinkAddr2);
            free(LinkAddr3);
            return 0;
        }
        
    } while((IntStatus & 0xf) != 0xf);
#endif		
    
    free(LinkAddr0);
    free(LinkAddr1);
    free(LinkAddr2);
    free(LinkAddr3);
    return TRUE;
    
}

void DMAC020_test_main(void)
{
	UINT32 i, pass;
	i = 0;
	pass = 0;
	fLib_printf("MEM(DDR) to MEM(DDR) test\n");

	while(1){
		i++;
		if(DMA_Mem_Mem_Test() == 1){
			pass++;
			fLib_printf("\nPASS: %d / %d\n", pass, i);
		}
		else
			fLib_printf("Fail: %d / %d\n", i-pass, i);
	}
	// DMAC020: SPI flash to SiRAM/DDR (XIP mode)
#ifdef FLASH2SiRAM
	fLib_printf("FLASH to MEM(SiRAM) test\n");
	DMA_Mem_Mem_Test(0x30000/*flash base*/,0x10100000/*SiRAM*/,0x81200000,0x91300000); 
#elif FLASH2DDR
	fLib_printf("FLASH to MEM(DDR) test\n");
	DMA_Mem_Mem_Test(0x40000,0x60000000/*DDR*/,0x81200000,0x91300000); 
#endif
}
