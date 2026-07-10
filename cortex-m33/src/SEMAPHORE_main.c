#include "DrvPWMTMR010.h"
#include "DrvUART010.h"
#include "leo_cm33.h"
#include "io.h"
#include "types.h"
#include <stdio.h>
#include "DrvSemaphore.h"
#include "Common_Include.h"

void semaphore_interrupt_test(void)
{
	UINT32 val;
	UINT32 ch;
	
	//disable interrupt first
	outw(SEMAPHORE_PA_BASE + 0x104, 0);
	//disable interrupt;
	NVIC_DisableIRQ(SEMAPHORE_FTSEMAPHRE_IRQ);

	delay_ms(5);
	//enable ubterryotl
	outw(SEMAPHORE_PA_BASE + 0x104, 1);
	//enable interrupt;
	NVIC_EnableIRQ(SEMAPHORE_FTSEMAPHRE_IRQ);
	
	for(ch = 0 ; ch < 15 ; ch++){
		//trigger each channel
		fLib_printf("enabling %d interrupt\n",ch);
		val = inw(SEMAPHORE_PA_BASE + 0x104);
		val |= 0x01 << (ch+1);
		outw(SEMAPHORE_PA_BASE + 0x104, val);
		delay_ms(50);
	}
	fLib_printf("Over\n");
	//disable interrupt
	outw(SEMAPHORE_PA_BASE + 0x104, 0);	
}

void semaphore_test_main(void)
{
	int i;
	
	fLib_printf("%s\n",__func__);
	semaphore_init(SEMAPHORE_PA_BASE);
	
	fLib_printf("[1]Check sempahore is available\n");
	for(i=0;i< MAX_SEPHAMORE_REG; i++){
		//check if semaphore is available and occupy!
		if(semaphore_slave_set_occupy(SEMAPHORE_PA_BASE, i)== TRUE){
			//fLib_printf("PASS: %d occupy\n",i);
		}else{
			fLib_printf("Fail: %d occupy fail\n",i);
			while(1);
		}
	}
	
	//check again
	fLib_printf("[2]Check sempahore is available\n");
	for(i=0;i< MAX_SEPHAMORE_REG; i++){
		//check if semaphore is available and occupy!		
		if(semaphore_slave_set_occupy(SEMAPHORE_PA_BASE, i)== FALSE){
			//fLib_printf("PASS: %d is still occupied %d\n",i);
		}else{
			fLib_printf("Fail: !! %d is not occupied ????\n",i);
			while(1);
		}
	}
	
	fLib_printf("[3]Check sempahore is released\n");
	for(i=0;i< MAX_SEPHAMORE_REG; i++){
		//write 1 to each semaphore to release
		semaphore_write_reg(SEMAPHORE_PA_BASE, i);
		//check if released;
		if(semaphore_slave_is_available(SEMAPHORE_PA_BASE, i)!=TRUE){
			fLib_printf("FAIL: semaphore %d is not released\n",i);
			while(1);
		}
	}	
	fLib_printf("Semaphore test PASSED\n");
	
	//IPC test
	semaphore_interrupt_test();
}