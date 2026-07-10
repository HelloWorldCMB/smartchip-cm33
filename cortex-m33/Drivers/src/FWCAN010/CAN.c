#include <string.h>
#include "Common_Include.h"
#include "CAN_regs.h"
#include "CAN.h"

const UINT8 DLCtoBytes_table[16] = {0, 1, 2, 3, 4, 5, 6, 7, 
								    8,12,16,20,24,32,48,64};


/******************************** CAN_Reset ************************************
	Introduction:
		Softwre Reset CAN controller
		After finishing the reset operation, CAN will enter Configuration mode
	Procedure:
		1. Enable RR bit in CE0 register
		2. There will check the RR bit = 0 in while loop. It means the Reset 
		   operation is done.
*******************************************************************************/
void CAN_Reset(CAN_TypeRegs* CANx)
{
	/* Enable RR bit */
	CANx->CAN_Control.All |= CAN_CE1_RR_W;

	/* Ensure the reset is done until the RR bit clear */
	while(CANx->CAN_Control.All & CAN_CE1_RR_W);
}

/********************************* CAN_Init ************************************  
    Introduction:
		Go to configuration mode and set the CAN features and bit timing.
		After setting the CAN register in configuration mode, the CAN will enter 
		the user specific mode by parameter CAN_Mode.
	Procedure:
		1. Determin the CAN is in the configuration mode
		   - If not, set OMR register to 3'b000 in CE0 for entry configuration 
		     mode.
		2. Clean the three registers of CAN_Bit_Timing.Nominal_L, 
		   CAN_Bit_Timing.Nominal_H, and CAN_Bit_Timing.Data.
		3. Set TSE if the time stamp support.
		4. Set RT (Retransmission) to ALWAYS, 1 time, 3 times, or 8 times.
		5. Bit timing setting. According CAN_InitStruct parameters to set into 
		   register.
		6. According the parameter (CAN_mode) to change the mode by setting the 
		   OMR field in CE0 register.
*******************************************************************************/
void CAN_Init(CAN_TypeRegs* CANx, CAN_InitTypeDef* CAN_InitStruct)
{
	/* If the CAN is not in configuration mode, set the OMR field to 3'b000 */
	while(CANx->CAN_Control.All & CAN_CE1_OMR_W)
	{
		CANx->CAN_Control.All &= ~CAN_CE1_OMR_W;
	}

	/* Reset registers */
	CANx->CAN_Bit_Timing.Nominal_L.All &= 0x0;
	CANx->CAN_Bit_Timing.Nominal_H.All &= 0x0;
	CANx->CAN_Bit_Timing.Data.All &= 0x0;

	/* Set the overload feature */
	if(CAN_InitStruct->CAN_TOE == ENABLE)
	{
		CANx->CAN_Control.All |= CAN_CE1_TOE_W;
	}
	else
	{
		CANx->CAN_Control.All &= ~CAN_CE1_TOE_W;
	}

	/* Set the timestamp feature */
	if(CAN_InitStruct->CAN_TSE == ENABLE)
	{
		CANx->CAN_Control.All |= CAN_CE0_TSE_W;
	}
	else
	{
		CANx->CAN_Control.All &= ~CAN_CE0_TSE_W;
	}

	/* Set the debounced feature */
	if(CAN_InitStruct->CAN_EnDBE == ENABLE)
	{
		CANx->CAN_Control.All |= CAN_CE0_EnDB_W;
	}
	else
	{
		CANx->CAN_Control.All &= ~CAN_CE0_EnDB_W;
	}

	/* Set the number of retransmission */
	CANx->CAN_Control.All &= ~CAN_CE1_RT_W;
	CANx->CAN_Control.All |= ((UINT32)(CAN_InitStruct->CAN_RT) << 27);

	/* Set bit timing */
	CANx->CAN_Bit_Timing.Nominal_L.All = \
		(UINT32)(((UINT32)(CAN_InitStruct->CAN_NSJW  - 1) << 3) | \
		((UINT32)(CAN_InitStruct->CAN_NBRP  - 1) << 8) | \
		((UINT32)(CAN_InitStruct->CAN_NProp - 1) << 17) | \
		((UINT32)(CAN_InitStruct->CAN_NPS1  - 1) << 27));
	CANx->CAN_Bit_Timing.Nominal_H.All = (UINT32)((UINT32)(CAN_InitStruct->CAN_NPS2 - 1) << 3);
	if(CAN_InitStruct->CAN_DBRP != 0)
	{
		/* If data prescalar is not 0, then setting data bit timing. */
		CANx->CAN_Bit_Timing.Data.All = (UINT32)((UINT32)(CAN_InitStruct->CAN_DBRP - 1) | \
			((UINT32)(CAN_InitStruct->CAN_DSJW - 1) <<  9) | \
			((UINT32)(CAN_InitStruct->CAN_DProp << 11)) | \
			((UINT32)(CAN_InitStruct->CAN_DPS1 - 1) << 21) | \
			((UINT32)(CAN_InitStruct->CAN_DPS2 - 1) << 29));
	}
	/* Set the operation mode */
	CANx->CAN_Control.All |= ((UINT32)(CAN_InitStruct->CAN_Mode) << 24);
}

/******************************* CAN_Sleep *************************************
	Introduction:
		Let the CAN enter Sleep mode. After the CAN detects the consecutive 
		recessive bits on the bus, it will wake-up and back to Normal mode.
	Procedure:
		Change the mode by setting the OMR field to 3'b010 in CE0 register.
	Comment:
		We can use WIE bit in IRE register to know that the CAN has been waken 
		up from the sleep mode.
*******************************************************************************/
void CAN_Sleep(CAN_TypeRegs* CANx)
{
	UINT32 reg32;
	/* Set the Sleep mode in OMR field of CE1 register */
	reg32 = CANx->CAN_Control.All;
	reg32 &= ~CAN_CE1_OMR_W;
	reg32 |= ((UINT32)(CAN_Mode_Sleep) << 16);
	CANx->CAN_Control.All = reg32;
}

/***************************** CAN_Mode_Change *********************************
	Introduction:
		Let the CAN enter the user-specific mode
	Input:
		mode: CAN_Mode_Config, 
			  CAN_Mode_Normal, 
			  CAN_Mode_Sleep, 
			  CAN_Mode_Listen, 
			  CAN_Mode_LoopBack
	Procedure:
		Change the mode by setting the OMR field in CE0 register.
*******************************************************************************/
void CAN_Mode_Change(CAN_TypeRegs* CANx, UINT8 mode)
{
	UINT32 reg32;
	
	/* Set the mode in OMR field of CE1 register */
	while((CANx->CAN_Control.All & CAN_CE1_OMR_W) >> 24 != mode)
	{
		/* Set the mode in OMR field of CE1 register */
		reg32 = CANx->CAN_Control.All;
		reg32 &= ~CAN_CE1_OMR_W;
		reg32 |= (UINT32)mode << 24;
		CANx->CAN_Control.All = reg32;
	}
}

/******************************* CAN_GetMode ***********************************
	Introduction:
		Get the current CAN mode by input argument.
*******************************************************************************/
UINT8 CAN_GetMode(CAN_TypeRegs* CANx)
{
	return (UINT8)((CANx->CAN_Control.All & CAN_CE1_OMR_W) >> 24);
}

/**************************** CAN_FilterInit ***********************************
	Introduction:
		FTCAN has 6 filters. The filter 0, 1, 2, and 3 is for Rx FIFO 0; The 
		filter 4 and 5 is for Rx FIFO 1.
		This function will change back the OMR to configuration mode for setting.
	Procedure:
		1. Determine the CAN is in the configuration mode or not.
		   - If not, it will change the mode to configuration mode by setting 
		     the OMR field in CE0 register.
		2. According the argument 'Filter_number n' to determine what number 
		   filter should be set.
		3. Set the Filter(ID or Data)
		   - The ACBIH and AFBIL registers will be filled in Filter_StdId.
		   - The AFEIH, AFEIM, and AFEIL registers will be filled in Filter_ExtId.
		   - If the data filter variable Filter_RxFIFOn_Bytex (n=0,1 x=0,1) is 
			 enable, the Filter_FIFOn_Bytex_data (n=0,1 x=0,1) should be set 
			 into AFDBxn (n=0,1 x=0,1), then the RBnDFBxE (n=0,1 x=0,1) must be 
			 set to 1b.
		   - The AFTn (n=0,1,2) register will be set the frame type value. This 
			 value is defined in <CAN.h>.
		4. Return to the original mode before the step 1.
*******************************************************************************/
void CAN_FilterInit(CAN_TypeRegs* CANx, CAN_FilterTypeDef* CAN_FilterStruct, UINT8 Filter_number)
{
	UINT8 ori_mode;
	UINT32 reg32 = 0x00000000;

	ori_mode = (UINT8)((CANx->CAN_Control.All & CAN_CE1_OMR_W) >> 24);

	/* If the CAN is not in configuration mode, set the OMR field to 3'b000 */
	while(CANx->CAN_Control.All & CAN_CE1_OMR_W)
	{
		CANx->CAN_Control.All &= ~CAN_CE1_OMR_W;
	}

	/* Set the based identifier and the extended identifier */
	CANx->CAN_Filter.Identifier[Filter_number].All = 
		(UINT32)(((CAN_FilterStruct->Filter_ExtId & 0x3FFFF) << 3 ) | 
		((CAN_FilterStruct->Filter_StdId & 0x7FF) << 21));

	/* Set the data byte 0 and 1 filter */
	if(CAN_FilterStruct->Filter_RxFIFO0_Byte0 == ENABLE)
	{
		/* Set FIFO0 byte0 filter value */
		reg32 |= (UINT32)CAN_FilterStruct->Filter_FIFO0_Byte0_data;
		/* Enable FIFO0 byte0 filter */
		CANx->CAN_Filter.Control.All |= CAN_AFC0_RB0DFB0E_W;
	}
	else
	{
		/* Disable FIFO0 byte0 filter */
		CANx->CAN_Filter.Control.All &= ~CAN_AFC0_RB0DFB0E_W;
		/* Clear FIFO0 byte0 filter value */
		reg32 &= ~CAN_FIFO0_AFDB0_W;
	}

	if(CAN_FilterStruct->Filter_RxFIFO0_Byte1 == ENABLE)
	{
		/* Set FIFO0 byte1 filter value */
		reg32 |= (UINT32)CAN_FilterStruct->Filter_FIFO0_Byte1_data << 8;
		/* Enable FIFO0 byte1 filter */
		CANx->CAN_Filter.Control.All |= CAN_AFC0_RB0DFB1E_W;
	}
	else
	{
		/* Disable FIFO0 byte1 filter */
		CANx->CAN_Filter.Control.All &= ~CAN_AFC0_RB0DFB1E_W;
		/* Clear FIFO0 byte1 filter value */
		reg32 &= ~CAN_FIFO0_AFDB1_W;
	}

	if(CAN_FilterStruct->Filter_RxFIFO1_Byte0 == ENABLE)
	{
		/* Set FIFO1 byte0 filter value */
		reg32 |= (UINT32)CAN_FilterStruct->Filter_FIFO1_Byte0_data << 16;
		/* Enable FIFO1 byte0 filter */
		CANx->CAN_Filter.Control.All |= CAN_AFC0_RB1DFB0E_W;
	}
	else
	{
		/* Disable FIFO1 byte0 filter */
		CANx->CAN_Filter.Control.All &= ~CAN_AFC0_RB1DFB0E_W;
		/* Clear FIFO1 byte0 filter value */
		reg32 &= ~CAN_FIFO1_AFDB0_W;
	}

	if(CAN_FilterStruct->Filter_RxFIFO1_Byte1 == ENABLE)
	{
		/* Set FIFO1 byte1 filter value */
		reg32 |= (UINT32)CAN_FilterStruct->Filter_FIFO1_Byte1_data << 24;
		/* Enable FIFO1 byte1 filter */
		CANx->CAN_Filter.Control.All |= CAN_AFC0_RB1DFB0E_W;
	}
	else
	{
		/* Disable FIFO1 byte1 filter */
		CANx->CAN_Filter.Control.All &= ~CAN_AFC0_RB1DFB1E_W;
		/* Clear FIFO1 byte1 filter value */
		reg32 &= ~CAN_FIFO1_AFDB1_W;
	}

	/* write back the data fiter values */
	CANx->CAN_Filter.Data.All = reg32;

	/* Set the frame type filter */
	CANx->CAN_Filter.Control.All &= ~ 0xFC00; //clear the AFC1 register
	CANx->CAN_Filter.Control.All |= ((UINT32)CAN_FilterStruct->Filter_frameType << 8); //Set the frame type in AFC1 register

	if(ori_mode)
	{
		reg32 = 0x00000000;
		reg32 = CANx->CAN_Control.All;
		reg32 &= ~CAN_CE1_OMR_W;
		reg32 |= ((UINT32)(ori_mode) << 24);
		CANx->CAN_Control.All = reg32;
	}
}

/**************************** CAN_FilterGroup **********************************
	Introduction:
		This function will go to enable the user-specific filter of group.
		The FTCAN has two group. The group 0 is for Rx FIFO 0 and includes 
		filter 0, 1, 2. The group 1 is for Rx FIFO 1 and includes filter 4, 5.
	Procedure:
		1. Determine the CAN is in the configuration mode or not.
		   - If not, it will change the mode to configuration mode by setting 
		     the OMR field in CE0 register.
		2. According the input argument Filter_group to enable the 
		   RBAFGnE (n=0,1).
		3. Return to the original mode before going to step 1.
*******************************************************************************/
void CAN_FilterGroup(CAN_TypeRegs* CANx, UINT8 Filter_group, BOOL NewState)
{
	UINT32 reg32 = 0x00000000;
	UINT8 ori_mode = 0x00;

	ori_mode = (UINT8)((CANx->CAN_Control.All & CAN_CE1_OMR_W) >> 24);
	
	/* If the CAN is not in configuration mode, set the OMR field to 3'b000 */
	while(CANx->CAN_Control.All & CAN_CE1_OMR_W)
	{
		CANx->CAN_Control.All &= ~CAN_CE1_OMR_W;
	}

	/* Enable or Disable filter 0 or 1 */
	if(Filter_group == 0)
	{
		if(NewState == ENABLE)
		{
			CANx->CAN_Filter.Control.All |= CAN_AFC0_RBAFG0E_W;
		}
		else
		{
			CANx->CAN_Filter.Control.All &= ~CAN_AFC0_RBAFG0E_W;
		}
	}
	else if(Filter_group == 1)
	{
		if(NewState == ENABLE)
		{
			CANx->CAN_Filter.Control.All |= CAN_AFC0_RBAFG1E_W;
		}
		else if(NewState == DISABLE)
		{
			CANx->CAN_Filter.Control.All &= ~CAN_AFC0_RBAFG1E_W;
		}
	}

	if(ori_mode)
	{
		reg32 = 0x00000000;
		reg32 = CANx->CAN_Control.All;
		reg32 &= ~CAN_CE1_OMR_W;
		reg32 |= ((UINT32)(ori_mode) << 24);
		CANx->CAN_Control.All = reg32;
	}
}

/************************** CAN_GetFilterStatus ********************************
	Introduction:
		Get the filter entire settings included the identifier, data, frame type.
	Procedure:
		According the input argument Filter_number to get the registers of 
		filter.
*******************************************************************************/
void CAN_GetFilterStatus(CAN_TypeRegs* CANx, CAN_FilterTypeDef* CAN_FilterStruct, UINT8 Filter_number)
{
	/* Fatch the identifier value */
	CAN_FilterStruct->Filter_StdId = (CANx->CAN_Filter.Identifier[Filter_number].All & CAN_AFBI_W) >> 13;
	CAN_FilterStruct->Filter_ExtId = (CANx->CAN_Filter.Identifier[Filter_number].All & CAN_AFEI_W) >> 3;
	/* Fatch the data filte value for FIFO 0 and 1 */
	CAN_FilterStruct->Filter_FIFO0_Byte0_data = (UINT8)CANx->CAN_Filter.Data.All & CAN_FIFO0_AFDB0_W;
	CAN_FilterStruct->Filter_FIFO0_Byte1_data = (UINT8)((CANx->CAN_Filter.Data.All & CAN_FIFO0_AFDB1_W) >> 8);
	CAN_FilterStruct->Filter_FIFO1_Byte0_data = (UINT8)((CANx->CAN_Filter.Data.All & CAN_FIFO1_AFDB0_W) >> 16);
	CAN_FilterStruct->Filter_FIFO1_Byte1_data = (UINT8)((CANx->CAN_Filter.Data.All & CAN_FIFO1_AFDB1_W) >> 24);
	/* Fatch the frame type filter */
	CAN_FilterStruct->Filter_frameType = (UINT8)((CANx->CAN_Filter.Control.All & CAN_AFC1_AFT_W) >> 10); 
	/* Ftach the Byte 0 and 1 filter enable in each FIFOs */
	CAN_FilterStruct->Filter_RxFIFO0_Byte0 = (CANx->CAN_Filter.Control.All & CAN_AFC0_RB0DFB0E_W) >> 5;
	CAN_FilterStruct->Filter_RxFIFO0_Byte1 = (CANx->CAN_Filter.Control.All & CAN_AFC0_RB0DFB1E_W) >> 4;
	CAN_FilterStruct->Filter_RxFIFO1_Byte0 = (CANx->CAN_Filter.Control.All & CAN_AFC0_RB1DFB0E_W) >> 3;
	CAN_FilterStruct->Filter_RxFIFO1_Byte1 = (CANx->CAN_Filter.Control.All & CAN_AFC0_RB1DFB1E_W) >> 2;
}

/***************************** CAN_MaskInit ************************************
	Introduction:
		There has two masks in the FTCAN. The mask 0 is for Rx FIFO 0, the other 
		is for Rx FIFO 1. The bit '1' in the mask means 'dont care', on the 
		other hand, the bit '0' means 'must care'.
		The 'mask all' bit in the register can force to 'dont care' all bits 
		about identifier, so if enable this bit, all the frame with any 
		identifier will be received in.
		After setting the mask, the user should enable this mask by 
		calling CAN_MaskNumber function.
	Procedure:
		1. Determine the CAN is in the configuration mode or not.
		   - If not, it will change the mode to configuration mode by setting 
		     the OMR field in CE0 register.
		2. According the input argument RxFIFO_number n to set the mask register.
		3. Return to the original mode before going to step 1.
*******************************************************************************/
void CAN_MaskInit(CAN_TypeRegs* CANx, CAN_MaskTypeDef* CAN_MaskStruct, UINT8 RxFIFO_number)
{
	UINT32 reg32 = 0x00000000;
	UINT8 ori_mode = 0x00;

	ori_mode = (UINT8)((CANx->CAN_Control.All & CAN_CE1_OMR_W) >> 24);

	/* If the CAN is not in configuration mode, set the OMR field to 3'b000 */
	while(CANx->CAN_Control.All & CAN_CE1_OMR_W)
	{
		CANx->CAN_Control.All &= ~CAN_CE1_OMR_W;
	}

	/* Set the based identifier and the extended identifier for mask*/
	CANx->CAN_Mask.Identifier[RxFIFO_number].All = 
		(UINT32)(((CAN_MaskStruct->Mask_ExtId & 0x3FFFF) << 3 ) | 
		((CAN_MaskStruct->Mask_StdId & 0x7FF) << 21));
	/* Set the data byte 0 and 1 mask */
	if(CAN_MaskStruct->Mask_RxFIFO0_Byte0 == ENABLE)
	{
		/* Set FIFO0 byte0 mask value */
		reg32 |= (UINT32)CAN_MaskStruct->Mask_FIFO0_Byte0_data;
		/* Enable FIFO0 byte0 mask */
		CANx->CAN_Mask.Control.All |= CAN_FMC_RB0DFMB0E_W;
	}
	else
	{
		/* Disable FIFO0 byte0 mask */
		CANx->CAN_Mask.Control.All &= ~CAN_FMC_RB0DFMB0E_W;
		/* Clear FIFO0 byte0 mask value */
		reg32 &= ~CAN_FIFO0_FMDB0_W;
	}

	if(CAN_MaskStruct->Mask_RxFIFO0_Byte1 == ENABLE)
	{
		/* Set FIFO0 byte1 mask value */
		reg32 |= ((UINT32)CAN_MaskStruct->Mask_FIFO0_Byte1_data << 8);
		/* Enable FIFO0 byte1 mask */
		CANx->CAN_Mask.Control.All |= CAN_FMC_RB0DFMB1E_W;
	}
	else
	{
		/* Disable FIFO0 byte1 mask */
		CANx->CAN_Mask.Control.All &= ~CAN_FMC_RB0DFMB1E_W;
		/* Clear FIFO0 byte1 mask value */
		reg32 &= ~CAN_FIFO0_FMDB1_W;
	}

	if(CAN_MaskStruct->Mask_RxFIFO1_Byte0 == ENABLE)
	{
		/* Set FIFO1 byte0 mask value */
		reg32 |= ((UINT32)CAN_MaskStruct->Mask_FIFO1_Byte0_data << 16);
		/* Enable FIFO1 byte0 mask */
		CANx->CAN_Mask.Control.All |= CAN_FMC_RB1DFMB0E_W;
	}
	else
	{
		/* Disable FIFO1 byte0 mask */
		CANx->CAN_Mask.Control.All &= ~CAN_FMC_RB1DFMB0E_W;
		/* Clear FIFO1 byte0 mask value */
		reg32 &= ~CAN_FIFO1_FMDB0_W;
	}

	if(CAN_MaskStruct->Mask_RxFIFO1_Byte1 == ENABLE)
	{
		/* Set FIFO1 byte1 mask value */
		reg32 |= ((UINT32)CAN_MaskStruct->Mask_FIFO1_Byte1_data << 24);
		/* Enable FIFO1 byte1 mask */
		CANx->CAN_Mask.Control.All |= CAN_FMC_RB1DFMB1E_W;
	}
	else
	{
		/* Disable FIFO1 byte1 mask */
		CANx->CAN_Mask.Control.All &= ~CAN_FMC_RB1DFMB1E_W;
		/* Clear FIFO1 byte1 mask value */
		reg32 &= ~CAN_FIFO1_FMDB1_W;
	}

	CANx->CAN_Mask.Data.All = reg32;

	/* Set the mask all feature */
	if(CAN_MaskStruct->Mask_All)
	{
		CANx->CAN_Mask.Control.All |= CAN_FMC_FMA_W;
	}
	else
	{
		CANx->CAN_Mask.Control.All &= ~CAN_FMC_FMA_W;
	}

	if(ori_mode)
	{
		reg32 = 0x00000000;
		reg32 = CANx->CAN_Control.All;
		reg32 &= ~CAN_CE1_OMR_W;
		reg32 |= ((UINT32)(ori_mode) << 24);
		CANx->CAN_Control.All = reg32;
	}
}

/*************************** CAN_MaskNumber ************************************
	Introduction:
		This function will enable or disable the Rx FIFO Mask.
	Procedure:
		1. Determine the CAN is in the configuration mode or not.
		   - If not, it will change the mode to configuration mode by setting 
		     the OMR field in CE0 register.
		2. According the input argument RxFIFO_number to set the mask register 
		   RBAFMnE (n=0,1).
		3. Return to the original mode before going to step 1.
*******************************************************************************/
void CAN_MaskNumber(CAN_TypeRegs* CANx, UINT8 RxFIFO_number, BOOL NewState)
{
	UINT8 ori_mode = 0x00;
	UINT32 reg32;

	ori_mode = (UINT8)((CANx->CAN_Control.All & CAN_CE1_OMR_W) >> 24);

	/* If the CAN is not in configuration mode, set the OMR field to 3'b000 */
	while(CANx->CAN_Control.All & CAN_CE1_OMR_W)
	{
		CANx->CAN_Control.All &= ~CAN_CE1_OMR_W;
	}

	/* Enable or disable the mask for each FIFO */
	if(RxFIFO_number == 0)
	{
		if(NewState == ENABLE)
		{
			CANx->CAN_Mask.Control.All |= CAN_FMC_RBAFM0E_W;
		}
		else if(NewState == DISABLE)
		{
			CANx->CAN_Mask.Control.All &= ~CAN_FMC_RBAFM0E_W;
		}
	}
	else if(RxFIFO_number == 1)
	{
		if(NewState == ENABLE)
		{
			CANx->CAN_Mask.Control.All |= CAN_FMC_RBAFM1E_W;
		}
		else if(NewState == DISABLE)
		{
			CANx->CAN_Mask.Control.All &= ~CAN_FMC_RBAFM1E_W;
		}
	}

	if(ori_mode)
	{
		reg32 = 0x00000000;
		reg32 = CANx->CAN_Control.All;
		reg32 &= ~CAN_CE1_OMR_W;
		reg32 |= ((UINT32)(ori_mode) << 24);
		CANx->CAN_Control.All = reg32;
	}
}

/************************** CAN_GetMaskStatus **********************************
	Introduction:
		Get the user-specific mask context. According the input argument 
		RxFIFO_number to select the mask register, and parse the mask context.
*******************************************************************************/
void CAN_GetMaskStatus(CAN_TypeRegs* CANx, CAN_MaskTypeDef* CAN_MaskStruct, UINT8 RxFIFO_number)
{
	/* Fatch the identifier value */
	CAN_MaskStruct->Mask_StdId = (CANx->CAN_Mask.Identifier[RxFIFO_number].All & CAN_FMBI_W) >> 13;
	CAN_MaskStruct->Mask_ExtId = (CANx->CAN_Mask.Identifier[RxFIFO_number].All & CAN_FMEI_W) >> 3;
	/* Fatch the data mask value */
	CAN_MaskStruct->Mask_FIFO0_Byte0_data = (UINT8)CANx->CAN_Mask.Data.All & CAN_FIFO0_FMDB0_W;
	CAN_MaskStruct->Mask_FIFO0_Byte1_data = (UINT8)((CANx->CAN_Mask.Data.All & CAN_FIFO0_FMDB1_W) >> 8);
	CAN_MaskStruct->Mask_FIFO1_Byte0_data = (UINT8)((CANx->CAN_Mask.Data.All & CAN_FIFO1_FMDB0_W) >> 16);
	CAN_MaskStruct->Mask_FIFO1_Byte1_data = (UINT8)((CANx->CAN_Mask.Data.All & CAN_FIFO1_FMDB1_W) >> 24);
	/* The mask all feature */
	CAN_MaskStruct->Mask_All = (UINT8)((CANx->CAN_Mask.Control.All & CAN_FMC_FMA_W) >> 7); 
	/* Ftach the Byte 0 and 1 mask enable in each FIFOs */
	CAN_MaskStruct->Mask_RxFIFO0_Byte0 = (CANx->CAN_Mask.Control.All & CAN_FMC_RB0DFMB0E_W) >> 4;
	CAN_MaskStruct->Mask_RxFIFO0_Byte1 = (CANx->CAN_Mask.Control.All & CAN_FMC_RB0DFMB1E_W) >> 3;
	CAN_MaskStruct->Mask_RxFIFO1_Byte0 = (CANx->CAN_Mask.Control.All & CAN_FMC_RB1DFMB0E_W) >> 2;
	CAN_MaskStruct->Mask_RxFIFO1_Byte1 = (CANx->CAN_Mask.Control.All & CAN_FMC_RB1DFMB1E_W) >> 1;
}

/******************************* CAN_Transmit **********************************
	Introduction:
		Select the one of tx buffer by input argument TxNumber to transmit the 
		frame. This function will set the ID, Data Length code, and data into 
		register. Otherwise, the setting of RTR, BRS, EIE bits would depend 
		on what frame type will be transmitted.
		After setting in CAN_Transmit function, the user should call CAN_TxBTR 
		function to request the tx buffer for transmission start.
    Procedure:
		1. Check the input argument Tx_number must be 0, 1, or 2.
		2. Check this Tx_number is empty or not by the BTRn (n = 0 ,1 ,2) bits 
		   in TRBS register.
		   If this BTRn is 1, the tx buffer n is being used now. For this case, 
		   the program will return -1.
		2. Set the transmitting context in register.
			- Set identifier: CAN is defined to Based and Extended identifier 
			  format. This two formats is set by TREIE bit in TEIR register.
			  However, the identifier is divided into several parts in this 
			  design.
			- If this frame is data frame, set the Data Lenght Code (DLC) in the 
			  TBILDL field. The DLC is defined in the CAN spec. If support FD, 
			  the 
			是data frame或remote frame，設定於TREIE的bit[7]-TRTR
			- 若是data frame則還要去設定DLC，TBILDL中的bit[3:0]-TDL，注意僅有FD能超過8 bytes
			- (FD)FD frame需要去設定TFD的bit[7]-TEDL，FD不可以有Remote frame且可以大於8 bytes
			- (FD)若data phase有switch bit rate，設定TFD的bit[6]-TBRS
			- 若是data frame(TRTR=0)，則去收取data[DLC]，只收取DLC範圍內的data就好。
		4. Return the current tx buffer number.
	Return:
		-2: Wrong TxNumber value
		-1: This TxNumber buffer is being used now
		0: The Tx buffer 0 is now ready
		1: The Tx buffer 1 is now ready
		2: The Tx buffer 2 is now ready
*******************************************************************************/
int CAN_Transmit(CAN_TypeRegs* CANx, CanTxMsgDef* TxMessage, UINT8 TxNumber)
{
	UINT32 reg32_Tx_L = 0x00000000;
	UINT32 reg32_Tx_H = 0x00000000;
	UINT32 i;

	/* Check the TxNumber is empty */
	switch(TxNumber)
	{
		case CAN_Txbuffer_0:
			if(CANx->CAN_Control.All & CAN_CR_BTR0_W)
			{
				return -1;
			}
			break;
		case CAN_Txbuffer_1:
			if(CANx->CAN_Control.All & CAN_CR_BTR1_W)
			{
				return -1;
			}
			break;
		case CAN_Txbuffer_2:
			if(CANx->CAN_Control.All & CAN_CR_BTR2_W)
			{
				return -1;
			}
			break;
		default:
			return -2;
	}

	/* Clear the available Tx buffer register */
	CANx->CAN_TR[TxNumber].TX_L.All &= 0x00000000;
	CANx->CAN_TR[TxNumber].TX_H.All &= 0x00000000;
	/* Clear the available Tx buffer data register */
	memset((void *)&CANx->CAN_TD[TxNumber].Data[0].All, 0, 64);

	/* Set the identifier value */
	reg32_Tx_H |= (TxMessage->StdId & 0x7FF) << 13;
	reg32_Tx_H |= (TxMessage->ExtId & 0x3FC00) >> 10;
	reg32_Tx_L |= (TxMessage->ExtId & 0x3FF) << 22;
	/* Set IDE bit */
	if(TxMessage->IDE)
	{
		reg32_Tx_L |= ((UINT32)(TxMessage->IDE & 0x1) << 14);
	}
	/* Set the frame is the remote frame or not */
	if(TxMessage->RTR) /* remote frame */
	{
		/* Set TRTR bit */
		reg32_Tx_L |= (UINT32)(0x1 << 15);
	}
	else /* data frmae */
	{
		/* Set DLC field */
		reg32_Tx_H |= ((UINT32)(TxMessage->DLC & 0xF) << 8);
	}
	/* If it is FD frame, then set the TEDL bit */
	if(TxMessage->FD)
	{
		reg32_Tx_L |= (UINT32)(0x1 << 7);
	}
	/* If the data phase will change speed, set the TBRS bit */
	if(TxMessage->BRS)
	{
		reg32_Tx_L |= (UINT32)(0x1 << 6);
	}

	CANx->CAN_TR[TxNumber].TX_L.All = reg32_Tx_L;
	CANx->CAN_TR[TxNumber].TX_H.All = reg32_Tx_H;

	/* If the frame is not remote frame and data length > 0 bytes, set the message data value */
	if((TxMessage->DLC > 0) && (TxMessage->RTR != 1))
	{
		if(TxMessage->DLC <= 8)
		{
			CANx->CAN_TD[TxNumber].Data[0].All |= ((TxMessage->Data[3] << 24) | 
				(TxMessage->Data[2] << 16) | 
				(TxMessage->Data[1] <<  8) | 
				(TxMessage->Data[0]));

			CANx->CAN_TD[TxNumber].Data[1].All |= ((TxMessage->Data[7] << 24)| 
				(TxMessage->Data[6] << 16)| 
				(TxMessage->Data[5] <<  8)| 
				(TxMessage->Data[4]));
		}
		else /* data lenght > 8 bytes (only in FD frame) */
		{
			for(i = 0; i < DLCtoBytes_table[TxMessage->DLC]/4; i++) 
			{
				/* Copy all 64 bytes data into data field register */
				CANx->CAN_TD[TxNumber].Data[i].All |= ((TxMessage->Data[i*4+3] << 24) | 
					(TxMessage->Data[i*4+2] << 16) | 
					(TxMessage->Data[i*4+1] <<  8) | 
					(TxMessage->Data[i*4]));
			}
		}
	}

	return TxNumber;
}

/********************************* CAN_TxBTR ***********************************
	Introduction:
		Start to transmit the tx buffer
	Input: 
		CAN_Txbuffer_0, CAN_Txbuffer_1, CAN_Txbuffer_2, CAN_Txbuffer_all
	Procedure:
		1. Check this tx buffer's BTR bit is empty '0'. If not, return -1.
		2. Write this BTRn to request the tx buffer work.
		3. return value n.
	Return:
		-1: This BTRn bit has been set before
		0: The BTR 0 for Tx buffer 0 is now set
		1: The BTR 1 for Tx buffer 2 is now set
		2: The BTR 1 for Tx buffer 2 is now set
	Comment:
		After successfully calling this function, the user can see if the BTRn
		turn to zero for transmission done.
		Or enable interrupt TIE bit to check TBIn in IR register when the 
		interrupt was happened.
*******************************************************************************/
int CAN_TxBTR(CAN_TypeRegs* CANx, UINT8 TxNumber)
{
	switch(TxNumber)
	{
		case CAN_Txbuffer_0:
			/* Check the BTR0 bit is not used */
			if(CANx->CAN_Control.All & CAN_CR_BTR0_W)
			{
				return -1;
			}
			/* Set the BTR0 bit */
			CANx->CAN_Control.All |= CAN_CR_BTR0_W;
			break;
		case CAN_Txbuffer_1:
			/* Check the BTR1 bit is not used */
			if(CANx->CAN_Control.All & CAN_CR_BTR1_W)
			{
				return -1;
			}
			/* Set the BTR1 bit */
			CANx->CAN_Control.All |= CAN_CR_BTR1_W;
			break;
		case CAN_Txbuffer_2:
			/* Check the BTR2 bit is not used */
			if(CANx->CAN_Control.All & CAN_CR_BTR2_W)
			{
				return -1;
			}
			/* Set the BTR2 bit */
			CANx->CAN_Control.All |= CAN_CR_BTR2_W;
			break;
		case CAN_Txbuffer_all:
			/* Set the BTR0,1,2 bit */
			CANx->CAN_Control.All |= (CAN_CR_BTR0_W | CAN_CR_BTR1_W | CAN_CR_BTR2_W);
			break;
	}

	return TxNumber;
}

/************************** CAN_Transmit_timeout *******************************
	Introduction:
		Almost same as CAN_Transmit, but this function is included CAN_TxBTR 
		part. The timeout feature is added into this function, so if the Tx 
		buffer n does not complete transmission and timeout, the program will 
		return -1 as failed transmit.
		This trasmitted function used the polling method to wait the transmission 
		done.
*******************************************************************************/
int CAN_Transmit_timeout(CAN_TypeRegs* CANx, CanTxMsgDef* TxMessage, int CANTxTimeOut)
{
	//UNDO
	return 0;
}

/******************************* CAN_Transmit_TXn ******************************
    Procedure:
	1. 簡查input parameter Tx_number只能0,1,2
	2. 簡查該Tx buffer是否有被使用中，從TRBS中的BTR0, BTR1,or BTR2看
		- 若該tx buffer的BTSn為1，表示使用中，return 0
	3. 填進該指定tx buffer identifier, data等資訊 (Transmit Register)
		- 設定ID：Based與Extended兩ID方式，TREIE的bit[6]-TEIR設定好後去填入id。
		  可以用macros來切開
		- 是data frame或remote frame，設定於TREIE的bit[7]-TRTR
		- 若是data frame則還要去設定DLC，TBILDL中的bit[3:0]-TDL，注意僅有FD能超過8 bytes
		- (FD)FD frame需要去設定TFD的bit[7]-TEDL，FD不可以有Remote frame且可以大於8 bytes
		- (FD)若data phase有switch bit rate，設定TFD的bit[6]-TBRS
		- 若是data frame(TRTR=0)，則去收取data[DLC]，只收取DLC範圍內的data就好。
	4. 去assert該tx buffer的request bit，CR的BTR0,1,2
	5. return此tx number回去
	Comment: 用這function後，可以從CR的BTR0,1,2看是該tx buffer是否完成
	         或是用interrupt方式：Enable IRE的TIE，然後在interrupt handler看IR的TBI0,1,2
	Return:
	    -1:表示該指定buffer無法使用
		0:有用指定該buffer0傳data
		1:有用指定該buffer1傳data
		2:有用指定該buffer2傳data
*******************************************************************************/
int CAN_Transmit_TXn(CAN_TypeRegs* CANx, CanTxMsgDef* TxMessage, UINT8 TxNumber)
{
	UINT32 reg32_Tx_L = 0x00000000;
	UINT32 reg32_Tx_H = 0x00000000;
	UINT32 i;

	/* Check the TxNumber is empty */
	switch(TxNumber)
	{
		case CAN_Txbuffer_0:
			if(CANx->CAN_Control.All & CAN_CR_BTR0_W)
			{
				return -1;
			}
			break;
		case CAN_Txbuffer_1:
			if(CANx->CAN_Control.All & CAN_CR_BTR1_W)
			{
				return -1;
			}
			break;
		case CAN_Txbuffer_2:
			if(CANx->CAN_Control.All & CAN_CR_BTR2_W)
			{
				return -1;
			}
			break;
	}

	/* Clear the available Tx buffer register */
	CANx->CAN_TR[TxNumber].TX_L.All &= 0x00000000;
	CANx->CAN_TR[TxNumber].TX_H.All &= 0x00000000;
	/* Clear the available Tx buffer data register */
	memset((void *)&CANx->CAN_TD[TxNumber].Data[0].All, 0, 64);

	/* Set the identifier value */
	reg32_Tx_H |= (TxMessage->StdId & 0x7FF) << 13;
	reg32_Tx_H |= (TxMessage->ExtId & 0x3FC00) >> 10;
	reg32_Tx_L |= (TxMessage->ExtId & 0x3FF) << 22;
	/* Set IDE bit */
	if(TxMessage->IDE)
	{
		reg32_Tx_L |= ((UINT32)(TxMessage->IDE & 0x1) << 14);
	}
	/* Set the frame is the remote frame or not */
	if(TxMessage->RTR) /* remote frame */
	{
		/* Set TRTR bit */
		reg32_Tx_L |= (UINT32)(0x1 << 15);
	}
	else /* data frmae */
	{
		/* Set DLC field */
		reg32_Tx_H |= ((UINT32)(TxMessage->DLC & 0xF) << 8);
	}
	/* If it is FD frame, then set the TEDL bit */
	if(TxMessage->FD)
	{
		reg32_Tx_L |= (UINT32)(0x1 << 7);
	}
	/* If the data phase will change speed, set the TBRS bit */
	if(TxMessage->BRS)
	{
		reg32_Tx_L |= (UINT32)(0x1 << 6);
	}

	CANx->CAN_TR[TxNumber].TX_L.All = reg32_Tx_L;
	CANx->CAN_TR[TxNumber].TX_H.All = reg32_Tx_H;

	/* If the frame is not remote frame and data length > 0 bytes, set the message data value */
	if((TxMessage->DLC > 0) && (TxMessage->RTR != 1))
	{
		if(TxMessage->DLC <= 8)
		{			
			CANx->CAN_TD[TxNumber].Data[0].All |= ((TxMessage->Data[3] << 24) | 
				(TxMessage->Data[2] << 16) | 
				(TxMessage->Data[1] <<  8) | 
				(TxMessage->Data[0]));

			CANx->CAN_TD[TxNumber].Data[1].All |= ((TxMessage->Data[7] << 24)| 
				(TxMessage->Data[6] << 16)| 
				(TxMessage->Data[5] <<  8)| 
				(TxMessage->Data[4]));
		}
		else /* data lenght > 8 bytes (only in FD frame) */
		{
			for(i = 0; i < DLCtoBytes_table[TxMessage->DLC]/4; i++) 
			{
				CANx->CAN_TD[TxNumber].Data[i].All |= ((TxMessage->Data[i*4+3] << 24) | 
					(TxMessage->Data[i*4+2] << 16) | 
					(TxMessage->Data[i*4+1] <<  8) | 
					(TxMessage->Data[i*4]));
			}
		}
	}

	/* Set the request bit of this Tx number */
	switch(TxNumber)
	{
		case CAN_Txbuffer_0:
			CANx->CAN_Control.All |= CAN_CR_BTR0_W;
			break;
		case CAN_Txbuffer_1:
			CANx->CAN_Control.All |= CAN_CR_BTR1_W;
			break;
		case CAN_Txbuffer_2:
			CANx->CAN_Control.All |= CAN_CR_BTR2_W;
			break;
	}

	return TxNumber;
}

/***************************** CAN_Receive_RXn *********************************
	Procedure:
	1. 簡查RxFIFO_number是否為0或1
	2. 看TRBS中的BRS0或BRS1來確認是否有message在裡面
		- 若BRSn為0，則return -1
	3. BRSn > 0則讀出該message
		- 根據TIER來收下ID，此TIER也會存進RxMessage的IDE
		  .TIER = 0表示為Based ID，去讀取RBIH、RBIL。然後放進RxMessage的StdId
		  .TIER = 1表示為Extended ID，除了RBIH、RBIL也要讀取進RxMessage的StdId外
		   ，REIH、REIM、REIL也都要讀出放進RxMessage的ExtId
		- 讀取RRTR來判斷是data frame還是remote frame，此RRTR也會存入RxMessage的RTR
		  .RRTR = 0，表示為data frame
		  .RRTR = 1，表示為remote frame
		- 讀取REDL來判斷是non-FD還是FD frame，並將此REDL存入RxMessage的FD
		  .REDL = 0表示為non-FD frame。RBRS檢查是不是等於0，並放進RxMessage的BRS。
		   另外也要檢查DLC，Data length不能大於8 bytes，然後將此DLC存入RxMessage。
		  .REDL = 1表示為FD frame。此時再去將RBRS值存入RxMessage的BRS，另外data 
		   length也可以允許大於8 bytes，然後將此DLC存入RxMessage。RESI也需要存入至RxMessage的ESI。
	4. 若有開啟TSE：
	    - Byte1與Byte0會是Tx的time stamp，將此兩bytes放入TxTimeStamp
		- 出此Rx buffer n的RBTSH與RBTSL，並合起來存至TimeStamp
	5. 根據DLC去收data[data length]
	6. 根據目前的Rx buffer number去release top FIFO，CR的RRB0 or 1
	   可以呼叫CAN_FIFORelease_RXn
	   return此Rx buffer number
	Comment:
		這function可以先用While loop方式持續看BRSn，確定有收入data後再呼叫此function。
		Interrupt方式可以enable RIE bit，然後在interrupt handler去看RBI0 or 1的status register。
	Return:
		-1:表示該Rx buffer沒有message可以收
		0:收到Rx buffer0 message
		1:收到Rx buffer1 message
*******************************************************************************/
int CAN_Receive_RXn(CAN_TypeRegs* CANx, UINT8 RxFIFO_number, CanRxMsgDef* RxMessage)
{
	UINT32 reg32_Rx_L;
	UINT32 reg32_Rx_H;
	UINT32 reg32_Rx_data;
	UINT32 i, j, k;

	/* check the BRSn field contain the message or not */
	switch(RxFIFO_number)
	{
		case CAN_RxFIFO_0:
			if(((CANx->CAN_Status.All & CAN_TRBS_BRS0_W) >> 11) == 0)
			{
				/* The RxFIFO0 does not contain the message */
				return -1;
			}
			break;
		case CAN_RxFIFO_1:
			if(((CANx->CAN_Status.All & CAN_TRBS_BRS1_W) >> 9) == 0)
			{
				/* The RxFIFO1 does not contain the message */
				return -1;
			}
			break;
	}

	/* Read the message from the related registers */
	reg32_Rx_L = CANx->CAN_RR[RxFIFO_number].RX_L.All;
	reg32_Rx_H = CANx->CAN_RR[RxFIFO_number].RX_H.All;

	RxMessage->StdId = (reg32_Rx_H & CAN_RBI_W) >> 13;
	RxMessage->ExtId = (((reg32_Rx_H & CAN_REIH_W) << 10) | ((reg32_Rx_L & CAN_REIL_W) >> 22));
	RxMessage->IDE = (UINT8)((reg32_Rx_L & CAN_RREIE_REIE_W) >> 14);
	RxMessage->RTR = (UINT8)((reg32_Rx_L & CAN_RREIE_RRTR_W) >> 15);
	RxMessage->BRS = (UINT8)((reg32_Rx_L & CAN_RFD_RBRS_W) >> 6);
	RxMessage->FD = (UINT8)((reg32_Rx_L & CAN_RFD_REDL_W) >> 7);
	RxMessage->ESI = (UINT8)((reg32_Rx_L & CAN_RFD_RESI_W) >> 5);
	RxMessage->DLC = (UINT8)((reg32_Rx_H & CAN_RBILDL_RDL_W) >> 8);
	
	memset(&RxMessage->Data, 0, 64); //Clear the 64-byte data to 0

	/* Read the data value */
	if((RxMessage->DLC > 0) && (RxMessage->RTR != 1)) /* only non-remote frame and data length more than 0 need to read */
	{
		if(RxMessage->DLC <= 8)/* the data lenght less than or equal to 8 bytes */
		{
			RxMessage->Data[0] = (UINT8)(CANx->CAN_RD[RxFIFO_number].Data[0].All & 0x000000FF);
			RxMessage->Data[1] = (UINT8)((CANx->CAN_RD[RxFIFO_number].Data[0].All & 0x0000FF00) >> 8);
			RxMessage->Data[2] = (UINT8)((CANx->CAN_RD[RxFIFO_number].Data[0].All & 0x00FF0000) >> 16);
			RxMessage->Data[3] = (UINT8)((CANx->CAN_RD[RxFIFO_number].Data[0].All & 0xFF000000) >> 24);
			RxMessage->Data[4] = (UINT8)(CANx->CAN_RD[RxFIFO_number].Data[1].All & 0x000000FF);
			RxMessage->Data[5] = (UINT8)((CANx->CAN_RD[RxFIFO_number].Data[1].All & 0x0000FF00) >> 8);
			RxMessage->Data[6] = (UINT8)((CANx->CAN_RD[RxFIFO_number].Data[1].All & 0x00FF0000) >> 16);
			RxMessage->Data[7] = (UINT8)((CANx->CAN_RD[RxFIFO_number].Data[1].All & 0xFF000000) >> 24);
		}
		else /* data lenght more than 8 bytes (only in FD frame) */
		{
			k = 0;
			for(i = 0; i < DLCtoBytes_table[RxMessage->DLC]/4; i++) 
			{
				reg32_Rx_data = CANx->CAN_RD[RxFIFO_number].Data[i].All;
				for(j = 0; j < 4; j++)
				{
					RxMessage->Data[k++] = (UINT8)((reg32_Rx_data >> (8*j)) & 0xFF);
				}
			}
		}
	}

	/* If the Time stamp feature is enable, reading the Tx timestamp at Byte0 ~ Byte1 and the Rx timestamp in register */
	if(CANx->CAN_Control.All & CAN_CE0_TSE_W)
	{
		/* When data field is greater than 2 bytes, the tx timestamp 16-bit value would replace the last two bytes */
		if(RxMessage->DLC >= 2)
		{
			RxMessage->TxTimeStamp = 
				((UINT16)RxMessage->Data[DLCtoBytes_table[RxMessage->DLC] - 2] << 8) |
				(UINT16)RxMessage->Data[DLCtoBytes_table[RxMessage->DLC] - 1];
		}
		RxMessage->RxTimeStamp = (UINT16)((CANx->CAN_Timestamp.All >> (16*RxFIFO_number)) & 0xFFFF);
	}

	/* Release a top buffer in this RxFIFO */
	switch(RxFIFO_number)
	{
		case CAN_RxFIFO_0:
			CANx->CAN_Control.All |= CAN_CR_RRB0_W;
			break;
		case CAN_RxFIFO_1:
			CANx->CAN_Control.All |= CAN_CR_RRB1_W;
			break;
	}

	return RxFIFO_number;
}

/************************ CAN_Receive_RXn_timeout ******************************
	Introduction:
		用Polling方式來在timeout時間內收取data。與CAN_Receive_RXn的流程有部分雷同
	Procedure:
		1. 查看指定的RxFIFO_number n 在TRBS中的BRSn是否有message在裡面
			採用While Loop來遞減Timeout，期間持續去檢查BRSn是否為0
			- 若Timeout遞減至0時BRSn還是為0，則return -1
			- 若Timeout在遞減為0前BRSn為1，則goto setp 2
		2. BRSn > 0則讀出該message
			- 根據TIER來收下ID，此TIER也會存進RxMessage的IDE
			  .TIER = 0表示為Based ID，去讀取RBIH、RBIL。然後放進RxMessage的StdId
			  .TIER = 1表示為Extended ID，除了RBIH、RBIL也要讀取進RxMessage的StdId外
			   ，REIH、REIM、REIL也都要讀出放進RxMessage的ExtId
			- 讀取RRTR來判斷是data frame還是remote frame，此RRTR也會存入RxMessage的RTR
			  .RRTR = 0，表示為data frame
			  .RRTR = 1，表示為remote frame
			- 讀取REDL來判斷是non-FD還是FD frame，並將此REDL存入RxMessage的FD
			  .REDL = 0表示為non-FD frame。RBRS檢查是不是等於0，並放進RxMessage的BRS。
			   另外也要檢查DLC，Data length不能大於8 bytes，然後將此DLC存入RxMessage。
			  .REDL = 1表示為FD frame。此時再去將RBRS值存入RxMessage的BRS，另外data 
			   length也可以允許大於8 bytes，然後將此DLC存入RxMessage。RESI也需要存入至RxMessage的ESI。
		3. 若有開啟TSE：
			- Byte1與Byte0會是Tx的time stamp，將此兩bytes放入TxTimeStamp
			- 出此Rx buffer n的RBTSH與RBTSL，並合起來存至TimeStamp
		4. 根據DLC去收data[data length]
		5. 根據目前的Rx buffer number去release top FIFO，CR的RRB0 or 1
		   可以呼叫CAN_FIFORelease_RXn
	       return此Rx buffer number
	Retrun:
		-1: 表示該Rx buffer n在表示Timeout發生時沒有message可以收
		0:收到Rx buffer0 message
		1:收到Rx buffer1 message
*******************************************************************************/
int CAN_Receive_RXn_timeout(CAN_TypeRegs* CANx, UINT8 RxFIFO_number, CanRxMsgDef* RxMessage, int CANRxTimeOut)
{
	UINT32 reg32_Rx_L;
	UINT32 reg32_Rx_H;
	UINT32 reg32_Rx_data;
	UINT32 i, j, k;

	/* check the BRSn field contain the message or not */
	if(RxFIFO_number == 0)
	{
		while((((CANx->CAN_Status.All & CAN_TRBS_BRS0_W) >> 11) == 0) && CANRxTimeOut >= 0)
		{
			CANRxTimeOut--;
		}
	}
	else
	{
		while((((CANx->CAN_Status.All & CAN_TRBS_BRS1_W) >> 9) == 0) && CANRxTimeOut >= 0)
		{
			CANRxTimeOut--;
		}
	}

	/* No data receiving before timeout */
	if(CANRxTimeOut < 0)
		return -1;

	/* Read the message from the related registers */
	reg32_Rx_L = CANx->CAN_RR[RxFIFO_number].RX_L.All;
	reg32_Rx_H = CANx->CAN_RR[RxFIFO_number].RX_H.All;

	RxMessage->StdId = (reg32_Rx_H & CAN_RBI_W) >> 13;
	RxMessage->ExtId = (((reg32_Rx_H & CAN_REIH_W) << 10) | ((reg32_Rx_L & CAN_REIL_W) >> 22));
	RxMessage->IDE = (UINT8)((reg32_Rx_L & CAN_RREIE_REIE_W) >> 14);
	RxMessage->RTR = (UINT8)((reg32_Rx_L & CAN_RREIE_RRTR_W) >> 15);
	RxMessage->BRS = (UINT8)((reg32_Rx_L & CAN_RFD_RBRS_W) >> 6);
	RxMessage->FD = (UINT8)((reg32_Rx_L & CAN_RFD_REDL_W) >> 7);
	RxMessage->ESI = (UINT8)((reg32_Rx_L & CAN_RFD_RESI_W) >> 5);
	RxMessage->DLC = (UINT8)((reg32_Rx_H & CAN_RBILDL_RDL_W) >> 8);

	/* Read the data value */
	if((RxMessage->DLC > 0) && (RxMessage->RTR != 1)) /* only non-remote frame and data length more than 0 need to read */
	{
		if(RxMessage->DLC <= 8)/* the data lenght less than or equal to 8 bytes */
		{
			RxMessage->Data[0] = (UINT8)(CANx->CAN_RD[RxFIFO_number].Data[0].All & 0x000000FF);
			RxMessage->Data[1] = (UINT8)((CANx->CAN_RD[RxFIFO_number].Data[0].All & 0x0000FF00) >> 8);
			RxMessage->Data[2] = (UINT8)((CANx->CAN_RD[RxFIFO_number].Data[0].All & 0x00FF0000) >> 16);
			RxMessage->Data[3] = (UINT8)((CANx->CAN_RD[RxFIFO_number].Data[0].All & 0xFF000000) >> 24);
			RxMessage->Data[4] = (UINT8)(CANx->CAN_RD[RxFIFO_number].Data[1].All & 0x000000FF);
			RxMessage->Data[5] = (UINT8)((CANx->CAN_RD[RxFIFO_number].Data[1].All & 0x0000FF00) >> 8);
			RxMessage->Data[6] = (UINT8)((CANx->CAN_RD[RxFIFO_number].Data[1].All & 0x00FF0000) >> 16);
			RxMessage->Data[7] = (UINT8)((CANx->CAN_RD[RxFIFO_number].Data[1].All & 0xFF000000) >> 24);
		}
		else /* data lenght more than 8 bytes (only in FD frame) */
		{
			k = 0;
			for(i = 0; i < DLCtoBytes_table[RxMessage->DLC]/4; i++) 
			{
				reg32_Rx_data = CANx->CAN_RD[RxFIFO_number].Data[i].All;
				for(j = 0; j < 4; j++)
				{
					RxMessage->Data[k++] = (UINT8)((reg32_Rx_data >> (8*j)) & 0xFF);
				}
			}
		}
	}

	/* If the Time stamp feature is enable, reading the Tx timestamp at Byte0 ~ Byte1 and the Rx timestamp in register */
	if(CANx->CAN_Control.All & CAN_CE0_TSE_W)
	{
		/* When data field is greater than 2 bytes, the tx timestamp 16-bit value would replace the last two bytes */
		if(RxMessage->DLC >= 2)
		{
			RxMessage->TxTimeStamp = 
				((UINT16)RxMessage->Data[DLCtoBytes_table[RxMessage->DLC] - 2] << 8) |
				(UINT16)RxMessage->Data[DLCtoBytes_table[RxMessage->DLC] - 1];
		}
		RxMessage->RxTimeStamp = (UINT16)((CANx->CAN_Timestamp.All >> (16*RxFIFO_number)) & 0xFFFF);
	}

	/* Release a top buffer in this RxFIFO */
	switch(RxFIFO_number)
	{
		case CAN_RxFIFO_0:
			CANx->CAN_Control.All |= CAN_CR_RRB0_W;
			break;
		case CAN_RxFIFO_1:
			CANx->CAN_Control.All |= CAN_CR_RRB1_W;
			break;
	}

	return RxFIFO_number;
}

/************************** CAN_FIFORelease_RXn ********************************
	Introduction:
		Used to release a buffer in the specified Rx FIFO n, clearing only one 
		buffer at a time.
	Procedure:
		1. 檢查該RxFIFO是否有message在，透過TRBS中的BRSn
			- 若BRSn為0，則return -1
		2. release該buffer，assert CR的RRBn
		3. 檢查TRBS的BRSn是否有少一
		4. return n
	Return:
	    -2:the RxFIFO_number is wrong
		-1:表示該Rx buffer沒有message可以release
		0:Release Rx buffer0 top message
		1:Release Rx buffer1 top message
*******************************************************************************/
int CAN_FIFORelease_RXn(CAN_TypeRegs* CANx, UINT8 RxFIFO_number)
{
	switch(RxFIFO_number)
	{
		case CAN_RxFIFO_0:
			if(((CANx->CAN_Status.All & CAN_TRBS_BRS0_W) >> 11) == 0)
			{
				/* The RxFIFO0 does not contain the message */
				return -1;
			}
			CANx->CAN_Control.All |= CAN_CR_RRB0_W;
			return 0;
		case CAN_RxFIFO_1:
			if(((CANx->CAN_Status.All & CAN_TRBS_BRS1_W) >> 9) == 0)
			{
				/* The RxFIFO1 does not contain the message */
				return -1;
			}
			CANx->CAN_Control.All |= CAN_CR_RRB1_W;
			return 1;
	}

	return -2;
}

/************************** CAN_FIFORelease_All ********************************
	Introduction:
		Clear all buffers in the Rx FIFO n.
	Procedure:
		According RxFIFO_number n:
			0: Clear Rx FIFO 0
			1: Clear Rx FIFO 1
			2: Clear both Rx FIFO 0 and 1
*******************************************************************************/
void CAN_FIFORelease_All(CAN_TypeRegs* CANx, UINT8 RxFIFO_number)
{
	switch(RxFIFO_number)
	{
		case CAN_RxFIFO_0:
			CANx->CAN_Control.All |= CAN_CR_RRBA0_W;
			break;
		case CAN_RxFIFO_1:
			CANx->CAN_Control.All |= CAN_CR_RRBA1_W;
			break;
		case CAN_RxFIFO_All:
			CANx->CAN_Control.All |= CAN_CR_RRBA0_W;
			CANx->CAN_Control.All |= CAN_CR_RRBA1_W;
			break;
	}
}

/**************************** CAN_GetFIFOStatus ********************************
	Introduction:
		Read how many data in this Rx FIFO now.
*******************************************************************************/
int CAN_GetFIFOStatus(CAN_TypeRegs* CANx, UINT8 RxFIFO_number)
{
	switch(RxFIFO_number)
	{
		case CAN_RxFIFO_0:
			return (UINT8)((CANx->CAN_Status.All & CAN_TRBS_BRS0_W) >> 11);
		case CAN_RxFIFO_1:
			return (UINT8)((CANx->CAN_Status.All & CAN_TRBS_BRS1_W) >> 9);
	}

	return -1;
}

/****************************** CAN_ITConfig ***********************************
	Introduction:
		Set the Interrupt request bits.
		The parameter NewState is ENABLE or DISABLE.
	Input:
		CAN_IRE_BIT is same as IRE 8-bit value, so the input parameter CAN_IRE_BIT
		can be CAN_IRE_RIE, CAN_IRE_TIE, CAN_IRE_EIE, CAN_IRE_OIE, or CAN_IRE_WIE
		'or' together.
*******************************************************************************/
void CAN_ITConfig(CAN_TypeRegs* CANx, UINT8 CAN_IRE_BIT, BOOL NewState)
{
	if (NewState != DISABLE)
	{
		CANx->CAN_Control.All |= ((UINT32)CAN_IRE_BIT << 8);
	}
	else
	{
		CANx->CAN_Control.All &= ~((UINT32)CAN_IRE_BIT << 8);
	}
}

/***************************** CAN_ClearITStatus *******************************
	Introduction:
		Use write-1-clear method to clear the RW1C interrupt status bit.
	Input:
		The CAN_IR_BIT parameter can be：
			CAN_IR_RBI1、CAN_IR_RBI0、CAN_IR_TBI2、CAN_IR_TBI1、
			CAN_IR_TBI0、CAN_IR_EIR、CAN_IR_OIR、CAN_IR_WIR
		Also this can be 'or' and not only one status bit.
	Procedure:
		CAN_IR_BIT bit[7:0] IRE regsiter write-1-clear
*******************************************************************************/
void CAN_ClearITStatus(CAN_TypeRegs* CANx, UINT8 CAN_IR_BIT)
{
	CANx->CAN_Interrupt.All = (UINT32)CAN_IR_BIT;
}

/****************************** CAN_GetStatusBit *******************************
	Introduction:
		Input the CAN_STATUS_BIT (defined in CAN.h). This function can return 
		the stauts bit or field across three registers (TS, TRBS, and BS).
		CAN_STATUS_BIT is contained the register, bit number, and  field width
		about this status infomation.
	Input:
		CAN_STATUS_BIT is consist of 8-bit:
		│ R │ X X │ X X X │ X X │
		       │      │      └──┬ 0:TS register
			   │      │         │ 1:TRBS register
			   │      │         │ 2:BS register
			   │      │         └ 3:IR register
			   │      └─┬ 0: bit[0]
			   │        │ 1: bit[1]
			   │        │ ...
			   │        └ 7: bit[7]
			   └─┬ If this status is field type:
			     │ 0: 1-bit field, mask = 0x1
				 │ 1: 2-bit field, mask = 0x3
				 └ 2,3: Reserved
	Procedure:
		1. Parsing CAN_STATUS_BIT bit[1:0]to get this status from which register
		2. Parsing CAN_STATUS_BIT bit[4:2]to get this status from this resgister 
		   which number bit.
		3. Parsing CAN_STATUS_BIT bit[6:5]to get this status how many bit fields
		4. return this status bit/field.
	Comment:
		This function only watch a status in a time, so CAN_STATUS_BIT cannot be
		'or' operated more CAN_STATUS_BIT.
	Return:
		Status bit/field value
*******************************************************************************/
UINT8 CAN_GetStatusBit(CAN_TypeRegs* CANx, UINT8 CAN_STATUS_BIT)
{
	UINT8 reg, reg_bit, reg_field;
	UINT8 mask_table[4] = {0x1, 0x3, 0x7, 0xF}; //0表示field=1 bit, 1表示field=2 bits, 2表示field=3 bits...

	/* Get the status bit info from parameter CAN_STATUS_BIT */
	reg = CAN_STATUS_BIT & 0x3;
	reg_bit = (CAN_STATUS_BIT & 0x1C) >> 2;
	reg_field = ((CAN_STATUS_BIT & 0x60) >> 5);

	if(reg == 0)
	{
		return (UINT8)((((CANx->CAN_Status.All >> 16) & 0xFF) >> reg_bit) & mask_table[reg_field]);
	}
	else if(reg == 1)
	{
		return (UINT8)((((CANx->CAN_Status.All >> 8) & 0xFF) >> reg_bit) & mask_table[reg_field]);
	}
	else if(reg == 2)
	{
		return (UINT8)((((CANx->CAN_Status.All) & 0xFF) >> reg_bit) & mask_table[reg_field]);
	}
	else if(reg == 3)
	{
		return (UINT8)((((CANx->CAN_Interrupt.All) & 0xFF) >> reg_bit) & mask_table[reg_field]);
	}
	return 0;
}

/***************************** CAN_GetIRStatus *********************************
	Introduction:
		Directly return the IR register 8-bit value.
*******************************************************************************/
UINT8 CAN_GetIRStatus(CAN_TypeRegs* CANx)
{
	return CANx->CAN_Interrupt.Byte.IR;
}

/******************************* CAN_ClearIT ***********************************
	Introduction:
		Clear a specific bit in IR reigster. The attribute of this bit is RW1C.
		CAN_STATUS_BIT is only one of below parameter:
			CAN_WIR_STATUS_BIT
			CAN_OIR_STATUS_BIT
			CAN_EIR_STATUS_BIT
			CAN_TBI0_STATUS_BIT
			CAN_TBI1_STATUS_BIT
			CAN_TBI2_STATUS_BIT
			CAN_RBI0_STATUS_BIT
			CAN_RBI1_STATUS_BIT
	Procedure:
		Use the CAN_STATUS_BIT bit[4:2] to get the target bit.
		And write one to clear this stauts bit.
*******************************************************************************/
void CAN_ClearIT(CAN_TypeRegs* CANx, UINT8 CAN_STATUS_BIT)
{
	UINT8 reg_bit;

	reg_bit = (CAN_STATUS_BIT & 0x1C) >> 2;
	CANx->CAN_Interrupt.All = (1 << reg_bit); //write-1-clear
}

/***************************** CAN_GetBTRStatus ********************************
	Introduction:
		Get the current BTRn bit status.
*******************************************************************************/
int CAN_GetBTRStatus(CAN_TypeRegs* CANx, UINT8 Tx_number)
{
	switch(Tx_number)
	{
		case CAN_Txbuffer_0:
			return (UINT8)((CANx->CAN_Control.All & CAN_CR_BTR0_W) >> 5);
		case CAN_Txbuffer_1:
			return (UINT8)((CANx->CAN_Control.All & CAN_CR_BTR1_W) >> 4);
		case CAN_Txbuffer_2:
			return (UINT8)((CANx->CAN_Control.All & CAN_CR_BTR2_W) >> 3);
	}
	return 0;
}

/************************** CAN_GetLastErrorCode *******************************
	Introduction:
		Get the last error code (type) from ET register.
*******************************************************************************/
UINT8 CAN_GetLastErrorCode(CAN_TypeRegs* CANx)
{
	return (UINT8)(CANx->CAN_Error_Monitor.Status.All & 0x000000FF);
}

/********************** CAN_GetReceiveErrorCounter *****************************
	Introduction:
		Get the REC value from RECH and RECL registers.
	Return:
		16-bit REC value
*******************************************************************************/
UINT16 CAN_GetReceiveErrorCounter(CAN_TypeRegs* CANx)
{
	return (UINT16)(CANx->CAN_Error_Monitor.Counter.All & 0x0000FFFF);
}

/********************** CAN_GetTransmitErrorCounter ****************************
	Introduction:
		Get the TEC value from TECH and TECL registers.
	Return:
		16-bit TEC value
*******************************************************************************/
UINT16 CAN_GetTransmitErrorCounter(CAN_TypeRegs* CANx)
{
	return (UINT16)((CANx->CAN_Error_Monitor.Counter.All & 0xFFFF0000) >> 16);
}