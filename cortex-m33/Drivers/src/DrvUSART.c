/* -----------------------------------------------------------------------------
 * Copyright (c) 2013-2014 ARM Ltd.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *
 * $Date:        25. Aug 2014
 * $Revision:    V1.00
 *
 * Driver:       Driver_USART0, Driver_USART1, Driver_USART2, and Driver_USART3
 * Configured:   via RTE_Device.h configuration file
 * Project:      USART Driver for Faraday FIE31XX 
 * -------------------------------------------------------------------------- */

/* History:
 *  Version 1.00
 *    - Initial release
 */

//#include "USART_FIE31XX.h"
#include "leo_cm33.h"
#include "Driver_USART.h"
#include "DrvUART010.h"
#include "io.h"
#define ARM_USART_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,0)   /* driver version */

// Driver Version
static const ARM_DRIVER_VERSION usart_driver_version = { ARM_USART_API_VERSION, ARM_USART_DRV_VERSION };
static USART_INFO USART0_Info = {0, 0, 0 , 0, 0, DEFAULT_CONSOLE_BAUD};

/* UART0 Resources */
static USART_RESOURCES USART0_Resources = {
  {
    1,  // supports UART (Asynchronous) mode 
    0,  // supports Synchronous Master mode
    0,  // supports Synchronous Slave mode
    0,  // supports UART Single-wire mode
    0,  // supports UART IrDA mode
    0,  // supports UART Smart Card mode
    0,  // Smart Card Clock generator
    1,  // RTS Flow Control available
    1,  // CTS Flow Control available
    0,  // Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE
    0,  // Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT
    1,  // RTS Line: 0=not available, 1=available
    1,  // CTS Line: 0=not available, 1=available
    1,  // DTR Line: 0=not available, 1=available
    1,  // DSR Line: 0=not available, 1=available
    1,  // DCD Line: 0=not available, 1=available
    1,  // RI Line: 0=not available, 1=available
		1,  // Signal CTS change event: \ref ARM_USART_EVENT_CTS
    1,  // Signal DSR change event: \ref ARM_USART_EVENT_DSR
    1,  // Signal DCD change event: \ref ARM_USART_EVENT_DCD
    1  // Signal RI change event: \ref ARM_USART_EVENT_RI
  },
  FT_UART0,
  DEBUG_CONSOLE,
  UART_FTUART010_0_IRQ,
  UART_CLOCK,  
	SERIAL_FCR_RXF_TRG_LVL_0,
	SERIAL_FCR_TXF_TRG_LVL_0,
  &USART0_Info
};

static USART_INFO USART1_Info = {0, 0, 0 , 0, 0, DEFAULT_CONSOLE_BAUD};

/* UART1 Resources */
static USART_RESOURCES USART1_Resources = {
  {
    1,  // supports UART (Asynchronous) mode 
    0,  // supports Synchronous Master mode
    0,  // supports Synchronous Slave mode
    0,  // supports UART Single-wire mode
    0,  // supports UART IrDA mode
    0,  // supports UART Smart Card mode
    0,  // Smart Card Clock generator
    1,  // RTS Flow Control available
    1,  // CTS Flow Control available
    0,  // Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE
    0,  // Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT
    1,  // RTS Line: 0=not available, 1=available
    1,  // CTS Line: 0=not available, 1=available
    1,  // DTR Line: 0=not available, 1=available
    1,  // DSR Line: 0=not available, 1=available
    1,  // DCD Line: 0=not available, 1=available
    1,  // RI Line: 0=not available, 1=available
    1,  // Signal CTS change event: \ref ARM_USART_EVENT_CTS
		1,  // Signal DSR change event: \ref ARM_USART_EVENT_DSR
    1,  // Signal DCD change event: \ref ARM_USART_EVENT_DCD
    1  // Signal RI change event: \ref ARM_USART_EVENT_RI
  },
  FT_UART1,
  DRVUART_PORT1,
  UART_FTUART010_1_IRQ, //31
  UART_CLOCK_2,  
	SERIAL_FCR_RXF_TRG_LVL_0,
	SERIAL_FCR_TXF_TRG_LVL_0,
  &USART1_Info
};

#ifdef __PLATFORM_FIE3100F__
static USART_INFO USART2_Info = {0, 0, 0 , 0, 0, BAUD_115200};

/* UART2 Resources */
static USART_RESOURCES USART2_Resources = {
  {
    1,  // supports UART (Asynchronous) mode 
    0,  // supports Synchronous Master mode
    0,  // supports Synchronous Slave mode
    0,  // supports UART Single-wire mode
    0,  // supports UART IrDA mode
    0,  // supports UART Smart Card mode
    0,  // Smart Card Clock generator
    1,  // RTS Flow Control available
    1,  // CTS Flow Control available
    0,  // Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE
    0,  // Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT
    1,  // RTS Line: 0=not available, 1=available
    1,  // CTS Line: 0=not available, 1=available
    1,  // DTR Line: 0=not available, 1=available
    1,  // DSR Line: 0=not available, 1=available
    1,  // DCD Line: 0=not available, 1=available
    1,  // RI Line: 0=not available, 1=available
    1,  // Signal CTS change event: \ref ARM_USART_EVENT_CTS
    1,  // Signal DSR change event: \ref ARM_USART_EVENT_DSR
    1,  // Signal DCD change event: \ref ARM_USART_EVENT_DCD
    1  // Signal RI change event: \ref ARM_USART_EVENT_RI
  },
  FT_UART2,
  DRVUART_PORT2,
  UART2_IRQn,
  UART_CLOCK_2,  
	SERIAL_FCR_RXF_TRG_LVL_0,
	SERIAL_FCR_TXF_TRG_LVL_0,
  &USART2_Info
};

static USART_INFO USART3_Info = {0, 0, 0 , 0, 0, DEFAULT_CONSOLE_BAUD};

/* UART3 Resources */
static USART_RESOURCES USART3_Resources = {
  {
    1,  // supports UART (Asynchronous) mode 
    0,  // supports Synchronous Master mode
    0,  // supports Synchronous Slave mode
    0,  // supports UART Single-wire mode
    0,  // supports UART IrDA mode
    0,  // supports UART Smart Card mode
    0,  // Smart Card Clock generator
    1,  // RTS Flow Control available
    1,  // CTS Flow Control available
    0,  // Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE
    0,  // Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT
    1,  // RTS Line: 0=not available, 1=available
    1,  // CTS Line: 0=not available, 1=available
    1,  // DTR Line: 0=not available, 1=available
    1,  // DSR Line: 0=not available, 1=available
    1,  // DCD Line: 0=not available, 1=available
    1,  // RI Line: 0=not available, 1=available
    1,  // Signal CTS change event: \ref ARM_USART_EVENT_CTS
    1,  // Signal DSR change event: \ref ARM_USART_EVENT_DSR
    1,  // Signal DCD change event: \ref ARM_USART_EVENT_DCD
    1  // Signal RI change event: \ref ARM_USART_EVENT_RI
  },
  FT_UART3,
  DRVUART_PORT3,
  UART3_IRQn,
  UART_CLOCK,
	SERIAL_FCR_RXF_TRG_LVL_0,
	SERIAL_FCR_TXF_TRG_LVL_0,
  &USART3_Info
};
#endif


/**
  \fn          uint32_t USART_RxLineIntHandler (USART_RESOURCES *usart)
  \brief       Receive line interrupt handler
  \param[in]   usart     Pointer to USART resources
  \return      Rx Line event mask
*/
static uint32_t USART_RxLineIntHandler (USART_RESOURCES *usart) {
  uint32_t lsr, event;

  event = 0;
  lsr   = usart->reg->LSR;

  // OverRun error
  if (lsr & SERIAL_LSR_OE) {
    usart->info->status.rx_overflow = 1;
    event |= ARM_USART_EVENT_RX_OVERFLOW;   
  }

  // Parity error
  if (lsr & SERIAL_LSR_PE) {
    usart->info->status.rx_parity_error = 1;
    event |= ARM_USART_EVENT_RX_PARITY_ERROR;
  }

  // Break detected
  if (lsr & SERIAL_LSR_BI) {
    usart->info->status.rx_break = 1;
    event |= ARM_USART_EVENT_RX_BREAK;
  }

  // Framing error
  else if(lsr & SERIAL_LSR_FE) {
    usart->info->status.rx_framing_error = 1;
    event |= ARM_USART_EVENT_RX_FRAMING_ERROR;
  }

  return event;
}

// USART Driver functions

/**
  \fn          ARM_DRIVER_VERSION USARTx_GetVersion (void)
  \brief       Get driver version.
  \return      \ref ARM_DRIVER_VERSION
*/
static ARM_DRIVER_VERSION USARTx_GetVersion (void) {
  return usart_driver_version;
}

/**
  \fn          ARM_USART_CAPABILITIES USARTx_GetCapabilities (void)
  \brief       Get driver capabilities.
  \return      \ref USART_CAPABILITIES
*/
static ARM_USART_CAPABILITIES USART_GetCapabilities (USART_RESOURCES         *usart) {
  return usart->capabilities;
}

/**
  \fn          int32_t USART_Initialize (ARM_USART_SignalEvent_t  cb_event
                                         USART_RESOURCES         *usart)
  \brief       Initialize USART Interface.
  \param[in]   cb_event  Pointer to \ref ARM_USART_SignalEvent
  \param[in]   usart     Pointer to USART resources
  \return      \ref execution_status
*/
static int32_t USART_Initialize (ARM_USART_SignalEvent_t  cb_event,
                                 USART_RESOURCES         *usart) {

  if (usart->info->flags & USART_FLAG_POWERED) {
    // Device is powered - could not be re-initialized
    return ARM_DRIVER_ERROR;
  }

  if (usart->info->flags & USART_FLAG_INITIALIZED) {
    // Driver is already initialized
    return ARM_DRIVER_OK;
  }

  // Initialize USART Run-time Resources
  usart->info->cb_event = cb_event;

  usart->info->status.tx_busy          = 0;
  usart->info->status.rx_busy          = 0;
  usart->info->status.tx_underflow     = 0;
  usart->info->status.rx_overflow      = 0;
  usart->info->status.rx_break         = 0;
  usart->info->status.rx_framing_error = 0;
  usart->info->status.rx_parity_error  = 0;

  usart->info->mode = 0;
  usart->info->xfer.tx_def_val = 0; 	
  usart->info->flags = USART_FLAG_INITIALIZED;

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USART_Uninitialize (USART_RESOURCES *usart)
  \brief       De-initialize USART Interface.
  \param[in]   usart     Pointer to USART resources
  \return      \ref execution_status
*/
static int32_t USART_Uninitialize (USART_RESOURCES *usart) {

  if (usart->info->flags & USART_FLAG_POWERED) {
    // Driver is powered - could not be uninitialized
    return ARM_DRIVER_ERROR;
  }

  if (usart->info->flags == 0) {
    // Driver not initialized
    return ARM_DRIVER_OK;
  }  
	
  // Reset USART status flags
  usart->info->flags = 0;

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USART_PowerControl (ARM_POWER_STATE state)
  \brief       Control USART Interface Power.
  \param[in]   state  Power state
  \param[in]   usart  Pointer to USART resources
  \return      \ref execution_status
*/
static int32_t USART_PowerControl (ARM_POWER_STATE  state,
                                   USART_RESOURCES *usart) {

  if ((usart->info->flags & USART_FLAG_INITIALIZED) == 0) {
    // Return error, if USART is not initialized
    return ARM_DRIVER_ERROR;
  }

  if (usart->info->status.rx_busy == 1) {
    // Receive busy
    return ARM_DRIVER_ERROR_BUSY;
  }

  if (usart->info->flags & USART_FLAG_SEND_ACTIVE) {
    // Transmit busy
    return ARM_DRIVER_ERROR_BUSY;
  }
  
  switch (state) {
    case ARM_POWER_OFF:
      if ((usart->info->flags & USART_FLAG_POWERED) == 0)
        return ARM_DRIVER_OK;			
      
			// Disable USART IRQ
			NVIC_DisableIRQ(usart->irq_num);			
      // Disable TX and RX Interrupts
			usart->reg->IER &= ~(SERIAL_IER_DR|SERIAL_IER_TE);			
			//TODO: Disable UART clock here
			
			usart->info->flags = USART_FLAG_INITIALIZED;
			
      break;

    case ARM_POWER_LOW:
      return ARM_DRIVER_ERROR_UNSUPPORTED;

    case ARM_POWER_FULL:
      if (usart->info->flags & USART_FLAG_POWERED)
        return ARM_DRIVER_OK;
      
			//TODO: Enable UART clock here
			
			// Configure FIFO Control register
      // Set trigger level                  
      usart->reg->FCR = (usart->rx_trig_lvl | usart->tx_trig_lvl);			
			usart->reg->FCR |= SERIAL_FCR_FE;
			
			// Clear and Enable USART IRQ
			
			NVIC_ClearPendingIRQ(usart->irq_num);
			//NVIC_EnableIRQ(usart->irq_num);		
      usart->info->flags = USART_FLAG_POWERED | USART_FLAG_INITIALIZED;			
      break;

    default: return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USART_Send (const void            *data,
                                         uint32_t         num,
                                         USART_RESOURCES *usart)
  \brief       Start sending data to USART transmitter.
  \param[in]   data  Pointer to buffer with data to send to USART transmitter
  \param[in]   num   Number of data items to send
  \param[in]   usart Pointer to USART resources
  \return      \ref execution_status
*/
static int32_t USART_Send (const void            *data,
                                 uint32_t         num,
                                 USART_RESOURCES *usart) {
  
  if ((data == NULL) || (num == 0)) {
    // Invalid parameters
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  if ((usart->info->flags & USART_FLAG_CONFIGURED) == 0) {
    // USART is not configured (mode not selected)
    return ARM_DRIVER_ERROR;
  }

  if (usart->info->flags & USART_FLAG_SEND_ACTIVE) {
    // Send is not completed yet
    return ARM_DRIVER_ERROR_BUSY;
  }

  // Set Send active flag
  usart->info->flags |= USART_FLAG_SEND_ACTIVE;

  // Save transmit buffer info
  usart->info->xfer.tx_buf = (uint8_t *)data;
  usart->info->xfer.tx_num = num;
  usart->info->xfer.tx_cnt = 0;

  // Enable transmit holding register empty interrupt
  usart->reg->IER |= SERIAL_IER_TE;
  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USART_Receive (void            *data,
                                      uint32_t         num,
                                      USART_RESOURCES *usart)
  \brief       Start receiving data from USART receiver.
  \param[out]  data  Pointer to buffer for data to receive from USART receiver
  \param[in]   num   Number of data items to receive
  \param[in]   usart Pointer to USART resources
  \return      \ref execution_status
*/
static int32_t USART_Receive (void            *data,
                              uint32_t         num,
                              USART_RESOURCES *usart) {

  if ((data == NULL) || (num == 0)) {
    // Invalid parameters
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  if ((usart->info->flags & USART_FLAG_CONFIGURED) == 0) {
    // USART is not configured (mode not selected)
    return ARM_DRIVER_ERROR;
  }

  // Check if receiver is busy
  if (usart->info->status.rx_busy == 1) 
    return ARM_DRIVER_ERROR_BUSY;

  // Set RX busy flag
  usart->info->status.rx_busy = 1; 

  // Save number of data to be received
  usart->info->xfer.rx_num = num;

  // Clear RX statuses
  usart->info->status.rx_break          = 0;
  usart->info->status.rx_framing_error  = 0;
  usart->info->status.rx_overflow       = 0;
  usart->info->status.rx_parity_error   = 0;

  // Save receive buffer info
  usart->info->xfer.rx_buf = (uint8_t *)data;
  usart->info->xfer.rx_cnt =            0;
  
  // Enable receive data available interrupt
  usart->reg->IER |= SERIAL_IER_DR; 
  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USART_Transfer (const void             *data_out,
                                             void             *data_in,
                                             uint32_t          num,
                                             USART_RESOURCES  *usart)
  \brief       Start sending/receiving data to/from USART transmitter/receiver.
  \param[in]   data_out  Pointer to buffer with data to send to USART transmitter
  \param[out]  data_in   Pointer to buffer for data to receive from USART receiver
  \param[in]   num       Number of data items to transfer
  \param[in]   usart     Pointer to USART resources
  \return      \ref execution_status
*/
static int32_t USART_Transfer (const void             *data_out,
                                     void             *data_in,
                                     uint32_t          num,
                                     USART_RESOURCES  *usart) {
  int32_t status;
																			 
																			 
  if ((data_out == NULL) || (data_in == NULL) || (num == 0)) {
    // Invalid parameters
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  if ((usart->info->flags & USART_FLAG_CONFIGURED) == 0) {
    // USART is not configured
    return ARM_DRIVER_ERROR;
  }
  
  if ((usart->info->mode == ARM_USART_MODE_SYNCHRONOUS_MASTER) ||
      (usart->info->mode == ARM_USART_MODE_SYNCHRONOUS_SLAVE )) {    

    // Receive
    status = USART_Receive (data_in, num, usart);
    if (status != ARM_DRIVER_OK) return status;

    // Send
    status = USART_Send (data_out, num, usart);
    if (status != ARM_DRIVER_OK) return status;
  } else {
    // Only in synchronous mode
    return ARM_DRIVER_ERROR;
  }
	
  return ARM_DRIVER_OK;
}

/**
  \fn          uint32_t USART_GetTxCount (USART_RESOURCES *usart)
  \brief       Get transmitted data count.
  \param[in]   usart     Pointer to USART resources
  \return      number of data items transmitted
*/
static uint32_t USART_GetTxCount (USART_RESOURCES *usart) {
  return usart->info->xfer.tx_cnt;
}

/**
  \fn          uint32_t USART_GetRxCount (USART_RESOURCES *usart)
  \brief       Get received data count.
  \param[in]   usart     Pointer to USART resources
  \return      number of data items received
*/
static uint32_t USART_GetRxCount (USART_RESOURCES *usart) {
  return usart->info->xfer.rx_cnt;
}

/**
  \fn          int32_t USART_Control (uint32_t          control,
                                      uint32_t          arg,
                                      USART_RESOURCES  *usart)
  \brief       Control USART Interface.
  \param[in]   control  Operation
  \param[in]   arg      Argument of operation (optional)
  \param[in]   usart    Pointer to USART resources
  \return      common \ref execution_status and driver specific \ref usart_execution_status
*/
static int32_t USART_Control (uint32_t          control,
                              uint32_t          arg,
                              USART_RESOURCES  *usart) {
  
  uint32_t ier, lcr, mode = 0, baudrate;  
  

  if ((usart->info->flags & USART_FLAG_POWERED) == 0) {
    // USART not powered
    return ARM_DRIVER_ERROR;
  }   
  
  switch (control & ARM_USART_CONTROL_Msk) {
    // Abort Send
    case ARM_USART_ABORT_SEND:
      // Disable transmit holding register empty interrupt
			usart->reg->IER &= ~SERIAL_IER_TE; 
			// Set trigger level                  
      usart->reg->FCR = usart->tx_trig_lvl;			 		
      usart->reg->FCR |= (SERIAL_FCR_TXFR|SERIAL_FCR_FE);		
      
      // Clear Send active flag
      usart->info->flags &= ~USART_FLAG_SEND_ACTIVE;
      return ARM_DRIVER_OK;

    // Abort receive
    case ARM_USART_ABORT_RECEIVE:
      // Disable receive data available interrupt
      usart->reg->IER &= ~SERIAL_IER_DR;
			// Set trigger level                  
      usart->reg->FCR = usart->rx_trig_lvl;			
      usart->reg->FCR |= (SERIAL_FCR_RXFR|SERIAL_FCR_FE);      

      // Clear RX busy status
      usart->info->status.rx_busy = 0;
      return ARM_DRIVER_OK;
		
		// Abort transfer
    case ARM_USART_ABORT_TRANSFER:      			
      usart->reg->IER &= ~SERIAL_IER_DR;
			usart->reg->IER &= ~SERIAL_IER_TE;
			// Set trigger level                  
      usart->reg->FCR = (usart->rx_trig_lvl | usart->tx_trig_lvl);			
			usart->reg->FCR |= (SERIAL_FCR_TXFR | SERIAL_FCR_RXFR | SERIAL_FCR_FE);			
      usart->info->flags &= ~USART_FLAG_SEND_ACTIVE;
		  usart->info->status.rx_busy = 0;
			return ARM_DRIVER_OK;
		
		// Default TX value
    case ARM_USART_SET_DEFAULT_TX_VALUE:
      usart->info->xfer.tx_def_val = arg;
      return ARM_DRIVER_OK;    
    
    case ARM_USART_MODE_ASYNCHRONOUS:      
    	mode = ARM_USART_MODE_ASYNCHRONOUS;
			usart->reg->MDR &= ~SERIAL_MDR_MODE_SEL;
			usart->reg->MDR |= SERIAL_MDR_UART;
    	break;
		case ARM_USART_MODE_SYNCHRONOUS_MASTER:    
			if (usart->capabilities.synchronous_master) {
				//TODO: Implement synchronous master mode here
				
			} else	return ARM_USART_ERROR_MODE;			      			
			mode = ARM_USART_MODE_SYNCHRONOUS_MASTER;
			break;
    case ARM_USART_MODE_SYNCHRONOUS_SLAVE:  
			if (usart->capabilities.synchronous_slave) {
				//TODO: Implement synchronous slave mode here
				
			} else return ARM_USART_ERROR_MODE;			 
			mode = ARM_USART_MODE_SYNCHRONOUS_SLAVE;			
			break;
		case ARM_USART_MODE_IRDA: 
			if (usart->capabilities.irda) {
        //TODO: Implement IrDA mode here
        
      } else return ARM_USART_ERROR_MODE;
      mode = ARM_USART_MODE_IRDA;
      break;			
		case ARM_USART_MODE_SINGLE_WIRE:   
			if (usart->capabilities.single_wire) {
        //TODO: Implement single wire mode here
        
      } else return ARM_USART_ERROR_MODE;
			mode = ARM_USART_MODE_SINGLE_WIRE;
			break;
    case ARM_USART_MODE_SMART_CARD:          
			if (usart->capabilities.smart_card) {
        //TODO: Implement single wire mode here
        
      } else return ARM_USART_ERROR_MODE;			      			
			mode = ARM_USART_MODE_SMART_CARD;
			break;
		// Control TX			
    case ARM_USART_CONTROL_TX:         
			if(arg)
			{
				usart->info->flags |= USART_FLAG_TX_ENABLED;
				usart->reg->IER |= SERIAL_IER_TE;
			}
			else
			{
				usart->info->flags &= ~USART_FLAG_TX_ENABLED;
				usart->reg->IER &= ~SERIAL_IER_TE;
			}
			return ARM_DRIVER_OK;
    // Control RX
    case ARM_USART_CONTROL_RX:
			if(arg)
			{
				usart->info->flags |= USART_FLAG_RX_ENABLED;
				usart->reg->IER |= SERIAL_IER_DR;
			}
			else
			{
				usart->info->flags &= ~USART_FLAG_RX_ENABLED;
				usart->reg->IER &= ~SERIAL_IER_DR;
			}
			return ARM_DRIVER_OK;      
    
    case ARM_USART_SET_IRDA_PULSE:    
    // SmartCard guard time
    case ARM_USART_SET_SMART_CARD_GUARD_TIME:     	
    // SmartCard clock
    case ARM_USART_SET_SMART_CARD_CLOCK:
    // SmartCard NACK
    case ARM_USART_CONTROL_SMART_CARD_NACK:        
    // Control break
    case ARM_USART_CONTROL_BREAK:    
    // Unsupported command
    default: return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  
 	// USART Data bits
  lcr = usart->reg->LCR & ~SERIAL_LCR_DLAB;
  
  switch (control & ARM_USART_DATA_BITS_Msk) {
   	case ARM_USART_DATA_BITS_5: lcr |= SERIAL_LCR_LEN5; break;
   	case ARM_USART_DATA_BITS_6: lcr |= SERIAL_LCR_LEN6; break;
   	case ARM_USART_DATA_BITS_7: lcr |= SERIAL_LCR_LEN7; break;
   	case ARM_USART_DATA_BITS_8: lcr |= SERIAL_LCR_LEN8; break;
   	default: return ARM_USART_ERROR_DATA_BITS;
  }

  // USART Parity
  switch (control & ARM_USART_PARITY_Msk) {
   	case ARM_USART_PARITY_NONE:                         break;
   	case ARM_USART_PARITY_EVEN: lcr |= SERIAL_LCR_EVEN; break;
   	case ARM_USART_PARITY_ODD:  lcr |= SERIAL_LCR_ODD;  break;
  }

  // USART Stop bits
  switch (control & ARM_USART_STOP_BITS_Msk) {
   	case ARM_USART_STOP_BITS_1:                       	break;
   	case ARM_USART_STOP_BITS_2: lcr |= SERIAL_LCR_STOP; break;
   	default: return ARM_USART_ERROR_STOP_BITS;
  }

  // USART Flow control  	
  ier = usart->reg->IER & ~(SERIAL_IER_RTSEN | SERIAL_IER_CTSEN);
    
  switch (control & ARM_USART_FLOW_CONTROL_Msk) {
    case ARM_USART_FLOW_CONTROL_NONE:
      break;
    case ARM_USART_FLOW_CONTROL_RTS:
			if (usart->capabilities.flow_control_rts)	
				ier |= SERIAL_IER_RTSEN;        
			else return ARM_USART_ERROR_FLOW_CONTROL;
      break;
    case ARM_USART_FLOW_CONTROL_CTS:
			if (usart->capabilities.flow_control_cts)
				ier |= SERIAL_IER_CTSEN;        
      else return ARM_USART_ERROR_FLOW_CONTROL;      
      break;
    case ARM_USART_FLOW_CONTROL_RTS_CTS:
			if (usart->capabilities.flow_control_rts && 
            usart->capabilities.flow_control_cts) {
				ier |= SERIAL_IER_RTSEN | SERIAL_IER_CTSEN;        
      } else return ARM_USART_ERROR_FLOW_CONTROL; 		
      break;
    default:
      return ARM_USART_ERROR_FLOW_CONTROL;
  }
    
  // USART Baudrate  	
	if(mode)
	{		
		// Configuration is OK - Mode is valid	
		usart->info->mode = mode; 
		
		switch(arg)
		{
			case BAUD_1200:			
			case BAUD_2400:			
			case BAUD_4800:
			case BAUD_9600:
			case BAUD_14400:
			case BAUD_19200:
			case BAUD_38400:
			case BAUD_57600:
			case BAUD_115200:
			case BAUD_921600:
			break;
			default:
			return ARM_USART_ERROR_BAUDRATE;
	 	}
		
		/* Set DLAB=1 */  
		usart->reg->LCR = SERIAL_LCR_DLAB;    		
		usart->info->baudrate = baudrate = arg;
		baudrate = (usart->port == DRVUART_PORT0 || usart->port == DRVUART_PORT3)?(UART_CLOCK/baudrate):(UART_CLOCK_2/baudrate);
  	baudrate >>= 4;	//divided by 16
  	usart->reg->DLM = ((baudrate & 0xff00) >> 8);
  	usart->reg->DLL = (baudrate & 0xff);  	 		
  }
  
  // Configure IER register  
  usart->reg->IER = ier;
  usart->reg->MCR = 2;
	//usart->reg->MCR = 3;
  
  // Configure Line control register
  usart->reg->LCR = lcr;

  // Set configured flag
  usart->info->flags |= USART_FLAG_CONFIGURED;

  return ARM_DRIVER_OK;
}

/**
  \fn          ARM_USART_STATUS USART_GetStatus (USART_RESOURCES *usart)
  \brief       Get USART status.
  \param[in]   usart     Pointer to USART resources
  \return      USART status \ref ARM_USART_STATUS
*/
static ARM_USART_STATUS USART_GetStatus (USART_RESOURCES *usart) {
  usart->info->status.tx_busy = (usart->reg->LSR & SERIAL_LSR_THRE ? (0) : (1));
  return usart->info->status;
}

/**
  \fn          int32_t USART_SetModemControl (ARM_USART_MODEM_CONTROL  control,
                                              USART_RESOURCES         *usart)
  \brief       Set USART Modem Control line state.
  \param[in]   control   \ref ARM_USART_MODEM_CONTROL
  \param[in]   usart     Pointer to USART resources
  \return      \ref execution_status
*/
static int32_t USART_SetModemControl (ARM_USART_MODEM_CONTROL  control,
                                      USART_RESOURCES         *usart) {

  if ((usart->info->flags & USART_FLAG_CONFIGURED) == 0) {
    // USART is not configured
    return ARM_DRIVER_ERROR;
  }

  if (usart->reg == NULL) return ARM_DRIVER_ERROR_UNSUPPORTED;

  if (control == ARM_USART_RTS_CLEAR) {
    if (usart->capabilities.rts) usart->reg->MCR &= ~SERIAL_MCR_RTS;    
		else return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  if (control == ARM_USART_RTS_SET) {
    if (usart->capabilities.rts) usart->reg->MCR |=  SERIAL_MCR_RTS;    
		else return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  if (control == ARM_USART_DTR_CLEAR) {
    if (usart->capabilities.dtr) usart->reg->MCR &= ~SERIAL_MCR_DTR;    
		else return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  if (control == ARM_USART_DTR_SET) {
    if (usart->capabilities.dtr) usart->reg->MCR |=  SERIAL_MCR_DTR;    
		else return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  return ARM_DRIVER_OK;
}

/**
  \fn          ARM_USART_MODEM_STATUS USART_GetModemStatus (USART_RESOURCES *usart)
  \brief       Get USART Modem Status lines state.
  \param[in]   usart     Pointer to USART resources
  \return      modem status \ref ARM_USART_MODEM_STATUS
*/
static ARM_USART_MODEM_STATUS USART_GetModemStatus (USART_RESOURCES *usart) {
  ARM_USART_MODEM_STATUS modem_status;
  uint32_t msr;

  
	if (usart->reg && (usart->info->flags & USART_FLAG_CONFIGURED)) {

    msr = usart->reg->MSR;

    modem_status.cts = (msr & SERIAL_MSR_CTS ? (1) : (0));
    modem_status.dsr = (msr & SERIAL_MSR_DSR ? (1) : (0));
    modem_status.ri  = (msr & SERIAL_MSR_RI  ? (1) : (0));
    modem_status.dcd = (msr & SERIAL_MSR_DCD ? (1) : (0));
  } else {
    modem_status.cts = 0;
    modem_status.dsr = 0;
    modem_status.ri  = 0;
    modem_status.dcd = 0;
  }

  return modem_status;
}

/**
  \fn          void USART_IRQHandler (UART_RESOURCES *usart)
  \brief       USART Interrupt handler.
  \param[in]   usart     Pointer to USART resources
*/
static void USART_IRQHandler (USART_RESOURCES *usart) {
  uint32_t iir, event;

  event = 0;
  iir   = usart->reg->IIR;
  if ((iir & SERIAL_IIR_NONE) == 0) {

    // Transmit holding register empty
    if (iir & SERIAL_IIR_TE) {
      
      // Write data to Tx FIFO      
			if(usart->reg->LSR & SERIAL_LSR_THRE)
				usart->reg->THR = usart->info->xfer.tx_buf[usart->info->xfer.tx_cnt++];                
      
      // Check if all data is transmitted
      if (usart->info->xfer.tx_num == usart->info->xfer.tx_cnt) {
        // Disable THRE interrupt
        usart->reg->IER &= ~SERIAL_IER_TE;

        // Clear TX busy flag
        usart->info->flags &= ~USART_FLAG_SEND_ACTIVE;

        // Set send complete event
        event |= ARM_USART_EVENT_SEND_COMPLETE;        
      }
    }

    // Receive line status
    if (iir & SERIAL_IIR_RLS) {
      event |= USART_RxLineIntHandler(usart);
    }

    // Receive data available and Character time-out indicator interrupt
    if ((iir & SERIAL_IIR_DR) | (iir & SERIAL_IIR_TIMEOUT)) {
      
			while (usart->reg->LSR & SERIAL_LSR_DR) {										
				usart->info->xfer.rx_buf[usart->info->xfer.rx_cnt++] = usart->reg->RBR;
			
				// Check if requested amount of data is received
				if (usart->info->xfer.rx_cnt == usart->info->xfer.rx_num) {
					// Disable RDA interrupt
					usart->reg->IER &= ~SERIAL_IER_DR;

					// Clear RX busy flag and set receive transfer complete event
					usart->info->status.rx_busy = 0;    					
					event |= ARM_USART_EVENT_RECEIVE_COMPLETE;                   
					break;
				}     
			}
    }

    // Character time-out indicator
    if (iir & SERIAL_IIR_TIMEOUT) {
      if ((usart->info->mode != ARM_USART_MODE_SYNCHRONOUS_MASTER) &&
          (usart->info->mode != ARM_USART_MODE_SYNCHRONOUS_SLAVE )) {
        // Signal RX Time-out event, if not all requested data received
        if (usart->info->xfer.rx_cnt != usart->info->xfer.rx_num) {
          event |= ARM_USART_EVENT_RX_TIMEOUT;
        }
      }
    }

    // Modem interrupt
    if (usart->reg) {
      if (iir & SERIAL_IIR_MODEM) {
        // CTS state changed
        if (usart->reg->MSR & SERIAL_MSR_DELTACTS)
          event |= ARM_USART_EVENT_CTS;
        // DSR state changed
        if (usart->reg->MSR & SERIAL_MSR_DELTADSR)
          event |= ARM_USART_EVENT_DSR;
        // Ring indicator
        if (usart->reg->MSR & SERIAL_MSR_TERI)
          event |= ARM_USART_EVENT_RI;
        // DCD state changed
        if (usart->reg->MSR & SERIAL_MSR_DELTACD)
          event |= ARM_USART_EVENT_DCD;
      }
    }
  }	
	
  if (usart->info->cb_event && event)
    usart->info->cb_event (event);
}

// USART0 Driver Wrapper functions
static ARM_USART_CAPABILITIES USART0_GetCapabilities (void) {
  return USART_GetCapabilities (&USART0_Resources);
}
static int32_t USART0_Initialize (ARM_USART_SignalEvent_t cb_event) {
  return USART_Initialize (cb_event, &USART0_Resources);
}
static int32_t USART0_Uninitialize (void) {
  return USART_Uninitialize(&USART0_Resources);
}
static int32_t USART0_PowerControl (ARM_POWER_STATE state) {
  return USART_PowerControl (state, &USART0_Resources);
}
static int32_t USART0_Send (const void *data, uint32_t num) {
  return USART_Send (data, num, &USART0_Resources);
}
static int32_t USART0_Receive (void *data, uint32_t num) {
  return USART_Receive (data, num, &USART0_Resources);
}
static int32_t USART0_Transfer (const void      *data_out,
                                      void      *data_in,
                                      uint32_t   num) {
  return USART_Transfer (data_out, data_in, num, &USART0_Resources);
}
static uint32_t USART0_GetTxCount (void) {
  return USART_GetTxCount (&USART0_Resources);
}
static uint32_t USART0_GetRxCount (void) {
  return USART_GetRxCount (&USART0_Resources); 
}
static int32_t USART0_Control (uint32_t control, uint32_t arg) {
  return USART_Control (control, arg, &USART0_Resources);
}
static ARM_USART_STATUS USART0_GetStatus (void) {
  return USART_GetStatus (&USART0_Resources);
}
static int32_t USART0_SetModemControl (ARM_USART_MODEM_CONTROL control) {
  return USART_SetModemControl (control, &USART0_Resources);
}
static ARM_USART_MODEM_STATUS USART0_GetModemStatus (void) {
  return USART_GetModemStatus (&USART0_Resources);
}

void UART0_IRQHandler (void) {
  USART_IRQHandler (&USART0_Resources);
}

// USART0 Driver Control Block
ARM_DRIVER_USART Driver_USART0 = {
    USARTx_GetVersion,
    USART0_GetCapabilities,
    USART0_Initialize,
    USART0_Uninitialize,
    USART0_PowerControl,
    USART0_Send, 
    USART0_Receive,
    USART0_Transfer,
    USART0_GetTxCount,
    USART0_GetRxCount,
    USART0_Control,
    USART0_GetStatus,
    USART0_SetModemControl,
    USART0_GetModemStatus
};

// USART1 Driver Wrapper functions
static ARM_USART_CAPABILITIES USART1_GetCapabilities (void) {
  return USART_GetCapabilities (&USART1_Resources);
}
static int32_t USART1_Initialize (ARM_USART_SignalEvent_t cb_event) {
  return USART_Initialize (cb_event, &USART1_Resources);
}
static int32_t USART1_Uninitialize (void) {
  return USART_Uninitialize(&USART1_Resources);
}
static int32_t USART1_PowerControl (ARM_POWER_STATE state) {
  return USART_PowerControl (state, &USART1_Resources);
}
static int32_t USART1_Send (const void *data, uint32_t num) {
  return USART_Send (data, num, &USART1_Resources);
}
static int32_t USART1_Receive (void *data, uint32_t num) {
  return USART_Receive (data, num, &USART1_Resources);
}
static int32_t USART1_Transfer (const void      *data_out,
                                      void      *data_in,
                                      uint32_t   num) {
  return USART_Transfer (data_out, data_in, num, &USART1_Resources);
}
static uint32_t USART1_GetTxCount (void) {
  return USART_GetTxCount (&USART1_Resources);
}
static uint32_t USART1_GetRxCount (void) {
  return USART_GetRxCount (&USART1_Resources); 
}
static int32_t USART1_Control (uint32_t control, uint32_t arg) {
  return USART_Control (control, arg, &USART1_Resources);
}
static ARM_USART_STATUS USART1_GetStatus (void) {
  return USART_GetStatus (&USART1_Resources);
}
static int32_t USART1_SetModemControl (ARM_USART_MODEM_CONTROL control) {
  return USART_SetModemControl (control, &USART1_Resources);
}
static ARM_USART_MODEM_STATUS USART1_GetModemStatus (void) {
  return USART_GetModemStatus (&USART1_Resources);
}

void UART1_IRQHandler (void) {
  USART_IRQHandler (&USART1_Resources);
}

// USART1 Driver Control Block
ARM_DRIVER_USART Driver_USART1 = {
    USARTx_GetVersion,
    USART1_GetCapabilities,
    USART1_Initialize,
    USART1_Uninitialize,
    USART1_PowerControl,
    USART1_Send, 
    USART1_Receive,
    USART1_Transfer,
    USART1_GetTxCount,
    USART1_GetRxCount,
    USART1_Control,
    USART1_GetStatus,
    USART1_SetModemControl,
    USART1_GetModemStatus
};

#ifdef Four_UART
// USART2 Driver Wrapper functions
static ARM_USART_CAPABILITIES USART2_GetCapabilities (void) {
  return USART_GetCapabilities (&USART2_Resources);
}
static int32_t USART2_Initialize (ARM_USART_SignalEvent_t cb_event) {
  return USART_Initialize (cb_event, &USART2_Resources);
}
static int32_t USART2_Uninitialize (void) {
  return USART_Uninitialize(&USART2_Resources);
}
static int32_t USART2_PowerControl (ARM_POWER_STATE state) {
  return USART_PowerControl (state, &USART2_Resources);
}
static int32_t USART2_Send (const void *data, uint32_t num) {
  return USART_Send (data, num, &USART2_Resources);
}
static int32_t USART2_Receive (void *data, uint32_t num) {
  return USART_Receive (data, num, &USART2_Resources);
}
static int32_t USART2_Transfer (const void      *data_out,
                                      void      *data_in,
                                      uint32_t   num) {
  return USART_Transfer (data_out, data_in, num, &USART2_Resources);
}
static uint32_t USART2_GetTxCount (void) {
  return USART_GetTxCount (&USART2_Resources);
}
static uint32_t USART2_GetRxCount (void) {
  return USART_GetRxCount (&USART2_Resources); 
}
static int32_t USART2_Control (uint32_t control, uint32_t arg) {
  return USART_Control (control, arg, &USART2_Resources);
}
static ARM_USART_STATUS USART2_GetStatus (void) {
  return USART_GetStatus (&USART2_Resources);
}
static int32_t USART2_SetModemControl (ARM_USART_MODEM_CONTROL control) {
  return USART_SetModemControl (control, &USART2_Resources);
}
static ARM_USART_MODEM_STATUS USART2_GetModemStatus (void) {
  return USART_GetModemStatus (&USART2_Resources);
}

void UART2_IRQHandler (void) {
  USART_IRQHandler (&USART2_Resources);
}

// USART2 Driver Control Block
ARM_DRIVER_USART Driver_USART2 = {
    USARTx_GetVersion,
    USART2_GetCapabilities,
    USART2_Initialize,
    USART2_Uninitialize,
    USART2_PowerControl,
    USART2_Send, 
    USART2_Receive,
    USART2_Transfer,
    USART2_GetTxCount,
    USART2_GetRxCount,
    USART2_Control,
    USART2_GetStatus,
    USART2_SetModemControl,
    USART2_GetModemStatus
};

// USART3 Driver Wrapper functions
static ARM_USART_CAPABILITIES USART3_GetCapabilities (void) {
  return USART_GetCapabilities (&USART3_Resources);
}
static int32_t USART3_Initialize (ARM_USART_SignalEvent_t cb_event) {
  return USART_Initialize (cb_event, &USART3_Resources);
}
static int32_t USART3_Uninitialize (void) {
  return USART_Uninitialize(&USART3_Resources);
}
static int32_t USART3_PowerControl (ARM_POWER_STATE state) {
  return USART_PowerControl (state, &USART3_Resources);
}
static int32_t USART3_Send (const void *data, uint32_t num) {
  return USART_Send (data, num, &USART3_Resources);
}
static int32_t USART3_Receive (void *data, uint32_t num) {
  return USART_Receive (data, num, &USART3_Resources);
}
static int32_t USART3_Transfer (const void      *data_out,
                                      void      *data_in,
                                      uint32_t   num) {
  return USART_Transfer (data_out, data_in, num, &USART3_Resources);
}
static uint32_t USART3_GetTxCount (void) {
  return USART_GetTxCount (&USART3_Resources);
}
static uint32_t USART3_GetRxCount (void) {
  return USART_GetRxCount (&USART3_Resources); 
}
static int32_t USART3_Control (uint32_t control, uint32_t arg) {
  return USART_Control (control, arg, &USART3_Resources);
}
static ARM_USART_STATUS USART3_GetStatus (void) {
  return USART_GetStatus (&USART3_Resources);
}
static int32_t USART3_SetModemControl (ARM_USART_MODEM_CONTROL control) {
  return USART_SetModemControl (control, &USART3_Resources);
}
static ARM_USART_MODEM_STATUS USART3_GetModemStatus (void) {
  return USART_GetModemStatus (&USART3_Resources);
}

void UART3_IRQHandler (void) {
  USART_IRQHandler (&USART3_Resources);
}

// USART3 Driver Control Block
ARM_DRIVER_USART Driver_USART3 = {
    USARTx_GetVersion,
    USART3_GetCapabilities,
    USART3_Initialize,
    USART3_Uninitialize,
    USART3_PowerControl,
    USART3_Send, 
    USART3_Receive,
    USART3_Transfer,
    USART3_GetTxCount,
    USART3_GetRxCount,
    USART3_Control,
    USART3_GetStatus,
    USART3_SetModemControl,
    USART3_GetModemStatus
};
#endif