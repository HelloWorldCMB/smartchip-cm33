#include "common_include.h"
#include "Driver_Common.h"
#include "Driver_SAI.h"
#include "DrvSSP010.h"

#define ARM_SAI_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0) /* driver version */

void i2s_dma(int base, UINT32 *buf, int dma_req, int len , int tx);
/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_SAI_API_VERSION,
    ARM_SAI_DRV_VERSION
};

/* Driver Capabilities */
static ARM_SAI_CAPABILITIES DriverCapabilities = {
    1, /* supports asynchronous Transmit/Receive */
    0, /* supports synchronous Transmit/Receive */
    0, /* supports user defined Protocol */
    1, /* supports I2S Protocol */
    0, /* supports MSB/LSB justified Protocol */
    0, /* supports PCM short/long frame Protocol */
    0, /* supports AC'97 Protocol */
    0, /* supports Mono mode */
    0, /* supports Companding */
    0, /* supports MCLK (Master Clock) pin */
    0  /* supports Frame error event: \ref ARM_SAI_EVENT_FRAME_ERROR */
};
I2S_RESOURCES I2S0_Resources =
{ 
	I2S_FTSSP010_0_PA_BASE,
	&DriverCapabilities,
	0,
	0,
};
//
//  Functions
//

ARM_DRIVER_VERSION ARM_SAI_GetVersion (void)
{
	return DriverVersion;
}

ARM_SAI_CAPABILITIES ARM_SAI_GetCapabilities (void)
{
	return DriverCapabilities;
}

int32_t ARM_SAI_Initialize (ARM_SAI_SignalEvent_t cb_event, I2S_RESOURCES *i2s) 
{
	UINT32 io_base; 

	io_base = i2s->iobase;	
	fLib_printf("ARM_SAI_Initialize");
	fLib_SetSSP_Enable(io_base, 0);
  fLib_I2S_Init(io_base, WM8731S_I2S, SSP_AS_SLAVE);

	return 0;
}

int32_t ARM_SAI_Uninitialize (I2S_RESOURCES *i2s) 
{
	fLib_SetSSP_Enable(i2s->iobase,0);
	return 0;
}

int32_t ARM_SAI_PowerControl (ARM_POWER_STATE state, I2S_RESOURCES *i2s)
{
    switch (state)
    {
    case ARM_POWER_OFF:
        break;

    case ARM_POWER_LOW:
        break;

    case ARM_POWER_FULL:
        break;

    default:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
		return 0;
}

#if 0
int32_t ARM_SAI_Send (unsigned int *data, uint32_t num, I2S_RESOURCES *i2s)
{
	uint32_t io_base = i2s->iobase;

	//Clear FIFO
	fLib_SSPClearTxFIFO(io_base);
	fLib_SSPClearRxFIFO(io_base);

  //Enable SSP1
	fLib_SetSSP_Enable(io_base, 1);

	int i=0;

	for (i=0; i<I2S_TOTAL_SIZE; )
			{		
				if(!fLib_SSP_GetTxFIFOValidEntries(io_base))
				{
					outw(io_base + SSP_DATA, inw(data + i));
					i++;
				}
			}
	//Disable SSP1
	fLib_SetSSP_Enable(io_base,0);
	fLib_printf("Playing done\n");

	i2s->txcount += num;
	return 0;
}
#else
int32_t ARM_SAI_Send (unsigned int *data, uint32_t num, I2S_RESOURCES *i2s)
{
	uint32_t io_base = i2s->iobase;
  uint32_t dma_req;
	//Clear FIFO
	fLib_SSPClearTxFIFO(io_base);
	fLib_SSPClearRxFIFO(io_base);

  //Enable SSP1
	fLib_SetSSP_Enable(io_base, 1);

	fLib_SetSSP_DMA(io_base, 1,0);
	
	if(io_base == I2S_FTSSP010_0_PA_BASE)
		dma_req = I2S_FTSSP010_0_DMA_TXREQ;
	else
		dma_req = I2S_FTSSP010_1_DMA_TXREQ;
	
	i2s_dma(io_base, data, dma_req, I2S_TOTAL_SIZE, 1);//tx 
	
	//Disable SSP1
	fLib_SetSSP_Enable(io_base,0);
	fLib_printf("Playing done\n");

	i2s->txcount += num;
	return 0;
}
#endif


#if 0
int32_t ARM_SAI_Receive (unsigned int *data, uint32_t num, I2S_RESOURCES *i2s)
{
	uint32_t io_base = i2s->iobase;
	
	//Clear FIFO
	fLib_SSPClearTxFIFO(io_base);
	fLib_SSPClearRxFIFO(io_base);

  //Enable SSP1
	fLib_SetSSP_Enable(io_base, 1);
	
	int i;
	for (i=0; i<I2S_TOTAL_SIZE; )
			{		
				if(fLib_SSP_GetRxFIFOValidEntries(io_base))
				{
					outw(data + i, inw(io_base + SSP_DATA));
					i++;
				}
			}
	fLib_SetSSP_Enable(io_base, 0);
	fLib_printf("Recording done\n");

	i2s->rxcount += num;
	return 0;
}
#else
//use dma
int32_t ARM_SAI_Receive (unsigned int *data, uint32_t num, I2S_RESOURCES *i2s)
{
	uint32_t io_base = i2s->iobase;
	uint32_t dma_req;
	
	//Clear FIFO
	fLib_SSPClearTxFIFO(io_base);
	fLib_SSPClearRxFIFO(io_base);

	//enable ssp dma
	fLib_SetSSP_DMA(io_base, 0,1);

  //Enable SSP1
	fLib_SetSSP_Enable(io_base, 1);
	
	memset(data, 0 , I2S_TOTAL_SIZE);
	if(io_base == I2S_FTSSP010_0_PA_BASE)
		dma_req = I2S_FTSSP010_0_DMA_RXREQ;
	else
		dma_req = I2S_FTSSP010_1_DMA_RXREQ;	
	
	i2s_dma(io_base, data, dma_req, I2S_TOTAL_SIZE, 0);//rx 
	
	fLib_SetSSP_Enable(io_base, 0);
	fLib_printf("Recording done\n");

	i2s->rxcount += num;
	return 0;
}
#endif

uint32_t ARM_SAI_GetTxCount (I2S_RESOURCES *i2s)
{
	return i2s->txcount;
}

uint32_t ARM_SAI_GetRxCount (I2S_RESOURCES *i2s)
{
	return i2s->rxcount;
}

int32_t ARM_SAI_Control (uint32_t control, uint32_t arg1, uint32_t arg2, I2S_RESOURCES *i2s)
{
	switch(control){
		case ARM_DRIVER_SAI_CONTROL_ENABLE:
			//Enable SSP1
			fLib_SetSSP_Enable(i2s->iobase, 1);
		break;
		case ARM_DRIVER_SAI_CONTROL_DISABLE:
			fLib_SetSSP_Enable(i2s->iobase, 0);
			break;
	}
	return 0;
}

ARM_SAI_STATUS ARM_SAI_GetStatus (I2S_RESOURCES *i2s)
{
	 ARM_SAI_STATUS status;
	 return status;
}

void ARM_SAI_SignalEvent(uint32_t event, I2S_RESOURCES *i2s)
{
    // function body
}

// I2S0 Driver Wrapper functions
static ARM_SAI_CAPABILITIES I2S0_GetCapabilities (void) {
  return ARM_SAI_GetCapabilities ();
}

static int32_t I2S0_Initialize (ARM_SAI_SignalEvent_t cb_event) {
  return ARM_SAI_Initialize (cb_event, &I2S0_Resources);
}

static int32_t I2S0_Uninitialize (void) {
  return ARM_SAI_Uninitialize (&I2S0_Resources);
}

static int32_t I2S0_PowerControl (ARM_POWER_STATE state) {
  return ARM_SAI_PowerControl (state, &I2S0_Resources);
}

static int32_t I2S0_Send (unsigned int *data, uint32_t num) {
  return ARM_SAI_Send (data, num, &I2S0_Resources);
}

static int32_t I2S0_Receive (unsigned int *data, uint32_t num) {
  return ARM_SAI_Receive (data, num, &I2S0_Resources);
}

static uint32_t I2S0_GetTxCount (void) {
  return ARM_SAI_GetTxCount (&I2S0_Resources);
}

static uint32_t I2S0_GetRxCount (void) {
  return ARM_SAI_GetRxCount (&I2S0_Resources);
}

static int32_t I2S0_Control (uint32_t control, uint32_t arg1, uint32_t arg2) {
  return ARM_SAI_Control (control, arg1, arg2, &I2S0_Resources);
}

static ARM_SAI_STATUS I2S0_GetStatus (void) {
  return ARM_SAI_GetStatus (&I2S0_Resources);
}


// End SAI Interface

ARM_DRIVER_SAI Driver_SAI0 = {
    ARM_SAI_GetVersion,
    I2S0_GetCapabilities,
    I2S0_Initialize,
    I2S0_Uninitialize,
    I2S0_PowerControl,
    I2S0_Send,
    I2S0_Receive,
    I2S0_GetTxCount,
    I2S0_GetRxCount,
    I2S0_Control,
    I2S0_GetStatus
};

