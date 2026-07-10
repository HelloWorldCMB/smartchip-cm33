#ifndef __CAN_H
#define __CAN_H
#include "types.h"
/* CAN Modes */
#define CAN_Mode_Config    ((UINT8)0x0)  //Configuration mode
#define CAN_Mode_Normal    ((UINT8)0x1)  //Normal mode
#define CAN_Mode_Sleep     ((UINT8)0x2)  //Sleep mode
#define CAN_Mode_Listen    ((UINT8)0x3)  //Listen mode
#define CAN_Mode_LoopBack  ((UINT8)0x4)  //Loopback mode

/* Rx FIFO number */
#define CAN_RxFIFO_0       ((UINT8)0x0) 
#define CAN_RxFIFO_1       ((UINT8)0x1) 
#define CAN_RxFIFO_All     ((UINT8)0x2) 

/* Tx buffer number */
#define CAN_Txbuffer_0     ((UINT8)0x0) 
#define CAN_Txbuffer_1     ((UINT8)0x1) 
#define CAN_Txbuffer_2     ((UINT8)0x2)
#define CAN_Txbuffer_all   ((UINT8)0xff)

/* Retransmission option */
#define CAN_RT_Always      ((UINT8)0x0)  //Always retransmit the message if the previous transmission is failed
#define CAN_RT_1time       ((UINT8)0x1)  //Only transmission one time whether is successful or not
#define CAN_RT_3times      ((UINT8)0x2)  //Retransmission three times
#define CAN_RT_8times      ((UINT8)0x3)  //Retransmission eight times

/* Bit in Frame */
#define CAN_ID_Based       ((UINT8)0x0)  //Standard Identifier
#define CAN_ID_Extended    ((UINT8)0x1)  //Extended Identifier
#define CAN_Data_frame     ((UINT8)0x0)  //Data frame and RTR = 0
#define CAN_Remote_frame   ((UINT8)0x1)  //Remote frame and RTR = 1

/* Data Length Code */
#define CAN_DataBytes_0    ((UINT8)0x00)
#define CAN_DataBytes_1    ((UINT8)0x01)
#define CAN_DataBytes_2    ((UINT8)0x02)
#define CAN_DataBytes_3    ((UINT8)0x03)
#define CAN_DataBytes_4    ((UINT8)0x04)
#define CAN_DataBytes_5    ((UINT8)0x05)
#define CAN_DataBytes_6    ((UINT8)0x06)
#define CAN_DataBytes_7    ((UINT8)0x07)
#define CAN_DataBytes_8    ((UINT8)0x08)
#define CAN_DataBytes_12   ((UINT8)0x09)
#define CAN_DataBytes_16   ((UINT8)0x0a)
#define CAN_DataBytes_20   ((UINT8)0x0b)
#define CAN_DataBytes_24   ((UINT8)0x0c)
#define CAN_DataBytes_32   ((UINT8)0x0d)
#define CAN_DataBytes_48   ((UINT8)0x0e)
#define CAN_DataBytes_64   ((UINT8)0x0f)


/************************** Identifier marcos **********************************

| - - - - - - - - - - - - - - Arbitraion field - - - - - - - - - - - - - - - - |
|                          |   |   |                                           |
|.Based ID field (11 bits).|SRR|IDE|.....Extended ID field (29-11=18 bits).....|
|--------- 8 --------|- 3 -| 1 - 1 |-------- 8 --------|------- 8 -------|- 2 -|
|------- TBIH -------|TBIL-|       |------- TEIH ------|------ TEIM -----|TEIL-|

Input id is 32-bit
These define macros can seperate the id into several segments.
*******************************************************************************/
#define CAN_BasedID_H(StID)       ((StID & 0x7F8) >> 3)    //TBIH
#define CAN_BasedID_L(StID)       ((StID & 0x7))           //TBIL
#define CAN_ExtendedID_H(ExID)    ((ExID & 0x3FC00) >> 10) //TEIH
#define CAN_ExtendedID_M(ExID)    ((ExID & 0x3FC) >> 2)    //TEIM
#define CAN_ExtendedID_L(ExID)    ((ExID & 0x3) << 6)      //TEIL
#define CAN_BasedID(TBIH, TBIL)   ((TBIH << 3) | TBIL)
#define CAN_ExtendedID(TEIH, TEIM, TEIL)    ((TEIH << 10) | (TEIM << 2) | TEIL)

/* Acceptance frame type (AFC register)*/
#define CAN_ALL                     ((UINT8)0x00)
#define CAN_OnlyNonFD               ((UINT8)0x04)
#define CAN_OnlyFD                  ((UINT8)0x08)
#define CAN_OnlyData                ((UINT8)0x10)
#define CAN_OnlyRemote              ((UINT8)0x20)
#define CAN_OnlyBased               ((UINT8)0x40)
#define CAN_OnlyExtended            ((UINT8)0x80)
#define CAN_NonFD_Data              ((UINT8)0x14)
#define CAN_NonFD_Remote            ((UINT8)0x24)
#define CAN_NonFD_Based             ((UINT8)0x44)
#define CAN_NonFD_Based_Data        ((UINT8)0x54)
#define CAN_NonFD_Based_Remote      ((UINT8)0x64)
#define CAN_NonFD_Extended          ((UINT8)0x84)
#define CAN_NonFD_Extended_Data     ((UINT8)0x94)
#define CAN_NonFD_Extended_Remote   ((UINT8)0xA4)
#define CAN_FD_Data                 ((UINT8)0x18)
#define CAN_FD_Based                ((UINT8)0x48)
#define CAN_FD_Based_Data           ((UINT8)0x58)
#define CAN_FD_Extended             ((UINT8)0x88)
#define CAN_FD_Extended_Data        ((UINT8)0x98)

#define CAN_TxTimeStamp(BYTE1, BYTE0)    ((BYTE1 << 8) | BYTE0)
#define CAN_RxTimeStamp(RBTSH, RBTSL)    ((RBTSH << 8) | RBTSL)

#define CAN_TEC(TECH, TECL)    ((TECH << 8) | TECL)
#define CAN_REC(RECH, RECL)    ((RECH << 8) | RECL)

/* CAN_STATUS_BIT */
#define CAN_BTA0_STATUS_BIT    ((UINT8)0x1C)
#define CAN_BTA1_STATUS_BIT    ((UINT8)0x18)
#define CAN_BTA2_STATUS_BIT    ((UINT8)0x14)
#define CAN_BTLA0_STATUS_BIT   ((UINT8)0x10)
#define CAN_BTLA1_STATUS_BIT   ((UINT8)0x0C)
#define CAN_BTLA2_STATUS_BIT   ((UINT8)0x08)
#define CAN_BTS_STATUS_BIT     ((UINT8)0x20)
#define CAN_BTS0_STATUS_BIT    ((UINT8)0x1D)
#define CAN_BTS1_STATUS_BIT    ((UINT8)0x19)
#define CAN_BTS2_STATUS_BIT    ((UINT8)0x0D)
#define CAN_BRS0_STATUS_BIT    ((UINT8)0x2D)
#define CAN_BRS1_STATUS_BIT    ((UINT8)0x25)
#define CAN_BO_STATUS_BIT      ((UINT8)0x1E)
#define CAN_EW_STATUS_BIT      ((UINT8)0x1A)
#define CAN_TS_STATUS_BIT      ((UINT8)0x16)
#define CAN_RS_STATUS_BIT      ((UINT8)0x12)
#define CAN_TC_STATUS_BIT      ((UINT8)0x0E)
#define CAN_RC_STATUS_BIT      ((UINT8)0x0A)
#define CAN_DO_STATUS_BIT      ((UINT8)0x2A)
#define CAN_WIR_STATUS_BIT     ((UINT8)0x1F)
#define CAN_OIR_STATUS_BIT     ((UINT8)0x1B)
#define CAN_EIR_STATUS_BIT     ((UINT8)0x17)
#define CAN_TBI0_STATUS_BIT    ((UINT8)0x13)
#define CAN_TBI1_STATUS_BIT    ((UINT8)0x0F)
#define CAN_TBI2_STATUS_BIT    ((UINT8)0x0B)
#define CAN_RBI0_STATUS_BIT    ((UINT8)0x07)
#define CAN_RBI1_STATUS_BIT    ((UINT8)0x03)

typedef enum
{
    OVERRUNERROR    = 2,
    ACKERROR        = 3,
    FORMERROR       = 4,
    CRCERROR        = 5,
    STUFFERROR      = 6,
    BITERROR        = 7
}ErrorType;

typedef struct
{
    UINT8 CAN_Mode; //Normal, Sleep, Listen, Loopback. bit[2:0]-OMR in CE0 register
    UINT8 CAN_RT; //CE0[bit4:3]:RT
    /* Bit timing setting */
    UINT8 CAN_NBRP; //Nominal baud rate prescaler. bit[7:2]-NBRP in NBTC1 register
    UINT8 CAN_NSJW; //Nominal Sync. Jump Width. bit[7:4]-NBRP in NBTC0 register
    UINT8 CAN_NProp; 
    UINT8 CAN_NPS1; 
    UINT8 CAN_NPS2;
    UINT8 CAN_DBRP;
    UINT8 CAN_DSJW;
    UINT8 CAN_DProp;
    UINT8 CAN_DPS1;
    UINT8 CAN_DPS2;
    /* Function setting */
    BOOL CAN_TOE; //CE1[bit5]:TOE
    BOOL CAN_TSE; //CE0[bit7]:TSE
    BOOL CAN_EnDBE; //CE0[bit6]:EnDB
} CAN_InitTypeDef;

typedef struct
{
    UINT32 StdId;
    UINT32 ExtId;
    UINT8 IDE;
    UINT8 RTR;
    UINT8 BRS;
    UINT8 FD;
    UINT8 DLC;
	  UINT8 Data[64];
} CanTxMsgDef;

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
	  UINT8 Data[64];
    UINT16 TxTimeStamp;
    UINT16 RxTimeStamp;
} CanRxMsgDef;

typedef struct
{
    UINT32 Filter_StdId;
    UINT32 Filter_ExtId;
    UINT8 Filter_FIFO0_Byte0_data;
    UINT8 Filter_FIFO0_Byte1_data;
    UINT8 Filter_FIFO1_Byte0_data;
    UINT8 Filter_FIFO1_Byte1_data;
    UINT8 Filter_frameType; //bit[7:6]-AFT0, bit[5:4]-AFT1 ,and bit[3:2]-AFT2, in AFC0 register
    BOOL Filter_RxFIFO0_Byte0; 
    BOOL Filter_RxFIFO0_Byte1;
    BOOL Filter_RxFIFO1_Byte0; 
    BOOL Filter_RxFIFO1_Byte1;
} CAN_FilterTypeDef;

typedef struct
{
    UINT32 Mask_StdId;
    UINT32 Mask_ExtId;
    UINT8 Mask_FIFO0_Byte0_data;
    UINT8 Mask_FIFO0_Byte1_data;
    UINT8 Mask_FIFO1_Byte0_data;
    UINT8 Mask_FIFO1_Byte1_data;
    UINT8 Mask_All;
    BOOL Mask_RxFIFO0_Byte0; 
    BOOL Mask_RxFIFO0_Byte1;
    BOOL Mask_RxFIFO1_Byte0; 
    BOOL Mask_RxFIFO1_Byte1;
} CAN_MaskTypeDef;


/************************** Global functions (API) ****************************/
void CAN_Reset(CAN_TypeRegs* CANx);
void CAN_Init(CAN_TypeRegs* CANx, CAN_InitTypeDef* CAN_InitStruct);
void CAN_Sleep(CAN_TypeRegs* CANx);
void CAN_Mode_Change(CAN_TypeRegs* CANx, UINT8 mode);
UINT8 CAN_GetMode(CAN_TypeRegs* CANx);
/* Filter and Mask */
void CAN_FilterInit(CAN_TypeRegs* CANx, CAN_FilterTypeDef* CAN_FilterStruct, UINT8 Filter_number);
void CAN_FilterGroup(CAN_TypeRegs* CANx, UINT8 Filter_group, BOOL NewState);
void CAN_GetFilterStatus(CAN_TypeRegs* CANx, CAN_FilterTypeDef* CAN_FilterStruct, UINT8 Filter_number);
void CAN_MaskInit(CAN_TypeRegs* CANx, CAN_MaskTypeDef* CAN_MaskStruct, UINT8 RxFIFO_number);
void CAN_MaskNumber(CAN_TypeRegs* CANx, UINT8 RxFIFO_number, BOOL NewState);
void CAN_GetMaskStatus(CAN_TypeRegs* CANx, CAN_MaskTypeDef* CAN_MaskStruct, UINT8 RxFIFO_number);
/* Transmit and Receive */
int CAN_Transmit(CAN_TypeRegs* CANx, CanTxMsgDef* TxMessage, UINT8 TxNumber);
int CAN_TxBTR(CAN_TypeRegs* CANx, UINT8 TxNumber);
int CAN_Transmit_timeout(CAN_TypeRegs* CANx, CanTxMsgDef* TxMessage, int CANTxTimeOut);
int CAN_Transmit_TXn(CAN_TypeRegs* CANx, CanTxMsgDef* TxMessage, UINT8 Tx_number);
int CAN_Receive_RXn(CAN_TypeRegs* CANx, UINT8 RxFIFO_number, CanRxMsgDef* RxMessage);
int CAN_Receive_RXn_timeout(CAN_TypeRegs* CANx, UINT8 RxFIFO_number, CanRxMsgDef* RxMessage, int CANRxTimeOut);
int CAN_FIFORelease_RXn(CAN_TypeRegs* CANx, UINT8 RxFIFO_number);
void CAN_FIFORelease_All(CAN_TypeRegs* CANx, UINT8 RxFIFO_number);
/* Interrupt enable and status */
int CAN_GetFIFOStatus(CAN_TypeRegs* CANx, UINT8 RxFIFO_number);
void CAN_ITConfig(CAN_TypeRegs* CANx, UINT8 CAN_IRE_BIT, BOOL NewState);
UINT8 CAN_GetStatusBit(CAN_TypeRegs* CANx, UINT8 CAN_STATUS_BIT);
UINT8 CAN_GetIRStatus(CAN_TypeRegs* CANx);
void CAN_ClearIT(CAN_TypeRegs* CANx, UINT8 CAN_STATUS_BIT);
int CAN_GetBTRStatus(CAN_TypeRegs* CANx, UINT8 Tx_number);
/* Error type and counter */
UINT8 CAN_GetLastErrorCode(CAN_TypeRegs* CANx);
UINT16 CAN_GetReceiveErrorCounter(CAN_TypeRegs* CANx);
UINT16 CAN_GetTransmitErrorCounter(CAN_TypeRegs* CANx);


#endif