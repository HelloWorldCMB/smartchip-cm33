#ifndef __PIN_MUX_H__
#define __PIN_MUX_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "flib.h"

void PinMux_SPI020(UINT8 no);
void PinMux_UART010(UINT8 no);
void PinMux_LCM(UINT8 no);	
void PinMux_SSP010(UINT8 no);	

#ifdef __cplusplus
}
#endif

#endif/* __PIN_MUX_H__ */
