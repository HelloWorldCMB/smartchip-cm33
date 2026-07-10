/***************************************************************************
* Copyright  Faraday Technology Corp 2008-2012.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:memtest.c                                                             *
* Description: SoFlexible board specfic routine                            *
* Author: Fred Chien                                                       *
****************************************************************************/


#include <stdio.h>
#include "types.h"
#include "common_include.h"
#include "Driver_Common.h"
#include "memtest.h"
#include "spec.h"

int MemTestByte(int start, int size, unsigned fill_content);
int MemTestByte1(int start, int size, unsigned fill_content);
int MemTestHalfWord(int start, int size, unsigned fill_content);
int MemTestHalfWord1(int start, int size, unsigned fill_content);
int MemTestWord(int start, int size, unsigned fill_content);
int MemTestWord1(int start, int size, unsigned fill_content);

typedef struct
{
	
    UINT32 StdId;
    UINT32 ExtId;
    UINT8 IDE;
    UINT8 RTR;

    UINT8 BRS;
    UINT8 FD;
    UINT8 ESI;
	
    UINT8 DLC;
	//  UINT8 REV[2]; //for alignment
    UINT8 Data[64];
    UINT16 TxTimeStamp;
    UINT16 RxTimeStamp;
} CanRxMsgDef1; 

#define PATTERN1(i) ((i^0x55aa)+((i^0xaa55)<<16))

#define CASE1
#ifdef CASE1
void sram_verify(void)
{
	int i;
  UINT8 data[8] = {0x00, 0xFF, 0x55, 0xAA, 0xCC, 0x33, 0xF0, 0x0F};		
		CanRxMsgDef1 TxMessage;
	  fLib_printf("%x sizeof %x\n",TxMessage.Data,sizeof(TxMessage));
		memcpy(TxMessage.Data, data, 8);	
	//fLib_printf("%x\n",&data2[1]);
	//   memcpy(&data2[1],data,8);
		fLib_printf("over\n");
		while(1);

}
#endif
#ifdef CASE2
void sram_verify(void)
{
	int i;
  UINT8 data[8] = {0x00, 0xFF, 0x55, 0xAA, 0xCC, 0x33, 0xF0, 0x0F};		
	
		CanRxMsgDef1 TxMessage;
		fLib_printf("%x sizeof %x\n",TxMessage.Data,sizeof(TxMessage));
		//memcpy(TxMessage.Data, data, 8);
		for(i=0;i<8;i++){
			//fLib_printf("i\n");
			TxMessage.Data[i]= data[i];
		}		
		fLib_printf("over\n");
		while(1);

}
#endif
#ifdef CASE3
void sram_verify(void)
{
	int i;
  UINT8 data[8] = {0x00, 0xFF, 0x55, 0xAA, 0xCC, 0x33, 0xF0, 0x0F};		
	
		CanRxMsgDef1 TxMessage;
		//fLib_printf("%x sizeof %x\n",TxMessage.Data,sizeof(TxMessage));
		//memcpy(TxMessage.Data, data, 8);
		for(i=0;i<8;i++){
			//fLib_printf("i\n");
			TxMessage.Data[i]= data[i];
		}		
		fLib_printf("over\n");
		while(1);

}
#endif
int ddr_sram_rw_verify()
{
    #if 1 // use DDR3
    //DDR_MEM_BASE: 0x60000000
    #define MEM_BASE (I2S_MEM_BASE)
    #define MEM_SIZE (0x80)
    #else // use SdRam
    //SdRAM_MEM_BASE: 0x10200000
	#define NSRAM 0x28000000
    #define MEM_BASE (SdRAM_MEM_BASE)//NSRAM//(SdRAM_MEM_BASE)
    #define MEM_SIZE (SdRAM_MEM_SIZE)
    #define MEM_BASE4NPU (0x20200000)
    #endif
    fLib_printf("Test mem................\n");
		int cnt=50000;
 
    int i, j, k, len, test_loop;
    k=0;
    int test1_err_cnt = 0;
    int test2_err_cnt = 0;
    int test3_err_cnt = 0;
    test_loop = 100;
    fLib_printf("Test mem 1\n");
    for (k = 0; k < test_loop; k++) {
        for (i = 0; i < MEM_SIZE / 4; i++) {
          outw(MEM_BASE+4 * i, PATTERN1(i)/*MEM_BASE+4 * i*/);
        }
        len = 0;
        for (i=0; i < MEM_SIZE / 4; i++) {
            //for( k=0; k<10; k++ );		
            j = inw(MEM_BASE+4*i);
            if (j != PATTERN1(i)/*MEM_BASE+4 * i*/)
            {
                test1_err_cnt++;
                fLib_printf("%d> Test mem 1 failed (expected=%08x, actual=%08x)\n", i, PATTERN1(i)/*MEM_BASE+4 * i*/, j);
                len++;
                if(len > 10) {
                    fLib_printf("%d> exit\n", i);
                    return 0;
                }
            }
        }
				
        fLib_printf("Test loop %d/%d\n", k, test_loop);
    }

    fLib_printf("Test mem 2\n");
    for (k = 0; k < test_loop; k++) {
        for (i = 0; i < MEM_SIZE / 4; i++) {
            outw(MEM_BASE + 4 * i, (i << 16) + ((i^0xffff)));
        }
        len = 0;
				
        for(i = 0; i < MEM_SIZE / 4; i++) {
            j = inw(MEM_BASE + 4 * i);
            if ( j != ((i << 16) + (i^0xffff))) {
                test2_err_cnt++;
                fLib_printf("Test mem 2 failed ev=%08x, av=%08x\n", (i<<16)+((i^0xffff)), j);
                len++;
                if(len>10) {
                    fLib_printf("%d> exit\n");
                    return 0;
                }
            }
        }
        fLib_printf("Test loop %d/%d\n", k, test_loop);
    }
    
    fLib_printf("Test mem 3\n");
    for (k = 0; k < test_loop; k++) {
        for (i = 0; i < MEM_SIZE / 4; i++) { 
            outw(MEM_BASE + 4 * i, 0);
        }
        len=0;
				
        for(i = 0; i < MEM_SIZE / 4; i++) {
            j = inw(MEM_BASE + 4 * i);
            if ( j != (0)) {
                test3_err_cnt++;
                fLib_printf("Test mem 3 failed ev=%08x, av=%08x\n", 0, j);
                len++;
                if (len > 10) {
                    fLib_printf("%d> exit\n");
                    return 0;
                }
            }
        }
        fLib_printf("Test loop %d/%d\n", k, test_loop);
    }
    fLib_printf("End of test\n");    
    fLib_printf("Test result mem1_test %d/%d, mem2_test=%d/%d, mem3_test=%d/%d\n", 
            test_loop - test1_err_cnt, test_loop, 
            test_loop - test2_err_cnt, test_loop, 
            test_loop - test3_err_cnt, test_loop);        
		
		return 0;
}

//void Mem_Test_Main(int test_base,int test_size,int test_iteration)
int Mem_Test_Main(int test_iteration)
{
    unsigned int low_start;
    unsigned int iter;
    unsigned int default_start = FREE_MEM_BASE;
    int test_size;
	     
		low_start = default_start;
		//test_iteration=1;
    test_size = 0x200;
	
    fLib_printf("default address and size: (Mem 0x%x~0x%x)\r\n", default_start, default_start+test_size-1);
    fLib_printf(" Start Mem Test 0x%x ~ 0x%x ....\n", low_start, low_start+test_size-1);

    for(iter=0;iter<test_iteration;iter++)
    {
        //Word Bank Width 32 
        fLib_printf("Mem test by WORD = Address ...\n");
        fLib_printf("(1)Clear all, (2)write, (3)read , (4)compare\n");
        if(!MemTestWord(low_start, test_size, FILL_CONTENT_ADDRESS))
					return 0;
     
        fLib_printf("(1)write all, (2)read , (3)compare\n");
        if(!MemTestWord1(low_start, test_size, FILL_CONTENT_ADDRESS))
					return 0;
    
        fLib_printf("Mem test by WORD = 0x%8x ...\n",DATA_PATTERN4);
        fLib_printf("(1)Clear all, (2)write, (3)read , (4)compare\n");
        if(!MemTestWord(low_start, test_size, DATA_PATTERN4))
					return 0;
  
        fLib_printf("(1)write all, (2)read , (3)compare\n");
        if(!MemTestWord1(low_start, test_size, DATA_PATTERN4))
					return 0;
    
        fLib_printf("Mem test by WORD = 0x%8x ...\n",DATA_PATTERN5 );        
        fLib_printf("(1)Clear all, (2)write, (3)read , (4)compare\n");
        if(!MemTestWord(low_start, test_size, DATA_PATTERN5))
					return 0;
        
				fLib_printf("(1)write all, (2)read , (3)compare\n");
        if(!MemTestWord1(low_start, test_size, DATA_PATTERN4))
					return 0;
#if 1
        //HalfWord Bank Width 16 
        fLib_printf("Mem test by HALFWORD = Address ...\n");
        fLib_printf("(1)Clear all, (2)write, (3)read , (4)compare\n");
        MemTestHalfWord(low_start, test_size, FILL_CONTENT_ADDRESS); 
    
        fLib_printf("(1)write all, (2)read , (3)compare\n");
        MemTestHalfWord1(low_start, test_size, FILL_CONTENT_ADDRESS);     
    
        fLib_printf("Mem test by HALFWORD = 0x%8x ...\n",DATA_PATTERN2);
        fLib_printf("(1)Clear all, (2)write, (3)read , (4)compare\n");
        MemTestHalfWord(low_start, test_size, DATA_PATTERN2); 
    
        fLib_printf("(1)write all, (2)read , (3)compare\n");
        MemTestHalfWord1(low_start, test_size, DATA_PATTERN2);     
    
        fLib_printf("Mem test by HALFWORD = 0x%8x ...\n",DATA_PATTERN3 );    
        fLib_printf("(1)Clear all, (2)write, (3)read , (4)compare\n");
        MemTestHalfWord(low_start, test_size, DATA_PATTERN3);    
        fLib_printf("(1)write all, (2)read , (3)compare\n");
        MemTestHalfWord1(low_start, test_size, DATA_PATTERN3);  
				
        //Byte Bank Width 8 
        fLib_printf("Mem test by BYTE = Address ...\n");
        fLib_printf("(1)Clear all, (2)write, (3)read , (4)compare\n");
        MemTestByte(low_start, test_size, FILL_CONTENT_ADDRESS); 
    
        fLib_printf("(1)write all, (2)read , (3)compare\n");
        MemTestByte1(low_start, test_size, FILL_CONTENT_ADDRESS);     
    
        fLib_printf("Mem test by BYTE = 0x%8x ...\n",DATA_PATTERN0);
        fLib_printf("(1)Clear all, (2)write, (3)read , (4)compare\n");
        MemTestByte(low_start, test_size, DATA_PATTERN0); 
    
        fLib_printf("(1)write all, (2)read , (3)compare\n");
        MemTestByte1(low_start, test_size, DATA_PATTERN0);     
    
        fLib_printf("Mem test by BYTE = 0x%8x ...\n",DATA_PATTERN1 );        
        fLib_printf("(1)Clear all, (2)write, (3)read , (4)compare\n");
        MemTestByte(low_start, test_size, DATA_PATTERN1);    
        fLib_printf("(1)write all, (2)read , (3)compare\n");
        MemTestByte1(low_start, test_size, DATA_PATTERN1);  
#endif
    }
    fLib_printf("Test pass!\r\n\n");
		return 1;
}

int MemTestByte(int start, int size, unsigned fill_content)
{
	volatile unsigned address, num, show, content, CompareContent;
    int nResultStatus;
    unsigned end;
    unsigned char* ptr;
    
    end = start + size;
    
    num = 0;
    show = 0;
    nResultStatus = EXIT_TUBE_PASS;
   
    address = start;
    
    // clear all content
    while (address < end)
    {
        outb(address,0x0);

        if (inb(address)!= 0x0) {
            nResultStatus = EXIT_TUBE_FAIL;
        }
        address += 1;
    }
    
    address = start;
    
    while (address < end)
    {
		if (fill_content == FILL_CONTENT_ADDRESS)
          	content = address;
      	else
          	content = fill_content;
          
      	ptr = (unsigned char*) & content;
       	outb(address,ptr[0]);
       	outb(address+1,ptr[1]);
       	outb(address+2,ptr[2]);
       	outb(address+3,ptr[3]); 
                    
      	ptr = (unsigned char*) &CompareContent;
      	ptr[0] = inb(address);		// * ((volatile unsigned char *) address);
      	address++;
      	ptr[1] = inb(address);		// * ((volatile unsigned char *) address);      
      	address++;      
      	ptr[2] = inb(address);		// * ((volatile unsigned char *) address);      
      	address++;      
      	ptr[3] = inb(address);		// * ((volatile unsigned char *) address);      
      	address++;
      
      	if ( CompareContent  != content){
			nResultStatus = EXIT_TUBE_FAIL;
          	fLib_printf("error address:%x, error data:%x, correct dara:%x\n", address-4, CompareContent, content);
      	}
      
      	num++;
      	if (num >= 0xff){
          	num = 0;
          	show ++;
      	}
    }

    if (nResultStatus == EXIT_TUBE_FAIL){   
    	fLib_printf("Test Fail\n");
			return 0;
		}
		else
			return 1;
}


int MemTestByte1(int start, int size, unsigned fill_content)
{
    volatile unsigned address, num, show, content, CompareContent;
    int nResultStatus;
    unsigned end;
    unsigned char* ptr;
    
    end = start + size;
     
    num = 0;
    show = 0;
    nResultStatus = EXIT_TUBE_PASS;
    
    address = start;
    
    // clear all content
    while (address < end)
    {
		outb(address,0x0);

		//if( (* ((volatile unsigned char *) address)) != 0x0){    	
    	if (inb(address)!=0x0) {
          nResultStatus = EXIT_TUBE_FAIL;
      }
      address += 1;
    }
    
	address = start;
     // write all content
    while (address < end)
    {
		if (fill_content == FILL_CONTENT_ADDRESS)
          	content = address;
      	else
          	content = fill_content;

       	ptr = (unsigned char*) & content;
       	outb(address,ptr[0]);
       	outb(address+1,ptr[1]);
       	outb(address+2,ptr[2]);
       	outb(address+3,ptr[3]); 
      	address +=4;
    }
    
	address = start;
    while (address < end)
    {
      	if (fill_content == FILL_CONTENT_ADDRESS)
          	content = address;
      	else
          	content = fill_content;
          
      	ptr = (unsigned char*) &CompareContent;
      	ptr[0] = inb(address);		// * ((volatile unsigned char *) address);
      	address++;
      	ptr[1] = inb(address);		// * ((volatile unsigned char *) address);      
      	address++;      
      	ptr[2] = inb(address);		// * ((volatile unsigned char *) address);      
      	address++;      
      	ptr[3] = inb(address);		// * ((volatile unsigned char *) address);      
      	address++;

      	if ( CompareContent  != content){
          	nResultStatus = EXIT_TUBE_FAIL;
          	fLib_printf("error address:%x, error data:%x, correct dara:%x\n", address-4, CompareContent, content);                
      	}
      
      	num++;
      	if (num >= 0xff){
          	num = 0;
          	show ++;
      	}
    }

    if (nResultStatus == EXIT_TUBE_FAIL){   
    	fLib_printf("Test Fail\n");
			return 0;
		}
		else 
			return 1;
}


int MemTestHalfWord(int start, int size, unsigned fill_content)
{
    volatile unsigned address, num, show, content, CompareContent;
    int nResultStatus;
    unsigned end;
    unsigned short* ptr;
    
    end = start + size;
        
    num = 0;
    show = 0;
    nResultStatus = EXIT_TUBE_PASS;
   
    address = start;
    
    // clear all content
    while (address < end)
    {
		outhw(address,0x0);
        
     	if(inhw(address)!=0x0){      
          	nResultStatus = EXIT_TUBE_FAIL;
      	}
      	address += 2;
    }
    
    address = start;    
    while (address < end)
    {
      	if (fill_content == FILL_CONTENT_ADDRESS)
          	content = address;
      	else
          	content = fill_content;
     
      	outhw(address,content);
      	outhw(address+2,content>>16);
              
      	ptr = (unsigned short*) &CompareContent;
      	ptr[0] = inhw(address);		//* ((volatile unsigned short *) address);
      	address+=2;
      	ptr[1] = inhw(address);		//* ((volatile unsigned short *) address);      
      	address+=2;

      	if ( CompareContent  != content) {
          	nResultStatus = EXIT_TUBE_FAIL;
          	fLib_printf("error address:%x, error data:%x, correct dara:%x\n", address-4, CompareContent, content);          
      	}
      
      	num++;
      	if (num >= 0xff){
          	num = 0;
          	show ++;
      	}
    }

    if (nResultStatus == EXIT_TUBE_FAIL){
    	fLib_printf("Test Fail\n");
			return 0;
		}
		else
			return 1;
}


int MemTestHalfWord1(int start, int size, unsigned fill_content)
{
    volatile unsigned address, num, show, content, CompareContent;
    int nResultStatus;
    unsigned end;
    unsigned short* ptr;
    
    end = start + size;

    num = 0;
    show = 0;
    nResultStatus = EXIT_TUBE_PASS;
  
    address = start;
    // clear all content
    while (address < end)
    {
		outhw(address,0x0);
        
     	if(inhw(address)!=0x0){      
          	nResultStatus = EXIT_TUBE_FAIL;
      	}
      	address += 2;
    }
    
    address = start;
    // write all content
    while (address < end)
    {
      	if (fill_content == FILL_CONTENT_ADDRESS)
          	content = address;
      	else
          	content = fill_content;

      	outhw(address, content);
      	outhw(address+2, content>>16);
      	address+=4;
    }
        
	address = start;
    //read and compare    
    while (address < end)
    {
		if (fill_content == FILL_CONTENT_ADDRESS)
          	content = address;
      	else
          	content = fill_content;
          
	  	ptr = (unsigned short*) &CompareContent;      
      	ptr[0] = inhw(address);		//* ((volatile unsigned short *) address);
      	address+=2;
      	ptr[1] = inhw(address);		//* ((volatile unsigned short *) address);      
      	address+=2;
      
      	if ( CompareContent  != content){
          	nResultStatus = EXIT_TUBE_FAIL;
          	fLib_printf("error address:%x, error data:%x, correct dara:%x\n", address-4, CompareContent, content);          
      	}
      
      	num++;
      	if (num >= 0xff){
          	num = 0;
          	show ++;
      	}
    }

    if (nResultStatus == EXIT_TUBE_FAIL){
    	fLib_printf("Test Fail\n");
			return 0;
		}
		else
			return 1;
}


int MemTestWord(int start, int size, unsigned fill_content)
{
    volatile unsigned address, num, show, content, data;
    int nResultStatus;
    unsigned end;
    
		end = start + size;
  
    num = 0;
    show = 0;
    nResultStatus = EXIT_TUBE_PASS;

    address = start;
    
    // clear all content
    while (address < end)
    {
    	outw(address, 0x0);		// * ((unsigned *) address) = 0x0;
    	data = inw(address);	// * ((volatile unsigned *) address);
      	if( data != 0x0){
          	nResultStatus = EXIT_TUBE_FAIL;
					fLib_printf("failed\n");
					break;
      	}
      	address += 4;
    }
    
    address = start;    
    while (address < end)
    {
      	if (fill_content == FILL_CONTENT_ADDRESS)
			content = address;
      	else
          	content = fill_content;
		
		outw(address, content);         // * ((unsigned *) address) = content;
      	if ( inw(address) != content){
          	fLib_printf("error address:%x, error data:%x, correct dara:%x\n", address, (* ((unsigned *) address)), content);
          	nResultStatus = EXIT_TUBE_FAIL;
      	}
      
      	address += 4;
      	num++;
      	if (num >= 0xff){
			num = 0;
          	show ++;
      	}
    }
     
    if (nResultStatus == EXIT_TUBE_FAIL){
    	fLib_printf("Test Fail\n");
			return 0;
		}
		else
			return 1;
}

int MemTestWord1(int start, int size, unsigned fill_content)
{
    volatile unsigned address, num, show, content;
    int nResultStatus;
    unsigned end;
    
		end = start + size;
  
    num = 0;
    show = 0;
    nResultStatus = EXIT_TUBE_PASS;
    
  
    address = start;
    // write all content
    while (address < end)
    {
      	if (fill_content == FILL_CONTENT_ADDRESS)
          	content = address;
      	else
          	content = fill_content;
         
      	outw(address, content);		//* ((unsigned *) address) = content;      
      	address += 4;
    }

    address = start;
    //read and compare    
    while (address < end)
    {
		if (fill_content == FILL_CONTENT_ADDRESS)
          	content = address;
      	else
          	content = fill_content;
          
      	if ( inw(address) != content){
          	fLib_printf("error address:%x, error data:%x, correct dara:%x\n", address, (* ((unsigned *) address)), content);            
          	nResultStatus = EXIT_TUBE_FAIL;
      	}
      
      	address += 4;
      	num++;
      	if (num >= 0xff){
          	num = 0;
          	show ++;
      	}
    }
    
    if (nResultStatus == EXIT_TUBE_FAIL){
    	fLib_printf("Test Fail\n");
			return 0;
		}
		else
			return 1;
}

void memory_test_main(void)
{
	UINT32 iteration, pass, overflow;
	iteration = 0;
	pass = 0;
	overflow = 0;
	//fLib_printf("Start Memory Test, please enter test iterations:\n");
	//iteration = fLib_getchar(DEBUG_CONSOLE);
	//iteration = 10;
	/*while(1){
		iteration++;
		if(Mem_Test_Main(iteration)){
			fLib_printf("\n (%d) Pass!\n", iteration);
			pass++;
		}
		else
			break;
		fLib_printf("PASS: %d / %d\n", pass, iteration);
	}*/
	ddr_sram_rw_verify();
	//sram_verify();
}
