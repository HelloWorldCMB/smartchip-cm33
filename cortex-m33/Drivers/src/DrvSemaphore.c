#include "DrvUART010.h"
#include "leo_cm33.h"
#include "io.h"
#include "types.h"
#include <stdio.h>
#include "DrvSemaphore.h"
#include "string.h"
#include "ipc.h"

extern int ffs(unsigned int val);
void Semaphore_Handler(void)
{
	UINT32 val, channel, i;
	
	val = inw(SEMAPHORE_PA_BASE + 0x104);
	
	if ((val>>16)&0xffff) {
		outw(SEMAPHORE_PA_BASE + 0x104, inw(SEMAPHORE_PA_BASE + 0x104));
	}
	channel = ffs((val>>16)&0xffff) -1;
	
	if (channel == 1)
		return;

#ifdef YAOXIN_ENABLE
	if (channel == IPC_CFG_CHANNEL) {
		fLib_printf("[yaoxin] semaphore irq, cfg channel\n");
		yaoxin_handle_cfg_cmd();
		return;
	}
#endif

	ipc_data_area_t *ipc_data= (ipc_data_area_t *)(0x50120000 + (channel * IPC_PER_CHANNEL_SRAM_SIZE));

	ipc_data->id = (channel);
	
	ipc_data->len = IPC_PER_CHANNEL_SRAM_SIZE - sizeof(ipc_data_area_t);
	
	memset(ipc_data->data, 0x55, ipc_data->len);
	
	channel = 1 << (channel+1);
	
	writel(channel|1,0x57f00100);
}

void CTI_Handler(void)
{
	UINT32 val;
	
  fLib_printf("CTI triggered!!\n");	
}


//read register status
UINT32 semaphore_read_reg(UINT32 io_base , UINT32 num)
{
	if(num > MAX_SEPHAMORE_REG) 
		return 0;
	
	return inw(io_base + 0x04*num);
}

UINT32 semaphore_write_reg(UINT32 io_base , UINT32 num)
{
	if(num > MAX_SEPHAMORE_REG) 
		return FALSE;

	outw(io_base + 0x04*num , SEMAPHORE_CLEAR_STATUS);
		return TRUE;
}

UINT32 semaphore_slave_is_available(UINT32 io_base, UINT32 num)
{
	if(semaphore_read_reg(io_base, num) == SEMAPHORE_SLAVE_AVAILABLE)
		return TRUE;
	
	return FALSE;
}

UINT32 semaphore_slave_set_occupy(UINT32 io_base, UINT32 num)
{
	if(semaphore_slave_is_available(io_base, num) == FALSE)
		return FALSE;
	
	if(semaphore_write_reg(io_base, num) == FALSE) 
		return FALSE;
	//check if it is available
	if(semaphore_slave_is_available(io_base, num) == TRUE){
		return TRUE;
	}else{
		return FALSE;
	}
}

//init
void semaphore_init(UINT32 io_base)
{
	int i;
	//clear all state
	#if 1
	outw(io_base + SEMAPHORE_CTRL, SEMAPHORE_CLEAR_STATUS);
	#else
	for(i=0;i< MAX_SEPHAMORE_REG; i++){
		//write 1 to each semaphore to release
		semaphore_write_reg(SEMAPHORE_PA_BASE, i);	
	}
	#endif
}