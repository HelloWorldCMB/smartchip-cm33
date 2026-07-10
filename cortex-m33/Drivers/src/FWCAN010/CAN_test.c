#include "Common_Include.h"
#include <stdlib.h>
#include <string.h>
//#include <common.h>
//#include "malloc.h"
#include "CAN_test.h"
//#include "types.h"
#include "int_trans.h"

#define ftuart_getc_t(timeout,port) fLib_getchar_timeout(port, timeout) 
/* The definitions of CANx_FTCAN are in CAN_test.h */
#ifdef CAN1_FTCAN
    #include "CAN_regs.h"   
    #include "CAN.h"
    #define CAN1_BASE       0x56300000
    #define CAN1            ((CAN_TypeRegs *)CAN1_BASE)
#endif
#ifdef CAN2_FTCAN
    #include "CAN_regs.h"   
    #include "CAN.h"
    #define CAN2_BASE       0x54900000
    #define CAN2            ((CAN_TypeRegs *)CAN2_BASE)
#endif
#ifdef CAN3_FTCAN
    #include "CAN_regs.h"   
    #include "CAN.h"
    #define CAN3_BASE       0x99000000
    #define CAN3            ((CAN_TypeRegs *)CAN3_BASE)
#endif    

/****************************** Global variables ******************************/
extern const UINT8 DLCtoBytes_table[16];
static UINT32 ITFLAG, ETFLAG;
static int CAN1ALCnt, CAN2ALCnt, TBCnt; //arbitration loss test use
static UINT32 TBI0Cnt, TBI1Cnt, TBI2Cnt; //Tx buffer x (x = 0,1,2) counter
static UINT32 RBI0Cnt, RBI1Cnt; //Rx buffer x (x = 0,1) counter
static UINT32 counter0, counter1; //test use
static UINT16 gREC, gTEC;


/* Debug print use array for record when USE_DBG == 2 */
#if USE_DBG == 2
    char CAN_DBGMSG[MAX_DBMSG_SIZE] = {0};
#endif
Register_Definition *APB_Wrapper_Reg;
//Register_Definition APB_Wrapper_Reg[Max_Reg_Num];

/* Need to add the TQ parameters maunally */
int bit_timing_table[BIT_RATE_NUMBER][4] = {
    /* {Prescaler, Propgation, Phase1, Phase2} */
    {40, 9, 5, 5},    //125kbps, 75%
    {20, 9, 5, 5},    //250kbps, 75%
    {10, 9, 5, 5},    //500kbps, 75%
    {5, 8, 8, 8},     //800kbps, 68%
    {4, 8, 8, 8}      //1Mbps, 68%
};
int fast_bit_timing_table[Fast_BIT_RATE_NUMBER][4] = {
    /* {Prescaler, Propgation, Phase1, Phase2} */
    {5, 8, 8, 3},   //1Mbps, 85% 
    {5, 1, 4, 4},   //2Mbps, 60%
    {2, 3, 8, 8},   //2.5Mbps, 60%
    {2, 5, 2, 2}    //5Mbps, 80%
};

/*******************************************************************************
                              Local Support function    
*******************************************************************************/
int CAN_DBGMSG_test(void);
int highest_bit_position(UINT8 x);
int irq_setting(int irq, PrHandler handle_irq);
float Oscillator_Tolerance_Calc(CAN_Param *dev, float *df);
void Generate_Random_Pattern(UINT8 length, UINT8 *pData);
void Generate_Random_Pattern_Length(Frame_Type frameType, UINT8 *dlc, UINT8 *pData);
void Generate_Random_Identifier(Frame_Type frameType, UINT32 *pSID, UINT32 *pEID);
UINT32 Verify_Pattern(Frame_Type frameType, CanTxMsgDef *pSource, CanRxMsgDef *pDest);

void APB_Reg_Init(void);

int CAN_DBGMSG_test(void)
{
    fLib_printf("test in CAN_test.c\n");
    CAN_DBGPRINTF(DBG_LEVEL_0, "test in CAN_test.c\n", NULL);
    CAN_DBGPRINTF(DBG_LEVEL_0, "test in CAN_test.c %d %d %d\n", 1, 2, 3);
    CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[Test_Normal]);
    CAN_DBGPRINTF(DBG_LEVEL_1, "%s", err_msg[Test_Normal]);
    CAN_DBGPRINTF(DBG_LEVEL_2, "%s", err_msg[Test_Normal]);
    CAN_DBGPRINTF(DBG_LEVEL_3, "%s", err_msg[Test_Normal]);
    
    CAN_DBGPRINTF2(DBG_LEVEL_0, ("test in CAN_test.c\n"));
    CAN_DBGPRINTF2(DBG_LEVEL_0, ("test in CAN_test.c %d %d %d\n", 1, 2, 3));
    CAN_DBGPRINTF2(DBG_LEVEL_0, ("%s", err_msg[Test_Normal]));
    CAN_DBGPRINTF2(DBG_LEVEL_1, ("%s", err_msg[Test_Normal]));
    CAN_DBGPRINTF2(DBG_LEVEL_2, ("%s", err_msg[Test_Normal]));
    CAN_DBGPRINTF2(DBG_LEVEL_3, ("%s", err_msg[Test_Normal]));
    
#if USE_DBG == 2
    fLib_printf("%s", CAN_DBGMSG);
    fLib_printf("CAN_DBGMSG size = %d", strlen(CAN_DBGMSG));
#endif
    return 0;
}

int highest_bit_position(UINT8 x)
{
    int number = 0;
    while(x)
    {
        number++;
        x = x >> 1;
    }
    return number - 1;
}

int irq_setting(int irq, PrHandler handle_irq)
{
#if 1
    fLib_CloseIRQ(irq);

    if (!fLib_ConnectIRQ( irq, handle_irq))
        
    fLib_SetIRQmode((UINT32)irq,LEVEL);
    //fLib_EnableIRQ((UINT32)irq);
#else
    disable_interrupts();
    
    irq_set_type(irq, IRQ_TYPE_LEVEL_LOW);
    irq_install_handler(irq, handle_irq, 0);
    irq_set_enable(irq);
    
    enable_interrupts();
#endif
    return 0;
}

float Oscillator_Tolerance_Calc(CAN_Param *dev, float *df)
{
    int bit_time_N, bit_time_D, i;
    float min_df;
    
    /* bit_time_N is total number of tq in a nominal bit */
    bit_time_N = 1 + dev->NPROP + dev->NPS1 + dev->NPS2;
    
    /* Condition 1: Resynchronization(Arbitration Phase) */
    df[0] = (float)dev->NSJW / (20 * bit_time_N);
    
    /* Condition 2: Sampling Bit Succeeding own Error Flag(Aritration Phase) */
    df[1] = (float)min_t(dev->NPS1, dev->NPS2) / (2 * (13 * bit_time_N - dev->NPS2));
    
    /* If non-FD use, only calculate df1 and df2. Besides, the df3, df4 and df5 should be calculated for FD use. */
    if(dev->FD_mode)
    {
        /* bit_time_D is total number of tq in a data bit */
        bit_time_D = 1 + dev->DPROP + dev->DPS1 + dev->DPS2;
        
        /* Condition 3: Resynchronization(Data Phase) */
        df[2] = (float)dev->DSJW / (20 * bit_time_D);
        
        /* Condition 4: Sampling Bit Succeeding own Error Flag(Data Phase) */
        df[3] = (float)min_t(dev->DPS1, dev->DPS2) / \
                (2 * ((6 * bit_time_D - dev->DPS2) * \
                (dev->DPRE / dev->NPRE) + 7 * bit_time_N));
        
        /* Condition 5: Switching from Arbitration Phase to Data Phase */
        df[4] = (dev->DSJW - ((float)dev->DPRE / dev->NPRE - 1)) / \
                (2 * ((2 * bit_time_N - dev->NPS2) * (dev->NPRE / dev->DPRE) + \
                dev->DPS2 + 4 * bit_time_D));
    }
    else
    {
        df[3] = 0;
        df[4] = 0;
        df[5] = 0;
    }
    
    /* find the minimum in 5 dfs */
    min_df = df[0];
    for(i = 1; i < (dev->FD_mode ? 5 : 2); i++)
    {
        if(df[i] < min_df)
        {
            min_df = df[i];
        }
    }

    return min_df;
}

void Generate_Random_Pattern(UINT8 length, UINT8 *pData)
{
    while(length--)
    {
        pData[length] = (rand() % 0x100); 
    }
}

void Generate_Random_Pattern_Length(Frame_Type frameType, UINT8 *dlc, UINT8 *pData)
{
    int length;
    
    if(frameType < 4)
    {
        *dlc = rand() % 9; //random byte length if non-FD frame
    }
    else
    {
        *dlc = rand() % 16; //random byte length if FD frame
    }
    
    length = DLCtoBytes_table[*dlc]; //covert the DLC to real number of bytes
    
    memset(pData, 0, 64); //reset the 64-byte data field to 0
    
    while(length--)
    {
        pData[length] = rand() % 0x100; 
    }
}

void Generate_Random_Identifier(Frame_Type frameType, UINT32 *pSID, UINT32 *pEID)
{
    UINT32 temp;

    if(frameType == CAN_A_Data || frameType == CAN_A_Remote || \
         frameType == CAN_FD_A || frameType == CAN_FD_A_BRS) /* Standard Identifier (11-bit) */
    {
        while(1)
        {
            temp = rand() % 0x800;

            /* The 7 most significant bits (bit[10:4]) must not be all 1s */
            if((temp & 0x7F0) != 0x7F0)
                break;
        }
        *pSID = temp & 0x7FF;
        *pEID = 0;
    }
    else if(frameType == CAN_B_Data || frameType == CAN_B_Remote || \
            frameType == CAN_FD_B || frameType == CAN_FD_B_BRS) /* Standard Identifier (11-bit) + Extended Identifier (18-bit) */
    {
        while(1)
        {
            temp = rand() % 0x20000000;

            /* The 7 most significant bits (bit[28:22]) must not be all 1s */
            if((temp & 0x1FC00000) != 0x1FC00000)
                break;
        }
        *pSID = (temp & 0x1FFC0000) >> 18;
        *pEID = (temp & 0x3FFFF);
    }
}

UINT32 Verify_Pattern(Frame_Type frameType, CanTxMsgDef *pSource, CanRxMsgDef *pDest)
{
    UINT32 i;
    
    switch((int)frameType)
    {
        case CAN_A_Data:
            /* Check the standard identifier (IDE = 0) */
            if(pDest->IDE != 0)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_IDE_ERROR]);
                return CAN_IDE_ERROR;
            }
            if(pDest->StdId != pSource->StdId)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_STDID_ERROR]);
                return CAN_STDID_ERROR;
            }
            /* Check the remote bit (RTR)*/
            if(pDest->RTR != 0)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_RTR_ERROR]);
                return CAN_RTR_ERROR;
            }
            /* Check the 8-byte data pattern (DLC = CAN_DataBytes_8) */
            if(pDest->DLC != pSource->DLC)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_DLC_ERROR]);
                return CAN_DLC_ERROR;
            }
            for(i = 0; i < DLCtoBytes_table[pDest->DLC]; i++)
            {
                if(pDest->Data[i] != pSource->Data[i])
                {
                    CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nThe data %d is error", i);
                    return CAN_DATA_ERROR;
                }

            }
            break;

        case CAN_A_Remote:
            /* Check the standard identifier (IDE = 0) */
            if(pDest->IDE != 0)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_IDE_ERROR]);
                return CAN_IDE_ERROR;
            }
            if(pDest->StdId != pSource->StdId)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_STDID_ERROR]);
                return CAN_STDID_ERROR;
            }
            /* Check the remote bit (RTR)*/
            if(pDest->RTR != 1)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_RTR_ERROR]);
                return CAN_RTR_ERROR;
            }
            /* Check the 8-byte data pattern (DLC = CAN_DataBytes_8) */
            if(pDest->DLC != CAN_DataBytes_0)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_DLC_ERROR]);
                return CAN_DLC_ERROR;
            }
            break;

        case CAN_B_Data:
            /* Check the standard and extended identifier (IDE = 1) */
            if(pDest->IDE != 1)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_IDE_ERROR]);
                return CAN_IDE_ERROR;
            }
            if(pDest->StdId != pSource->StdId)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_STDID_ERROR]);
                return CAN_STDID_ERROR;
            }
            if(pDest->ExtId != pSource->ExtId)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_EXTID_ERROR]);
                return CAN_EXTID_ERROR;
            }
            /* Check the remote bit (RTR)*/
            if(pDest->RTR != 0)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_RTR_ERROR]);
                return CAN_RTR_ERROR;
            }
            /* Check the 8-byte data pattern (DLC = CAN_DataBytes_8) */
            if(pDest->DLC != pSource->DLC)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_DLC_ERROR]);
                return CAN_DLC_ERROR;
            }
            for(i = 0; i < DLCtoBytes_table[pDest->DLC]; i++)
            {
                if(pDest->Data[i] != pSource->Data[i])
                {
                    //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_DATA_ERROR]);
                    return CAN_DATA_ERROR;
                }
            }
            break;

        case CAN_B_Remote:
            /* Check the standard and extended identifier (IDE = 1) */
            if(pDest->IDE != 1)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_IDE_ERROR]);
                return CAN_IDE_ERROR;
            }
            if(pDest->StdId != pSource->StdId)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_STDID_ERROR]);
                return CAN_STDID_ERROR;
            }
            if(pDest->ExtId != pSource->ExtId)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_EXTID_ERROR]);
                return CAN_EXTID_ERROR;
            }
            /* Check the remote bit (RTR)*/
            if(pDest->RTR != 1)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_RTR_ERROR]);
                return CAN_RTR_ERROR;
            }
            /* Check the 8-byte data pattern (DLC = CAN_DataBytes_8) */
            if(pDest->DLC != CAN_DataBytes_0)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_DLC_ERROR]);
                return CAN_DLC_ERROR;
            }
            break;

        case CAN_FD_A:
        case CAN_FD_A_BRS:
            /* Check the standard identifier (IDE = 0) */
            if(pDest->IDE != 0)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_IDE_ERROR]);
                return CAN_IDE_ERROR;
            }
            if(pDest->StdId != pSource->StdId)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_STDID_ERROR]);
                return CAN_STDID_ERROR;
            }

            /* Check the 64-byte data pattern (DLC = CAN_DataBytes_64) */
            if(pDest->DLC != pSource->DLC)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_DLC_ERROR]);
                return CAN_DLC_ERROR;
            }

            for(i = 0; i < DLCtoBytes_table[pDest->DLC]; i++)
            {
                if(pDest->Data[i] != pSource->Data[i])
                {
                    //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_DATA_ERROR]);
                    return CAN_DATA_ERROR;
                }
            }

            /* Check the others bit (RTR, FD, ESI, BRS)*/
            if(pDest->RTR != 0)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_RTR_ERROR]);
                return CAN_RTR_ERROR;
            }
            if(pDest->FD != 1)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_FD_ERROR]);
                return CAN_FD_ERROR;
            }
            if(pDest->ESI == 1)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_ESI_BIT]);
                return CAN_ESI_BIT;
            }
            if(pDest->BRS != pSource->BRS)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_BRS_ERROR]);
                return CAN_BRS_ERROR;
            }
            break;

        case CAN_FD_B:
        case CAN_FD_B_BRS:
            /* Check the standard and extended identifier (IDE = 1) */
            if(pDest->IDE != 1)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_IDE_ERROR]);
                return CAN_IDE_ERROR;
            }
            if(pDest->StdId != pSource->StdId)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_STDID_ERROR]);
                return CAN_STDID_ERROR;
            }
            if(pDest->ExtId != pSource->ExtId)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_EXTID_ERROR]);
                return CAN_EXTID_ERROR;
            }

            /* Check the 64-byte data pattern (DLC = CAN_DataBytes_64) */
            if(pDest->DLC != pSource->DLC)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_DLC_ERROR]);
                return CAN_DLC_ERROR;
            }
            for(i = 0; i < DLCtoBytes_table[pDest->DLC]; i++)
            {
                if(pDest->Data[i] != pSource->Data[i])
                {
                    //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_DATA_ERROR]);
                    return CAN_DATA_ERROR;
                }
            }

            /* Check the others bit (RTR, FD, ESI, BRS)*/
            if(pDest->RTR != 0)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_RTR_ERROR]);
                return CAN_RTR_ERROR;
            }
            if(pDest->FD != 1)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_FD_ERROR]);
                return CAN_FD_ERROR;
            }
            if(pDest->ESI == 1)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_ESI_BIT]);
                return CAN_ESI_BIT;
            }
            if(pDest->BRS != pSource->BRS)
            {
                //CAN_DBGPRINTF(DBG_LEVEL_0, "%s", err_msg[CAN_BRS_ERROR]);
                return CAN_BRS_ERROR;
            }
            break;
    }

    return 0;
}

void Generate_Frame(Frame_Type frameType, CanTxMsgDef *pSource)
{
    switch((int)frameType)
    {
        case CAN_A_Data:
            Generate_Random_Identifier(CAN_A_Data, &pSource->StdId, &pSource->ExtId);
            pSource->IDE = 0;
            pSource->BRS = 0;
            pSource->FD = 0;
            pSource->RTR = 0;
            Generate_Random_Pattern_Length(CAN_A_Data, &pSource->DLC, pSource->Data);
            break;
        case CAN_A_Remote:
            Generate_Random_Identifier(CAN_A_Remote, &pSource->StdId, &pSource->ExtId);
            pSource->IDE = 0;
            pSource->BRS = 0;
            pSource->FD = 0;
            pSource->RTR = 1;
            pSource->DLC = CAN_DataBytes_0;
            break;
        case CAN_B_Data:
            Generate_Random_Identifier(CAN_B_Data, &pSource->StdId, &pSource->ExtId);
            pSource->IDE = 1;
            pSource->BRS = 0;
            pSource->FD = 0;
            pSource->RTR = 0;
            Generate_Random_Pattern_Length(CAN_B_Data, &pSource->DLC, pSource->Data);
            break;
        case CAN_B_Remote:
            Generate_Random_Identifier(CAN_B_Remote, &pSource->StdId, &pSource->ExtId);
            pSource->IDE = 1;
            pSource->BRS = 0;
            pSource->FD = 0;
            pSource->RTR = 1;
            pSource->DLC = CAN_DataBytes_0;
            break;
        case CAN_FD_A:
            Generate_Random_Identifier(CAN_FD_A, &pSource->StdId, &pSource->ExtId);
            pSource->IDE = 0;
            pSource->BRS = 0;
            pSource->FD = 1;
            pSource->RTR = 0;
            Generate_Random_Pattern_Length(CAN_FD_A, &pSource->DLC, pSource->Data);
            break;
        case CAN_FD_B:
            Generate_Random_Identifier(CAN_B_Data, &pSource->StdId, &pSource->ExtId);
            pSource->IDE = 1;
            pSource->BRS = 0;
            pSource->FD = 1;
            pSource->RTR = 0;
            Generate_Random_Pattern_Length(CAN_B_Data, &pSource->DLC, pSource->Data);
            break;
        case CAN_FD_A_BRS:
            Generate_Random_Identifier(CAN_FD_A, &pSource->StdId, &pSource->ExtId);
            pSource->IDE = 0;
            pSource->BRS = 1;
            pSource->FD = 1;
            pSource->RTR = 0;
            Generate_Random_Pattern_Length(CAN_FD_A, &pSource->DLC, pSource->Data);
            break;
        case CAN_FD_B_BRS:
            Generate_Random_Identifier(CAN_B_Data, &pSource->StdId, &pSource->ExtId);
            pSource->IDE = 1;
            pSource->BRS = 1;
            pSource->FD = 1;
            pSource->RTR = 0;
            Generate_Random_Pattern_Length(CAN_B_Data, &pSource->DLC, pSource->Data);
            break;
    }
}

/*******************************************************************************
                        CAN Interrupt Handler functions
*******************************************************************************/
void CAN1Handler_WIE(void * data)
{
    UINT8 IR;
    
    /* Get the IR register status bits */
    IR = CAN_GetIRStatus(CAN1);
    
    /* 檢查是否Wake-up (bit[7]:WIR) */
    if(IR & CAN_IR_WIR)
    {
        /* check OMR is normal mode */
        if(CAN_GetMode(CAN1) != CAN_Mode_Normal)
            while(1);
    
        CAN_ClearIT(CAN1, CAN_WIR_STATUS_BIT);
    }
    
    /* clear WIR bit */
    //CAN_ClearIT(CAN1, CAN_WIR_STATUS_BIT);
    
    ITFLAG = 0;
}

void CAN1Handler(void * data)
{
    UINT8 IR, ET, FIFOn;
    
    /* Get the IR register status bits */
    IR = CAN_GetIRStatus(CAN1);
    
    /* Check Error bit (bit[5]:EIR)*/
    if(IR & CAN_IR_EIR)
    {
        CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nCAN1 %s",err_msg[CAN_ERROR_STATUS]);
        
        /* Get error type and counter*/
        ET = CAN_GetLastErrorCode(CAN1);
        
        gREC = CAN_GetReceiveErrorCounter(CAN1);
        gTEC = CAN_GetTransmitErrorCounter(CAN1);
        CAN_DBGPRINTF(DBG_LEVEL_0, "\rCAN1 %s", errorType_msg[highest_bit_position(ET)]);
        CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nCAN1 TEC: %d", gTEC);
        CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nCAN1 REC: %d\n", gREC);
        
        /* RW1C this status bit */
        CAN_ClearIT(CAN1, CAN_EIR_STATUS_BIT);
        
        ETFLAG = 1; //frame type test use
    }
    
    /* Check Overrun (bit[6]:OIR) */
    if(IR & CAN_IR_OIR)
    {
        CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nCAN1 %s", err_msg[CAN_OVERRUN_STATUS]);
        
        /* Read DO field to know which FIFO overrun happened */
        FIFOn = CAN_GetStatusBit(CAN1, CAN_DO_STATUS_BIT);
        switch(FIFOn)
        {
            case 0:
                CAN_DBGPRINTF(DBG_LEVEL_1, "\r\nNo FIFO occurs overrun.\n");
                break;
            case 1:
                CAN_DBGPRINTF(DBG_LEVEL_1, "\r\nFIFO 0 occurs overrun.\n");
                break;
            case 2:
                CAN_DBGPRINTF(DBG_LEVEL_1, "\r\nFIFO 1 occurs overrun.\n");
                break;
            case 3:
                CAN_DBGPRINTF(DBG_LEVEL_1, "\r\nBoth FIFOs occur overrun.\n");
                break; 
        }
        
        /* RW1C this status bit */
        CAN_ClearIT(CAN1, CAN_OIR_STATUS_BIT);
    }
    
    /* Check Wake-up (bit[7]:WIR) */
    if(IR & CAN_IR_WIR)
    {
        CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nCAN1 %s",err_msg[CAN_WAKEUP_STATUS]);
        
        /* RW1C this status bit */
        CAN_ClearIT(CAN1, CAN_WIR_STATUS_BIT);
    }
    
    /* Check transmit bits TBIn */
    if(IR & CAN_IR_TB)
    {
        if(IR & CAN_IR_TBI0)
        {
            CAN_DBGPRINTF(DBG_LEVEL_3, "\r\nCAN1 Tx buffer 0 is successful.\n", NULL);
            /* RW1C this status bit */
            CAN_ClearIT(CAN1, CAN_TBI0_STATUS_BIT);
        }
        if(IR & CAN_IR_TBI1)
        {
            CAN_DBGPRINTF(DBG_LEVEL_3, "\r\nCAN1 Tx buffer 1 is successful.\n", NULL);
            /* RW1C this status bit */
            CAN_ClearIT(CAN1, CAN_TBI1_STATUS_BIT);
        }
        if(IR & CAN_IR_TBI2)
        {
            CAN_DBGPRINTF(DBG_LEVEL_3, "\r\nCAN1 Tx buffer 2 is successful.\n", NULL);
            /* RW1C this status bit */
            CAN_ClearIT(CAN1, CAN_TBI2_STATUS_BIT);
        }
        
        /* Flag for some functions */
        ITFLAG++;
    }
    
    /* check receive bits RBIn */
    if(IR & CAN_IR_RB)
    {
        if(IR & CAN_IR_RBI0)
        {
            CAN_DBGPRINTF(DBG_LEVEL_3, "\r\nCAN1 Rx FIFO 0 is successful.\n", NULL);
            /* RW1C this status bit */
            CAN_ClearIT(CAN1, CAN_RBI0_STATUS_BIT);
        }
        if(IR & CAN_IR_RBI1)
        {
            CAN_DBGPRINTF(DBG_LEVEL_3, "\r\nCAN1 Rx FIFO 1 is successful.\n", NULL);
            /* RW1C this status bit */
            CAN_ClearIT(CAN1, CAN_RBI1_STATUS_BIT);
        }
        
        /* Flag for some functions */
        ITFLAG++;
    }
}

void CAN2Handler(void * data)
{
    UINT8 IR, ET, FIFOn;
    
    /* Get the IR register status bits */
    IR = CAN_GetIRStatus(CAN2);
    
    /* Check Error bit (bit[5]:EIR)*/
    if(IR & CAN_IR_EIR)
    {
        CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nCAN2 %s",err_msg[CAN_ERROR_STATUS]);
        
        /* Get error type and counter*/
        ET = CAN_GetLastErrorCode(CAN2);
        
        CAN_DBGPRINTF(DBG_LEVEL_0, "\rCAN2 %s", errorType_msg[highest_bit_position(ET)]);
        CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nCAN2 TEC: %d", CAN_GetTransmitErrorCounter(CAN2));
        CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nCAN2 REC: %d\n", CAN_GetReceiveErrorCounter(CAN2));
        
        /* RW1C this status bit */
        CAN_ClearIT(CAN2, CAN_EIR_STATUS_BIT);
        
        ETFLAG = 1; //frame type test use
    }
    
    /* Check Overrun (bit[6]:OIR) */
    if(IR & CAN_IR_OIR)
    {
        CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nCAN2 %s", err_msg[CAN_OVERRUN_STATUS]);
        
        /* Read DO field to know which FIFO overrun happened */
        FIFOn = CAN_GetStatusBit(CAN2, CAN_DO_STATUS_BIT);
        switch(FIFOn)
        {
            case 0:
                CAN_DBGPRINTF(DBG_LEVEL_1, "\r\nNo FIFO occurs overrun.\n");
                break;
            case 1:
                CAN_DBGPRINTF(DBG_LEVEL_1, "\r\nFIFO 0 occurs overrun.\n");
                break;
            case 2:
                CAN_DBGPRINTF(DBG_LEVEL_1, "\r\nFIFO 1 occurs overrun.\n");
                break;
            case 3:
                CAN_DBGPRINTF(DBG_LEVEL_1, "\r\nBoth FIFOs occur overrun.\n");
                break; 
        }
        
        /* RW1C this status bit */
        CAN_ClearIT(CAN2, CAN_OIR_STATUS_BIT);
    }
    
    /* Check Wake-up (bit[7]:WIR) */
    if(IR & CAN_IR_WIR)
    {
        CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nCAN2 %s",err_msg[CAN_WAKEUP_STATUS]);
        
        /* RW1C this status bit */
        CAN_ClearIT(CAN2, CAN_WIR_STATUS_BIT);
    }
    
    /* Check transmit bits TBIn */
    if(IR & CAN_IR_TB)
    {
        if(IR & CAN_IR_TBI0)
        {
            CAN_DBGPRINTF(DBG_LEVEL_3, "\r\nCAN2 Tx buffer 0 is successful.\n", NULL);
            /* RW1C this status bit */
            CAN_ClearIT(CAN2, CAN_TBI0_STATUS_BIT);
        }
        if(IR & CAN_IR_TBI1)
        {
            CAN_DBGPRINTF(DBG_LEVEL_3, "\r\nCAN2 Tx buffer 1 is successful.\n", NULL);
            /* RW1C this status bit */
            CAN_ClearIT(CAN2, CAN_TBI1_STATUS_BIT);
        }
        if(IR & CAN_IR_TBI2)
        {
            CAN_DBGPRINTF(DBG_LEVEL_3, "\r\nCAN2 Tx buffer 2 is successful.\n", NULL);
            /* RW1C this status bit */
            CAN_ClearIT(CAN2, CAN_TBI2_STATUS_BIT);
        }
    }
    
    /* Check receive bits RBIn */
    if(IR & CAN_IR_RB)
    {
        if(IR & CAN_IR_RBI0)
        {
            CAN_DBGPRINTF(DBG_LEVEL_3, "\r\nCAN2 Rx FIFO 0 is successful.\n", NULL);
            /* RW1C this status bit */
            CAN_ClearIT(CAN2, CAN_RBI0_STATUS_BIT);
        }
        if(IR & CAN_IR_RBI1)
        {
            CAN_DBGPRINTF(DBG_LEVEL_3, "\r\nCAN2 Rx FIFO 1 is successful.\n", NULL);
            /* RW1C this status bit */
            CAN_ClearIT(CAN2, CAN_RBI1_STATUS_BIT);
        }
        
        /* Flag for some functions */
        ITFLAG++;
    }
}

#ifdef CAN3_FTCAN
void CAN3Handler(void * data)
{
    UINT8 IR, ET, FIFOn;
    
    /* Get the IR register status bits */
    IR = CAN_GetIRStatus(CAN3);
    
    /* Check Error bit (bit[5]:EIR)*/
    if(IR & CAN_IR_EIR)
    {
        CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nCAN3 %s",err_msg[CAN_ERROR_STATUS]);
        
        /* Get error type and counter*/
        ET = CAN_GetLastErrorCode(CAN3);
        
        CAN_DBGPRINTF(DBG_LEVEL_0, "\rCAN3 %s", errorType_msg[highest_bit_position(ET)]);
        CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nCAN3 TEC: %d", CAN_GetTransmitErrorCounter(CAN3));
        CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nCAN3 REC: %d", CAN_GetReceiveErrorCounter(CAN3));
        
        /* RW1C this status bit */
        CAN_ClearIT(CAN3, CAN_EIR_STATUS_BIT);
    }
    
    /* Check Overrun (bit[6]:OIR) */
    if(IR & CAN_IR_OIR)
    {
        CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nCAN3 %s", err_msg[CAN_OVERRUN_STATUS]);
        
        /* Read DO field to know which FIFO overrun happened */
        FIFOn = CAN_GetStatusBit(CAN3, CAN_DO_STATUS_BIT);
        switch(FIFOn)
        {
            case 0:
                CAN_DBGPRINTF(DBG_LEVEL_1, "\r\nNo FIFO occurs overrun.\n");
                break;
            case 1:
                CAN_DBGPRINTF(DBG_LEVEL_1, "\r\nFIFO 0 occurs overrun.\n");
                break;
            case 2:
                CAN_DBGPRINTF(DBG_LEVEL_1, "\r\nFIFO 1 occurs overrun.\n");
                break;
            case 3:
                CAN_DBGPRINTF(DBG_LEVEL_1, "\r\nBoth FIFOs occur overrun.\n");
                break; 
        }
        
        /* RW1C this status bit */
        CAN_ClearIT(CAN3, CAN_OIR_STATUS_BIT);
    }
    
    /* Check Wake-up (bit[7]:WIR) */
    if(IR & CAN_IR_WIR)
    {
        CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nCAN3 %s",err_msg[CAN_WAKEUP_STATUS]);
        
        /* RW1C this status bit */
        CAN_ClearIT(CAN3, CAN_WIR_STATUS_BIT);
    }
    
    /* Check transmit bits TBIn */
    if(IR & CAN_IR_TB)
    {
        if(IR & CAN_IR_TBI0)
        {
            CAN_DBGPRINTF(DBG_LEVEL_3, "\r\nCAN3 Tx buffer 0 is successful.\n", NULL);
            /* RW1C this status bit */
            CAN_ClearIT(CAN3, CAN_TBI0_STATUS_BIT);
        }
        if(IR & CAN_IR_TBI1)
        {
            CAN_DBGPRINTF(DBG_LEVEL_3, "\r\nCAN3 Tx buffer 1 is successful.\n", NULL);
            /* RW1C this status bit */
            CAN_ClearIT(CAN3, CAN_TBI1_STATUS_BIT);
        }
        if(IR & CAN_IR_TBI2)
        {
            CAN_DBGPRINTF(DBG_LEVEL_3, "\r\nCAN3 Tx buffer 2 is successful.\n", NULL);
            /* RW1C this status bit */
            CAN_ClearIT(CAN3, CAN_TBI2_STATUS_BIT);
        }
    }
    
    /* Check receive bits RBIn */
    if(IR & CAN_IR_RB)
    {
        if(IR & CAN_IR_RBI0)
        {
            CAN_DBGPRINTF(DBG_LEVEL_3, "\r\nCAN3 Rx FIFO 0 is successful.\n", NULL);
            /* RW1C this status bit */
            CAN_ClearIT(CAN3, CAN_RBI0_STATUS_BIT);
        }
        if(IR & CAN_IR_RBI1)
        {
            CAN_DBGPRINTF(DBG_LEVEL_3, "\r\nCAN3 Rx FIFO 1 is successful.\n", NULL);
            /* RW1C this status bit */
            CAN_ClearIT(CAN3, CAN_RBI1_STATUS_BIT);
        }
    }
}
#endif

void CAN1Handler_ALtest(void * data)
{
    UINT8 IR, ET, TBIn;
    
    /* Get the IR register status bits */
    IR = CAN_GetIRStatus(CAN1);
    
    if(IR & CAN_IR_EIR)
    {
        CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nCAN1 %s",err_msg[CAN_ERROR_STATUS]);
        
        ET = CAN_GetLastErrorCode(CAN1);
        
        CAN_DBGPRINTF(DBG_LEVEL_0, "\rCAN1 %s", errorType_msg[highest_bit_position(ET)]);
        CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nCAN1 TEC: %d", CAN_GetTransmitErrorCounter(CAN1));
        CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nCAN1 REC: %d", CAN_GetReceiveErrorCounter(CAN1));
        
        /* RW1C this status bit */
        CAN_ClearIT(CAN1, CAN_EIR_STATUS_BIT);
    }
    
    if(IR & CAN_IR_TB)
    {
        TBCnt++;
        if(IR & CAN_IR_TBI0)
        {
            /* RW1C this status bit */
            CAN_ClearIT(CAN1, CAN_TBI0_STATUS_BIT);

            CAN1->CAN_Control.All |= CAN_CR_BTR0_W;
        }
        if(IR & CAN_IR_TBI1)
        {
            /* RW1C this status bit */
            CAN_ClearIT(CAN1, CAN_TBI1_STATUS_BIT);

            CAN1->CAN_Control.All |= CAN_CR_BTR1_W;
        }
        if(IR & CAN_IR_TBI2)
        {
            /* RW1C this status bit */
            CAN_ClearIT(CAN1, CAN_TBI2_STATUS_BIT);

            CAN1->CAN_Control.All |= CAN_CR_BTR2_W;
        }

#ifdef APB_USE       
        /* See BTLAn to check Arbitration Loss was happened or not */
        if(CAN1->CAN_Status.All & 0x1C0000)
        {
            CAN1ALCnt++;
        }
        
        if(CAN2->CAN_Status.All & 0x1C0000)
        {
            //CAN_DBGPRINTF(DBG_LEVEL_3, "\r\nCAN2 AL.\n", NULL);
            CAN2ALCnt++;
        }
#endif
    }
}

void CAN2Handler_ALtest(void * data)
{
    UINT8 IR, ET, TBIn;
    
    /* Get the IR register status bits */
    IR = CAN_GetIRStatus(CAN2);
    
    if(IR & CAN_IR_EIR)
    {
        CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nCAN2 %s",err_msg[CAN_ERROR_STATUS]);
        
        ET = CAN_GetLastErrorCode(CAN2);
        
        CAN_DBGPRINTF(DBG_LEVEL_0, "\rCAN2 %s", errorType_msg[highest_bit_position(ET)]);
        CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nCAN2 TEC: %d", CAN_GetTransmitErrorCounter(CAN2));
        CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nCAN2 REC: %d", CAN_GetReceiveErrorCounter(CAN2));
        
        /* RW1C this status bit */
        CAN_ClearIT(CAN2, CAN_EIR_STATUS_BIT);
    }
    
    if(IR & CAN_IR_TB)
    {
        TBCnt++;
        if(IR & CAN_IR_TBI0)
        {
            /* RW1C this status bit */
            CAN_ClearIT(CAN2, CAN_TBI0_STATUS_BIT);

            CAN2->CAN_Control.All |= CAN_CR_BTR0_W;
        }
        if(IR & CAN_IR_TBI1)
        {
            /* RW1C this status bit */
            CAN_ClearIT(CAN2, CAN_TBI1_STATUS_BIT);

            CAN2->CAN_Control.All |= CAN_CR_BTR1_W;
        }
        if(IR & CAN_IR_TBI2)
        {
            /* RW1C this status bit */
            CAN_ClearIT(CAN2, CAN_TBI2_STATUS_BIT);

            CAN2->CAN_Control.All |= CAN_CR_BTR2_W;
        }
    }
    
    /* See BTLAn to check Arbitration Loss was happened or not */
    if(CAN1->CAN_Status.All & 0x1C0000)
    {
        CAN1ALCnt++;
    }
    
    if(CAN2->CAN_Status.All & 0x1C0000)
    {
        //CAN_DBGPRINTF(DBG_LEVEL_3, "\r\nCAN2 AL.\n", NULL);
        CAN2ALCnt++;
    }
    
    
}

void CAN1Handler_RMtest(void * data)
{
    UINT8 IR, ET, TBIn;
    
    IR = CAN_GetIRStatus(CAN1);
    
    if(IR & CAN_IR_EIR)
    {
        CAN_DBGPRINTF(DBG_LEVEL_0, "\r\n\tCAN1 %s",err_msg[CAN_ERROR_STATUS]);
        
        ET = CAN_GetLastErrorCode(CAN1);
        
        CAN_DBGPRINTF(DBG_LEVEL_0, "\tCAN1 %s\n", errorType_msg[highest_bit_position(ET)]);
        CAN_DBGPRINTF(DBG_LEVEL_0, "\tCAN1 TEC: %d\n", CAN_GetTransmitErrorCounter(CAN1));
        CAN_DBGPRINTF(DBG_LEVEL_0, "\tCAN1 REC: %d\n", CAN_GetReceiveErrorCounter(CAN1));
        
        /* RW1C this status bit */
        CAN_ClearIT(CAN1, CAN_EIR_STATUS_BIT);
    }
    
    if(IR & CAN_IR_WIR)
    {
        CAN_DBGPRINTF(DBG_LEVEL_0, "\r\n\tCAN1 %s\n",err_msg[CAN_WAKEUP_STATUS]);
        
        /* RW1C this status bit */
        CAN_ClearIT(CAN1, CAN_WIR_STATUS_BIT);
        
        CAN_ITConfig(CAN1, CAN_IRE_WIE, DISABLE);
        
        ITFLAG = 0;
    }
    
    if(IR & CAN_IR_OIR)
    {
        CAN_DBGPRINTF(DBG_LEVEL_0, "\r\n\tCAN1 %s\n", err_msg[CAN_OVERRUN_STATUS]);
        
        /* Read DO field to know which FIFO overrun happened */
        // FIFOn = CAN_GetStatusBit(CAN1, CAN_DO_STATUS_BIT);
        // switch(FIFOn)
        // {
            // case 0:
                // CAN_DBGPRINTF(DBG_LEVEL_1, "\r\nNo FIFO occurs overrun.\n");
                // break;
            // case 1:
                // CAN_DBGPRINTF(DBG_LEVEL_1, "\r\nFIFO 0 occurs overrun.\n");
                // break;
            // case 2:
                // CAN_DBGPRINTF(DBG_LEVEL_1, "\r\nFIFO 1 occurs overrun.\n");
                // break;
            // case 3:
                // CAN_DBGPRINTF(DBG_LEVEL_1, "\r\nBoth FIFOs occur overrun.\n");
                // break; 
        // }
        
        /* RW1C this status bit */
        CAN_ClearIT(CAN1, CAN_OIR_STATUS_BIT);
        
        /* Release all */
        CAN_FIFORelease_All(CAN1, CAN_RxFIFO_0);
    }
    
    if(IR & CAN_IR_RB)
    {
        if(IR & CAN_IR_RBI0)
        {
#ifdef APB_USE            
            /* no receive this frame, only pop FIFO after receiving. */
            CAN_FIFORelease_RXn(CAN1, CAN_RxFIFO_0);
            
            /* Data increment */
            //CAN1->CAN_TD[0].Data[0].All++;
#endif
            /* RW1C this status bit */
            CAN_ClearIT(CAN1, CAN_RBI0_STATUS_BIT);
        }
        if(IR & CAN_IR_RBI1)
        {
            /* RW1C this status bit */
            CAN_ClearIT(CAN1, CAN_RBI1_STATUS_BIT);
        }
    }
}

void CAN2Handler_RMtest(void * data)
{
    UINT8 IR, ET, TBIn;
    int i = 0;

    /* Get the IR register status bits */
    IR = CAN_GetIRStatus(CAN2);
    
    if(IR & CAN_IR_EIR)
    {
        CAN_DBGPRINTF(DBG_LEVEL_0, "\r\n\tCAN2 %s",err_msg[CAN_ERROR_STATUS]);
        
        ET = CAN_GetLastErrorCode(CAN2);
        
        CAN_DBGPRINTF(DBG_LEVEL_0, "\tCAN2 %s\n", errorType_msg[highest_bit_position(ET)]);
        CAN_DBGPRINTF(DBG_LEVEL_0, "\tCAN2 TEC: %d\n", CAN_GetTransmitErrorCounter(CAN2));
        CAN_DBGPRINTF(DBG_LEVEL_0, "\tCAN2 REC: %d\n", CAN_GetReceiveErrorCounter(CAN2));
        
        /* RW1C this status bit */
        CAN_ClearIT(CAN2, CAN_EIR_STATUS_BIT);
    }
    
    if(IR & CAN_IR_TB)
    {
        // if(ITFLAG == 1)
        // {
            // /* CAN1進sleep */
            // CAN_Mode_Change(CAN1, CAN_Mode_Sleep);
            
            // ITFLAG = 0;
        // }
        
        if(IR & CAN_IR_TBI0)
        {
            TBI0Cnt++;
            
            /* RW1C this status bit */
            CAN_ClearIT(CAN2, CAN_TBI0_STATUS_BIT);           
            //if(TBI0Cnt < 2)
            CAN2->CAN_Control.All |= CAN_CR_BTR0_W;
            
        }
        if(IR & CAN_IR_TBI1)
        {
            TBI1Cnt++;
            
            /* RW1C this status bit */
            CAN_ClearIT(CAN2, CAN_TBI1_STATUS_BIT);            
            //if(TBI1Cnt < 2)
            CAN2->CAN_Control.All |= CAN_CR_BTR1_W;
        }
        if(IR & CAN_IR_TBI2)
        {
            TBI2Cnt++;
            
            /* RW1C this status bit */
            CAN_ClearIT(CAN2, CAN_TBI2_STATUS_BIT);            
            //if(TBI2Cnt < 2)
            CAN2->CAN_Control.All |= CAN_CR_BTR2_W;
        }
        
        
        /* Workaround: 等三個tx buffer都送完在一次三個request
           三個buffer大概會78% bus load
           兩個buffer大概會69% bus load
           一個buffer大概會53% bus load*/
        // if(IR & CAN_IR_TBI0)
        // {
            // CAN_ClearIT(CAN2, CAN_TBI0_STATUS_BIT);
            // TBCnt++;
        // }
        // if(IR & CAN_IR_TBI1)
        // {
            // CAN_ClearIT(CAN2, CAN_TBI1_STATUS_BIT);
            // TBCnt++;
        // }
        // if(IR & CAN_IR_TBI2)
        // {
            // CAN_ClearIT(CAN2, CAN_TBI2_STATUS_BIT);
            // TBCnt++;
        // }
        // if(TBCnt == 3)
        // {
            // CAN2->CAN_Control.All |= 0x38; //確定三筆都發出去在一次request三個tx buffer
            // TBCnt = 0; //reset TB counter
        // }
        
        
        /* See BTLAn to check the Arbitration Loss was happened or not */
        // if(CAN2->CAN_Status.All & 0x1C0000)
        // {
            // //CAN_DBGPRINTF(DBG_LEVEL_3, "\r\nCAN2 AL.\n", NULL);
            // CAN2ALCnt++;
        // }
    }
    
}

void CAN1_Simple_Tx_Handler(void * data)
{
    UINT8 IR;

    /* Get the IR register status bits */
    IR = CAN_GetIRStatus(CAN1);

    if(IR & CAN_IR_TB)
    {
        if(IR & CAN_IR_TBI0)
        {
            CAN_DBGPRINTF(DBG_LEVEL_3, "\r\nCAN1 Tx buffer 0 is successful.\n", NULL);
            /* RW1C this status bit */
            CAN_ClearIT(CAN1, CAN_TBI0_STATUS_BIT);
        }
        if(IR & CAN_IR_TBI1)
        {
            CAN_DBGPRINTF(DBG_LEVEL_3, "\r\nCAN1 Tx buffer 1 is successful.\n", NULL);
            /* RW1C this status bit */
            CAN_ClearIT(CAN1, CAN_TBI1_STATUS_BIT);
        }
        if(IR & CAN_IR_TBI2)
        {
            CAN_DBGPRINTF(DBG_LEVEL_3, "\r\nCAN1 Tx buffer 2 is successful.\n", NULL);
            /* RW1C this status bit */
            CAN_ClearIT(CAN1, CAN_TBI2_STATUS_BIT);
        }
    }
}

void CAN2_Simple_Rx_Handler(void * data)
{
    UINT8 IR;

    /* Get the IR register status bits */
    IR = CAN_GetIRStatus(CAN1);

    if(IR & CAN_IR_RB)
    {
        if(IR & CAN_IR_RBI0)
        {
            CAN_DBGPRINTF(DBG_LEVEL_3, "\r\nCAN2 Rx FIFO 0 is successful.\n", NULL);
            /* RW1C this status bit */
            CAN_ClearIT(CAN2, CAN_RBI0_STATUS_BIT);
        }
        if(IR & CAN_IR_RBI1)
        {
            CAN_DBGPRINTF(DBG_LEVEL_3, "\r\nCAN2 Rx FIFO 1 is successful.\n", NULL);
            /* RW1C this status bit */
            CAN_ClearIT(CAN2, CAN_RBI1_STATUS_BIT);
        }
    }
}

/*******************************************************************************
                                Regitser test functions
*******************************************************************************/
/*------------------------------------------------------------------------------
    1. reset CAN
    2. Initial the information of APB register space
    3. Read every register by 4-byte access 
    4. Compare each 8-bit register with step 2 context
------------------------------------------------------------------------------*/
#ifdef APB_REG_TEST
int CAN_Register_default_value_test()
{
    UINT32 i, j, temp_reg, reg_idx;
    UINT8 temp_reg_b;

    CAN_Reset(CAN1);

    /* Initial the APB registers information */
    APB_Reg_Init();

    reg_idx = 0; //Index of 8-bit register

    for(i = 0; i <= Max_Reg_Num; i+=4)
    {
        /* Read the registers in 4 byte align */
        temp_reg = *(volatile UINT32*)(CAN1_BASE + i);

        /* Read each 8-bit in a 32-bit register */
        for(j = 0; j < 32; j+=8)
        {
            if(APB_Wrapper_Reg[reg_idx].Default_Value != ((temp_reg >> j) & 0xFF))
            {
                CAN_DBGPRINTF(DBG_LEVEL_0, "%s",err_msg[CAN_Reg_Default_ERROR]);
                CAN_DBGPRINTF(DBG_LEVEL_1, "Regster offset: 0x%04X. Default value: %#X. Current value: %#X.\n" \
                           , reg_idx, APB_Wrapper_Reg[reg_idx].Default_Value, (temp_reg >> j) & 0xFF);
            }
            reg_idx++;
        }
    }
    return 0;
}

int CAN_Register_Attribute_test()
{
    UINT32 i, j, k, temp_reg, reg_idx;
    UINT8 reg_ori_b, reg_test_b;

    /* Initial the APB registers information */
    APB_Reg_Init();

    reg_idx = 0; //Index of 8-bit register

    for(i = 0; i <= Max_Reg_Num; i+=4)
    {
        /* Read the registers in 4 byte align */
        temp_reg = *(volatile unsigned int *)(CAN1_BASE + i);
        
        /* Read each byte in a 32-bit register */
        for(j = 0; j < 32; j+=8)
        {
            /* Verify bit attribute in each bit */
            for(k = 0; k < 8; k++)
            {
                switch(APB_Wrapper_Reg[reg_idx].Bit_Type[k])
                {
                    case Bit_Type_RO:
                        /* Case by 0 and 1 for this bit */
                        if(temp_reg & ((0x1 << k) << j))
                        {
                            /* Try to clear this bit */
                            *(volatile unsigned int *)(CAN1_BASE + i) &= ~((0x1 << k) << j);
                        
                            /* Verify this bit, it should not be writable */
                            if(temp_reg != *(volatile unsigned int *)(CAN1_BASE + i))
                            {
                                CAN_DBGPRINTF(DBG_LEVEL_0, "%s",err_msg[CAN_REG_RO_ERROR]);
                                CAN_DBGPRINTF(DBG_LEVEL_0, "Regster offset: 0x%04X. Bit[%d] RO Error.\n", reg_idx & 0xFFFFFFFC, k+j);
                            }
                        }
                        else
                        {
                            /* Try to write this bit */
                            *(volatile unsigned int *)(CAN1_BASE + i) |= (0x1 << k) << j;
                            
                            /* Verify this bit, it should not be writable */
                            if(temp_reg != *(volatile unsigned int *)(CAN1_BASE + i))
                            {
                                CAN_DBGPRINTF(DBG_LEVEL_0, "%s",err_msg[CAN_REG_RO_ERROR]);
                                CAN_DBGPRINTF(DBG_LEVEL_0, "Regster offset: 0x%04X. Bit[%d] RO Error.\n", reg_idx & 0xFFFFFFFC, k+j);
                            }
                        }
                        
                        break;
                        
                    case Bit_Type_RW:
                        /* Case by 0 and 1 for this bit */
                        if(temp_reg & ((0x1 << k) << j))
                        {
                            /* Try to clear this bit */
                            *(volatile unsigned int *)(CAN1_BASE + i) &= ~((0x1 << k) << j);
                            
                            /* Verify this bit, it should be writable */
                            if(temp_reg == *(volatile unsigned int *)(CAN1_BASE + i))
                            {
                                CAN_DBGPRINTF(DBG_LEVEL_0, "%s",err_msg[CAN_REG_RW_ERROR]);
                                CAN_DBGPRINTF(DBG_LEVEL_0, "Regster offset: 0x%04X. Bit[%d] RW Error.\n", reg_idx & 0xFFFFFFFC, k+j);
                            }
                            
                            /* Revert this bit to the default value */
                            *(volatile unsigned int *)(CAN1_BASE + i) = temp_reg;
                        }
                        else
                        {
                            /* Try to write this bit */
                            *(volatile unsigned int *)(CAN1_BASE + i) |= (0x1 << k) << j;
                            
                            /* Verify this bit, it should be writable */
                            if(temp_reg == *(volatile unsigned int *)(CAN1_BASE + i))
                            {
                                CAN_DBGPRINTF(DBG_LEVEL_0, "%s",err_msg[CAN_REG_RW_ERROR]);
                                CAN_DBGPRINTF(DBG_LEVEL_0, "Regster offset: 0x%04X. Bit[%d] RW Error.\n", reg_idx & 0xFFFFFFFC, k+j);
                            }
                            
                            /* Revert this bit to the default value */
                            *(volatile unsigned int *)(CAN1_BASE + i) = temp_reg;
                        }
                        
                        break;
                        
                    case Bit_Type_RW1C:
                        /* Case by 0 and 1 for this bit */
                        if(temp_reg & ((0x1 << k) << j))
                        {
                            /* Try to clear this bit by writing 0 */
                            *(volatile unsigned int *)(CAN1_BASE + i) &= ~((0x1 << k) << j);
                            
                            if(temp_reg != *(volatile unsigned int *)(CAN1_BASE + i))
                            {
                                CAN_DBGPRINTF(DBG_LEVEL_0, "%s",err_msg[CAN_REG_RW1C_ERROR]);
                                CAN_DBGPRINTF(DBG_LEVEL_0, "Regster offset: 0x%04X. Bit[%d] RW1C Error.\n", reg_idx & 0xFFFFFFFC, k+j);
                            }
                            
                            /* Try to clear this bit by writing 1 */
                            *(volatile unsigned int *)(CAN1_BASE + i) = ((0x1 << k) << j);
                            
                            if(temp_reg == *(volatile unsigned int *)(CAN1_BASE + i))
                            {
                                CAN_DBGPRINTF(DBG_LEVEL_0, "%s",err_msg[CAN_REG_RW1C_ERROR]);
                                CAN_DBGPRINTF(DBG_LEVEL_0, "Regster offset: 0x%04X. Bit[%d] RW1C Error.\n", reg_idx & 0xFFFFFFFC, k+j);
                            }
                        }
                        else
                        {
                            /* Try to set this bit by writing 1 */
                            *(volatile unsigned int *)(CAN1_BASE + i) |= ((0x1 << k) << j);
                            
                            if(temp_reg != *(volatile unsigned int *)(CAN1_BASE + i))
                            {
                                CAN_DBGPRINTF(DBG_LEVEL_0, "%s",err_msg[CAN_REG_RW1C_ERROR]);
                                CAN_DBGPRINTF(DBG_LEVEL_0, "Regster offset: 0x%04X. Bit[%d] RW1C Error.\n", reg_idx & 0xFFFFFFFC, k+j);
                            }
                        }
                        break;
                        
                    case Bit_Type_Rsvd:
                        /* Try to write this bit */
                        // *(volatile unsigned int *)(CAN1_BASE + i) |= (0x1 << k) << j;
                        
                        // /* Verify this bit, it should not be writable */
                        // if(temp_reg != *(volatile unsigned int *)(CAN1_BASE + i))
                        // {
                            // CAN_DBGPRINTF(DBG_LEVEL_0, "%s",err_msg[CAN_REG_Rsvd_ERROR]);
                            // CAN_DBGPRINTF(DBG_LEVEL_0, "Regster offset: 0x%04X. Bit[%d] Rsvd Error.\n", reg_idx & 0xFFFFFFFC, k+j);
                        // }
                        break;
                    case Bit_DontTouchMe:
                        CAN_DBGPRINTF(DBG_LEVEL_1, "Regster offset: 0x%04X. Bit[%d] cannot be tested.\n", reg_idx & 0xFFFFFFFC, k+j);
                        break;
                }
            }//For Loop: Each bit in a byte
            
            reg_idx++;
        }//For Loop: Each byte in a word
    }//For Loop: Each 4-byte aligned registers

    return 0;
}

#endif
/*******************************************************************************
                                Mode test functions
*******************************************************************************/
/*
    Input:
        mode: 0. Loopback mode
              1. Normal mode
              2. Listen mode
              3. Sleep mode
              4. Random all modes
    Procedure:
    0. Loopback mode (3 CANs)
        a. Reset CAN1, CAN2 and CAN3
        b. CAN1, CAN2 and CAN3 go Configuration mode for setting.
        c. CAN1 entry to Loopback mode and transmit the data to itself (CAN1->CAN1).
        d. CAN2 and CAN3 go to normal mode
        e. (Not sure)CAN2 received this loopback message is acceptable (CAN1->CAN2).
        f. CAN2 try to send the message on the bus, the CAN should not 
           receive this meesage (CAN2-/->CAN1). But CAN1 will not ack to CAN2, the CAN3 should be standby to ack.
    1. Normal mode
        a. Reset CANs and go Normal mode for setting.
        b. Entry the Normal mode and transmit the data to the other CAN (CAN1->CAN2).
        c. The other CAN will loopback this message, then the CAN will receive the 
           message which it send before (CAN2->CAN1).
    2. Listen mode (3 CANs)
        a. Reset CAN1, CAN2 and CAN3
        b. CAN1, CAN2 and CAN3 go Configuration mode for setting.
        c. CAN1 goes to listen mode. CAN2 and CAN3 go to normal mode.
        d. CAN2 send the message that CAN3 is charge of acking to CAN2.
        e. CAN1 receives the message from the others CAN in listen mode (CAN2->CAN1)
        f. In the listen mode, CAN1 try to send the message to the others CAN, but it should 
           be unsuccessful (CAN1-/->CAN2)
    3. Sleep mode
        a. Reset CAN1 and CAN2
        b. CAN1 and CAN2 go Configuration mode for setting.
        c. CAN1 entries sleep mode and CAN2 entries normal mode
        d. Enable WIE interrupt bit and interrupt handler set
        e. After delay time, CAN2 send the first message, then CAN1 should wakeup immediately and go to normal mode for receiving. (CAN2->CAN1)
        f. check CAN1 will entry normal mode when EOF detected
        g. CAN2 send the second message when CAN1 is normal mode now
        h. Pop fisrt and second message in CAN1 rx FIFO 0, and check pattern
    4. Random mode
        a. Reset CAN1, CAN2 and CAN3
        b. CAN2 and CAN3 go Normal mode for setting. All CAN enable EIE bit.
        c. The ID prority is CAN1 > CAN2 > CAN3 > PCAN (if PCAN joins together for bus load increment)
        d. CAN2 and CAN3 start transmission (The verification feature is optional choose)
        e. CAN1 random the next mode
            i. Loopback mode: internally transmit several times, but the others node should not be received.
            ii. Listen mode: receiving the frame on the bus
            iii. Sleep mode: Wake-up after the SOF of new frame detected
            iv. normal mode: Normally transfer and receive the frame
    
    Comment:
        If the mode test need the three nodes, the no.3 node should be use the correct driver API.
        
        If the third CAN node is not supported FD, the test parameters should not be set FD frame. 
*/
int loopback_test(CAN_InitTypeDef *CAN_init, CAN_FilterTypeDef *CAN_filter, CAN_MaskTypeDef *CAN_mask);
int normal_test(CAN_InitTypeDef *CAN_init, CAN_FilterTypeDef *CAN_filter, CAN_MaskTypeDef *CAN_mask);
int listen_test(CAN_InitTypeDef *CAN_init, CAN_FilterTypeDef *CAN_filter, CAN_MaskTypeDef *CAN_mask);
int sleep_test(CAN_InitTypeDef *CAN_init, CAN_FilterTypeDef *CAN_filter, CAN_MaskTypeDef *CAN_mask);
int random_test(CAN_InitTypeDef *CAN_init, CAN_FilterTypeDef *CAN_filter, CAN_MaskTypeDef *CAN_mask);
static int (*mode_test_cmd[])(CAN_InitTypeDef *CAN_init, \
                              CAN_FilterTypeDef *CAN_filter, \
                              CAN_MaskTypeDef *CAN_mask) \
                              = {loopback_test, normal_test, listen_test, sleep_test, random_test};

int loopback_test(CAN_InitTypeDef *CAN_init, CAN_FilterTypeDef *CAN_filter, CAN_MaskTypeDef *CAN_mask)
{
    CanTxMsgDef TxMessage;
    CanRxMsgDef RxMessage1;
    CanRxMsgDef RxMessage2;

    int ret = 0;
    UINT8 data[8] = {0x00, 0xFF, 0x55, 0xAA, 0xCC, 0x33, 0xF0, 0x0F};

    fLib_printf("\r\n<Loopback test>\n");

    /* CAN1, CAN2 and CAN3 reset */
    CAN_Reset(CAN1);
    CAN_Reset(CAN2);
#ifdef CAN3_FTCAN
    CAN_Reset(CAN3);
#endif
    
    /* set CAN1, CAN2 and CAN3 filter and mask (mask-all enable) */
    CAN_FilterInit(CAN1, CAN_filter, 0);
    CAN_FilterGroup(CAN1, 0, ENABLE);
    CAN_MaskInit(CAN1, CAN_mask, 0);
    CAN_MaskNumber(CAN1, 0, ENABLE);
    CAN_FilterInit(CAN2, CAN_filter, 0);
    CAN_FilterGroup(CAN2, 0, ENABLE);
    CAN_MaskInit(CAN2, CAN_mask, 0);
    CAN_MaskNumber(CAN2, 0, ENABLE);
#ifdef CAN3_FTCAN
    CAN_FilterInit(CAN3, CAN_filter, 0);
    CAN_FilterGroup(CAN3, 0, ENABLE);
    CAN_MaskInit(CAN3, CAN_mask, 0);
    CAN_MaskNumber(CAN3, 0, ENABLE);
#endif 

    /* CAN1 init and entry loopback mode */
    CAN_Init(CAN1, CAN_init);
    CAN_Mode_Change(CAN1, CAN_Mode_LoopBack);
    
    /* CAN2 init and go normal mode */
    CAN_Init(CAN2, CAN_init);
    CAN_Mode_Change(CAN2, CAN_Mode_Normal);
    
    /* CAN3 init and go normal mode 
       CAN3 is for acking to CAN2. */
#ifdef CAN3_FTCAN
    CAN_Init(CAN3, CAN_init);
    CAN_Mode_Change(CAN3, CAN_Mode_Normal);
#endif 
    
    /* Check: CAN1, CAN2 and CAN3 OMR field */
    if(CAN_GetMode(CAN1) != CAN_Mode_LoopBack)
        return CAN_Mode_ERROR;
    if(CAN_GetMode(CAN2) != CAN_Mode_Normal)
        return CAN_Mode_ERROR;
#ifdef CAN3_FTCAN
    if(CAN_GetMode(CAN3) != CAN_Mode_Normal)
        return CAN_Mode_ERROR;
#endif
    
    /* CAN1 send message (8 type frames) and receive itself */
    memset(&TxMessage, 0, sizeof(CanTxMsgDef)); //clear Tx message struct
    memset(&RxMessage1, 0, sizeof(CanRxMsgDef)); //clear Rx message struct (CAN1 Rx)
    memset(&RxMessage2, 0, sizeof(CanRxMsgDef)); //clear Rx message struct (CAN2 Rx)
    
    TxMessage.StdId = 1234;
    TxMessage.ExtId = 0;
    TxMessage.IDE = 0;
    TxMessage.RTR = 0;
    TxMessage.BRS = 0;
    TxMessage.FD = 0;
    TxMessage.DLC = CAN_DataBytes_8;
    memcpy(TxMessage.Data, &data, 8);
    
    /* CAN1 transfer the message */
    CAN_Transmit_TXn(CAN1, &TxMessage, 0);
    while(CAN_GetBTRStatus(CAN1, 0));
    
    /* Check: CAN1 verify the message which is loopback itself*/
    while(CAN_GetStatusBit(CAN1, CAN_BRS0_STATUS_BIT) == 0);
    CAN_Receive_RXn(CAN1, 0, &RxMessage1);
    ret = Verify_Pattern(CAN_A_Data, &TxMessage, &RxMessage1);
    if(ret != 0)
        return ret;
    
    /* Check: CAN2 recevied the messages from CAN1?! */
    if(CAN_GetStatusBit(CAN2, CAN_BRS0_STATUS_BIT) != 0)
    {
        CAN_Receive_RXn(CAN2, 0, &RxMessage2);
        ret = Verify_Pattern(CAN_A_Data, &TxMessage, &RxMessage1);
        if(ret != 0)
            return ret;
    }
    
    /* CAN2 send the message and CAN1 should not receive */
    CAN_Transmit_TXn(CAN2, &TxMessage, 0);
    while(CAN_GetBTRStatus(CAN2, 0));
    
    if(CAN_GetStatusBit(CAN1, CAN_BRS0_STATUS_BIT) != 0)
    {
        return CAN_Loopback_ERROR;
    }
    
    /******* CAN1 use TIE and RIE to transmission (Interrupt) *******/
    /* CAN1 change to config mode */
    CAN_Mode_Change(CAN1, CAN_Mode_Config);
    
    /* Enable TIE and RIE */
    irq_setting(IRQ_CAN1, (PrHandler)CAN1Handler);
    CAN_ITConfig(CAN1, CAN_IRE_TIE|CAN_IRE_RIE|CAN_IRE_EIE, ENABLE);
    
    /* CAN1 change to loopback mode */
    CAN_Mode_Change(CAN1, CAN_Mode_LoopBack);
    
    /* CAN1 send the message */
    ITFLAG = 0;
    CAN_Transmit_TXn(CAN1, &TxMessage, 0);
    while(ITFLAG != 2); //workaround: TIE and RIE handler function have been called
    
    /* Check: CAN1 verify the message which is loopback itself*/
    while(CAN_GetStatusBit(CAN1, CAN_BRS0_STATUS_BIT) == 0);
    CAN_Receive_RXn(CAN1, 0, &RxMessage1);
    ret = Verify_Pattern(CAN_A_Data, &TxMessage, &RxMessage1);
    if(ret != 0)
        return ret;
    
    return ret;
}

int normal_test(CAN_InitTypeDef *CAN_init, CAN_FilterTypeDef *CAN_filter, CAN_MaskTypeDef *CAN_mask)
{
    CanTxMsgDef TxMessage;
    CanRxMsgDef    RxMessage1;
    CanRxMsgDef    RxMessage2;
    int ret = 0;
    UINT8 data[8] = {0x00, 0xFF, 0x55, 0xAA, 0xCC, 0x33, 0xF0, 0x0F};

    fLib_printf("\r\n<Normal test>\n");
    
    /* CAN1 and CAN2 reset */
    CAN_Reset(CAN1);
    CAN_Reset(CAN2);
    
    /* CAN1 and CAN2 filter and mask set (mask-all enable) */
    CAN_FilterInit(CAN1, CAN_filter, 0);
    CAN_FilterGroup(CAN1, 0, ENABLE);
    CAN_MaskInit(CAN1, CAN_mask, 0);
    CAN_MaskNumber(CAN1, 0, ENABLE);
    CAN_FilterInit(CAN2, CAN_filter, 0);
    CAN_FilterGroup(CAN2, 0, ENABLE);
    CAN_MaskInit(CAN2, CAN_mask, 0);
    CAN_MaskNumber(CAN2, 0, ENABLE);
    
    /* Enable Error/Overrun Interrupts */
    irq_setting(IRQ_CAN1, (PrHandler)CAN1Handler);
    CAN_ITConfig(CAN1, CAN_IRE_EIE, ENABLE);
    irq_setting(IRQ_CAN2, (PrHandler)CAN2Handler);
    CAN_ITConfig(CAN2, CAN_IRE_EIE, ENABLE);
    
    /* CAN1 and CAN2 init and entry the normal mode */
    CAN_Init(CAN1, CAN_init);
    CAN_Mode_Change(CAN1, CAN_Mode_Normal);
    CAN_Init(CAN2, CAN_init);
    CAN_Mode_Change(CAN2, CAN_Mode_Normal);
    
    /* Check: CAN1 and CAN2 OMR field */
    if(CAN_GetMode(CAN1) != CAN_Mode_Normal)
        return CAN_Mode_ERROR;
    if(CAN_GetMode(CAN2) != CAN_Mode_Normal)
        return CAN_Mode_ERROR;
    
    /* Clear the Tx and Rx message struct */
    memset(&TxMessage, 0, sizeof(CanTxMsgDef)); 
    memset(&RxMessage1, 0, sizeof(CanRxMsgDef)); 
    memset(&RxMessage2, 0, sizeof(CanRxMsgDef)); 
    
    /* CAN1 transfer message */
    TxMessage.StdId = 1234;
    TxMessage.ExtId = 0;
    TxMessage.IDE = 0;
    TxMessage.RTR = 0;
    TxMessage.BRS = 0;
    TxMessage.FD = 0;
    TxMessage.DLC = CAN_DataBytes_8;

    memcpy(TxMessage.Data, data, 8);
    
    CAN_Transmit_TXn(CAN1, &TxMessage, 0);
    while(CAN_GetBTRStatus(CAN1, 0));
    
    /**** External Loopback through CAN bus: CAN1 -> CAN2 -> CAN1 ****/
    /* CAN2 receive message */
    while(CAN_GetStatusBit(CAN2, CAN_BRS0_STATUS_BIT) == 0);
    CAN_Receive_RXn(CAN2, 0, &RxMessage2);
    
    /* CAN2 transfer the previous message back */
    TxMessage.StdId = RxMessage2.StdId;
    TxMessage.ExtId = RxMessage2.ExtId;
    TxMessage.IDE = RxMessage2.IDE;
    TxMessage.RTR = RxMessage2.RTR;
    TxMessage.BRS = RxMessage2.BRS;
    TxMessage.FD = RxMessage2.FD;
    TxMessage.DLC = RxMessage2.DLC;
    memcpy(TxMessage.Data, RxMessage2.Data, 8);
    
    CAN_Transmit_TXn(CAN2, &TxMessage, 0);
    while(CAN_GetBTRStatus(CAN2, 0));
    
    /* CAN1 receive the message that it send at the beginning */
    while(CAN_GetStatusBit(CAN1, CAN_BRS0_STATUS_BIT) == 0);
    CAN_Receive_RXn(CAN1, 0, &RxMessage1);
    
    /* CAN1 verify the message */
    ret = Verify_Pattern(CAN_A_Data, &TxMessage, &RxMessage1);
    if(ret != 0)
        return ret;
    
    return ret;
}

int listen_test(CAN_InitTypeDef *CAN_init, CAN_FilterTypeDef *CAN_filter, CAN_MaskTypeDef *CAN_mask)
{
    CanTxMsgDef TxMessage;
    CanRxMsgDef    RxMessage;

    int ret = 0;
    int REC, TEC;
    UINT8 data[8] = {0x00, 0xFF, 0x55, 0xAA, 0xCC, 0x33, 0xF0, 0x0F};
    fLib_printf("\r\n<Listen test>\n");

    /* CAN1, CAN2 and CAN3 reset */
    CAN_Reset(CAN1);
    CAN_Reset(CAN2);
#ifdef CAN3_FTCAN
    CAN_Reset(CAN3);
#endif

    /* set CAN1, CAN2 and CAN3 filter and mask (mask-all enable) */
    CAN_FilterInit(CAN1, CAN_filter, 0);
    CAN_FilterGroup(CAN1, 0, ENABLE);
    CAN_MaskInit(CAN1, CAN_mask, 0);
    CAN_MaskNumber(CAN1, 0, ENABLE);
    CAN_FilterInit(CAN2, CAN_filter, 0);
    CAN_FilterGroup(CAN2, 0, ENABLE);
    CAN_MaskInit(CAN2, CAN_mask, 0);
    CAN_MaskNumber(CAN2, 0, ENABLE);
#ifdef CAN3_FTCAN
    CAN_FilterInit(CAN3, CAN_filter, 0);
    CAN_FilterGroup(CAN3, 0, ENABLE);
    CAN_MaskInit(CAN3, CAN_mask, 0);
    CAN_MaskNumber(CAN3, 0, ENABLE);
#endif

    /* Enable Error/Overrun Interrupts */
    irq_setting(IRQ_CAN1, (PrHandler)CAN1Handler);
    CAN_ITConfig(CAN1, CAN_IRE_EIE, ENABLE);
    irq_setting(IRQ_CAN2, (PrHandler)CAN2Handler);
    CAN_ITConfig(CAN2, CAN_IRE_EIE, ENABLE);
#ifdef CAN3_FTCAN
    irq_setting(IRQ_CAN3, CAN3Handler);
    CAN_ITConfig(CAN3, CAN_IRE_EIE, ENABLE);
#endif

    /* CAN1 init and entry the listening mode */
    CAN_Init(CAN1, CAN_init);
    CAN_Mode_Change(CAN1, CAN_Mode_Listen);

    /* CAN2 init and entry the normal mode */
    CAN_Init(CAN2, CAN_init);
    CAN_Mode_Change(CAN2, CAN_Mode_Normal);

    /* CAN3 init and entry the normal mode */
#ifdef CAN3_FTCAN
    CAN_Init(CAN3, CAN_init);
    CAN_Mode_Change(CAN3, CAN_Mode_Normal);
#endif

    /* Check: CAN1 and CAN2 OMR field */
    if(CAN_GetMode(CAN1) != CAN_Mode_Listen)
        return CAN_Mode_ERROR;
    if(CAN_GetMode(CAN2) != CAN_Mode_Normal)
        return CAN_Mode_ERROR;
#ifdef CAN3_FTCAN
    if(CAN_GetMode(CAN3) != CAN_Mode_Normal)
        return CAN_Mode_ERROR;
#endif

    /* Clear the Tx and Rx message struct */
    memset(&TxMessage, 0, sizeof(CanTxMsgDef)); 
    memset(&RxMessage, 0, sizeof(CanRxMsgDef)); 

    /* CAN2 transfer message */
    TxMessage.StdId = 1234;
    TxMessage.ExtId = 0;
    TxMessage.IDE = 0;
    TxMessage.RTR = 0;
    TxMessage.BRS = 0;
    TxMessage.FD = 0;
    TxMessage.DLC = CAN_DataBytes_8;
    memcpy(TxMessage.Data, &data, 8);

    CAN_Transmit_TXn(CAN2, &TxMessage, 0);
    while(CAN_GetBTRStatus(CAN2, 0));

    /* CAN1 receive message */
    while(CAN_GetStatusBit(CAN1, CAN_BRS0_STATUS_BIT) == 0);
    CAN_Receive_RXn(CAN1, 0, &RxMessage);

    /* Verify message */
    ret = Verify_Pattern(CAN_A_Data, &TxMessage, &RxMessage);
    if(ret != 0)
        return ret;

    /* CAN1 try to send message */
    REC = CAN_GetReceiveErrorCounter(CAN1);
    TEC = CAN_GetTransmitErrorCounter(CAN1);
    CAN_Transmit_TXn(CAN1, &TxMessage, 0);
    while(CAN_GetBTRStatus(CAN2, 0));

    /* Check: CAN2 must not receive the message from CAN1 */
    if(CAN_GetStatusBit(CAN2, CAN_BRS0_STATUS_BIT) != 0)
    {
        return CAN_Listen_ERROR;
    }

    /* Check: no Error Counter increase for CAN1 */
    if(CAN_GetReceiveErrorCounter(CAN1) != REC || CAN_GetTransmitErrorCounter(CAN1) != TEC)
    {
        return CAN_Listen_ERROR;
    }

    return ret;
}

int sleep_test(CAN_InitTypeDef *CAN_init, CAN_FilterTypeDef *CAN_filter, CAN_MaskTypeDef *CAN_mask)
{
    CanTxMsgDef TxMessage;
    CanRxMsgDef    RxMessage;

    int ret = 0;
    UINT8 data[8] = {0x00, 0xFF, 0x55, 0xAA, 0xCC, 0x33, 0xF0, 0x0F};
    fLib_printf("\r\n<Sleep test>\n");
    
    /* CAN1, CAN2 and CAN3 reset */
    CAN_Reset(CAN1);
    CAN_Reset(CAN2);
#ifdef CAN3_FTCAN
    CAN_Reset(CAN3);
#endif
    
    /* set CAN1, CAN2 and CAN3 filter and mask (mask-all enable) */
    CAN_FilterInit(CAN1, CAN_filter, 0);
    CAN_FilterGroup(CAN1, 0, ENABLE);
    CAN_MaskInit(CAN1, CAN_mask, 0);
    CAN_MaskNumber(CAN1, 0, ENABLE);
    CAN_FilterInit(CAN2, CAN_filter, 0);
    CAN_FilterGroup(CAN2, 0, ENABLE);
    CAN_MaskInit(CAN2, CAN_mask, 0);
    CAN_MaskNumber(CAN2, 0, ENABLE);
#ifdef CAN3_FTCAN
    CAN_FilterInit(CAN3, CAN_filter, 0);
    CAN_FilterGroup(CAN3, 0, ENABLE);
    CAN_MaskInit(CAN3, CAN_mask, 0);
    CAN_MaskNumber(CAN3, 0, ENABLE);
#endif
    
    /* CAN1 and CAN2 interrupt setting */
    irq_setting(IRQ_CAN1, (PrHandler)CAN1Handler_WIE);
    CAN_ITConfig(CAN1, CAN_IRE_WIE | CAN_IRE_EIE, ENABLE);
    irq_setting(IRQ_CAN2, (PrHandler)CAN2Handler);
    CAN_ITConfig(CAN2, CAN_IRE_EIE, ENABLE);
    
    /* CAN1 init and entry the sleep mode */
    CAN_Init(CAN1, CAN_init);
    CAN_Mode_Change(CAN1, CAN_Mode_Sleep);
    
    /* CAN2 init and entry the normal mode */
    CAN_Init(CAN2, CAN_init);
    CAN_Mode_Change(CAN2, CAN_Mode_Normal);
    
    /* CAN3 init and entry the normal mode (for ACK CAN2)*/
#ifdef CAN3_FTCAN
    CAN_Init(CAN3, CAN_init);
    CAN_Mode_Change(CAN3, CAN_Mode_Normal);
#endif
    
    /* Check: The OMR field in CAN1, CAN2, and CAN3 */
    if(CAN_GetMode(CAN1) != CAN_Mode_Sleep)
        return CAN_Mode_ERROR;
    if(CAN_GetMode(CAN2) != CAN_Mode_Normal)
        return CAN_Mode_ERROR;
#ifdef CAN3_FTCAN
    if(CAN_GetMode(CAN3) != CAN_Mode_Normal)
        return CAN_Mode_ERROR;
#endif
    
    /* Clear the Tx and Rx message struct */
    memset(&TxMessage, 0, sizeof(CanTxMsgDef)); 
    memset(&RxMessage, 0, sizeof(CanRxMsgDef)); 
    
    delay_ms(100); //故意讓sleep久一點
    
    /* Check the CAN1 is still in Sleep mode. */
    if(CAN_GetMode(CAN1) != CAN_Mode_Sleep)
        return CAN_Mode_ERROR;
    
    /* CAN2 send the first message and CAN1 should be waken up */
    ITFLAG = 1; //WIR
    TxMessage.StdId = 1234;
    TxMessage.ExtId = 0;
    TxMessage.IDE = 0;
    TxMessage.RTR = 0;
    TxMessage.BRS = 0;
    TxMessage.FD = 0;
    TxMessage.DLC = CAN_DataBytes_8;
    memcpy(TxMessage.Data, &data, 8);
    CAN_Transmit_TXn(CAN2, &TxMessage, 0);
    while(CAN_GetBTRStatus(CAN2, 0));
    
    /* CAN1 waked up after the first frame from CAN2*/
    while(ITFLAG); //ensure WIR is assert
    
    /* Check the CAN1 is in Normal mode automatically. */
    if(CAN_GetMode(CAN1) != CAN_Mode_Normal)
        return CAN_Mode_ERROR;
    
    /* CAN2 send the second message again after CAN1 wake-up */
    TxMessage.Data[0] = 0x77; //change the value of data[0]
    CAN_Transmit_TXn(CAN2, &TxMessage, 0);
    while(CAN_GetBTRStatus(CAN2, 0));
    
    /* CAN1 receive the first message from CAN2 */
    while(CAN_GetStatusBit(CAN1, CAN_BRS0_STATUS_BIT) == 0);
    CAN_Receive_RXn(CAN1, 0, &RxMessage);
    
    /* CAN1 receive the second message from CAN2 */
    while(CAN_GetStatusBit(CAN1, CAN_BRS0_STATUS_BIT) == 0);
    CAN_Receive_RXn(CAN1, 0, &RxMessage);
    
    /* Verify message */
    ret = Verify_Pattern(CAN_A_Data, &TxMessage, &RxMessage);
    if(ret != 0)
        return ret;
    
    return ret;
}

int random_test(CAN_InitTypeDef *CAN_init, CAN_FilterTypeDef *CAN_filter, CAN_MaskTypeDef *CAN_mask)
{
    CanTxMsgDef TxMessage1, TxMessage2; 
    CanRxMsgDef RxMessage1, RxMessage2; 
    int mConfigCnt = 0, mNormalCnt = 0, mSleepCnt = 0, mListenCnt = 0, mLoopbackCnt = 0;
    int ret = 0;
    char c;
    int mode = 0;
    int opt_1, opt_2;
    UINT32 temp;
    UINT8 data0[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    UINT8 data1[8] = {0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1};
    UINT8 data2[8] = {0xB2, 0xB2, 0xB2, 0xB2, 0xB2, 0xB2, 0xB2, 0xB2};
    UINT8 data3[8] = {0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3};
    
    /* Global counter reset */
    TBCnt = 0;
    
    /* Clear the Tx and Rx message struct */
    memset(&TxMessage1, 0, sizeof(CanTxMsgDef)); 
    memset(&TxMessage2, 0, sizeof(CanTxMsgDef)); 
    memset(&RxMessage1, 0, sizeof(CanRxMsgDef)); 
    memset(&RxMessage2, 0, sizeof(CanRxMsgDef)); 
    
    /* Some options for user choose */
    fLib_printf("Need CAN2 to send frames to make bus busy? yes(1) no(0): ");
    scanf("%d", &opt_1);
    fLib_printf("\nDo CAN1 need to enter Config mode before entering any mode? yes(1) no(0): ");
    scanf("%d", &opt_2);
    fLib_printf("\n");
    
    /* CAN1, CAN2 reset */
    CAN_Reset(CAN1);
    CAN_Reset(CAN2);

    /* CAN1(FTCAN)用來切換mode，
            用Txbuffer0傳SID=1230，Priority最高
       若使用者需要bus上為busy狀態，CAN2(FTCAN)用來一直傳送frames
            用Txbuffer0傳SID=1234
              Txbuffer1傳SID=1235 EID=5678
              Txbuffer2傳SID=1236 no BRS
       CAN2會用interrupt方式不斷傳送
    */
    /* set CAN1 filter and mask */
    CAN_filter->Filter_StdId = 264;
    CAN_filter->Filter_ExtId = 0;
    CAN_filter->Filter_frameType = CAN_ALL;
    CAN_mask->Mask_StdId = 0;
    CAN_mask->Mask_ExtId = 0;
    CAN_mask->Mask_All = 0;
    CAN_FilterInit(CAN1, CAN_filter, 0);
    CAN_FilterGroup(CAN1, 0, DISABLE); 
    CAN_MaskInit(CAN1, CAN_mask, 0);
    CAN_MaskNumber(CAN1, 0, DISABLE);
    /* set CAN2 filter and mask */
    CAN_filter->Filter_StdId = 0;
    CAN_filter->Filter_ExtId = 0;
    CAN_filter->Filter_frameType = CAN_NonFD_Based_Data;
    CAN_mask->Mask_StdId = 0;
    CAN_mask->Mask_ExtId = 0;
    CAN_mask->Mask_All = 0;
    CAN_FilterInit(CAN2, CAN_filter, 0);
    CAN_FilterGroup(CAN2, 0, ENABLE);
    CAN_MaskInit(CAN2, CAN_mask, 0);
    CAN_MaskNumber(CAN2, 0, ENABLE);
    
    /* CAN1 and CAN2 interrupt setting */
    irq_setting(IRQ_CAN1, (PrHandler)CAN1Handler_RMtest);
    CAN_ITConfig(CAN1, CAN_IRE_EIE | CAN_IRE_OIE | CAN_IRE_RIE, ENABLE); 
    irq_setting(IRQ_CAN2, (PrHandler)CAN2Handler_RMtest);
    CAN_ITConfig(CAN2, (opt_1)?(CAN_IRE_EIE | CAN_IRE_TIE):(CAN_IRE_EIE), ENABLE); 
    
    /* CAN1 init and standby the config mode */
    CAN_Init(CAN1, CAN_init);
    //CAN_Mode_Change(CAN1, CAN_Mode_Config);
    CAN_Mode_Change(CAN1, CAN_Mode_Normal);
    
    /* CAN2 init and entry the normal mode */
    CAN_Init(CAN2, CAN_init);
    CAN_Mode_Change(CAN2, CAN_Mode_Normal);

    /* Prepare CAN1 tx frame setting */
    TxMessage1.StdId = 0; //priority high id
    TxMessage1.ExtId = 0;
    TxMessage1.IDE = 0;
    TxMessage1.RTR = 0;
    TxMessage1.BRS = 0;
    TxMessage1.FD = 0;
    TxMessage1.DLC = CAN_DataBytes_8;
    memcpy(TxMessage1.Data, &data0, 8);
    CAN_Transmit(CAN1, &TxMessage1, CAN_Txbuffer_0);
    
    if(opt_1)
    {
        /* Prepare CAN2 tx frames setting */
        TxMessage2.ExtId = 0;
        TxMessage2.IDE = 0;
        TxMessage2.RTR = 0;
        TxMessage2.BRS = 0;
        TxMessage2.FD = 0;
        TxMessage2.DLC = CAN_DataBytes_8;
        /* CAN2 tx buffer 0 */
        TxMessage2.StdId = 264; 
        memcpy(TxMessage2.Data, &data1, 8);
        CAN_Transmit(CAN2, &TxMessage2, CAN_Txbuffer_0);
        /* CAN2 tx buffer 1 */
        TxMessage2.StdId = 621; 
        //TxMessage2.ExtId = 5678;
        memcpy(TxMessage2.Data, &data2, 8);
        CAN_Transmit(CAN2, &TxMessage2, CAN_Txbuffer_1);
        /* CAN2 tx buffer 2 */
        TxMessage2.StdId = 1223;
        //TxMessage2.ExtId = 0;
        //TxMessage2.BRS = 0;
        memcpy(TxMessage2.Data, &data3, 8);
        CAN_Transmit(CAN2, &TxMessage2, CAN_Txbuffer_2);
        
        /* Reset counter for CAN2 each tx buffer */
        TBI0Cnt = 0;
        TBI1Cnt = 0;
        TBI2Cnt = 0;
        
        /* CAN2 start to transmission */
        CAN_TxBTR(CAN2, CAN_Txbuffer_all); //一次BTR0,1,2都寫1
        //CAN_TxBTR(CAN2, CAN_Txbuffer_0);
        //CAN_TxBTR(CAN2, CAN_Txbuffer_1);
        //CAN_TxBTR(CAN2, CAN_Txbuffer_2);
    }
    
    while(1)
    {
        /* if key the space or enter, the burnin will stop. */
        c = ftuart_getc_t(10000, 0);
        if (c == 'q')
            break;
        
        /* CAN1 choose mode */
        //mode = rand()%5; //五種mode隨機選一個
        //mode = (mode + 1)%5; //有規律的五個輪流
        mode = CAN_Mode_Sleep; //先直接固定只有normal mode
        
        /* If opt_2 is enable, each round will enter config mode before enter the mode we requested.*/
        if(opt_2)
        {
            CAN_Mode_Change(CAN1, CAN_Mode_Config);
            //delay_ms(10);
        }
            
        
        switch(mode)
        {
            case CAN_Mode_Config:
                mConfigCnt++;
                CAN_DBGPRINTF(DBG_LEVEL_1, "Entry Config Mode (%d)\n", mConfigCnt);
                
                /* Change mode to config mode */
                CAN_Mode_Change(CAN1, CAN_Mode_Config);
                
                delay_ms(10); //間隔個10000us (10ms) 再運作
                
                /* 目前不確定是否一定要回normal才能進sleep, listen, loopback三種modes */
                break;
            case CAN_Mode_Normal:
                mNormalCnt++; 
                CAN_DBGPRINTF(DBG_LEVEL_1, "Entry Normal Mode (%d)\n", mNormalCnt);
                
                CAN_Mode_Change(CAN1, CAN_Mode_Config); //先回normal
                
                /* Normal mode下，CAN1會間隔打出一個ID最大的message */
                delay_ms(10); //間隔個10000us (10ms) 再運作
                
                /* Change mode to normal mode */
                CAN_Mode_Change(CAN1, CAN_Mode_Normal);
                
                /* CAN1 transmit*/
                CAN_TxBTR(CAN1, CAN_Txbuffer_0);
                while(CAN_GetBTRStatus(CAN1, 0));
                
                /* CAN2 receive 改用interrupt來處理 */
                while(CAN_GetStatusBit(CAN2, CAN_BRS0_STATUS_BIT) == 0);
                //CAN_Receive_RXn(CAN2, 0, &RxMessage1); //怕檢查太久，先不檢查
                
                
                break;
            case CAN_Mode_Sleep:
                mSleepCnt++;
                CAN_DBGPRINTF(DBG_LEVEL_1, "Entry Sleep Mode (%d)\n", mSleepCnt);
                
                CAN_ITConfig(CAN1, CAN_IRE_WIE, ENABLE);
                
                //CAN_Mode_Change(CAN1, CAN_Mode_Normal);
                
                ITFLAG = 1; //reset ITFLAG
                
                /* Change mode to sleep mode */
                CAN_Mode_Change(CAN1, CAN_Mode_Sleep); //有wake up interrupt後會在handler中再把WIE關掉
                
                //CAN_TxBTR(CAN2, CAN_Txbuffer_2);
                
                while(ITFLAG); //等handler中的WIE解除
                //check CAN1回到normal mode了
                if(CAN_GetMode(CAN1) != CAN_Mode_Normal)
                    return CAN_Mode_ERROR;
                
                delay_ms(10); //間隔個10000us (10ms) 再運作
                
                break;
            case CAN_Mode_Listen:
                mListenCnt++;
                CAN_DBGPRINTF(DBG_LEVEL_1, "Entry Listening Mode (%d)\n", mListenCnt);
            
                /* Change mode to listen mode */
                CAN_Mode_Change(CAN1, CAN_Mode_Listen);
                
                delay_ms(10); //間隔個10000us (10ms) 再運作
            
                break;
            case CAN_Mode_LoopBack:
                mLoopbackCnt++;
                CAN_DBGPRINTF(DBG_LEVEL_1, "Entry Loopback Mode (%d)\n", mLoopbackCnt);
            
                /* Change mode to listen mode */
                CAN_Mode_Change(CAN1, CAN_Mode_LoopBack);
                
                delay_ms(10); //間隔個10000us (10ms) 再運作
                
                /* CAN1 transmit*/
                CAN_TxBTR(CAN1, CAN_Txbuffer_0);
                while(CAN_GetBTRStatus(CAN1, 0));
                
                /* CAN1 receive 改用interrupt來處理 */
                while(CAN_GetStatusBit(CAN1, CAN_BRS0_STATUS_BIT) == 0);
            
                break;
        }
    }
    
    /* Disable CAN2 TIE bit to stop transmitting */
    CAN_ITConfig(CAN2, CAN_IRE_TIE, DISABLE); 
    
    return ret;
    
}

int CAN_Mode_Change_test(CAN_MODE mode)
{
	  char buf[32];
    CAN_InitTypeDef CAN_InitData;
    CAN_FilterTypeDef CAN_FilterConfig;
    CAN_MaskTypeDef CAN_MaskConfig;
    int ret, opt_brs, opt_NSpeed, opt_DSpeed;
    
    /* Clear these structs value */
    memset(&CAN_InitData, 0, sizeof(CAN_InitTypeDef));
    memset(&CAN_FilterConfig, 0, sizeof(CAN_FilterTypeDef));
    memset(&CAN_MaskConfig, 0, sizeof(CAN_MaskTypeDef));
    
    /* Need to add the TQ parameters maunally */
// int bit_timing_table[BIT_RATE_NUMBER][4] = {
    // /* {Prescaler, Propgation, Phase1, Phase2} */
    // {40, 9, 5, 5},    //125kbps, 75%
    // {20, 9, 5, 5},    //250kbps, 75%
    // {10, 9, 5, 5},    //500kbps, 75%
    // {5, 8, 8, 8},     //800kbps, 68%
    // {4, 8, 8, 8}      //1Mbps, 68%
// };
// int fast_bit_timing_table[Fast_BIT_RATE_NUMBER][4] = {
    // /* {Prescaler, Propgation, Phase1, Phase2} */
    // {5, 8, 8, 3},   //1Mbps, 85% 
    // {5, 1, 4, 4},   //2Mbps, 60%
    // {2, 3, 8, 8},   //2.5Mbps, 60%
    // {2, 5, 2, 2}    //5Mbps, 80%
// };
    
    /* Ask user what speed to use */
    fLib_printf("Baud Rate Switch (BRS) yes(1) no(0):\n");
		fLib_gets(DEBUG_CONSOLE, (char*)buf);
		opt_brs = atoi((char*)buf);					
    fLib_printf("\n");
    if(opt_brs) //bit_timing_table
    {
        fLib_printf("Select speed:\n");
        fLib_printf("0. 125Kbps 75%\n");
        fLib_printf("1. 250Kbps 75%\n");
        fLib_printf("2. 500Kbps 75%\n");
        fLib_printf("3. 800Kbps 75%\n");
        fLib_printf("4. 1Mbps 75%\n");
//        scanf("%d", &opt_NSpeed);
				fLib_gets(DEBUG_CONSOLE, (char*)buf);
				opt_NSpeed = atoi((char*)buf);					
			
        
        CAN_InitData.CAN_NBRP = bit_timing_table[opt_NSpeed][0];
        CAN_InitData.CAN_NProp = bit_timing_table[opt_NSpeed][1];
        CAN_InitData.CAN_NPS1 = bit_timing_table[opt_NSpeed][2];
        CAN_InitData.CAN_NPS2 = bit_timing_table[opt_NSpeed][3];
        CAN_InitData.CAN_NSJW = min_t(CAN_InitData.CAN_NPS2, 32);
    }
    else //fast_bit_timing_table
    {
        fLib_printf("Select speed in nominal bit rate:\n");
        fLib_printf("0. 125Kbps 75%\n");
        fLib_printf("1. 250Kbps 75%\n");
        fLib_printf("2. 500Kbps 75%\n");
        fLib_printf("3. 800Kbps 75%\n");
        fLib_printf("4. 1Mbps 75%\n");
//        scanf("%d", &opt_NSpeed);
				fLib_gets(DEBUG_CONSOLE, (char*)buf);
				opt_NSpeed = atoi((char*)buf);					
			
        fLib_printf("\nSelect speed in nominal bit rate:\n");
        fLib_printf("0. 1Mbps 85%\n");
        fLib_printf("1. 2Mbps 60%\n");
        fLib_printf("2. 2.5Mbps 60%\n");
        fLib_printf("3. 5Mbps 80%\n");
        //scanf("%d", &opt_DSpeed);
 				fLib_gets(DEBUG_CONSOLE, (char*)buf);
				opt_DSpeed = atoi((char*)buf);					
      
        CAN_InitData.CAN_NBRP = bit_timing_table[opt_NSpeed][0];
        CAN_InitData.CAN_NProp = bit_timing_table[opt_NSpeed][1];
        CAN_InitData.CAN_NPS1 = bit_timing_table[opt_NSpeed][2];
        CAN_InitData.CAN_NPS2 = bit_timing_table[opt_NSpeed][3];
        CAN_InitData.CAN_NSJW = min_t(CAN_InitData.CAN_NPS2, 32);
        CAN_InitData.CAN_DBRP = fast_bit_timing_table[opt_DSpeed][0];
        CAN_InitData.CAN_DProp = fast_bit_timing_table[opt_DSpeed][1];
        CAN_InitData.CAN_DPS1 = fast_bit_timing_table[opt_DSpeed][2];
        CAN_InitData.CAN_DPS2 = fast_bit_timing_table[opt_DSpeed][3];
        CAN_InitData.CAN_DSJW = min_t(CAN_InitData.CAN_DPS2, 4);
    }
    fLib_printf("\n");
    
    
    /* Non-FD and FD CAN's bit timing setting */
    CAN_InitData.CAN_Mode = CAN_Mode_Config;
    CAN_InitData.CAN_RT = CAN_RT_1time;
    //CAN_InitData.CAN_RT = CAN_RT_Always;
    /* 500k 75% -> 5M 80% (FD format)*/
    // CAN_InitData.CAN_NBRP = 10;
    // CAN_InitData.CAN_NSJW = 4;
    // CAN_InitData.CAN_NProp = 9;
    // CAN_InitData.CAN_NPS1 = 5;
    // CAN_InitData.CAN_NPS2 = 5;
    // CAN_InitData.CAN_DBRP = 2;
    // CAN_InitData.CAN_DSJW = 2;
    // CAN_InitData.CAN_DProp = 5;
    // CAN_InitData.CAN_DPS1 = 2;
    // CAN_InitData.CAN_DPS2 = 2;
    /* 500k 75% -> 1M 85% (FD format)*/
    // CAN_InitData.CAN_NBRP = 10;
    // CAN_InitData.CAN_NSJW = 4;
    // CAN_InitData.CAN_NProp = 9;
    // CAN_InitData.CAN_NPS1 = 5;
    // CAN_InitData.CAN_NPS2 = 5;
    // CAN_InitData.CAN_DBRP = 5;
    // CAN_InitData.CAN_DSJW = 3;
    // CAN_InitData.CAN_DProp = 8;
    // CAN_InitData.CAN_DPS1 = 8;
    // CAN_InitData.CAN_DPS2 = 3;
    /* 125K 75% */
    // CAN_InitData.CAN_NBRP = 50;
    // CAN_InitData.CAN_NSJW = 4;
    // CAN_InitData.CAN_NProp = 7;
    // CAN_InitData.CAN_NPS1 = 4;
    // CAN_InitData.CAN_NPS2 = 4;
    // CAN_InitData.CAN_DBRP = 0;
    // CAN_InitData.CAN_DSJW = 0;
    // CAN_InitData.CAN_DProp = 0;
    // CAN_InitData.CAN_DPS1 = 0;
    // CAN_InitData.CAN_DPS2 = 0;
    /* 1M 75% */
    // CAN_InitData.CAN_NBRP = 5;
    // CAN_InitData.CAN_NSJW = 4;
    // CAN_InitData.CAN_NProp = 9;
    // CAN_InitData.CAN_NPS1 = 5;
    // CAN_InitData.CAN_NPS2 = 5;
    // CAN_InitData.CAN_DBRP = 0;
    // CAN_InitData.CAN_DSJW = 0;
    // CAN_InitData.CAN_DProp = 0;
    // CAN_InitData.CAN_DPS1 = 0;
    // CAN_InitData.CAN_DPS2 = 0;
    /* 500K 75% */
    // CAN_InitData.CAN_NBRP = 10;
    // CAN_InitData.CAN_NSJW = 4;
    // CAN_InitData.CAN_NProp = 9;
    // CAN_InitData.CAN_NPS1 = 5;
    // CAN_InitData.CAN_NPS2 = 5;
    // CAN_InitData.CAN_DBRP = 0;
    // CAN_InitData.CAN_DSJW = 0;
    // CAN_InitData.CAN_DProp = 0;
    // CAN_InitData.CAN_DPS1 = 0;
    // CAN_InitData.CAN_DPS2 = 0;
    
    /* Filter and mask setting */
    CAN_FilterConfig.Filter_StdId = 1234;
    CAN_FilterConfig.Filter_ExtId = 0;
    CAN_FilterConfig.Filter_frameType = CAN_ALL;
    CAN_MaskConfig.Mask_StdId = 0;
    CAN_MaskConfig.Mask_ExtId = 0;
    CAN_MaskConfig.Mask_All = 1;
    
    /* Call the target mode function for test */
    ret = mode_test_cmd[mode](&CAN_InitData, &CAN_FilterConfig, &CAN_MaskConfig);
    
    return ret;
}


/*******************************************************************************
                                Simple test functions
*******************************************************************************/
int CAN_Transmission_polling_template()
{
    CAN_InitTypeDef CAN_InitData;
    CAN_FilterTypeDef CAN_FilterConfig;
    CAN_MaskTypeDef CAN_MaskConfig;
    CanTxMsgDef TxMessage;
    CanRxMsgDef RxMessage;
    
    /* Clear these structs value */
    memset(&CAN_InitData, 0, sizeof(CAN_InitTypeDef));
    memset(&CAN_FilterConfig, 0, sizeof(CAN_FilterTypeDef));
    memset(&CAN_MaskConfig, 0, sizeof(CAN_MaskTypeDef));
    memset(&TxMessage, 0, sizeof(CanTxMsgDef));
    memset(&RxMessage, 0, sizeof(CanRxMsgDef));
    
    CAN_Reset(CAN1);
    CAN_Reset(CAN2);
    
    CAN_InitData.CAN_Mode = CAN_Mode_Normal;
    CAN_InitData.CAN_RT = CAN_RT_Always;
    CAN_InitData.CAN_NBRP = 5;
    CAN_InitData.CAN_NSJW = 4;
    CAN_InitData.CAN_NProp = 9;
    CAN_InitData.CAN_NPS1 = 5;
    CAN_InitData.CAN_NPS2 = 5;
    CAN_InitData.CAN_DBRP = 0;
    CAN_InitData.CAN_DSJW = 0;
    CAN_InitData.CAN_DProp = 0;
    CAN_InitData.CAN_DPS1 = 0;
    CAN_InitData.CAN_DPS2 = 0;
    CAN_InitData.CAN_TOE = DISABLE; 
    CAN_InitData.CAN_TSE = DISABLE; 
    CAN_InitData.CAN_EnDBE = DISABLE;
    
    CAN_FilterConfig.Filter_StdId = 1234;
    CAN_FilterConfig.Filter_ExtId = 0;
    CAN_FilterConfig.Filter_frameType = CAN_ALL;
    CAN_MaskConfig.Mask_StdId = 0;
    CAN_MaskConfig.Mask_ExtId = 0;
    CAN_MaskConfig.Mask_All = 1;
    CAN_FilterInit(CAN2, &CAN_FilterConfig, 0);
    CAN_FilterGroup(CAN2, 0, ENABLE); 
    CAN_MaskInit(CAN2, &CAN_MaskConfig, 0);
    CAN_MaskNumber(CAN2, 0, ENABLE); 
    
    CAN_Init(CAN1, &CAN_InitData);
    CAN_Init(CAN2, &CAN_InitData);
    
    /* transfer message */
    TxMessage.StdId = 1234;
    TxMessage.ExtId = 0;
    TxMessage.IDE = 0;
    TxMessage.BRS = 0;
    TxMessage.FD = 0;
    TxMessage.RTR = 0;
    TxMessage.DLC = CAN_DataBytes_4;
    TxMessage.Data[0] = 0xC3;
    TxMessage.Data[1] = 0x3C;
    TxMessage.Data[2] = 0xFF;
    TxMessage.Data[3] = 0x00;
    CAN_Transmit_TXn(CAN1, &TxMessage, CAN_Txbuffer_0);
    while(CAN_GetBTRStatus(CAN1, CAN_Txbuffer_0));
    
    /* receive message */
    while(CAN_GetStatusBit(CAN2, CAN_BRS0_STATUS_BIT) == 0);
    CAN_Receive_RXn(CAN2, CAN_RxFIFO_0, &RxMessage);
    
    return 0;
}

int CAN_Transmission_interrupt_template()
{
    CAN_InitTypeDef CAN_InitData;
    CAN_FilterTypeDef CAN_FilterConfig;
    CAN_MaskTypeDef CAN_MaskConfig;
    CanTxMsgDef TxMessage;
    CanRxMsgDef RxMessage;
    
    /* Clear these structs value */
    memset(&CAN_InitData, 0, sizeof(CAN_InitTypeDef));
    memset(&CAN_FilterConfig, 0, sizeof(CAN_FilterTypeDef));
    memset(&CAN_MaskConfig, 0, sizeof(CAN_MaskTypeDef));
    memset(&TxMessage, 0, sizeof(CanTxMsgDef));
    memset(&RxMessage, 0, sizeof(CanRxMsgDef));
    
    CAN_Reset(CAN1);
    CAN_Reset(CAN2);
    
    CAN_InitData.CAN_Mode = CAN_Mode_Normal;
    CAN_InitData.CAN_RT = CAN_RT_Always;
    CAN_InitData.CAN_NBRP = 5;
    CAN_InitData.CAN_NSJW = 4;
    CAN_InitData.CAN_NProp = 9;
    CAN_InitData.CAN_NPS1 = 5;
    CAN_InitData.CAN_NPS2 = 5;
    CAN_InitData.CAN_DBRP = 0;
    CAN_InitData.CAN_DSJW = 0;
    CAN_InitData.CAN_DProp = 0;
    CAN_InitData.CAN_DPS1 = 0;
    CAN_InitData.CAN_DPS2 = 0;
    CAN_InitData.CAN_TOE = DISABLE; 
    CAN_InitData.CAN_TSE = DISABLE; 
    CAN_InitData.CAN_EnDBE = DISABLE;
    
    CAN_FilterConfig.Filter_StdId = 1234;
    CAN_FilterConfig.Filter_ExtId = 0;
    CAN_FilterConfig.Filter_frameType = CAN_ALL;
    CAN_MaskConfig.Mask_StdId = 0;
    CAN_MaskConfig.Mask_ExtId = 0;
    CAN_MaskConfig.Mask_All = 1;
    CAN_FilterInit(CAN2, &CAN_FilterConfig, 0);
    CAN_FilterGroup(CAN2, 0, ENABLE); 
    CAN_MaskInit(CAN2, &CAN_MaskConfig, 0);
    CAN_MaskNumber(CAN2, 0, ENABLE);
    
    CAN_ITConfig(CAN1, CAN_IRE_TIE, ENABLE);
    CAN_ITConfig(CAN2, CAN_IRE_RIE, ENABLE);
    irq_setting(IRQ_CAN1, (PrHandler)CAN1_Simple_Tx_Handler);
    irq_setting(IRQ_CAN2, (PrHandler)CAN2_Simple_Rx_Handler);

    CAN_Transmit_TXn(CAN1, &TxMessage, CAN_Txbuffer_0);

    return 0;
}

int CAN_Simple_Transmission_test(CAN_Param *dev)
{
    CAN_InitTypeDef CAN_InitData;
    CAN_FilterTypeDef CAN_FilterConfig;
    CAN_MaskTypeDef CAN_MaskConfig;
    CanTxMsgDef TxMessage;
    CanRxMsgDef RxMessage;
    int ret, data_random, id_random;
    char c;
    UINT32 counter;
    UINT32 SID, EID;
	  char buf[32];
    
    counter = 0;
    UINT8 data8[8] = {0x07, 0xc1, 0xf0, 0x7c, 0x1f, 0x07, 0xc1, 0xf0};
    UINT8 data64[64] = {0x07, 0xc1, 0xf0, 0x7c, 0x1f, 0x07, 0xc1, 0xf0, \
                        0x7c, 0x1f, 0x07, 0xc1, 0xf0, 0x7c, 0x1f, 0x07, \
                        0xc1, 0xf0, 0x7c, 0x1f, 0x07, 0xc1, 0xf0, 0x7c, \
                        0x1f, 0x07, 0xc1, 0xf0, 0x7c, 0x1f, 0x07, 0xc1, \
                        0xf0, 0x7c, 0x1f, 0x07, 0xc1, 0xf0, 0x7c, 0x1f, \
                        0x07, 0xc1, 0xf0, 0x7c, 0x1f, 0x07, 0xc1, 0xf0, \
                        0x7c, 0x1f, 0x07, 0xc1};
    float df[5];
    
    /* Clear these structs value */
    memset(&CAN_InitData, 0, sizeof(CAN_InitTypeDef));
    memset(&CAN_FilterConfig, 0, sizeof(CAN_FilterTypeDef));
    memset(&CAN_MaskConfig, 0, sizeof(CAN_MaskTypeDef));
    memset(&TxMessage, 0, sizeof(CanTxMsgDef));
    memset(&RxMessage, 0, sizeof(CanRxMsgDef));
    
    fLib_printf("ID random? yes(1) no(0): ");
    fLib_gets(DEBUG_CONSOLE, (char*)buf);
		id_random = atoi((char*)buf);												
    fLib_printf("\n");
    fLib_printf("Data random? yes(1) no(0): ");
    fLib_gets(DEBUG_CONSOLE, (char*)buf);
		data_random = atoi((char*)buf);												
    fLib_printf("\n");
    
    /* Reset the CAN1 and CAN2 */
    CAN_Reset(CAN1);
    CAN_Reset(CAN2);
    
    CAN_InitData.CAN_Mode = CAN_Mode_Normal;
    CAN_InitData.CAN_RT = CAN_RT_1time;
    if(!dev->FD_mode)
    {
        /* Non-FD mode */
        CAN_InitData.CAN_NBRP = dev->NPRE;
        CAN_InitData.CAN_NSJW = dev->NSJW;
        CAN_InitData.CAN_NProp = dev->NPROP;
        CAN_InitData.CAN_NPS1 = dev->NPS1;
        CAN_InitData.CAN_NPS2 = dev->NPS2;
    }
    else
    {
        /* FD mode */
        CAN_InitData.CAN_NBRP = dev->NPRE;
        CAN_InitData.CAN_NSJW = dev->NSJW;
        CAN_InitData.CAN_NProp = dev->NPROP;
        CAN_InitData.CAN_NPS1 = dev->NPS1;
        CAN_InitData.CAN_NPS2 = dev->NPS2;
        CAN_InitData.CAN_DBRP = dev->DPRE;
        CAN_InitData.CAN_DSJW = dev->DSJW;
        CAN_InitData.CAN_DProp = dev->DPROP;
        CAN_InitData.CAN_DPS1 = dev->DPS1;
        CAN_InitData.CAN_DPS2 = dev->DPS2;
    }
    
    if(id_random)
    {
        Generate_Random_Identifier(dev->FrameType, &SID, &EID);
    }
    else
    {
        SID = 124;
        EID = 230368;
    }
    
    TxMessage.StdId = SID;
    if(dev->FrameType == CAN_B_Data || dev->FrameType == CAN_B_Remote || \
       dev->FrameType == CAN_FD_B   || dev->FrameType == CAN_FD_B_BRS)
    {
        TxMessage.ExtId = EID;
        TxMessage.IDE = 1;
        CAN_FilterConfig.Filter_ExtId = EID;
    }
    else
    {
        TxMessage.ExtId = 0;
        TxMessage.IDE = 0;
        CAN_FilterConfig.Filter_ExtId = 0;
    }
    TxMessage.BRS = (dev->FrameType >= 6)?1:0;
    TxMessage.FD = dev->FD_mode;
    
    CAN_FilterConfig.Filter_StdId = SID;
    CAN_FilterConfig.Filter_frameType = CAN_ALL;
    CAN_MaskConfig.Mask_StdId = 0;
    CAN_MaskConfig.Mask_ExtId = 0;
    CAN_MaskConfig.Mask_All = 1;
    
    /* Only set CAN2's mask and filter for receiving */
    CAN_FilterInit(CAN2, &CAN_FilterConfig, 0);
    CAN_FilterGroup(CAN2, 0, ENABLE); 
    CAN_MaskInit(CAN2, &CAN_MaskConfig, 0);
    CAN_MaskNumber(CAN2, 0, ENABLE); 
    
    /* Set interrupt handler and enable interrupt */
    irq_setting(IRQ_CAN1, (PrHandler)CAN1Handler);
    irq_setting(IRQ_CAN2, (PrHandler)CAN2Handler);
    CAN_ITConfig(CAN1, CAN_IRE_EIE, ENABLE);
    CAN_ITConfig(CAN2, CAN_IRE_EIE, ENABLE);
    
    CAN_Init(CAN1, &CAN_InitData);
    CAN_Init(CAN2, &CAN_InitData);

    do
    {
        /* if key the space or enter, the burnin will stop. */
        //c = fLib_getchar(DEBUG_CONSOLE);			    
			  //c = ftuart_getc_t(10000, 0);
        if (c == 'q')
            break;
        
        /* According FD support or not to the max data length for DLC */
        if(dev->FrameType == CAN_A_Remote || dev->FrameType == CAN_B_Remote)
        {
            TxMessage.RTR = 1;
            TxMessage.DLC = CAN_DataBytes_0;
        }
        else
        {
            TxMessage.RTR = 0;
            if(data_random)
            {
                /* Randomize the DLC according to frame type, and generate the data values. */
                Generate_Random_Pattern_Length(dev->FrameType, &TxMessage.DLC, TxMessage.Data);
            }
            else
            {
                /* If no data random request, here it uses the default pattern.
                   This pattern is a special one that there has no resync during 10-bit (e.g,.0000011111) */
                if(dev->FrameType > 3)
                {
                    TxMessage.DLC = CAN_DataBytes_64;
                    memcpy(TxMessage.Data, &data64, 64);
                }
                else
                {
                    TxMessage.DLC = CAN_DataBytes_8;
                    memcpy(TxMessage.Data, &data8, 8);
                }
            }
        }

        /* transfer message */
				delay_ms(100);
        CAN_Transmit_TXn(CAN1, &TxMessage, 0);
        while(CAN_GetBTRStatus(CAN1, 0));
        /* Receive message */
        while(CAN_GetStatusBit(CAN2, CAN_BRS0_STATUS_BIT) == 0);
        CAN_Receive_RXn(CAN2, 0, &RxMessage);

        /* Verify message */
        ret = Verify_Pattern(dev->FrameType, &TxMessage, &RxMessage);
        if(ret != 0)
            return ret;
    
        counter++;
        CAN_DBGPRINTF(DBG_LEVEL_1, "\r%08d", counter);

    }while(dev->FreeRun);

    CAN_DBGPRINTF(DBG_LEVEL_1, " times");


    //Oscillator_Tolerance_Calc(dev, &df); //for test

    return ret;
}

int CAN_Simple_Transmission_Interrupt_test(CAN_Param *dev)
{
    CAN_InitTypeDef CAN_InitData;
    CAN_FilterTypeDef CAN_FilterConfig;
    CAN_MaskTypeDef CAN_MaskConfig;
    CanTxMsgDef TxMessage;
    CanRxMsgDef RxMessage;
    int ret, id_random, data_random;
    UINT32 counter, SID, EID;
    char c;
	  char buf[32];
    
    counter = 0;
    UINT8 data8[8] = {0x07, 0xc1, 0xf0, 0x7c, 0x1f, 0x07, 0xc1, 0xf0};
    UINT8 data64[64] = {0x07, 0xc1, 0xf0, 0x7c, 0x1f, 0x07, 0xc1, 0xf0, \
                        0x7c, 0x1f, 0x07, 0xc1, 0xf0, 0x7c, 0x1f, 0x07, \
                        0xc1, 0xf0, 0x7c, 0x1f, 0x07, 0xc1, 0xf0, 0x7c, \
                        0x1f, 0x07, 0xc1, 0xf0, 0x7c, 0x1f, 0x07, 0xc1, \
                        0xf0, 0x7c, 0x1f, 0x07, 0xc1, 0xf0, 0x7c, 0x1f, \
                        0x07, 0xc1, 0xf0, 0x7c, 0x1f, 0x07, 0xc1, 0xf0, \
                        0x7c, 0x1f, 0x07, 0xc1};
    
    /* Clear these structs value */
    memset(&CAN_InitData, 0, sizeof(CAN_InitTypeDef));
    memset(&CAN_FilterConfig, 0, sizeof(CAN_FilterTypeDef));
    memset(&CAN_MaskConfig, 0, sizeof(CAN_MaskTypeDef));
    memset(&TxMessage, 0, sizeof(CanTxMsgDef));
    memset(&RxMessage, 0, sizeof(CanRxMsgDef));
    
    fLib_printf("ID random? yes(1) no(0): ");
    //scanf("%d",&id_random);
    fLib_gets(DEBUG_CONSOLE, (char*)buf);
		id_random = atoi((char*)buf);														
    fLib_printf("\n");
    fLib_printf("Data random? yes(1) no(0): ");
//    scanf("%d",&data_random);
    fLib_gets(DEBUG_CONSOLE, (char*)buf);
    data_random = atoi((char*)buf);														
												
    fLib_printf("\n");
    
    CAN_InitData.CAN_Mode = CAN_Mode_Normal;
    CAN_InitData.CAN_RT = CAN_RT_1time;
    if(!dev->FD_mode)
    {
        /* Non-FD mode */
        CAN_InitData.CAN_NBRP = dev->NPRE;
        CAN_InitData.CAN_NSJW = dev->NSJW;
        CAN_InitData.CAN_NProp = dev->NPROP;
        CAN_InitData.CAN_NPS1 = dev->NPS1;
        CAN_InitData.CAN_NPS2 = dev->NPS2;
    }
    else
    {
        /* FD mode */
        CAN_InitData.CAN_NBRP = dev->NPRE;
        CAN_InitData.CAN_NSJW = dev->NSJW;
        CAN_InitData.CAN_NProp = dev->NPROP;
        CAN_InitData.CAN_NPS1 = dev->NPS1;
        CAN_InitData.CAN_NPS2 = dev->NPS2;
        CAN_InitData.CAN_DBRP = dev->DPRE;
        CAN_InitData.CAN_DSJW = dev->DSJW;
        CAN_InitData.CAN_DProp = dev->DPROP;
        CAN_InitData.CAN_DPS1 = dev->DPS1;
        CAN_InitData.CAN_DPS2 = dev->DPS2;
    }
    
    if(id_random)
    {
        Generate_Random_Identifier(dev->FrameType, &SID, &EID);
    }
    else
    {
        SID = 124;
        EID = 230368;
    }
    
    TxMessage.StdId = SID;
    if(dev->FrameType == CAN_B_Data || dev->FrameType == CAN_B_Remote || \
       dev->FrameType == CAN_FD_B   || dev->FrameType == CAN_FD_B_BRS)
    {
        TxMessage.ExtId = EID;
        TxMessage.IDE = 1;
        CAN_FilterConfig.Filter_ExtId = EID;
    }
    else
    {
        TxMessage.ExtId = 0;
        TxMessage.IDE = 0;
        CAN_FilterConfig.Filter_ExtId = 0;
    }
    TxMessage.BRS = (dev->FrameType >= 6)?1:0;
    TxMessage.FD = dev->FD_mode;
    CAN_FilterConfig.Filter_StdId = SID;
    CAN_FilterConfig.Filter_frameType = CAN_ALL;
    CAN_MaskConfig.Mask_StdId = 0;
    CAN_MaskConfig.Mask_ExtId = 0;
    CAN_MaskConfig.Mask_All = 1;
    
    /* Only CAN2 need receiving, so setting CAN2 filter and mask */
    CAN_FilterInit(CAN2, &CAN_FilterConfig, 0);
    CAN_FilterGroup(CAN2, 0, ENABLE);
    CAN_MaskInit(CAN2, &CAN_MaskConfig, 0);
    CAN_MaskNumber(CAN2, 0, ENABLE);
    
    /* Set interrupt handler and enable interrupt */
    irq_setting(IRQ_CAN1, (PrHandler)CAN1Handler);
    irq_setting(IRQ_CAN2, (PrHandler)CAN2Handler);
		fLib_EnableIRQ(IRQ_CAN1);
		fLib_EnableIRQ(IRQ_CAN2);
    CAN_ITConfig(CAN1, CAN_IRE_TIE | CAN_IRE_EIE | CAN_IRE_OIE, ENABLE);
    CAN_ITConfig(CAN2, CAN_IRE_RIE | CAN_IRE_EIE | CAN_IRE_OIE, ENABLE);
    
    /* Initial CAN1 and CAN2 */
    CAN_Init(CAN1, &CAN_InitData);
    CAN_Init(CAN2, &CAN_InitData);
    
    do
    {
        /* if key the space or enter, the burnin will stop. */
        c = ftuart_getc_t(10000, 0);
        if (c == 'q')
            break;
        
        if(dev->FrameType == CAN_A_Remote || dev->FrameType == CAN_B_Remote)
        {
            TxMessage.RTR = 1;
            TxMessage.DLC = CAN_DataBytes_0;
        }
        else
        {
            TxMessage.RTR = 0;
            if(data_random)
            {
                /* Randomize the DLC according to frame type, and generate the data values. */
                Generate_Random_Pattern_Length(dev->FrameType, &TxMessage.DLC, TxMessage.Data);
            }
            else
            {
                if(dev->FrameType > 3)
                {
                    TxMessage.DLC = CAN_DataBytes_64;
                    memcpy(TxMessage.Data, &data64, 64);
                }
                else
                {
                    TxMessage.DLC = CAN_DataBytes_8;
                    memcpy(TxMessage.Data, &data8, 8);
                }
            }
        }
        
    
        /* CAN1 send the message */
        ITFLAG = 0;
        CAN_Transmit_TXn(CAN1, &TxMessage, 0);
        fLib_printf("ITFLAG %d\n",ITFLAG);
        while(ITFLAG != 2); //workaround: TIE and RIE handler function have been called

        /* CAN2 receive the message */
        CAN_Receive_RXn(CAN2, 0, &RxMessage);
        
        /* Verify message */
        ret = Verify_Pattern(dev->FrameType, &TxMessage, &RxMessage);
        if(ret != 0)
            return ret;

        counter++;
        CAN_DBGPRINTF(DBG_LEVEL_1, "\r%08d", counter);
        
    }while(dev->FreeRun);
    
        CAN_DBGPRINTF(DBG_LEVEL_1, " times");
    
    return ret;
}

int CAN_data_length_test(CAN_Param *dev)
{
    CAN_InitTypeDef CAN_InitData;
    CAN_FilterTypeDef CAN_FilterConfig;
    CAN_MaskTypeDef CAN_MaskConfig;
    CanTxMsgDef TxMessage;
    CanRxMsgDef    RxMessage;
    UINT8 * pData;
    UINT32 i, DLC_i, ret, t, counter;
    char c;
    
    counter = 0;
    pData = (UINT8*)malloc(64);
    
    /* Clear these structs value */
    memset(&CAN_InitData, 0, sizeof(CAN_InitTypeDef));
    memset(&CAN_FilterConfig, 0, sizeof(CAN_FilterTypeDef));
    memset(&CAN_MaskConfig, 0, sizeof(CAN_MaskTypeDef));
    memset(&TxMessage, 0, sizeof(CanTxMsgDef));
    memset(&RxMessage, 0, sizeof(CanRxMsgDef));
    
    CAN_InitData.CAN_Mode = CAN_Mode_Normal;
    CAN_InitData.CAN_RT = CAN_RT_Always;
    if(!dev->FD_mode)
    {
        /* Non-FD mode */
        CAN_InitData.CAN_NBRP = dev->NPRE;
        CAN_InitData.CAN_NSJW = dev->NSJW;
        CAN_InitData.CAN_NProp = dev->NPROP;
        CAN_InitData.CAN_NPS1 = dev->NPS1;
        CAN_InitData.CAN_NPS2 = dev->NPS2;
    }
    else
    {
        /* FD mode */
        CAN_InitData.CAN_NBRP = dev->NPRE;
        CAN_InitData.CAN_NSJW = dev->NSJW;
        CAN_InitData.CAN_NProp = dev->NPROP;
        CAN_InitData.CAN_NPS1 = dev->NPS1;
        CAN_InitData.CAN_NPS2 = dev->NPS2;
        CAN_InitData.CAN_DBRP = dev->DPRE;
        CAN_InitData.CAN_DSJW = dev->DSJW;
        CAN_InitData.CAN_DProp = dev->DPROP;
        CAN_InitData.CAN_DPS1 = dev->DPS1;
        CAN_InitData.CAN_DPS2 = dev->DPS2;
    }
    
    TxMessage.StdId = 1234;
    TxMessage.ExtId = 0;
    TxMessage.IDE = 0;
    TxMessage.RTR = 0;

    CAN_FilterConfig.Filter_StdId = 1234;
    CAN_FilterConfig.Filter_ExtId = 0;
    CAN_FilterConfig.Filter_frameType = CAN_OnlyBased; //this test is focus on the length of data, so they only send the standard id frame. 
    
    CAN_MaskConfig.Mask_StdId = 0;
    CAN_MaskConfig.Mask_ExtId = 0;
    CAN_MaskConfig.Mask_All = 1;
    
    CAN_FilterInit(CAN2, &CAN_FilterConfig, 0);
    CAN_FilterGroup(CAN2, 0, ENABLE);
    CAN_MaskInit(CAN2, &CAN_MaskConfig, 0);
    CAN_MaskNumber(CAN2, 0, ENABLE);
    
    /* Set CAN2 error interrupt */
    irq_setting(IRQ_CAN2, (PrHandler)CAN2Handler);
    CAN_ITConfig(CAN2, CAN_IRE_EIE, ENABLE);
    
    CAN_Init(CAN1, &CAN_InitData);
    CAN_Init(CAN2, &CAN_InitData);

    Generate_Random_Pattern(64, pData);
    
    do
    {
        /* if key the space or enter, the burnin will stop. */
        c = ftuart_getc_t(10000, 0);
        if (c == 'q')
            break;
        
       /* i = 0: Classical CAN, 
          i = 1: CAN FD (no speed change), 
          i = 2: CAN FD (speed change)      */
        for(i = 0; i < (dev->FD_mode ? 3 : 1); i++)
        {
            memset(&TxMessage.Data, 0, 64);
            memset(&RxMessage.Data, 0, 64);
            
            for(DLC_i = 0;DLC_i < 16; DLC_i++)
            {
                TxMessage.FD = (i > 0)?1:0;
                TxMessage.BRS = (i == 2)?1:0;
                TxMessage.DLC = (UINT8)DLC_i;

                if(i == 0)
                {
                    memcpy(TxMessage.Data, pData, DLCtoBytes_table[8]);
                }
                else
                {
                    memcpy(TxMessage.Data, pData, DLCtoBytes_table[DLC_i]);
                }
                
                /* CAN1 transfer message */
                CAN_Transmit_TXn(CAN1, &TxMessage, 0);
                while(CAN_GetBTRStatus(CAN1, 0));
                
                /* CAN2 receive message */
                while(CAN_GetStatusBit(CAN2, CAN_BRS0_STATUS_BIT) == 0);
                CAN_Receive_RXn(CAN2, 0, &RxMessage);
                
                /* Check the message */
                ret = Verify_Pattern(((i == 0)?CAN_A_Data:CAN_FD_A), &TxMessage, &RxMessage);
                
                if(ret != 0)
                {
                    /* if an error occur, dump the current setting(frame type, counter) */
                    
                    return ret;
                }  
            } 
            
            if(i == 0)
            {
                CAN_DBGPRINTF(DBG_LEVEL_1, "\r\nClassical CAN done\r\n");
            }
            else if(i == 1)
            {
                CAN_DBGPRINTF(DBG_LEVEL_1, "CAN FD (no speed change) done\r\n");
            }
            else
                CAN_DBGPRINTF(DBG_LEVEL_1, "CAN FD (speed change) done\r\n");
        }
        
        counter++;
        CAN_DBGPRINTF(DBG_LEVEL_1, "\r%08d", counter);
    }while(dev->FreeRun);
    
    free(pData);

    return ret;
}


/*******************************************************************************
                            Bit timing test functions
*******************************************************************************/
int CAN_Bit_Timing_Classical_test1(UINT32 bitrate, float SPP, char opt1, char opt2, char opt3)
{
    CAN_InitTypeDef CAN_InitData;
    CAN_FilterTypeDef CAN_FilterConfig;
    CAN_MaskTypeDef CAN_MaskConfig;
    CanTxMsgDef TxMessage;
    CanRxMsgDef RxMessage;
    int i, j, ret, counter, tqs_upper, sjw_upper;

    UINT32 CANCLK, TQs;
    UINT8 Prop, PS1, PS2, segment1;
    float SPP_low, SPP_up;

    UINT8 data[8] = {0x07, 0xc1, 0xf0, 0x7c, 0x1f, 0x07, 0xc1, 0xf0};
    counter = 0;
    
    /* Option for the maximun Tq number is 25 or 129 */
    if(opt1 == 'Y')
        tqs_upper = 25;
    else
        tqs_upper = 129;
    
    /* Clear these structs value */
    memset(&CAN_InitData, 0, sizeof(CAN_InitTypeDef));
    memset(&CAN_FilterConfig, 0, sizeof(CAN_FilterTypeDef));
    memset(&CAN_MaskConfig, 0, sizeof(CAN_MaskTypeDef));
    memset(&TxMessage, 0, sizeof(CanTxMsgDef));
    memset(&RxMessage, 0, sizeof(CanRxMsgDef));
    
    CAN_InitData.CAN_Mode = CAN_Mode_Normal;
    CAN_InitData.CAN_RT = CAN_RT_1time;
    
    TxMessage.StdId = 1234;
    TxMessage.ExtId = 0;
    TxMessage.IDE = 0;
    TxMessage.RTR = 0;
    TxMessage.BRS = 0;
    TxMessage.FD = 0;
    TxMessage.DLC = CAN_DataBytes_8;
    memcpy(TxMessage.Data, &data, 8);
    
    CAN_FilterConfig.Filter_StdId = 1234;
    CAN_FilterConfig.Filter_ExtId = 0;
    CAN_FilterConfig.Filter_frameType = CAN_ALL;
    
    CAN_MaskConfig.Mask_StdId = 0;
    CAN_MaskConfig.Mask_ExtId = 0;
    CAN_MaskConfig.Mask_All = 1;
    
    /* Reset the CAN1 and CAN2 */
    CAN_Reset(CAN1);
    CAN_Reset(CAN2);
    
    /* Set interrupt handler and enable interrupt */
    irq_setting(IRQ_CAN1, (PrHandler)CAN1Handler);
    CAN_ITConfig(CAN1, CAN_IRE_EIE, ENABLE);
    irq_setting(IRQ_CAN2, (PrHandler)CAN2Handler);
    CAN_ITConfig(CAN2, CAN_IRE_EIE, ENABLE);
    
    /* Set CAN2 filter and mask */
    CAN_FilterInit(CAN2, &CAN_FilterConfig, 0);
    CAN_FilterGroup(CAN2, 0, ENABLE);
    CAN_MaskInit(CAN2, &CAN_MaskConfig, 0);
    CAN_MaskNumber(CAN2, 0, ENABLE);
    
    /**
     * Due to the Prescaler(i) 2-255 is too large, we consider the total number of
     * tq and bitrate from argument to reduce the prescaler range.
     * Equation: 
     *     The total number of Tq in a bit * bitrate(bps) = system clock(Hz) * 1/Prescaler
     */
    for(i = (CAN_PCLK / (tqs_upper * bitrate)); i <= min_t((CAN_PCLK / (8 * bitrate)), 255); i++)
    {
        /* the prescaler should not be less than 2 */
        if(i <= 1)
            continue;

        /* check the number of tq should be integer number */
        CANCLK = CAN_PCLK/i;
        if((CANCLK % bitrate) != 0)
            continue;
        
        TQs = CANCLK / bitrate;
        
        /* Exclude combinations of non-integer tq number for sampling point */
        if(((TQs * SPP) - (int)(TQs * SPP)) != 0)
                continue;

        /* calculate the PS2 segment */
        PS2 = TQs - (UINT8)(TQs * SPP);
        
        /* PS2 must not be less than IPT(2) */
        if(PS2 < IPT_Tq)
            continue;
        
        /* Option for only present maximun SJW or not */
        if(opt3 == 'Y')
            sjw_upper = PS2;
        else
            sjw_upper = 1;
        
        /* Option for 'PS2 = PS1' or not */
        if(opt2 == 'Y')
        {
            /* calculate the PS1 segment */
            PS1 = PS2; //PS2 = max(PS1, IPT)
            Prop = TQs - Sync_seg - PS1 - PS2;

            /* Exclude combinations of propagation segment more than 64 and propagation segment less than 0 */
            if((Prop < 1) || (Prop > 64))
                continue;
            
            /* The SJW range is 1 to min(PS2, 32) */
            for(j = sjw_upper; j <= min_t(PS2, 32); j++)
            {
                /* dump the setting info in this round */
                CAN_DBGPRINTF(DBG_LEVEL_1, "%d <Prescaler: %d. Prop_seg: %d. Phase_seg1: %d. Phase_seg2: %d. SJW: %d.>\n", counter, i, Prop, PS1, PS2, j);
                    
                /* Set CAN bit timing */
                CAN_InitData.CAN_NBRP = i;
                CAN_InitData.CAN_NSJW = j;
                CAN_InitData.CAN_NProp = Prop;
                CAN_InitData.CAN_NPS1 = PS1;
                CAN_InitData.CAN_NPS2 = PS2;
                    
                /* Set configuration mode */
                CAN_Mode_Change(CAN1, CAN_Mode_Config);
                CAN_Mode_Change(CAN2, CAN_Mode_Config);
                    
                /* Initial CAN1 and CAN2 and go to normal mode */
                CAN_Init(CAN1, &CAN_InitData);
                CAN_Init(CAN2, &CAN_InitData);
                    
                /* transfer message */
                CAN_Transmit_TXn(CAN1, &TxMessage, 0);
                while(CAN_GetBTRStatus(CAN1, 0));
                    
                /* Receive message */
                while(CAN_GetStatusBit(CAN2, CAN_BRS0_STATUS_BIT) == 0);
                CAN_Receive_RXn(CAN2, 0, &RxMessage);
                    
                /* Verify message */
                ret = Verify_Pattern(CAN_A_Data, &TxMessage, &RxMessage);
                if(ret != 0)
                    return ret;
                    
                counter++;

                // CAN_DBGPRINTF(DBG_LEVEL_1, "\t0x60: 0x%08x, 0x64: 0x%08x, 0x68: 0x%08x\n", 
                              // CAN2->CAN_Bit_Timing.Nominal_L, 
                              // CAN2->CAN_Bit_Timing.Nominal_H, 
                              // CAN2->CAN_Bit_Timing.Data);
            }
        }
        else
        {
            segment1 = TQs - Sync_seg - PS2;
            if(segment1 > (tqs_upper - PS2 - Sync_seg))
                continue;
            
            for(PS1 = IPT_Tq; PS1 < min_t(segment1 - 1, 32); PS1++)
            {
                Prop = segment1 - PS1;
                
                if((Prop < 1) || (Prop > 64))
                    continue;
            
                if((Sync_seg+ Prop + PS1 + PS2) != TQs)
                    continue;
            
                /* The SJW range is 1 to min(PS2, 32) */
                for(j = sjw_upper; j <= min_t(PS2, 32); j++)
                {
                    /* dump the setting info in this round */
                    CAN_DBGPRINTF(DBG_LEVEL_1, "%d <Prescaler: %d. Prop_seg: %d. Phase_seg1: %d. Phase_seg2: %d. SJW: %d.>\n", counter, i, Prop, PS1, PS2, j);
                        
                    /* Set CAN bit timing */
                    CAN_InitData.CAN_NBRP = i;
                    CAN_InitData.CAN_NSJW = j;
                    CAN_InitData.CAN_NProp = Prop;
                    CAN_InitData.CAN_NPS1 = PS1;
                    CAN_InitData.CAN_NPS2 = PS2;
                        
                    /* Set configuration mode */
                    CAN_Mode_Change(CAN1, CAN_Mode_Config);
                    CAN_Mode_Change(CAN2, CAN_Mode_Config);
                        
                    /* Initial CAN1 and CAN2 and go to normal mode */
                    CAN_Init(CAN1, &CAN_InitData);
                    CAN_Init(CAN2, &CAN_InitData);
                        
                    /* transfer message */
                    CAN_Transmit_TXn(CAN1, &TxMessage, 0);
                    while(CAN_GetBTRStatus(CAN1, 0));
                        
                    /* Receive message */
                    while(CAN_GetStatusBit(CAN2, CAN_BRS0_STATUS_BIT) == 0);
                    CAN_Receive_RXn(CAN2, 0, &RxMessage);
                        
                    /* Verify message */
                    ret = Verify_Pattern(CAN_A_Data, &TxMessage, &RxMessage);
                    if(ret != 0)
                        return ret;
                        
                    counter++;

                    // CAN_DBGPRINTF(DBG_LEVEL_1, "\t0x60: 0x%08x, 0x64: 0x%08x, 0x68: 0x%08x\n", 
                                  // CAN2->CAN_Bit_Timing.Nominal_L, 
                                  // CAN2->CAN_Bit_Timing.Nominal_H, 
                                  // CAN2->CAN_Bit_Timing.Data);
                }
            }
        }
    }
    
    if(counter > 0)
    {
        CAN_DBGPRINTF(DBG_LEVEL_1, "The total number of PASS setting is %d!\n", counter);
    }
    else
    {
        CAN_DBGPRINTF(DBG_LEVEL_1, "No setting exist in %dbps at %.3f%%\n", bitrate, SPP);
    }
		return 0;
}

int CAN_Bit_Timing_Classical_test2()
{
    CAN_InitTypeDef CAN_InitData;
    CAN_FilterTypeDef CAN_FilterConfig;
    CAN_MaskTypeDef CAN_MaskConfig;
    CanTxMsgDef TxMessage;
    CanRxMsgDef RxMessage;
    UINT32 prescaler_i, tq_i, prop, ps1_i, ps2, sjw_i;
    UINT8 data[8] = {0x07, 0xc1, 0xf0, 0x7c, 0x1f, 0x07, 0xc1, 0xf0};
    int bps, ret, timeout, counter, total, errCnt;
    int limit_1;
    
    /* Clear these structs value */
    memset(&CAN_InitData, 0, sizeof(CAN_InitTypeDef));
    memset(&CAN_FilterConfig, 0, sizeof(CAN_FilterTypeDef));
    memset(&CAN_MaskConfig, 0, sizeof(CAN_MaskTypeDef));
    memset(&TxMessage, 0, sizeof(CanTxMsgDef));
    memset(&RxMessage, 0, sizeof(CanRxMsgDef));
    
    total = 1;
    errCnt = 0;
    
    /* Some limit options */
    fLib_printf("Can the speed be smaller than 125Kps or larger than 1Mbps in arbitration phase?\n yes(1) no(0): ");
    scanf("%d", &limit_1);
    fLib_printf("\n");
    
    CAN_InitData.CAN_Mode = CAN_Mode_Normal;
    CAN_InitData.CAN_RT = CAN_RT_1time;
    
    TxMessage.StdId = 31;
    TxMessage.ExtId = 7943;
    TxMessage.IDE = 1;
    TxMessage.RTR = 0;
    TxMessage.BRS = 0;
    TxMessage.FD = 0;
    TxMessage.DLC = CAN_DataBytes_8;
    memcpy(TxMessage.Data, &data, 8);
    
    CAN_FilterConfig.Filter_StdId = 31;
    CAN_FilterConfig.Filter_ExtId = 7943;
    CAN_FilterConfig.Filter_frameType = CAN_ALL;
    
    CAN_MaskConfig.Mask_StdId = 0;
    CAN_MaskConfig.Mask_ExtId = 0;
    CAN_MaskConfig.Mask_All = 1;
    
    
    /* Set CAN2 filter and mask */
    CAN_FilterInit(CAN2, &CAN_FilterConfig, 0);
    CAN_FilterGroup(CAN2, 0, ENABLE);
    CAN_MaskInit(CAN2, &CAN_MaskConfig, 0);
    CAN_MaskNumber(CAN2, 0, ENABLE);
    
    /* Set interrupt handler and enable interrupt */
    irq_setting(IRQ_CAN1, (PrHandler)CAN1Handler);
    CAN_ITConfig(CAN1, CAN_IRE_EIE, ENABLE);
    irq_setting(IRQ_CAN2, (PrHandler)CAN2Handler);
    CAN_ITConfig(CAN2, CAN_IRE_EIE, ENABLE);
    
    /* Run the rules in the Bosch CAN spec */
    /* Due to our prescaler cannot be set 1, the prescaler in the for loop is start at 2. */
    for(prescaler_i = 2; prescaler_i <= 255; prescaler_i++)
    {
        /* The range of Tq is 8~25 */
        for(tq_i = 8; tq_i < 26; tq_i++)
        {
            bps = CAN_PCLK/(prescaler_i*tq_i);

            /* The speed for classical CAN should be btween 125Kbps and 1Mbps */
            if((limit_1) && (bps > 1000000 || bps < 125000))
            {
               continue;
            }
            
            /* The phase segment 1 (ps1_i) is run from 8 or tq_i/2-1 and decreasement.
               This is because PS2 and PS1 are basically the same, unless PS2 is 
               equal to IPT when PS1 < IPT. */
            for(ps1_i = (tq_i < 18)?(tq_i/2 - 1):8; ps1_i > 0; ps1_i--)
            {
                /* PS2 = MAX(PS1, IPT) */
                ps2 = max_t(ps1_i,IPT_Tq);
                /* prop = tq_i(The number of TQ) - ps1_i(PS1) - ps2(PS2) - 1(sync_seg)  */
                prop = tq_i - ps1_i - ps2 - Sync_seg;
                
                /* If prop_seg is more than 8, the ps1_i should be smaller. */
                if(prop > 8)
                {
                    break;
                }
                
                /* SJW is between 1 and min(PS1, 4) */
                for(sjw_i = 1; sjw_i <= min_t(ps1_i,4); sjw_i++)
                {
                    /* dump the setting info in this round */
                    // CAN_DBGPRINTF(DBG_LEVEL_1, "\r\n==================================================\n");
                    // CAN_DBGPRINTF(DBG_LEVEL_1, "Speed:%dbps. SPP=%f%%\nTime Quanta:%d\n ", \
                                               // bps, ((float)100*(Sync_seg+prop+ps1_i))/tq_i, tq_i);
                    // CAN_DBGPRINTF(DBG_LEVEL_1, "Prescaler:%d.\n(Sync,Prop,PS1,PS2) = ", \
                                               // prescaler_i);
                    // CAN_DBGPRINTF(DBG_LEVEL_1, "(1,%d,%d,%d) SJW:%d.\n", \
                                               // prop, ps1_i, ps2, sjw_i);
                    
                    
                    /* Set CAN bit timing */
                    CAN_InitData.CAN_NBRP = prescaler_i;
                    CAN_InitData.CAN_NSJW = sjw_i;
                    CAN_InitData.CAN_NProp = prop;
                    CAN_InitData.CAN_NPS1 = ps1_i;
                    CAN_InitData.CAN_NPS2 = ps2;
                    
                    /* Reset the CAN1 and CAN2 */
                    CAN_Reset(CAN1);
                    CAN_Reset(CAN2);
                    
                    /* Initial CAN1 and CAN2 and go to normal mode */
                    CAN_Init(CAN1, &CAN_InitData);
                    CAN_Init(CAN2, &CAN_InitData);
                    
                    counter = 1;
                    while(counter <= 30)
                    {
                        /* transfer message */
                        CAN_Transmit_TXn(CAN1, &TxMessage, 0);
                        while(CAN_GetBTRStatus(CAN1, 0));
                        
                        /* Receive message */
                        timeout = 5000;
                        while(CAN_GetStatusBit(CAN2, CAN_BRS0_STATUS_BIT) == 0 && timeout > 0 )
                        {
                            timeout--;
                        }
                        
                        if(timeout)
                        {
                            /* no timeout and receive the data */
                            CAN_Receive_RXn(CAN2, 0, &RxMessage);
                            //CAN_DBGPRINTF(DBG_LEVEL_1, "\r%03d times PASS!", counter); //為了減少dump出來的資訊，所以直接三十次成功再印
                            
                            /* Verify message */
                            ret =  Verify_Pattern(CAN_B_Data, &TxMessage, &RxMessage);
                            if(ret != 0)
                                return ret;
                        }
                        else
                        {
                            errCnt++;
                            CAN_DBGPRINTF(DBG_LEVEL_1, "\r\n==================================================\n");
                            if(bps > 1000000 || bps < 125000)
                            {
                                CAN_DBGPRINTF(DBG_LEVEL_1, "*"); //If out of range 125K~1M, take a '*' for mark.
                            }
                            else
                            {
                                CAN_DBGPRINTF(DBG_LEVEL_1, "+"); //Mark '+' for a legal case
                            }
                            CAN_DBGPRINTF(DBG_LEVEL_1, "Speed:%dbps. SPP=%f%%\nTime Quanta:%d\n", \
                                                       bps, ((float)100*(Sync_seg+prop+ps1_i))/tq_i, tq_i);
                            CAN_DBGPRINTF(DBG_LEVEL_1, "Prescaler:%d.\n(Sync,Prop,PS1,PS2) = ", \
                                                       prescaler_i);
                            CAN_DBGPRINTF(DBG_LEVEL_1, "(1,%d,%d,%d) SJW:%d.\n", \
                                                       prop, ps1_i, ps2, sjw_i);

                            CAN_DBGPRINTF(DBG_LEVEL_1, "0x60: 0x%08x\n", CAN2->CAN_Bit_Timing.Nominal_L);
                            CAN_DBGPRINTF(DBG_LEVEL_1, "0x64: 0x%08x\n", CAN2->CAN_Bit_Timing.Nominal_H);
                            CAN_DBGPRINTF(DBG_LEVEL_1, "0x68: 0x%08x\n", CAN2->CAN_Bit_Timing.Data);

                            CAN_DBGPRINTF(DBG_LEVEL_1, "\r\nFAIL! %d\n", counter);
                            
                            break;
                        }
                        counter++;
                    }
                    
                    if(timeout)
                    {
                        //CAN_DBGPRINTF(DBG_LEVEL_1, "30 times PASS!\n", counter);
                    }
                    total++;
                }
            }
        }
    }
    CAN_DBGPRINTF(DBG_LEVEL_1, "Total cases: %d (%d are error)\n", total, errCnt);
    return 0;
}

/* This function suppout the maximun Tq is 129 */
int CAN_Bit_Timing_Classical_test3()
{
    CAN_InitTypeDef CAN_InitData;
    CAN_FilterTypeDef CAN_FilterConfig;
    CAN_MaskTypeDef CAN_MaskConfig;
    CanTxMsgDef TxMessage;
    CanRxMsgDef    RxMessage;
    UINT32 prescaler_i, tq_i, prop, ps1_i, ps2, sjw_i;
    UINT8 data8[8] = {0x07, 0xc1, 0xf0, 0x7c, 0x1f, 0x07, 0xc1, 0xf0};
    UINT8 data64[64] = {0x07, 0xc1, 0xf0, 0x7c, 0x1f, 0x07, 0xc1, 0xf0, \
                      0x7c, 0x1f, 0x07, 0xc1, 0xf0, 0x7c, 0x1f, 0x07, \
                      0xc1, 0xf0, 0x7c, 0x1f, 0x07, 0xc1, 0xf0, 0x7c, \
                      0x1f, 0x07, 0xc1, 0xf0, 0x7c, 0x1f, 0x07, 0xc1, \
                      0xf0, 0x7c, 0x1f, 0x07, 0xc1, 0xf0, 0x7c, 0x1f, \
                      0x07, 0xc1, 0xf0, 0x7c, 0x1f, 0x07, 0xc1, 0xf0, \
                      0x7c, 0x1f, 0x07, 0xc1};
    int ret, timeout, counter;
    long long total, errCnt;
    float delay_t;
    int bps;
    int limit_1, opt_ft, limit_ns;
    
    /* Clear these structs value */
    memset(&CAN_InitData, 0, sizeof(CAN_InitTypeDef));
    memset(&CAN_FilterConfig, 0, sizeof(CAN_FilterTypeDef));
    memset(&CAN_MaskConfig, 0, sizeof(CAN_MaskTypeDef));
    memset(&TxMessage, 0, sizeof(CanTxMsgDef));
    memset(&RxMessage, 0, sizeof(CanRxMsgDef));
    
    total = 1;
    errCnt = 0;
    
    /* Some limit options */
    fLib_printf("Can the speed be smaller than 125Kps or larger than 1Mbps in arbitration phase?\n yes(1) no(0): ");
    scanf("%d", &limit_1);
    fLib_printf("\nChoose frame type:\n");
    fLib_printf("(0) Based data frame (non-FD)\n");
    fLib_printf("(1) Based remote frame (non-FD)\n");
    fLib_printf("(2) Extended data frame (non-FD)\n");
    fLib_printf("(3) Extended remote frame (non-FD)\n");
    fLib_printf("(4) FD based frame\n");
    fLib_printf("(5) FD extended frame\n");
    scanf("%d", &opt_ft);
    fLib_printf("\nInput the limit of delay time (ns) from the transmit point to the sample point: ");
    scanf("%d", &limit_ns);
    //(UNDO)Ask for user the error info should be dumpped or not
    fLib_printf("\n");
    
    CAN_InitData.CAN_Mode = CAN_Mode_Normal;
    CAN_InitData.CAN_RT = CAN_RT_1time;
    
    TxMessage.StdId = 124;
    TxMessage.BRS = 0; //no baud rate switch in this test function
    switch(opt_ft)
    {
        case CAN_A_Data:
            TxMessage.ExtId = 0;
            TxMessage.IDE = 0;
            TxMessage.RTR = 0;
            TxMessage.FD = 0;
            TxMessage.DLC = CAN_DataBytes_8;
            memcpy(TxMessage.Data, &data8, 8);
            break;
        case CAN_A_Remote:
            TxMessage.ExtId = 0;
            TxMessage.IDE = 0;
            TxMessage.RTR = 1;
            TxMessage.FD = 0;
            TxMessage.DLC = CAN_DataBytes_0;
            break;
        case CAN_B_Data:
            TxMessage.ExtId = 230368;
            TxMessage.IDE = 1;
            TxMessage.RTR = 0;
            TxMessage.FD = 0;
            TxMessage.DLC = CAN_DataBytes_8;
            memcpy(TxMessage.Data, &data8, 8);
            break;
        case CAN_B_Remote:
            TxMessage.ExtId = 230368;
            TxMessage.IDE = 1;
            TxMessage.RTR = 1;
            TxMessage.FD = 0;
            TxMessage.DLC = CAN_DataBytes_0;
            break;
        case CAN_FD_A:
            TxMessage.ExtId = 0;
            TxMessage.IDE = 0;
            TxMessage.RTR = 0;
            TxMessage.FD = 1;
            TxMessage.DLC = CAN_DataBytes_64;
            memcpy(TxMessage.Data, &data64, 64);
            break;
        case CAN_FD_B:
            TxMessage.ExtId = 230368;
            TxMessage.IDE = 1;
            TxMessage.RTR = 0;
            TxMessage.FD = 1;
            TxMessage.DLC = CAN_DataBytes_64;
            memcpy(TxMessage.Data, &data64, 64);
            break;
    }
    
    /* Set CAN2 filter and mask */
    CAN_FilterConfig.Filter_StdId = 0;
    CAN_FilterConfig.Filter_ExtId = 0;
    CAN_FilterConfig.Filter_frameType = CAN_ALL;
    CAN_MaskConfig.Mask_StdId = 0;
    CAN_MaskConfig.Mask_ExtId = 0;
    CAN_MaskConfig.Mask_All = 1;
    CAN_FilterInit(CAN2, &CAN_FilterConfig, 0);
    CAN_FilterGroup(CAN2, 0, ENABLE);
    CAN_MaskInit(CAN2, &CAN_MaskConfig, 0);
    CAN_MaskNumber(CAN2, 0, ENABLE);
    
    /* Set interrupt handler and enable interrupt */
    irq_setting(IRQ_CAN1, (PrHandler)CAN1Handler);
    CAN_ITConfig(CAN1, CAN_IRE_EIE, ENABLE);
    irq_setting(IRQ_CAN2, (PrHandler)CAN2Handler);
    CAN_ITConfig(CAN2, CAN_IRE_EIE, ENABLE);
    
    /* Run the prescaler form 2 to 255 and compatible with Bosch spec. */
    for(prescaler_i = 2; prescaler_i <= 255; prescaler_i++)
    {
        /* The number of Tq is 8~129(1+64+32+32) */
        for(tq_i = 8; tq_i < 130; tq_i++)
        {
            bps = CAN_PCLK/(prescaler_i*tq_i);
            /* The speed for classical CAN should be btween 125Kbps and 1Mbps */
            if((limit_1 == 0) && (bps > 1000000 || bps < 125000))
            {
                continue;
            }
            
            /* If the number of Tq is more than 66 (1+1+32+32), the phase 
               segment 1 (ps1_i) will decreased from (tq_i/2 - 1), otherwise,
               the it will decreased from 32. */
            for(ps1_i = (tq_i < 66)?(tq_i/2 - 1):32; ps1_i > 0; ps1_i--)
            {
                /* PS2 = MAX(PS1, IPT) */
                ps2 = max_t(ps1_i,IPT_Tq);
                /* prop = tq_i(The number of Tq) - ps1_i(PS1) - ps2(PS2) - 1(sync_seg)  */
                prop = tq_i - ps1_i - ps2 - Sync_seg;
                
                /* If prop_seg is more than 64, it means the ps1_i need to be 
                   decreased. */
                if(prop > 64 || prop < 1)
                {
                    break;
                }
                
                /* SJW's range is from 1 to min(PS1, 32) 
                   In Bosch spec, the SJW is up to 16 tqs, but there has 32 tqs 
                   for SJW in our design. */
                for(sjw_i = 1; sjw_i <= min_t(ps1_i, 32); sjw_i++)
                {
                    // if(sjw_i <= 16) //TEST for Connor that the NSJW > 16 cases
                        // continue;
                    
                    /* dump the setting info in this round */
                    CAN_DBGPRINTF(DBG_LEVEL_1, "==================================================\n");
                    CAN_DBGPRINTF(DBG_LEVEL_1, "Speed:%dbps. SPP=%f%%\nTime Quanta:%d\n", \
                                               bps, ((float)100*(Sync_seg+prop+ps1_i))/tq_i, tq_i);
                    CAN_DBGPRINTF(DBG_LEVEL_1, "Prescaler:%d\n(Sync,Prop,PS1,PS2) = ", \
                                               prescaler_i);
                    CAN_DBGPRINTF(DBG_LEVEL_1, "(1,%d,%d,%d) SJW:%d.\n", \
                                               prop, ps1_i, ps2, sjw_i);
                    
                    /* Consider the transciver delay from a SOF to sample point 
                       must not be less than 150ns in our design. */
                    delay_t = (float)(1000000000/ CAN_PCLK) * prescaler_i * tq_i * (Sync_seg + prop + ps1_i);
                    if(delay_t < limit_ns)
                    {
                        CAN_DBGPRINTF(DBG_LEVEL_1, "The delay time of this set is %.0fns! Less than %dns\n", delay_t, limit_ns);
                        //break;
                    }
                    
                    /* Set CAN bit timing */
                    CAN_InitData.CAN_NBRP = prescaler_i;
                    CAN_InitData.CAN_NSJW = sjw_i;
                    CAN_InitData.CAN_NProp = prop;
                    CAN_InitData.CAN_NPS1 = ps1_i;
                    CAN_InitData.CAN_NPS2 = ps2;
                    
                    /* Reset the CAN1 and CAN2 */
                    CAN_Reset(CAN1);
                    CAN_Reset(CAN2);
                    
                    /* Initial CAN1 and CAN2 and go to normal mode */
                    CAN_Init(CAN1, &CAN_InitData);
                    CAN_Init(CAN2, &CAN_InitData);
                    
                    counter = 1;
                    while(counter <= 30)
                    {
                        /* transfer message */
                        CAN_Transmit_TXn(CAN1, &TxMessage, 0);
                        while(CAN_GetBTRStatus(CAN1, 0));
                        
                        /* Receive message */
                        timeout = 5000;
                        while(CAN_GetStatusBit(CAN2, CAN_BRS0_STATUS_BIT) == 0 && timeout > 0 )
                        {
                            timeout--;
                        }
                        
                        if(timeout)
                        {
                            /* no timeout and receive the data */
                            CAN_Receive_RXn(CAN2, 0, &RxMessage);
                            //CAN_DBGPRINTF(DBG_LEVEL_1, "\r%03d times PASS!", counter); //為了減少dump出來的資訊，所以直接三十次成功再印
                            
                            /* Verify message */
                            ret =  Verify_Pattern((Frame_Type)opt_ft, &TxMessage, &RxMessage);
                            if(ret != 0)
                                return ret;
                            
                        }
                        else
                        {
                            errCnt++;
                            CAN_DBGPRINTF(DBG_LEVEL_1, "\r\n==================================================\n");
                            if(bps > 1000000 || bps < 125000)
                            {
                                CAN_DBGPRINTF(DBG_LEVEL_1, "*"); //If the speed is out of 125K~1M, mark it '*'.
                            }
                            else
                            {
                                CAN_DBGPRINTF(DBG_LEVEL_1, "+"); //If this setting is legal, mark it '+'.
                            }
                            // CAN_DBGPRINTF(DBG_LEVEL_1, "Speed:%dbps. SPP=%f%%\nTime Quanta:%d\n", \
                                                       // bps, ((float)100*(Sync_seg+prop+ps1_i))/tq_i, tq_i);
                            // CAN_DBGPRINTF(DBG_LEVEL_1, "Prescaler:%d.\n(Sync,Prop,PS1,PS2) = ", \
                                                       // prescaler_i);
                            // CAN_DBGPRINTF(DBG_LEVEL_1, "(1,%d,%d,%d) SJW:%d.\n", \
                                                       // prop, ps1_i, ps2, sjw_i);

                            CAN_DBGPRINTF(DBG_LEVEL_1, "0x60: 0x%08x\n", CAN2->CAN_Bit_Timing.Nominal_L);
                            CAN_DBGPRINTF(DBG_LEVEL_1, "0x64: 0x%08x\n", CAN2->CAN_Bit_Timing.Nominal_H);
                            CAN_DBGPRINTF(DBG_LEVEL_1, "0x68: 0x%08x\n", CAN2->CAN_Bit_Timing.Data);

                            CAN_DBGPRINTF(DBG_LEVEL_1, "\r\nFAIL! %d\n", counter);
                            
                            //break;
                        }
                        counter++;
                    }
                    
                    if(timeout)
                    {
                        CAN_DBGPRINTF(DBG_LEVEL_1, "30 times transmissions in this set PASS!\n", counter);
                    }
                    total++;
                }
            }
        }
    }
    CAN_DBGPRINTF(DBG_LEVEL_1, "Total cases: %lld (%lld are error)\n", total, errCnt);
    return 0;
}

int CAN_Bit_Timing_FD_test1(UINT32 bitrate1, UINT32 bitrate2, float SPP1, float SPP2, char opt1, char opt2, char opt3)
{
    CAN_InitTypeDef CAN_InitData;
    CAN_FilterTypeDef CAN_FilterConfig;
    CAN_MaskTypeDef CAN_MaskConfig;
    CanTxMsgDef TxMessage;
    CanRxMsgDef RxMessage;
    int PRSCLR1_i, PRSCLR2_i, SJW_1_i, SJW_2_i;
    int CANCLK1, TQs_1, PS1_1, PS2_1, Prop_1;
    int CANCLK2, TQs_2, PS1_2, PS2_2, Prop_2; 
    int ret, counter = 0, segment1, sjw_upper;
    
    UINT8 data[8] = {0x07, 0xc1, 0xf0, 0x7c, 0x1f, 0x07, 0xc1, 0xf0};
    
    /* Clear these structs value */
    memset(&CAN_InitData, 0, sizeof(CAN_InitTypeDef));
    memset(&CAN_FilterConfig, 0, sizeof(CAN_FilterTypeDef));
    memset(&CAN_MaskConfig, 0, sizeof(CAN_MaskTypeDef));
    memset(&TxMessage, 0, sizeof(CanTxMsgDef));
    memset(&RxMessage, 0, sizeof(CanRxMsgDef));
    
    CAN_InitData.CAN_Mode = CAN_Mode_Normal;
    CAN_InitData.CAN_RT = CAN_RT_Always;
    
    TxMessage.StdId = 1234;
    TxMessage.ExtId = 0;
    TxMessage.IDE = 0;
    TxMessage.RTR = 0;
    TxMessage.BRS = 1;
    TxMessage.FD = 1;
    TxMessage.DLC = CAN_DataBytes_8;
    memcpy(TxMessage.Data, &data, 8);
    
    CAN_FilterConfig.Filter_StdId = 1234;
    CAN_FilterConfig.Filter_ExtId = 0;
    CAN_FilterConfig.Filter_frameType = CAN_ALL;
    
    CAN_MaskConfig.Mask_StdId = 0;
    CAN_MaskConfig.Mask_ExtId = 0;
    CAN_MaskConfig.Mask_All = 1;
    
    /* Reset the CAN1 and CAN2 */
    CAN_Reset(CAN1);
    CAN_Reset(CAN2);
    
    /* Set CAN2 filter and mask */
    CAN_FilterInit(CAN2, &CAN_FilterConfig, 0);
    CAN_FilterGroup(CAN2, 0, ENABLE);
    CAN_MaskInit(CAN2, &CAN_MaskConfig, 0);
    CAN_MaskNumber(CAN2, 0, ENABLE);
    
    /* In the arbitration phase, for reducing the impossible prescaler(N) cases,
       we use the below equation1(1) to calulate the initial value of prescaler(N).
       Equation(1): The number of Tq * bitrate(bps) = system clock(Hz) * 1/Presacler
       => Presacler = system clock(Hz) / (The number of Tq * bitrate(bps)) 
       Especially the number of Tq in our design is 8 to 129, so we can get an 
       equation(2): (system clock(Hz) / 129 * bitrate1) <= Presacler <= (system clock(Hz) / 8 * bitrate1) */
    for(PRSCLR1_i = (CAN_PCLK/(129 * bitrate1)); PRSCLR1_i <= min_t((CAN_PCLK/(8 * bitrate1)), 255); PRSCLR1_i++)
    {
        /* the prescaler should not be less than 2 */
        if(PRSCLR1_i <= 1)
            continue;
        
        /* check the number of tq should be integer number */
        CANCLK1 = CAN_PCLK/PRSCLR1_i;
        if((CANCLK1%bitrate1) != 0)
            continue;
        
        TQs_1 = CANCLK1/bitrate1;
        
        /* Exclude combinations of non-integer tq number for sampling point */
        if(((TQs_1 * SPP1) - (int)(TQs_1 * SPP1)) != 0)
                continue;
        
        /* calculate the NPS2 segment */
        PS2_1 = TQs_1 - (UINT32)(TQs_1 * SPP1);
        
        /* NPS2 must not be less than IPT(2) */
        if(PS2_1 < IPT_Tq)
            continue;
    
        /* calculate the NPS1 segment and NProp segment */
        PS1_1 = PS2_1;
        Prop_1 = TQs_1 - Sync_seg - PS1_1 - PS2_1;

        /* Exclude combinations of NPROP more than 64 and propagation segment less than 0 */
        if((Prop_1 < 1) || (Prop_1 > 64))
            continue;
        
        if(opt3 == 'Y')
            sjw_upper = PS2_1;
        else
            sjw_upper = 1;
        
        /* The SJW is 1 to min(PS2, 32) */
        for(SJW_1_i = sjw_upper; SJW_1_i <= min_t(PS2_1, 32); SJW_1_i++)
        {
            /* Nominal bit rate setting */
            CAN_InitData.CAN_NBRP = PRSCLR1_i;
            CAN_InitData.CAN_NSJW = SJW_1_i;
            CAN_InitData.CAN_NProp = Prop_1;
            CAN_InitData.CAN_NPS1 = PS1_1;
            CAN_InitData.CAN_NPS2 = PS2_1;
            
            /* The same reason as prescaler(N) for the prescaler(D), but here 
               the number of Tq is 8 to 25. */
            for(PRSCLR2_i = (CAN_PCLK/(25 * bitrate2)); PRSCLR2_i <= min_t((CAN_PCLK/(8 * bitrate2)), 255); PRSCLR2_i++)
            {
                /* The prescalr(D) cannot be less than 2 */
                if(PRSCLR2_i <= 1)
                    continue;
                
                /* Check this Prescaler(D) has an integer number of TQs at this bit rate. */
                CANCLK2 = CAN_PCLK/PRSCLR2_i;
                if((CANCLK2%bitrate2) != 0)
                {
                    continue;
                }
                
                TQs_2 = CANCLK2/bitrate2;

                /* Exclude combinations of non-integer tq number for sampling point */
                if(((TQs_2 * SPP2) - (int)(TQs_2 * SPP2)) != 0)
                    continue;

                /* After obtaining the total number of Tqs, the lengths of 
                  Prop_1, PS1_1, and PS2_1 are obtained by the input value SPP1. */
                PS2_2 = TQs_2 - (UINT32)(TQs_2 * SPP2);
                if(PS2_2 < IPT_Tq)
                {
                    /* The PS2 cannot be less than IPT(2) */
                    continue;
                }
                
                if(opt2 == 'Y')
                {
                    PS1_2 = PS2_2;
                    Prop_2 = TQs_2 - Sync_seg - PS1_2 - PS2_2;
                    if((Prop_2 < 0) || (Prop_2 > 8))
                    {
                        continue;
                    }
                }
                else
                {
                    segment1 = TQs_2 - Sync_seg - PS2_2;
                    if(segment1 > (25 - PS2_2 - Sync_seg))
                        continue;
                    
                    for(PS1_2 = IPT_Tq; PS1_2 < min_t(segment1 - 1, 8); PS1_2++)
                    {
                        Prop_2 = segment1 - PS1_2;
                        
                        if((Prop_2 < 0) || (Prop_2 > 8))
                            continue;
                    
                        if((Sync_seg+ Prop_2 + PS1_2 + PS2_2) != TQs_2)
                            continue;
                    }
                }
                
                if(opt3 == 'Y')
                    sjw_upper = PS2_2;
                else
                    sjw_upper = 1;
        
                /* SJW is 1 to min(PS2, 4) */
                for(SJW_2_i = sjw_upper; SJW_2_i <= min_t(PS2_2, 4); SJW_2_i++)
                {
                    /* print the current settings */
                    CAN_DBGPRINTF(DBG_LEVEL_1, "\r\nBit rate(Arbitration phase): %d(%.3f%%)\nBit rate(Data phase): %d(%.3f%%)\n", bitrate1, SPP1, bitrate2, SPP2);
                    CAN_DBGPRINTF(DBG_LEVEL_1, "(Phase)     | (PRSCL, Prop,  PS1,  PS2,  SJW)\n", NULL);
                    CAN_DBGPRINTF(DBG_LEVEL_1, "Arbitration | (%5d,%5d,%5d,%5d,%5d)\n", PRSCLR1_i, Prop_1, PS1_1, PS2_1, SJW_1_i);
                    CAN_DBGPRINTF(DBG_LEVEL_1, "Data        | (%5d,%5d,%5d,%5d,%5d)\n", PRSCLR2_i, Prop_2, PS1_2, PS2_2, SJW_2_i);
                    
                    /* Data bit rate setting */
                    CAN_InitData.CAN_DBRP = PRSCLR2_i;
                    CAN_InitData.CAN_DSJW = SJW_2_i;
                    CAN_InitData.CAN_DProp = Prop_2;
                    CAN_InitData.CAN_DPS1 = PS1_2;
                    CAN_InitData.CAN_DPS2 = PS2_2;
                    
                    /* 回到configuration mode */
                    CAN_Mode_Change(CAN1, CAN_Mode_Config);
                    CAN_Mode_Change(CAN2, CAN_Mode_Config);
                    
                    /* Initial CAN1 and CAN2 and go to normal mode */
                    CAN_Init(CAN1, &CAN_InitData);
                    CAN_Init(CAN2, &CAN_InitData);
                    
                    /* transfer message */
                    CAN_Transmit_TXn(CAN1, &TxMessage, 0);
                    
                    while(CAN_GetBTRStatus(CAN1, 0));
                    while(CAN_GetStatusBit(CAN2, CAN_BRS0_STATUS_BIT) == 0);
                    
                    /* Receive message */
                    CAN_Receive_RXn(CAN2, 0, &RxMessage);
                    
                    /* Verify message */
                    ret = Verify_Pattern(CAN_FD_A_BRS, &TxMessage, &RxMessage);
                    if(ret != 0)
                        return ret;

                    counter++;
                }
            }
        }
    }
    
    if(counter > 0)
    {
        CAN_DBGPRINTF(DBG_LEVEL_1, "The total number of PASS setting is %d!\n", counter);
    }
    else
    {
        CAN_DBGPRINTF(DBG_LEVEL_1, "No setting exist.\n");
    }
    
    return 0;
}

int CAN_Bit_Timing_FD_test2()
{
    CAN_InitTypeDef CAN_InitData;
    CAN_FilterTypeDef CAN_FilterConfig;
    CAN_MaskTypeDef CAN_MaskConfig;
    CanTxMsgDef TxMessage;
    CanRxMsgDef    RxMessage;
    int npre_i, ntq_i, nprop, nps1_i, nps2, nsjw_i;
    int dpre_i, dtq_i, dprop, dps1_i, dps2, dsjw_i;
    UINT8 data[64] = {0x07, 0xc1, 0xf0, 0x7c, 0x1f, 0x07, 0xc1, 0xf0, \
                      0x7c, 0x1f, 0x07, 0xc1, 0xf0, 0x7c, 0x1f, 0x07, \
                      0xc1, 0xf0, 0x7c, 0x1f, 0x07, 0xc1, 0xf0, 0x7c, \
                      0x1f, 0x07, 0xc1, 0xf0, 0x7c, 0x1f, 0x07, 0xc1, \
                      0xf0, 0x7c, 0x1f, 0x07, 0xc1, 0xf0, 0x7c, 0x1f, \
                      0x07, 0xc1, 0xf0, 0x7c, 0x1f, 0x07, 0xc1, 0xf0, \
                      0x7c, 0x1f, 0x07, 0xc1};
    int ret, timeout, counter;
    long long errCnt, total;
    int nbps, dbps; 
    int limit_1, limit_2;
    
    /* Clear these structs value */
    memset(&CAN_InitData, 0, sizeof(CAN_InitTypeDef));
    memset(&CAN_FilterConfig, 0, sizeof(CAN_FilterTypeDef));
    memset(&CAN_MaskConfig, 0, sizeof(CAN_MaskTypeDef));
    memset(&TxMessage, 0, sizeof(CanTxMsgDef));
    memset(&RxMessage, 0, sizeof(CanRxMsgDef));
    
    total = 1;
    errCnt = 0;
    
    /* Some limit options */
    fLib_printf("Can the speed be smaller than 125Kps or larger than 1Mbps in arbitration phase?\n yes(1) no(0): ");
    scanf("%d", &limit_1);
    fLib_printf("\nCan the speed of Arbitration phase be faster than the speed of Data phase?\n yes(1) no(0): ");
    scanf("%d", &limit_2);
    fLib_printf("\n");
    
    CAN_InitData.CAN_Mode = CAN_Mode_Normal;
    CAN_InitData.CAN_RT = CAN_RT_1time;
    
    /* In this test, we only use FD Extended data frame to transmission */
    TxMessage.StdId = 124;
    TxMessage.ExtId = 230368;
    TxMessage.IDE = 1;
    TxMessage.RTR = 0;
    TxMessage.BRS = 1;
    TxMessage.FD = 1;
    TxMessage.DLC = CAN_DataBytes_64;
    memcpy(TxMessage.Data, &data, 64);
    
    /* Set CAN2 filter and mask */
    CAN_FilterConfig.Filter_StdId = 124;
    CAN_FilterConfig.Filter_ExtId = 230368;
    CAN_FilterConfig.Filter_frameType = CAN_ALL;
    CAN_MaskConfig.Mask_StdId = 0;
    CAN_MaskConfig.Mask_ExtId = 0;
    CAN_MaskConfig.Mask_All = 1;
    CAN_FilterInit(CAN2, &CAN_FilterConfig, 0);
    CAN_FilterGroup(CAN2, 0, ENABLE);
    CAN_MaskInit(CAN2, &CAN_MaskConfig, 0);
    CAN_MaskNumber(CAN2, 0, ENABLE);
    
    /* Set interrupt handler and enable interrupt */
    irq_setting(IRQ_CAN1, (PrHandler)CAN1Handler);
    CAN_ITConfig(CAN1, CAN_IRE_EIE, ENABLE);
    irq_setting(IRQ_CAN2, (PrHandler)CAN2Handler);
    CAN_ITConfig(CAN2, CAN_IRE_EIE, ENABLE);
    
    /********************** Arbitration (Nominal) phase ***********************/
    
    /* Nominal prescaler from 2 to 255 */
    for(npre_i = 2; npre_i <= 255; npre_i++)
    {
        /* Nominal Tq = 8~129(1+64+32+32) */
        for(ntq_i = 8; ntq_i < 130; ntq_i++)
        {
            nbps = CAN_PCLK/(npre_i*ntq_i);
            
            if((limit_1 == 0) && (nbps > 1000000 || nbps < 125000))
            {
                continue;
            }
            
            for(nps1_i = (ntq_i < 66)?(ntq_i/2 - 1):32; nps1_i > 0; nps1_i--)
            {
                /* PS2(N) = MAX(PS1(N), IPT) */
                nps2 = max_t(nps1_i,IPT_Tq);
                 /* PROP(N) = ntq_i(TQ(N)) - nps1_i(PS1(N)) 
                              - nps2(PS2(N)) - 1(sync_seg)  */
                nprop = ntq_i - nps1_i - nps2 - Sync_seg;
                
                if(nprop > 64 || nprop < 1)
                {
                    break;
                }
                
                /* SJW(N) is 1 to min(PS1(N), 32) */
                for(nsjw_i = 1; nsjw_i <= min_t(nps1_i,32); nsjw_i++)
                {
                    /********************** Data phase ************************/
                    
                    /* Data prescaler from 2 to 255 */
                    for(dpre_i = 2; dpre_i <= 255; dpre_i++)
                    {
                        /* Data Tq = 8~25(1+8+8+8) */
                        for(dtq_i = 8; dtq_i < 26; dtq_i++)
                        {
                            dbps = CAN_PCLK/(dpre_i*dtq_i);
                            
                            if((limit_2 == 0) && (dbps < nbps))
                                continue;
                            
                            for(dps1_i = (dtq_i < 18)?(dtq_i/2):8; dps1_i > 0; dps1_i--)
                            {
                                /* PS2(D) = MAX(PS1(D), IPT) */
                                dps2 = max_t(dps1_i,IPT_Tq);
                                /* PROP(D) = dtq_i(TQ(D)) - dps1_i(PS1(D)) 
                                             - dps2(PS2(D)) - 1(sync_seg)  */
                                dprop = dtq_i - dps1_i - dps2 - Sync_seg;
                                
                                if(dprop > 8)
                                {
                                    break;
                                }
                                
                                if(dprop < 0)
                                {
                                    continue;
                                }   
                                
                                for(dsjw_i = 1; dsjw_i <= min_t(dps1_i,4); dsjw_i++)
                                {
                    
                    
                                    /* Dump the Arbitration phase setting info in this round */
                                    CAN_DBGPRINTF(DBG_LEVEL_1, "\r\n====================(No.%lld)====================\n", total);
                                    CAN_DBGPRINTF(DBG_LEVEL_1, "Arbitration phase:\n%dbps. SPP=%f%%\nTime Quanta(N):%d Prescaler(N):%d\n", \
                                                               nbps, ((float)100*(Sync_seg+nprop+nps1_i))/ntq_i, ntq_i, npre_i);
                                    CAN_DBGPRINTF(DBG_LEVEL_1, "(Sync,Prop,PS1,PS2) = (1,%d,%d,%d) SJW(N):%d.\n", \
                                                               nprop, nps1_i, nps2, nsjw_i);
                                    CAN_DBGPRINTF(DBG_LEVEL_1, "Data phase:\n%dbps. SPP=%f%%\nTime Quanta(D):%d Prescaler(D):%d\n ", \
                                                               dbps, ((float)100*(Sync_seg+dprop+dps1_i))/dtq_i, dtq_i, dpre_i);
                                    CAN_DBGPRINTF(DBG_LEVEL_1, "(Sync,Prop,PS1,PS2) = (1,%d,%d,%d) SJW(D):%d.\n", \
                                                               dprop, dps1_i, dps2, dsjw_i);
                                    
                                    CAN_InitData.CAN_NBRP = npre_i;
                                    CAN_InitData.CAN_NSJW = nsjw_i;
                                    CAN_InitData.CAN_NProp = nprop;
                                    CAN_InitData.CAN_NPS1 = nps1_i;
                                    CAN_InitData.CAN_NPS2 = nps2;
                                    CAN_InitData.CAN_DBRP = dpre_i;
                                    CAN_InitData.CAN_DSJW = dsjw_i;
                                    CAN_InitData.CAN_DProp = dprop;
                                    CAN_InitData.CAN_DPS1 = dps1_i;
                                    CAN_InitData.CAN_DPS2 = dps2;
                                    
                                    /* Reset the CAN1 and CAN2 */
                                    CAN_Reset(CAN1);
                                    CAN_Reset(CAN2);
                                    
                                    /* Initial CAN1 and CAN2 and go to normal mode */
                                    CAN_Init(CAN1, &CAN_InitData);
                                    CAN_Init(CAN2, &CAN_InitData);
                                    
                                    counter = 1;
                                    while(counter <= 30)
                                    {
                                        /* transfer message */
                                        CAN_Transmit_TXn(CAN1, &TxMessage, 0);
                                        while(CAN_GetBTRStatus(CAN1, 0));
                                        
                                        /* Receive message */
                                        timeout = 5000;
                                        while(CAN_GetStatusBit(CAN2, CAN_BRS0_STATUS_BIT) == 0 && timeout > 0 )
                                        {
                                            timeout--;
                                        }
                                        
                                        if(timeout > 0)
                                        {
                                            /* no timeout and receive the data */
                                            CAN_Receive_RXn(CAN2, 0, &RxMessage);
                                            //CAN_DBGPRINTF(DBG_LEVEL_1, "\r%03d times PASS!", counter);
                                            
                                            /* Verify message */
                                            ret =  Verify_Pattern(CAN_FD_B_BRS, &TxMessage, &RxMessage);
                                            if(ret != 0)
                                                return ret;
                                        }
                                        else
                                        {
                                            errCnt++;
                                            CAN_DBGPRINTF(DBG_LEVEL_1, "\r\n\n========No.%08lld FAIL! %d========\n", total, counter);
                                            // CAN_DBGPRINTF(DBG_LEVEL_1, "Arbitration phase:\n%dbps. SPP=%f%%\nTime Quanta(N):%d Prescaler(N):%d\n", \
                                                                       // nbps, ((float)100*(Sync_seg+nprop+nps1_i))/ntq_i, ntq_i, npre_i);
                                            // CAN_DBGPRINTF(DBG_LEVEL_1, "(Sync,Prop,PS1,PS2) = (1,%d,%d,%d) SJW(N):%d.\n", \
                                                                       // nprop, nps1_i, nps2, nsjw_i);
                                            // CAN_DBGPRINTF(DBG_LEVEL_1, "Data phase:\n%dbps. SPP=%f%%\nTime Quanta(D):%d Prescaler(D):%d\n", \
                                                                       // dbps, ((float)100*(Sync_seg+dprop+dps1_i))/dtq_i, dtq_i, dpre_i);
                                            // CAN_DBGPRINTF(DBG_LEVEL_1, "(Sync,Prop,PS1,PS2) = (1,%d,%d,%d) SJW(D):%d.\n", \
                                                                       // dprop, dps1_i, dps2, dsjw_i);

                                            CAN_DBGPRINTF(DBG_LEVEL_1, "0x60: 0x%08x\n", CAN2->CAN_Bit_Timing.Nominal_L);
                                            CAN_DBGPRINTF(DBG_LEVEL_1, "0x64: 0x%08x\n", CAN2->CAN_Bit_Timing.Nominal_H);
                                            CAN_DBGPRINTF(DBG_LEVEL_1, "0x68: 0x%08x\n", CAN2->CAN_Bit_Timing.Data);

                                            break;
                                        }
                                        counter++;
                                    }
                                    
                                    if(timeout > 0)
                                    {
                                        //CAN_DBGPRINTF(DBG_LEVEL_1, "30 times PASS!\n", counter);
                                    }
                                    total++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    CAN_DBGPRINTF(DBG_LEVEL_1, "Total %lld cases (%lld fail)\n", total, errCnt);
    return 0;
}


/*******************************************************************************
                            Special test functions
*******************************************************************************/

/** 
 * This function is for testing ACK Error.
 * Due to there is only one FTCAN node on the bus, the ACK error will occur 
 * after this node issue the frame.
 * Addition to observe an error happened, the transmit error counter (TEC) 
 * should increase by 8 for each ack error. 
 * However, the Bosch spec indirect that the TEC should not be greater than 128. 
 * This function will create serveral ACK errors until TEC is equal to 128, 
 * and test the next ACK error must not make TEC increased.
 */
int CAN_ACKError_test()
{
    CAN_InitTypeDef CAN_InitData;
    CanTxMsgDef TxMessage;
    int i;
    UINT8 TEC, REC;
    
    /* Clear these structs value */
    memset(&CAN_InitData, 0, sizeof(CAN_InitTypeDef));
    memset(&TxMessage, 0, sizeof(CanTxMsgDef));
    
    /* Reset the CAN1 */
    CAN_Reset(CAN1);

    CAN_InitData.CAN_Mode = CAN_Mode_Normal;
    CAN_InitData.CAN_RT = CAN_RT_1time; //only re-transmit once
    CAN_InitData.CAN_NBRP = 5;
    CAN_InitData.CAN_NSJW = 10;
    CAN_InitData.CAN_NProp = 19;
    CAN_InitData.CAN_NPS1 = 10;
    CAN_InitData.CAN_NPS2 = 10;
    
    TxMessage.StdId = 1234;
    TxMessage.ExtId = 0;
    TxMessage.IDE = 0;
    TxMessage.RTR = 1;
    TxMessage.BRS = 0;
    TxMessage.FD = 0;
    TxMessage.DLC = CAN_DataBytes_0;
    
    /* CAN1 interrupt setting */
    irq_setting(IRQ_CAN1, (PrHandler)CAN1Handler);
    CAN_ITConfig(CAN1, CAN_IRE_EIE, ENABLE);
    
    CAN_Init(CAN1, &CAN_InitData);
    
    fLib_printf("Start Testing. Please ensure that no others device is connecting the bus.\nPress any key to continue.");
    getchar(); 
    fLib_printf("\n");
    
    /* Send the 20 frames and each transmit will cause the TEC plus 8.
     * The 12th frames (TEC=12*8=96) will cause the CAN go to Error Warning state.
     * The 16th frames (TEC=16*8=128) will casue the CAN go to Passive Error state.
     */
    for(i = 1; i <= 20; i++)
    {
        CAN_DBGPRINTF(DBG_LEVEL_1, "no.%d frame transfered",i);
        
        /* transfer message first */
        CAN_Transmit_TXn(CAN1, &TxMessage, 0);
        while(CAN_GetBTRStatus(CAN1, 0));
        
        /* Get the TEC and REC */
        REC = CAN_GetReceiveErrorCounter(CAN1);
        
        if(REC != 0)
            CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nThe REC should not be increased for ACK error.\n");

        TEC = CAN_GetTransmitErrorCounter(CAN1);
        //CAN_DBGPRINTF(DBG_LEVEL_1, " (The TEC = %d)\n", TEC);
        
        /* Check Error Warning status bit when TEC is equal to 96 */
        if((i == 12) && (CAN_GetStatusBit(CAN1, CAN_EW_STATUS_BIT) != 1))
            CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nThe Error Warning status bit is wrong!\n");
            
        /* There is no Passive error status bit in FTCAN, so we cannot verify 
         * the CAN has aleady entering passive state after TEC = 128. 
         */
        
        /* Check TEC */
        if(i < 17)
        {
            if(TEC != (i*8))
                CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nTEC is not correct after ACK error occur\n");
        }
        else
        {
            if(TEC != 128)
                CAN_DBGPRINTF(DBG_LEVEL_0, "\r\nTEC should not be increased after the node entered passive state.\n");
        }
    }
    
    fLib_printf("\r\nPlease connect PCAN and transfer data. (500Kbps_75%)\nPress any key to continue.");
    getchar(); 
    fLib_printf("\n");
    
    /* Recover the TEC by PCAN */
    while(TEC != 0)
    {
        /* transfer message first */
        CAN_Transmit_TXn(CAN1, &TxMessage, 0);
        while(CAN_GetBTRStatus(CAN1, 0));
        
        TEC = CAN_GetTransmitErrorCounter(CAN1);
        CAN_DBGPRINTF(DBG_LEVEL_1, "Send the frame and the TEC now is %d\n", TEC);
    }
    
    return 0;
}

int CAN_CRCError_test()
{
    CAN_InitTypeDef CAN_InitData;
    CAN_FilterTypeDef CAN_FilterConfig;
    CAN_MaskTypeDef CAN_MaskConfig;
    CanRxMsgDef    RxMessage;
    UINT8 REC;
    
    /* Clear these structs value */
    memset(&CAN_InitData, 0, sizeof(CAN_InitTypeDef));
    memset(&CAN_FilterConfig, 0, sizeof(CAN_FilterTypeDef));
    memset(&CAN_MaskConfig, 0, sizeof(CAN_MaskTypeDef));
    memset(&RxMessage, 0, sizeof(CanRxMsgDef));
    
    /* Non-FD and FD CAN's bit timing setting */
    CAN_InitData.CAN_Mode = CAN_Mode_Normal;
    CAN_InitData.CAN_RT = CAN_RT_1time;
    /* arbitration phase 500Kbps 75% */
    CAN_InitData.CAN_NBRP = 5;
    CAN_InitData.CAN_NProp = 19;
    CAN_InitData.CAN_NPS1 = 10;
    CAN_InitData.CAN_NPS2 = 10;
    CAN_InitData.CAN_NSJW = 10;
    /* data phase 1Mbps 85% */
    CAN_InitData.CAN_DBRP = 5;
    CAN_InitData.CAN_DProp = 8;
    CAN_InitData.CAN_DPS1 = 8;
    CAN_InitData.CAN_DPS2 = 3;
    CAN_InitData.CAN_DSJW = 3;
    
    /* Filter and mask setting */
    CAN_MaskConfig.Mask_All = 1;
    
    /* Set filter and mask */
    CAN_FilterInit(CAN1, &CAN_FilterConfig, 0);
    CAN_FilterGroup(CAN1, 0, ENABLE);
    CAN_MaskInit(CAN1, &CAN_MaskConfig, 0);
    CAN_MaskNumber(CAN1, 0, ENABLE);
    
    /* Set interrupt handler and enable interrupt */
    irq_setting(IRQ_CAN1, (PrHandler)CAN1Handler);
    CAN_ITConfig(CAN1, CAN_IRE_EIE, ENABLE);
    
    CAN_Init(CAN1, &CAN_InitData);
    
    fLib_printf("Start Testing. Please set the PCAN to transmit ISO-CRC FD frame (500Kbps_75%->1Mbps_85%).\nPress any key to continue.");
    getchar(); 
    fLib_printf("\n");
    
    /* Receiving until the CRC error occur */
    while((CAN_GetLastErrorCode(CAN1) & 0xFF) != CAN_ET_CE);
    
    delay_ms(50);
    
    /* Check REC 
     * Because the PCAN will be re-transmittion always when it sent the frame unsuccessfully.
     * Each ISO CRC frame in PCAN will be caused the Bit Error and PCAN's TEC plus 8.
     * After 32 times (8*32=256), PCAN will be Bus-off state, and FTCAN will not receive more frames. 
     * Therefore, FTCAN's REC will keep in 32 by each receive error plus one.*/
    REC = CAN_GetReceiveErrorCounter(CAN1);
    if(REC != 32)
        CAN_DBGPRINTF(DBG_LEVEL_0, "After a CRC error happened, the REC is wrong\n");
    
    return 0;
}

int CAN_Overrun_case_burnin_test(CAN_Param *dev)
{
    /*  This function test the Rx FIFO will be hanppened overrun or not after the 4th frame coming.
        The 4th frame wont be received in Rx FIFO.
        The test will check FIFO data ,overrun interrupt status bit, data overrun field. 
        The user can enable TOE (overload frame) feature and choose ramdomized frame type.
        
        If the sequential frame type use, frame type will operate in the following order: 
            2.0A_D->2.0A_R->2.0B_D->2.0B_R->FD_A->FD_B->FD_A_BRS->FD_B_BRS
        
        Each FIFO will be test 240 frames, and every four frames will be checked.
        
        Notice that the input CAN_Param parameter should be FD speed.
    */
    CAN_InitTypeDef CAN_InitData;
    CAN_FilterTypeDef CAN_FilterConfig;
    CAN_MaskTypeDef CAN_MaskConfig;
    CanTxMsgDef TxMessage[4];
    CanRxMsgDef    RxMessage;
    int fifo_i, frame_i, frameCnt, FIFOCnt, i, ret, OLF, opt;
    Frame_Type frameType, ftarray[4];
    
    FIFOCnt = 0;
    ret = 0;
    UINT8 data[8] = {0x07, 0xc1, 0xf0, 0x7c, 0x1f, 0x07, 0xc1, 0xf0};
    
    /* Clear these structs value */
    memset(&CAN_InitData, 0, sizeof(CAN_InitTypeDef));
    memset(&CAN_FilterConfig, 0, sizeof(CAN_FilterTypeDef));
    memset(&CAN_MaskConfig, 0, sizeof(CAN_MaskTypeDef));
    memset(&TxMessage[0], 0, sizeof(CanTxMsgDef));
    memset(&TxMessage[1], 0, sizeof(CanTxMsgDef));
    memset(&TxMessage[2], 0, sizeof(CanTxMsgDef));
    memset(&TxMessage[3], 0, sizeof(CanTxMsgDef));
    memset(&RxMessage, 0, sizeof(CanRxMsgDef));
    
    /* Ask user for additional features enable or not */
    fLib_printf("\r\nEnable Overload frame(1) or not(0)\n");
    scanf("%d", &OLF);
    fLib_printf("\r\nSequential frame types(0) or Random frame types(1) ");
    scanf("%d", &opt);
    
    /* Reset the CAN1 and CAN2 */
    CAN_Reset(CAN1);
    CAN_Reset(CAN2);
    
    CAN_InitData.CAN_Mode = CAN_Mode_Normal;
    CAN_InitData.CAN_RT = CAN_RT_1time;
    CAN_InitData.CAN_TOE = OLF;
    if(!dev->FD_mode)
    {
        /* Non-FD mode */
        CAN_InitData.CAN_NBRP = dev->NPRE;
        CAN_InitData.CAN_NSJW = dev->NSJW;
        CAN_InitData.CAN_NProp = dev->NPROP;
        CAN_InitData.CAN_NPS1 = dev->NPS1;
        CAN_InitData.CAN_NPS2 = dev->NPS2;
    }
    else
    {
        /* FD mode */
        CAN_InitData.CAN_NBRP = dev->NPRE;
        CAN_InitData.CAN_NSJW = dev->NSJW;
        CAN_InitData.CAN_NProp = dev->NPROP;
        CAN_InitData.CAN_NPS1 = dev->NPS1;
        CAN_InitData.CAN_NPS2 = dev->NPS2;
        CAN_InitData.CAN_DBRP = dev->DPRE;
        CAN_InitData.CAN_DSJW = dev->DSJW;
        CAN_InitData.CAN_DProp = dev->DPROP;
        CAN_InitData.CAN_DPS1 = dev->DPS1;
        CAN_InitData.CAN_DPS2 = dev->DPS2;
    }
    
    /* Set interrupt handler and enable interrupt */
    irq_setting(IRQ_CAN1, (PrHandler)CAN1Handler);
    irq_setting(IRQ_CAN2, (PrHandler)CAN2Handler);
    CAN_ITConfig(CAN1, CAN_IRE_EIE, ENABLE);
    CAN_ITConfig(CAN2, CAN_IRE_EIE | CAN_IRE_OIE | CAN_IRE_RIE, ENABLE);
    
    CAN_Init(CAN1, &CAN_InitData);
    CAN_Init(CAN2, &CAN_InitData);
    
    for(fifo_i = 0; fifo_i < 2; fifo_i++) //2 rx FIFOs
    {
        frameCnt = 0;
        
        while(frameCnt < 240) //total 240 frames for each FIFO
        {
            CAN_DBGPRINTF(DBG_LEVEL_1, "\r\n%03d. ", frameCnt);
            
            frame_i = frameCnt%4;
            
            if(opt)
            {
                frameType = rand()%8; //randomize frame type
                ftarray[frame_i] = frameType;
            }
            else
            {
                frameType = (Frame_Type)frameCnt%6; //Sequential frame type 
            }
            
            /* change to config mode */
            CAN_Mode_Change(CAN2, CAN_Mode_Config);
            
            switch((int)frameType)
            {
                case CAN_A_Data:
                    CAN_DBGPRINTF(DBG_LEVEL_1, "CAN 2.0A Data frame");
                    
                    TxMessage[frame_i].StdId = 1234;
                    TxMessage[frame_i].ExtId = 0;
                    //Generate_Random_Identifier(CAN_A_Data, &TxMessage[frame_i].StdId, &TxMessage[frame_i].ExtId);
                    TxMessage[frame_i].IDE = 0;
                    TxMessage[frame_i].BRS = 0;
                    TxMessage[frame_i].FD = 0;
                    TxMessage[frame_i].RTR = 0;
                    TxMessage[frame_i].DLC = CAN_DataBytes_8;
                    memcpy(TxMessage[frame_i].Data, &data, 8);
                    //Generate_Random_Pattern_Length(CAN_A_Data, &TxMessage[frame_i].DLC, TxMessage[frame_i].Data);
                    
                    CAN_FilterConfig.Filter_StdId = TxMessage[frame_i].StdId;
                    CAN_FilterConfig.Filter_ExtId = TxMessage[frame_i].ExtId;
                    CAN_FilterConfig.Filter_frameType = CAN_NonFD_Based_Data;
                    CAN_MaskConfig.Mask_All = 1;
                    
                    break;
                case CAN_A_Remote:
                    CAN_DBGPRINTF(DBG_LEVEL_1, "CAN 2.0A Remote frame");
                
                    TxMessage[frame_i].StdId = 1234;
                    TxMessage[frame_i].ExtId = 0;
                    //Generate_Random_Identifier(CAN_A_Remote, &TxMessage[frame_i].StdId, &TxMessage[frame_i].ExtId);
                    TxMessage[frame_i].IDE = 0;
                    TxMessage[frame_i].BRS = 0;
                    TxMessage[frame_i].FD = 0;
                    TxMessage[frame_i].RTR = 1;
                    TxMessage[frame_i].DLC = CAN_DataBytes_0;
                    
                    CAN_FilterConfig.Filter_StdId = TxMessage[frame_i].StdId;
                    CAN_FilterConfig.Filter_ExtId = TxMessage[frame_i].ExtId;
                    CAN_FilterConfig.Filter_frameType = CAN_NonFD_Based_Remote;
                    CAN_MaskConfig.Mask_All = 1;
                    
                    break;
                case CAN_B_Data:
                    CAN_DBGPRINTF(DBG_LEVEL_1, "CAN 2.0B Data frame");
                
                    TxMessage[frame_i].StdId = 1234;
                    TxMessage[frame_i].ExtId = 5678;
                    //Generate_Random_Identifier(CAN_B_Data, &TxMessage[frame_i].StdId, &TxMessage[frame_i].ExtId);
                    TxMessage[frame_i].IDE = 1;
                    TxMessage[frame_i].BRS = 0;
                    TxMessage[frame_i].FD = 0;
                    TxMessage[frame_i].RTR = 0;
                    TxMessage[frame_i].DLC = CAN_DataBytes_8;
                    memcpy(TxMessage[frame_i].Data, &data, 8);
                    //Generate_Random_Pattern_Length(CAN_B_Data, &TxMessage[frame_i].DLC, TxMessage[frame_i].Data);
                    
                    CAN_FilterConfig.Filter_StdId = TxMessage[frame_i].StdId;
                    CAN_FilterConfig.Filter_ExtId = TxMessage[frame_i].ExtId;
                    CAN_FilterConfig.Filter_frameType = CAN_NonFD_Extended_Data;
                    CAN_MaskConfig.Mask_All = 1;
                    
                    break;
                case CAN_B_Remote:
                    CAN_DBGPRINTF(DBG_LEVEL_1, "CAN 2.0B Remote frame");
                
                    TxMessage[frame_i].StdId = 1234;
                    TxMessage[frame_i].ExtId = 5678;
                    //Generate_Random_Identifier(CAN_B_Remote, &TxMessage[frame_i].StdId, &TxMessage[frame_i].ExtId);
                    TxMessage[frame_i].IDE = 1;
                    TxMessage[frame_i].BRS = 0;
                    TxMessage[frame_i].FD = 0;
                    TxMessage[frame_i].RTR = 1;
                    TxMessage[frame_i].DLC = CAN_DataBytes_0;
                    
                    CAN_FilterConfig.Filter_StdId = TxMessage[frame_i].StdId;
                    CAN_FilterConfig.Filter_ExtId = TxMessage[frame_i].ExtId;
                    CAN_FilterConfig.Filter_frameType = CAN_NonFD_Extended_Remote;
                    CAN_MaskConfig.Mask_All = 1;
                    
                    break;
                case CAN_FD_A:
                    CAN_DBGPRINTF(DBG_LEVEL_1, "CAN FD Based non-BRS frame");
                
                    TxMessage[frame_i].StdId = 1234;
                    TxMessage[frame_i].ExtId = 0;
                    //Generate_Random_Identifier(CAN_FD_A, &TxMessage[frame_i].StdId, &TxMessage[frame_i].ExtId);
                    TxMessage[frame_i].IDE = 0;
                    TxMessage[frame_i].BRS = 0;
                    TxMessage[frame_i].FD = 1;
                    TxMessage[frame_i].RTR = 0;
                    TxMessage[frame_i].DLC = CAN_DataBytes_8;
                    memcpy(TxMessage[frame_i].Data, &data, 8);
                    //Generate_Random_Pattern_Length(CAN_FD_A, &TxMessage[frame_i].DLC, TxMessage[frame_i].Data);
                    
                    CAN_FilterConfig.Filter_StdId = TxMessage[frame_i].StdId;
                    CAN_FilterConfig.Filter_ExtId = TxMessage[frame_i].ExtId;
                    CAN_FilterConfig.Filter_frameType = CAN_FD_Based_Data;
                    CAN_MaskConfig.Mask_All = 1;
                    
                    break;
                case CAN_FD_B:
                    CAN_DBGPRINTF(DBG_LEVEL_1, "CAN FD Extended non-BRS frame");
                
                    TxMessage[frame_i].StdId = 1234;
                    TxMessage[frame_i].ExtId = 5678;
                    //Generate_Random_Identifier(CAN_B_Data, &TxMessage[frame_i].StdId, &TxMessage[frame_i].ExtId);
                    TxMessage[frame_i].IDE = 1;
                    TxMessage[frame_i].BRS = 0;
                    TxMessage[frame_i].FD = 1;
                    TxMessage[frame_i].RTR = 0;
                    TxMessage[frame_i].DLC = CAN_DataBytes_8;
                    memcpy(TxMessage[frame_i].Data, &data, 8);
                    //Generate_Random_Pattern_Length(CAN_B_Data, &TxMessage[frame_i].DLC, TxMessage[frame_i].Data);
                    
                    CAN_FilterConfig.Filter_StdId = TxMessage[frame_i].StdId;
                    CAN_FilterConfig.Filter_ExtId = TxMessage[frame_i].ExtId;
                    CAN_FilterConfig.Filter_frameType = CAN_FD_Extended_Data;
                    CAN_MaskConfig.Mask_All = 1;
                    
                    break;
                case CAN_FD_A_BRS:
                    CAN_DBGPRINTF(DBG_LEVEL_1, "CAN FD Based BRS frame");
                
                    TxMessage[frame_i].StdId = 1234;
                    TxMessage[frame_i].ExtId = 0;
                    //Generate_Random_Identifier(CAN_FD_A, &TxMessage[frame_i].StdId, &TxMessage[frame_i].ExtId);
                    TxMessage[frame_i].IDE = 0;
                    TxMessage[frame_i].BRS = 1;
                    TxMessage[frame_i].FD = 1;
                    TxMessage[frame_i].RTR = 0;
                    TxMessage[frame_i].DLC = CAN_DataBytes_8;
                    memcpy(TxMessage[frame_i].Data, &data, 8);
                    //Generate_Random_Pattern_Length(CAN_FD_A, &TxMessage[frame_i].DLC, TxMessage[frame_i].Data);
                    
                    CAN_FilterConfig.Filter_StdId = TxMessage[frame_i].StdId;
                    CAN_FilterConfig.Filter_ExtId = TxMessage[frame_i].ExtId;
                    CAN_FilterConfig.Filter_frameType = CAN_FD_Based_Data;
                    CAN_MaskConfig.Mask_All = 1;
                    
                    break;
                case CAN_FD_B_BRS:
                    CAN_DBGPRINTF(DBG_LEVEL_1, "CAN FD Extended BRS frame");
                
                    TxMessage[frame_i].StdId = 1234;
                    TxMessage[frame_i].ExtId = 5678;
                    //Generate_Random_Identifier(CAN_B_Data, &TxMessage[frame_i].StdId, &TxMessage[frame_i].ExtId);
                    TxMessage[frame_i].IDE = 1;
                    TxMessage[frame_i].BRS = 1;
                    TxMessage[frame_i].FD = 1;
                    TxMessage[frame_i].RTR = 0;
                    TxMessage[frame_i].DLC = CAN_DataBytes_8;
                    memcpy(TxMessage[frame_i].Data, &data, 8);
                    //Generate_Random_Pattern_Length(CAN_B_Data, &TxMessage[frame_i].DLC, TxMessage[frame_i].Data);
                    
                    CAN_FilterConfig.Filter_StdId = TxMessage[frame_i].StdId;
                    CAN_FilterConfig.Filter_ExtId = TxMessage[frame_i].ExtId;
                    CAN_FilterConfig.Filter_frameType = CAN_FD_Extended_Data;
                    CAN_MaskConfig.Mask_All = 1;
                    
                    break;
            }
            
            CAN_FilterInit(CAN2, &CAN_FilterConfig, fifo_i);
            CAN_MaskInit(CAN2, &CAN_MaskConfig, fifo_i);
            
            /* enable fifo number in this round */
            CAN_FilterGroup(CAN2, fifo_i, ENABLE);
            CAN_FilterGroup(CAN2, fifo_i?0:1, DISABLE);
            CAN_MaskNumber(CAN2, fifo_i, ENABLE);
            CAN_MaskNumber(CAN2, fifo_i?0:1, DISABLE);
            
            /* init CAN to normal mode */
            CAN_Mode_Change(CAN2, CAN_Mode_Normal);
            
            delay_ms(100);//delay for ensure the 11 recessive bits (maybe...)
            
            /* transmit this frame */
            ITFLAG = 0; //CAN2 receive interrupt flag
            CAN_Transmit_TXn(CAN1, &TxMessage[frame_i], 0);
            while(CAN_GetBTRStatus(CAN1, 0));
            FIFOCnt++;
            
            /* See CAN2 RIR bit for receive message finish */
            while(ITFLAG == 0 && FIFOCnt < 3);
            ITFLAG = 0;
            
            /* Check BRSn field value */
            if(FIFOCnt < 4)
            {
                /* Check the FIFO receive the last message by BRS field increment */
                if(CAN_GetStatusBit(CAN2, fifo_i?CAN_BRS1_STATUS_BIT:CAN_BRS0_STATUS_BIT) != FIFOCnt)
                {
                    return CAN_BRSn_Uncorrect;
                }
                
                /* check DO field */
                if(CAN_GetStatusBit(CAN2, CAN_DO_STATUS_BIT) != 0)
                {
                    return CAN_DO_Uncorrect;
                }
            }
            else if(FIFOCnt == 4)
            {
                /* If the fourth message is sent, the FIFO overrun should be happended
                   , and the number of buffer in FIFO must keep in three. */
                if(CAN_GetStatusBit(CAN2, fifo_i?CAN_BRS1_STATUS_BIT:CAN_BRS0_STATUS_BIT) != 3)
                {
                    return CAN_BRSn_Uncorrect;
                }
                FIFOCnt = 3; //FIFO full
            }
            
            /* if frameCnt%4 equals 3, this FIFO should full and overrun.
               We must verify the first three messages, and the fourth message should be discard. */
            if(frameCnt%4 == 3)
            {
                for(i = 0; i < 3; i++)
                {
                    /* Get the buffer from FIFO */
                    CAN_Receive_RXn(CAN2, fifo_i, &RxMessage);
                    FIFOCnt--;
                    
                    if(opt)
                        ret = Verify_Pattern(ftarray[frame_i - 3 + i], &TxMessage[i], &RxMessage);
                    else
                        ret = Verify_Pattern((frameType - 3 + i), &TxMessage[i], &RxMessage);
                    
                    if(ret != 0)
                        return ret;
                    
                    /* check the BRSn is decreased for release FIFO */
                    if(CAN_GetStatusBit(CAN2, fifo_i?CAN_BRS1_STATUS_BIT:CAN_BRS0_STATUS_BIT) != FIFOCnt)
                    {
                        return CAN_BRSn_Uncorrect;
                    }
                }
                
                CAN_FIFORelease_All(CAN2, fifo_i);
                /* check Error Type - OE = 0 */
                if(CAN_GetLastErrorCode(CAN2) & CAN_ET_OE)
                {
                    return CAN_OE_ERROR;
                }
            }
            
            frameCnt++;
        }
    }
    
    CAN_DBGPRINTF(DBG_LEVEL_1, "\n\nRun 240 frames for 2 Rx FIFOs done!\n ");
    
    return 0;
}

int CAN_GenError_Transmitter_test()
{
    /* FTCAN as a transmitter and PEAK CAN as a receiver.
       PEAK CAN will be set to destroy a specified bit during the FTCAN's frame.
       When the FTCAN was sending its frames to the bus, it monitors the Bit 
       Error since the PEAK CAN issue the Error Frame. After a Bit Error, the
       FTCAN will also entry error frame state. Finally every nodes on the bus 
       leave error delimiter together and ready to receive the new SOF. (The 
       detail of Error Frame can refer to Bosch spec)
       
       The following steps to use this function:
       1. Open the PEAK CAN and set which bit should be destroy by error injector
       2. FTCAN normal mode, EIE, RT=1
       3. FTCAN send a frame
       4. This frame will be destroyed by PEAK CAN, the error frame now appears
          on the CAN bus.
       5. FTCAN detect the error and entry Error handler */
    
    CAN_InitTypeDef CAN_InitData;
    CanTxMsgDef TxMessage;
    int i;
    UINT8 data[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    
    /* Clear these structs value */
    memset(&CAN_InitData, 0, sizeof(CAN_InitTypeDef));
    memset(&TxMessage, 0, sizeof(CanTxMsgDef));
    
    /* Non-FD and FD CAN's bit timing setting */
    CAN_InitData.CAN_Mode = CAN_Mode_Normal;
    CAN_InitData.CAN_RT = CAN_RT_1time;
    // /* arbitration phase 500Kbps 75% */
    // CAN_InitData.CAN_NBRP = 5;
    // CAN_InitData.CAN_NProp = 19;
    // CAN_InitData.CAN_NPS1 = 10;
    // CAN_InitData.CAN_NPS2 = 10;
    // CAN_InitData.CAN_NSJW = 10;
    // /* data phase 1Mbps 85% */
    // CAN_InitData.CAN_DBRP = 5;
    // CAN_InitData.CAN_DProp = 8;
    // CAN_InitData.CAN_DPS1 = 8;
    // CAN_InitData.CAN_DPS2 = 3;
    // CAN_InitData.CAN_DSJW = 3;
    
    CAN_InitData.CAN_NBRP = 5;
    CAN_InitData.CAN_NSJW = 4;
    CAN_InitData.CAN_NProp = 9;
    CAN_InitData.CAN_NPS1 = 5;
    CAN_InitData.CAN_NPS2 = 5;
    CAN_InitData.CAN_DBRP = 0;
    CAN_InitData.CAN_DSJW = 0;
    CAN_InitData.CAN_DProp = 0;
    CAN_InitData.CAN_DPS1 = 0;
    CAN_InitData.CAN_DPS2 = 0;
    
    TxMessage.StdId = 1234;
    TxMessage.ExtId = 0;
    TxMessage.IDE = 0;
    TxMessage.RTR = 0;
    TxMessage.BRS = 0;
    TxMessage.FD = 1;
    TxMessage.DLC = CAN_DataBytes_8;
    memcpy(TxMessage.Data, &data, 8);
    
    /* Set interrupt handler and enable interrupt */
    irq_setting(IRQ_CAN1, (PrHandler)CAN1Handler);
    CAN_ITConfig(CAN1, CAN_IRE_EIE, ENABLE);
    
    CAN_Init(CAN1, &CAN_InitData);
    
    
    CAN_Transmit_TXn(CAN1, &TxMessage, 0);
    while(CAN_GetBTRStatus(CAN1, 0));

    fLib_printf("\ndone\n");
    return 0;
}

int CAN_GenError_Receiver_test()
{
    /* FTCAN as a receiver, PEAK CAN as a transmitter.
       PEAK CAN will send a data frame and destroy it in a specified bit by the 
       error injector tool. After this specified bit, the PEAK CAN will issue 
       the Error Frame at the next bit. Because FTCAN is a receiver, it detects
       the successive six dominant bits from PEAK CAN's error frame. 
       The 6th dominant bit will cause FTCAN stuff error, so FTCAN will also 
       issue the error frame. Every nodes on the bus issue error flag, and 
       finally go error delimiter together for new SOF coming.
       
       1. FTCAN normal mode, EIE, F/M=all
       2. FTCAN wait the data from the bus
       3. PEAK CAN use error injector to determin which bit will be destroyed.
       4. PEAK CAN start to transfer data
       5. FTCAN detect the error and go error handler */
       
    CAN_InitTypeDef CAN_InitData;
    CAN_FilterTypeDef CAN_FilterConfig;
    CAN_MaskTypeDef CAN_MaskConfig;
    CanRxMsgDef    RxMessage;
    UINT16 REC_t;
    int rx_flag;
    
    /* Clear these structs value */
    memset(&CAN_InitData, 0, sizeof(CAN_InitTypeDef));
    memset(&CAN_FilterConfig, 0, sizeof(CAN_FilterTypeDef));
    memset(&CAN_MaskConfig, 0, sizeof(CAN_MaskTypeDef));
    memset(&RxMessage, 0, sizeof(CanRxMsgDef));
    
    /* Non-FD and FD CAN's bit timing setting */
    CAN_InitData.CAN_Mode = CAN_Mode_Normal;
    CAN_InitData.CAN_RT = CAN_RT_1time;
    /* arbitration phase 500Kbps 75% */
    CAN_InitData.CAN_NBRP = 5;
    CAN_InitData.CAN_NProp = 19;
    CAN_InitData.CAN_NPS1 = 10;
    CAN_InitData.CAN_NPS2 = 10;
    CAN_InitData.CAN_NSJW = 10;
    /* data phase 1Mbps 85% */
    CAN_InitData.CAN_DBRP = 5;
    CAN_InitData.CAN_DProp = 8;
    CAN_InitData.CAN_DPS1 = 8;
    CAN_InitData.CAN_DPS2 = 3;
    CAN_InitData.CAN_DSJW = 3;
    
    /* Filter and mask setting */
    CAN_FilterConfig.Filter_StdId = 1234;
    CAN_FilterConfig.Filter_ExtId = 0;
    CAN_FilterConfig.Filter_frameType = CAN_ALL;
    CAN_MaskConfig.Mask_StdId = 0;
    CAN_MaskConfig.Mask_ExtId = 0;
    CAN_MaskConfig.Mask_All = 1;
    
    /* Set filter and mask */
    CAN_FilterInit(CAN1, &CAN_FilterConfig, 0);
    CAN_FilterGroup(CAN1, 0, ENABLE);
    CAN_MaskInit(CAN1, &CAN_MaskConfig, 0);
    CAN_MaskNumber(CAN1, 0, ENABLE);
    
    /* Set interrupt handler and enable interrupt */
    irq_setting(IRQ_CAN1, (PrHandler)CAN1Handler);
    CAN_ITConfig(CAN1, CAN_IRE_EIE, ENABLE);
    
    CAN_Init(CAN1, &CAN_InitData);
    CAN_Init(CAN2, &CAN_InitData); //To avoid the ACK error happened in PEAK, because the FTCAN1 has a lot of errors and bus off.
    
    do
    {
        /* Receive message */
        while(CAN_GetStatusBit(CAN1, CAN_BRS0_STATUS_BIT) == 0);
        rx_flag = CAN_Receive_RXn(CAN1, 0, &RxMessage);
        
        /* if receiving message successfully, check the REC recoveried or not */
        if(rx_flag == CAN_RxFIFO_0)
        {
            /* If REC = 0, then it still REC = 0 after successful reception. 
               If REC = 1~127, then REC will decreased by 1 after successful reception. 
               If REC > 127, then REC will be set to a value between 119 and 127. */
            REC_t = CAN_GetReceiveErrorCounter(CAN1);
            CAN_DBGPRINTF(DBG_LEVEL_1, "CAN1 REC: %d (After successfully receiving.)\n", REC_t);
            
            /* The gREC is the number read from the error handler when Error 
               happened. The REC_t is the number after receiving a frame.
               The REC_t should be decreased by 1. */
            if(gREC == 0 && REC_t != 0)
            {
                CAN_DBGPRINTF(DBG_LEVEL_0, "The original REC is 0, but current REC is not 0.\n");
                return -1;
            }
            if((gREC > 0 && gREC <128) && (REC_t != (gREC - 1)))
            {
                CAN_DBGPRINTF(DBG_LEVEL_0, "The original REC is between 1 and 127, but current REC is not decreased by 1.\n");
                return -1;
            }
            if(gREC > 127 && REC_t > 127)
            {
                CAN_DBGPRINTF(DBG_LEVEL_0, "The original REC is more than 127, but current REC is not between 119 and 127.\n");
                return -1;
            }
        }
    }while(1);
    
    
    return 0;
}

int CAN_Arbitration_random_test()
{
    CAN_InitTypeDef CAN_InitData;
    CanTxMsgDef TxMessage1, TxMessage2;
    Frame_Type ft;
    UINT32 ID1, ID2;
    
    UINT8 data[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    
    memset(&CAN_InitData, 0, sizeof(CAN_InitTypeDef));
    memset(&TxMessage1, 0, sizeof(CanTxMsgDef));
    memset(&TxMessage2, 0, sizeof(CanTxMsgDef));
    
    CAN1ALCnt = 0; //Count for Arbitration loss happened
    CAN2ALCnt = 0; //Count for Arbitration loss happened
    TBCnt = 0; //Count for transmission successfully
    
    CAN_InitData.CAN_Mode = CAN_Mode_Normal;
    CAN_InitData.CAN_RT = CAN_RT_Always;
    
    /* 500k 75% */
    CAN_InitData.CAN_NBRP = 10;
    CAN_InitData.CAN_NSJW = 4;
    CAN_InitData.CAN_NProp = 9;
    CAN_InitData.CAN_NPS1 = 5;
    CAN_InitData.CAN_NPS2 = 5;
    /* 1M 85% */
    CAN_InitData.CAN_DBRP = 5;
    CAN_InitData.CAN_DSJW = 3;
    CAN_InitData.CAN_DProp = 8;
    CAN_InitData.CAN_DPS1 = 8;
    CAN_InitData.CAN_DPS2 = 3;
    
    //init CAN1, CAN2
    CAN_Reset(CAN1);
    CAN_Reset(CAN2);
    
    CAN_Init(CAN1, &CAN_InitData);
    CAN_Init(CAN2, &CAN_InitData);
    
    irq_setting(IRQ_CAN1, (PrHandler)CAN1Handler_ALtest);
    CAN_ITConfig(CAN1, CAN_IRE_EIE | CAN_IRE_TIE, ENABLE);
    irq_setting(IRQ_CAN2, (PrHandler)CAN2Handler_ALtest);
    CAN_ITConfig(CAN2, CAN_IRE_EIE | CAN_IRE_TIE, ENABLE);
    
    //tx buffer 1 and 2 setting
    ft = rand()%8;
    
    Generate_Frame(ft, &TxMessage1);
    ID1 = (((TxMessage1.StdId & 0x7ff) << 18) | (TxMessage1.ExtId & 0x3ffff));
    
    do
    {
        Generate_Frame(ft, &TxMessage2);
        ID2 = (((TxMessage2.StdId & 0x7ff) << 18) | (TxMessage2.ExtId & 0x3ffff));
        
    } while(ID1 <= ID2);
        
    CAN_DBGPRINTF(DBG_LEVEL_1, "CAN1 ID: %08d (0x%x), CAN2 ID: %08d (0x%x).\n(Lower ID Higher priority!)\n", ID1, ID1, ID2, ID2);
    
    
    //fill frame context into two CANs tx registers
    CAN_Transmit(CAN1, &TxMessage1, CAN_Txbuffer_0);
    CAN_Transmit(CAN1, &TxMessage1, CAN_Txbuffer_1);
    CAN_Transmit(CAN1, &TxMessage1, CAN_Txbuffer_2);
    CAN_Transmit(CAN2, &TxMessage2, CAN_Txbuffer_0);
    CAN_Transmit(CAN2, &TxMessage2, CAN_Txbuffer_1);
    CAN_Transmit(CAN2, &TxMessage2, CAN_Txbuffer_2);
    
    /* Enable two CANs to send frames through 3 tx buffers */
    CAN1->CAN_Control.All |= 0x38;
    CAN2->CAN_Control.All |= 0x38;
    

    while(CAN2ALCnt == 0 && CAN1ALCnt == 0);
    
//    disable_interrupts();
    fLib_CloseIRQ(IRQ_CAN1);
    fLib_CloseIRQ(IRQ_CAN2);
    
    CAN_DBGPRINTF(DBG_LEVEL_0, "Total:%d\n",TBCnt);
    
    CAN_DBGPRINTF(DBG_LEVEL_0, "CAN1 AL:%d, CAN2 AL:%d\n",CAN1ALCnt, CAN2ALCnt);
    
    if (CAN2ALCnt != 0 || CAN1ALCnt == 0)
    {
        CAN_DBGPRINTF(DBG_LEVEL_0, "Fail!\n");
    }
    else
    {
        CAN_DBGPRINTF(DBG_LEVEL_0, "Pass!\n");
    }
        
    
    return 0;
}

int CAN_TimeStamp_test(CAN_Param *dev)
{
    /* If timestamp (TS) is enable, this program will check the TS pattern when 
       DLC > 2. The Tx TS is in the last 2 data bytes, and the Rx TS is in the 
       register.
       The program will test the three consecutive frames, each intervals shoud
       be same or differ 1.
       The program also concern 3 Tx buffers and 2 Rx FIFOs should work this TS
       feature. */
    CAN_InitTypeDef CAN_InitData;
    CAN_FilterTypeDef CAN_FilterConfig;
    CAN_MaskTypeDef CAN_MaskConfig;
    CanTxMsgDef TxMessage;
    CanRxMsgDef RxMessage;
    int ret, FIFO_i, TxBuffer_i, i, flag, test;
    int TxTS01_diff, TxTS12_diff, RxTS01_diff, RxTS12_diff;
    char c;
    UINT32 counter, RBTS0, RBTS1;
    UINT8 RxFIFOBRS;
    
    UINT16 TxTS[3] = {0x0, 0x0, 0x0};
    UINT16 RxTS[3] = {0x0, 0x0, 0x0};
    
    counter = 0;
    flag = 1;
    
    /* Clear these structs value */
    memset(&CAN_InitData, 0, sizeof(CAN_InitTypeDef));
    memset(&CAN_FilterConfig, 0, sizeof(CAN_FilterTypeDef));
    memset(&CAN_MaskConfig, 0, sizeof(CAN_MaskTypeDef));
    memset(&TxMessage, 0, sizeof(CanTxMsgDef));
    memset(&RxMessage, 0, sizeof(CanRxMsgDef));
    
    /* Reset the CAN1 and CAN2 */
    CAN_Reset(CAN1);
    CAN_Reset(CAN2);
    
    CAN_InitData.CAN_Mode = CAN_Mode_Normal;
    CAN_InitData.CAN_RT = CAN_RT_1time;
    CAN_InitData.CAN_TSE = ENABLE;
    if(!dev->FD_mode)
    {
        /* Non-FD mode */
        CAN_InitData.CAN_NBRP = dev->NPRE;
        CAN_InitData.CAN_NSJW = dev->NSJW;
        CAN_InitData.CAN_NProp = dev->NPROP;
        CAN_InitData.CAN_NPS1 = dev->NPS1;
        CAN_InitData.CAN_NPS2 = dev->NPS2;
    }
    else
    {
        /* FD mode */
        CAN_InitData.CAN_NBRP = dev->NPRE;
        CAN_InitData.CAN_NSJW = dev->NSJW;
        CAN_InitData.CAN_NProp = dev->NPROP;
        CAN_InitData.CAN_NPS1 = dev->NPS1;
        CAN_InitData.CAN_NPS2 = dev->NPS2;
        CAN_InitData.CAN_DBRP = dev->DPRE;
        CAN_InitData.CAN_DSJW = dev->DSJW;
        CAN_InitData.CAN_DProp = dev->DPROP;
        CAN_InitData.CAN_DPS1 = dev->DPS1;
        CAN_InitData.CAN_DPS2 = dev->DPS2;
    }
    
    TxMessage.StdId = 1234;
    if(dev->FrameType == CAN_B_Data || dev->FrameType == CAN_B_Remote || \
       dev->FrameType == CAN_FD_B   || dev->FrameType == CAN_FD_B_BRS)
    {
       TxMessage.ExtId = 5678;
       TxMessage.IDE = 1;
       CAN_FilterConfig.Filter_ExtId = 5678;
    }
    else
    {
        TxMessage.ExtId = 0;
        TxMessage.IDE = 0;
        CAN_FilterConfig.Filter_ExtId = 0;
    }
    TxMessage.BRS = (dev->FrameType >= 6)?1:0;
    TxMessage.FD = dev->FD_mode;
    
    CAN_FilterConfig.Filter_StdId = 1234;
    CAN_FilterConfig.Filter_frameType = CAN_ALL;
    CAN_MaskConfig.Mask_StdId = 0;
    CAN_MaskConfig.Mask_ExtId = 0;
    CAN_MaskConfig.Mask_All = 1;
    
    CAN_FilterInit(CAN2, &CAN_FilterConfig, 0); //RxFIFO0
    CAN_MaskInit(CAN2, &CAN_MaskConfig, 0);     //RxFIFO0
    CAN_FilterInit(CAN2, &CAN_FilterConfig, 4); //RxFIFO1
    CAN_MaskInit(CAN2, &CAN_MaskConfig, 1);     //RxFIFO1
    
    /* Set interrupt handler and enable interrupt */
    irq_setting(IRQ_CAN1, (PrHandler)CAN1Handler);
    irq_setting(IRQ_CAN2, (PrHandler)CAN2Handler);
    CAN_ITConfig(CAN1, CAN_IRE_EIE, ENABLE);
    CAN_ITConfig(CAN2, CAN_IRE_EIE, ENABLE);
    
    CAN_Init(CAN1, &CAN_InitData);
    CAN_Init(CAN2, &CAN_InitData);
    
    CAN_DBGPRINTF(DBG_LEVEL_1, "<<Start test! Press 'q' to stop.>>\n");
    
    test = 0;
    
    do
    {
        /* if key the space or enter, the burnin will stop. */
        c = ftuart_getc_t(10000, 0);
        if (c == 'q')
            break;
        
        if(dev->FrameType == CAN_A_Remote || dev->FrameType == CAN_B_Remote)
        {
            TxMessage.RTR = 1;
            TxMessage.DLC = CAN_DataBytes_0;
        }
        else
        {
            TxMessage.RTR = 0;
            /* Randomize the DLC according to frame type, and generate the data values. */
            Generate_Random_Pattern_Length(dev->FrameType, &TxMessage.DLC, TxMessage.Data);
            
            // //debug for DLC increment
            // do
                // Generate_Random_Pattern_Length(dev->FrameType, &TxMessage.DLC, TxMessage.Data);
            // while(TxMessage.DLC != test);
            // test = (test++);
            
            
            /* print the current DLC and pattern for debugging */
            // CAN_DBGPRINTF(DBG_LEVEL_1, "DLC = %d\n Data:", TxMessage.DLC);
            // for(i = 0; i < DLCtoBytes_table[TxMessage.DLC]; i++)
            // {
                // CAN_DBGPRINTF(DBG_LEVEL_1, "%02x ", TxMessage.Data[i]);
            // }
            // CAN_DBGPRINTF(DBG_LEVEL_1, "\n");
        }
        
        /* for loop 3 txbuffer */
        for(TxBuffer_i = 0; TxBuffer_i < 3; TxBuffer_i++)
        {
            /* for loop 2 FIFO */
            for(FIFO_i = 0; FIFO_i < 2; FIFO_i++)
            {
                CAN_DBGPRINTF(DBG_LEVEL_1, "Txbuffer%d and RxFIFO%d\n", TxBuffer_i, FIFO_i);
                
                flag = 1; //reset the error flag (0: no receiving. 1: received msg.)
                
                /* Use filter 0 for RxFIFO0 and filter 4 for RxFIFO1 */
                if(FIFO_i == 0)
                {
                    RxFIFOBRS = CAN_BRS0_STATUS_BIT;
                    CAN_Mode_Change(CAN2, CAN_Mode_Config);
                    CAN_FilterGroup(CAN2, CAN_RxFIFO_0, ENABLE); //Enable RxFIFO0
                    CAN_MaskNumber(CAN2, CAN_RxFIFO_0, ENABLE);  //Enable RxFIFO0
                    CAN_FilterGroup(CAN2, CAN_RxFIFO_1, DISABLE);//Disable RxFIFO1
                    CAN_MaskNumber(CAN2, CAN_RxFIFO_1, DISABLE); //Disable RxFIFO1
                    CAN_Mode_Change(CAN2, CAN_Mode_Normal);
                    
                }
                else if(FIFO_i == 1)
                {
                    RxFIFOBRS = CAN_BRS1_STATUS_BIT;
                    CAN_Mode_Change(CAN2, CAN_Mode_Config);
                    CAN_FilterGroup(CAN2, CAN_RxFIFO_0, DISABLE);//Disable RxFIFO0
                    CAN_MaskNumber(CAN2, CAN_RxFIFO_0, DISABLE); //Disable RxFIFO0
                    CAN_FilterGroup(CAN2, CAN_RxFIFO_1, ENABLE); //Enable RxFIFO1
                    CAN_MaskNumber(CAN2, CAN_RxFIFO_1, ENABLE);  //Enable RxFIFO1
                    CAN_Mode_Change(CAN2, CAN_Mode_Normal);
                }
                
                /* Part of transmission */
                /* 3 times transfer message */
                for(i = 0; i < 3; i++)
                {
                    CAN_Transmit_TXn(CAN1, &TxMessage, TxBuffer_i);
                    while(CAN_GetBTRStatus(CAN1, TxBuffer_i));
                }
                /* 3 times receiving message */
                for(i = 0; i < 3; i++)
                {
                    ret = CAN_Receive_RXn_timeout(CAN2, FIFO_i, &RxMessage, CAN_Rx_Timeout);
                    if(ret < 0)
                    {
                        /* receive timeout */
                        flag = 0;
                        break;
                    }
                    
                    /* If DLC >= 2-byte, then get the Tx timsstamp from the last 2-byte of data field */
                    TxTS[i] = (TxMessage.DLC >= 2)?RxMessage.TxTimeStamp:0x0;
                    RxTS[i] = RxMessage.RxTimeStamp;
                    CAN_DBGPRINTF(DBG_LEVEL_1, "1.Tx TS:%04x  Rx TS:%04x\n", TxTS[i], RxTS[i]);
                }
                
                /* Part of verification */
                if(flag)
                {
                    TxTS01_diff = TxTS[1] - TxTS[0];
                    TxTS12_diff = TxTS[2] - TxTS[1];
                    RxTS01_diff = RxTS[1] - RxTS[0];
                    RxTS12_diff = RxTS[2] - RxTS[1];
                    TxTS01_diff &= 0xffff;
                    TxTS12_diff &= 0xffff;
                    RxTS01_diff &= 0xffff;
                    RxTS12_diff &= 0xffff;
                    
                    /* Check Tx timestamps */
                    if(TxMessage.DLC >= 2)
                    {
                        if((TxTS[0] == TxTS[1]) || (TxTS[1] == TxTS[2]) || (TxTS[0] == TxTS[2]))
                        {
                            CAN_DBGPRINTF(DBG_LEVEL_0, "\tTx timestamp ERROR (Have same tx timestamps)\n");
                            return -1;
                        }
                            
                        if((TxTS[1] - TxTS[0] < 0) || (TxTS[2] - TxTS[1] < 0))
                        {
                            CAN_DBGPRINTF(DBG_LEVEL_0, "\tTx timestamp overflow!\n"); //Not error
                        }
                            
                        /* Check the interval btween Tx and Rx timestamp */
                        if((TxTS01_diff - RxTS01_diff > 1) || (TxTS12_diff - RxTS12_diff > 1) || \
                           (TxTS01_diff - RxTS01_diff < -1) || (TxTS12_diff - RxTS12_diff < -1))
                        {
                            CAN_DBGPRINTF(DBG_LEVEL_0, "\tTimestamp interval is not correct\n");
                            
                            CAN_DBGPRINTF(DBG_LEVEL_0, "\tTS1 - TS0: Tx=%d, Rx=%d\n", TxTS01_diff, RxTS01_diff);
                            CAN_DBGPRINTF(DBG_LEVEL_0, "\tTS2 - TS1: Tx=%d, Rx=%d\n", TxTS12_diff, RxTS12_diff);
                            return -1;
                        }
                    }
                    
                    /* Check Rx timestamps */
                    if(RxTS[0] == RxTS[1] || RxTS[1] == RxTS[2] || RxTS[0] == RxTS[2])
                        CAN_DBGPRINTF(DBG_LEVEL_0, "\tRx timestamp error\n");
                    if((RxTS01_diff < 0) || (RxTS12_diff < 0))
                        CAN_DBGPRINTF(DBG_LEVEL_0, "\tRx timestamp overflow!\n");
                    
                }
            }
        }
        
        counter++;
        
    }while(dev->FreeRun);
    
        CAN_DBGPRINTF(DBG_LEVEL_1, "%08d round", counter);
    
    return ret;
}

int CAN_Filter_Frame_Type_test(CAN_Param *dev)
{
    CAN_InitTypeDef CAN_InitData;
    CAN_FilterTypeDef CAN_FilterConfig;
    CAN_MaskTypeDef CAN_MaskConfig;
    CanTxMsgDef TxMessage;
    CanRxMsgDef    RxMessage;
    int ret, data_random, id_random, frameType_i, AFC1_i, flag;
    char c;
    UINT32 counter;
    UINT32 SID, EID;
    
    counter = 0;
    UINT8 data[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    UINT8 array_ft[20] = {CAN_ALL, 
                          CAN_OnlyNonFD, 
                          CAN_OnlyFD, 
                          CAN_OnlyData, 
                          CAN_OnlyRemote, 
                          CAN_OnlyBased, 
                          CAN_OnlyExtended, 
                          CAN_NonFD_Data, 
                          CAN_NonFD_Remote, 
                          CAN_NonFD_Based, 
                          CAN_NonFD_Based_Data, 
                          CAN_NonFD_Based_Remote, 
                          CAN_NonFD_Extended, 
                          CAN_NonFD_Extended_Data, 
                          CAN_NonFD_Extended_Remote, 
                          CAN_FD_Data,
                          CAN_FD_Based, 
                          CAN_FD_Based_Data, 
                          CAN_FD_Extended, 
                          CAN_FD_Extended_Data}; 
    UINT8 accept_tb[20] = {0xff, 0x0f, 0xf0, 0xf5, 0x0a, 
                           0x53, 0xac, 0x05, 0x0a, 0x03, 
                           0x01, 0x02, 0x0c, 0x04, 0x08,
                           0xf0, 0x50, 0x50, 0xa0, 0xa0}; 
    
    /* Clear these structs value */
    memset(&CAN_InitData, 0, sizeof(CAN_InitTypeDef));
    memset(&CAN_FilterConfig, 0, sizeof(CAN_FilterTypeDef));
    memset(&CAN_MaskConfig, 0, sizeof(CAN_MaskTypeDef));
    
    fLib_printf("ID random? yes(1) no(0): ");
    scanf("%d",&id_random);
    fLib_printf("\n");
    fLib_printf("Data random? yes(1) no(0): ");
    scanf("%d",&data_random);
    fLib_printf("\n");
    
    /* Reset the CAN1 and CAN2 */
    CAN_Reset(CAN1);
    CAN_Reset(CAN2);
    
    CAN_InitData.CAN_Mode = CAN_Mode_Config;
    CAN_InitData.CAN_RT = CAN_RT_1time;
    if(!dev->FD_mode)
    {
        /* Non-FD mode */
        CAN_InitData.CAN_NBRP = dev->NPRE;
        CAN_InitData.CAN_NSJW = dev->NSJW;
        CAN_InitData.CAN_NProp = dev->NPROP;
        CAN_InitData.CAN_NPS1 = dev->NPS1;
        CAN_InitData.CAN_NPS2 = dev->NPS2;
    }
    else
    {
        /* FD mode */
        CAN_InitData.CAN_NBRP = dev->NPRE;
        CAN_InitData.CAN_NSJW = dev->NSJW;
        CAN_InitData.CAN_NProp = dev->NPROP;
        CAN_InitData.CAN_NPS1 = dev->NPS1;
        CAN_InitData.CAN_NPS2 = dev->NPS2;
        CAN_InitData.CAN_DBRP = dev->DPRE;
        CAN_InitData.CAN_DSJW = dev->DSJW;
        CAN_InitData.CAN_DProp = dev->DPROP;
        CAN_InitData.CAN_DPS1 = dev->DPS1;
        CAN_InitData.CAN_DPS2 = dev->DPS2;
    }
    
    /* Set interrupt handler and enable interrupt */
    irq_setting(IRQ_CAN1, (PrHandler)CAN1Handler);
    irq_setting(IRQ_CAN2, (PrHandler)CAN2Handler);
    CAN_ITConfig(CAN1, CAN_IRE_EIE, ENABLE);
    CAN_ITConfig(CAN2, CAN_IRE_EIE, ENABLE);
    
    CAN_Init(CAN1, &CAN_InitData);
    CAN_Init(CAN2, &CAN_InitData);
    
    CAN_Mode_Change(CAN1, CAN_Mode_Normal);
    
    CAN_DBGPRINTF(DBG_LEVEL_1, "<<Start test! Press 'q' to stop.>>\n");
    
    do
    {
        /* if key the space or enter, the burnin will stop. */
        c = ftuart_getc_t(10000, 0);
        if (c == 'q')
            break;
        
        /* 20 acceptable frame types */
        for(AFC1_i = 0; AFC1_i < 20; AFC1_i++)
        {
            /* 8 frame types */
            for(frameType_i = 0; frameType_i < 8; frameType_i++)
            {
                dev->FrameType = (Frame_Type)frameType_i;
                
                /* Reset Tx and Rx buffer */
                memset(&TxMessage, 0, sizeof(CanTxMsgDef));
                memset(&RxMessage, 0, sizeof(CanRxMsgDef));
                
                if(id_random)
                {
                    Generate_Random_Identifier(dev->FrameType, &SID, &EID);
                }
                else
                {
                    SID = 1234;
                    EID = 5678;
                }
                
                TxMessage.StdId = SID;
                if(dev->FrameType == CAN_B_Data || dev->FrameType == CAN_B_Remote || \
                   dev->FrameType == CAN_FD_B   || dev->FrameType == CAN_FD_B_BRS)
                {
                    TxMessage.ExtId = EID;
                    TxMessage.IDE = 1;
                    CAN_FilterConfig.Filter_ExtId = EID;
                }
                else
                {
                    TxMessage.ExtId = 0;
                    TxMessage.IDE = 0;
                    CAN_FilterConfig.Filter_ExtId = 0;
                }
                TxMessage.BRS = (dev->FrameType >= 6)?1:0;
                TxMessage.FD = (dev->FrameType >= 4)?1:0;
                
                CAN_FilterConfig.Filter_StdId = SID;
                CAN_FilterConfig.Filter_frameType = array_ft[AFC1_i];
                CAN_MaskConfig.Mask_StdId = 0;
                CAN_MaskConfig.Mask_ExtId = 0;
                CAN_MaskConfig.Mask_All = 1;
                
                /* Only set CAN2 rx filter and mask for receiving */
                CAN_Mode_Change(CAN2, CAN_Mode_Config);
                CAN_FilterInit(CAN2, &CAN_FilterConfig, 0);
                CAN_FilterGroup(CAN2, 0, ENABLE);
                CAN_MaskInit(CAN2, &CAN_MaskConfig, 0);
                CAN_MaskNumber(CAN2, 0, ENABLE);
                CAN_Mode_Change(CAN2, CAN_Mode_Normal);
                
                if(dev->FrameType == CAN_A_Remote || dev->FrameType == CAN_B_Remote)
                {
                    TxMessage.RTR = 1;
                    TxMessage.DLC = CAN_DataBytes_0;
                }
                else
                {
                    TxMessage.RTR = 0;
                    if(data_random)
                    {
                        /* Randomize the DLC according to frame type, and generate the data values. */
                        Generate_Random_Pattern_Length(dev->FrameType, &TxMessage.DLC, TxMessage.Data);
                    }
                    else
                    {
                        TxMessage.DLC = CAN_DataBytes_8;
                        memcpy(TxMessage.Data, &data, 8);
                    }
                }
                
                flag = 1;
                ETFLAG = 0;
                /* transfer message */
                CAN_Transmit_TXn(CAN1, &TxMessage, 0);
                while(CAN_GetBTRStatus(CAN1, 0));
                
                /* Receive message */
                ret = CAN_Receive_RXn_timeout(CAN2, 0, &RxMessage, CAN_Rx_Timeout);
                if(ret < 0)
                {
                    /* receive timeout 
                       case1: Error was happened, no message coming
                       case2: No error, but the message cannot pass the filter */
                    flag = 0;
                    
                    if(ETFLAG)
                    {
                        //while(1);
                        CAN_DBGPRINTF(DBG_LEVEL_0, "Error happened\n");
                        continue;
                    }
                    else
                    {
                        /* if case 2, confirm that this message should be accepted */
                        if(accept_tb[AFC1_i] & (1 << dev->FrameType))
                        {
                            CAN_DBGPRINTF(DBG_LEVEL_0, "Frame type:%s AFC1:%u", frame_type_table[dev->FrameType], array_ft[AFC1_i]);
                            return 0;
                        }
                    }
                    
                    
                    continue;
                }
                
                if(flag)
                {
                    /* Confirm that this message should not be accepted */
                    if(!(accept_tb[AFC1_i] & (1 << dev->FrameType)))
                    {
                        CAN_DBGPRINTF(DBG_LEVEL_0, "Frame type:%s AFC1:%u", frame_type_table[dev->FrameType], array_ft[AFC1_i]);
                        return 0;
                    }
                    
                    /* Verify message */
                    ret = Verify_Pattern(dev->FrameType, &TxMessage, &RxMessage);
                    if(ret != 0)
                        return ret;
                }
            }
            
            
        }
        counter++;
        CAN_DBGPRINTF(DBG_LEVEL_1, "\r%08d", counter);
        
    }while(dev->FreeRun);
    
        CAN_DBGPRINTF(DBG_LEVEL_1, " times");
    
    return ret;
}

/*******************************************************************************
                            Burn-in test functions 
*******************************************************************************/
int CAN_Speed_check(Frame_Type ft, CAN_InitTypeDef * can)
{
    const UINT32 maxSpeed = 1000000, minSpeed = 125000;
    UINT32 currentNSpeed, currentDSpeed;
    float sampleLimit, sampleTime;
    
    //Nominal speed must less than 1Mbps and greater than 125Kbps
    currentNSpeed = (CAN_PCLK / can->CAN_NBRP) / \
        (Sync_seg + can->CAN_NProp + can->CAN_NPS1 + can->CAN_NPS2);
    
    if(currentNSpeed > maxSpeed || currentNSpeed < minSpeed)
        return -1;
    
    
    if(ft > 5)
    {
        //Data speed must attend the 150ns sample time
        sampleTime = ((float)can->CAN_DBRP * 1000000000 / CAN_PCLK) * \
            (float)(Sync_seg + can->CAN_DProp + can->CAN_DPS1);
        
        currentDSpeed = (CAN_PCLK / can->CAN_DBRP) / \
        (Sync_seg + can->CAN_DProp + can->CAN_DPS1 + can->CAN_DPS2);
        
        if(currentDSpeed < currentNSpeed)
            return -2;
        
        if(can->CAN_EnDBE)
            sampleLimit = 180; //150ns + 3 * clock cycle time in 100MHz
        else
            sampleLimit = 150;
        
        if(sampleTime < sampleLimit)
            return -3;
    }
    
    return 1;
}

int CAN_Random_BurnIn_test()
{
    CAN_InitTypeDef CAN_InitData;
    CAN_FilterTypeDef CAN_FilterConfig;
    CAN_MaskTypeDef CAN_MaskConfig;
    CanTxMsgDef TxMessage;
    CanRxMsgDef RxMessage;
    Frame_Type ft;
    int rx, tx, NTqs, DTqs, DPRE_upper, filter, i, ret, counter = 0;
    float NSamplePer, DSamplePer;
    char c;
    
    /* Clear these structs value */
    memset(&CAN_InitData, 0, sizeof(CAN_InitTypeDef));
    memset(&CAN_FilterConfig, 0, sizeof(CAN_FilterTypeDef));
    memset(&CAN_MaskConfig, 0, sizeof(CAN_MaskTypeDef));
    memset(&TxMessage, 0, sizeof(CanTxMsgDef));
    memset(&RxMessage, 0, sizeof(CanRxMsgDef));

    while(1)
    {
        /* if key the space or enter, the burnin will stop. */
        c = ftuart_getc_t(10000, 0);
        if (c == 'q')
            break;

        CAN_DBGPRINTF(DBG_LEVEL_1, "\n====================================%08d====================================\n", ++counter);

        /* Random pattern */
        ft = rand()%8;

        ret = 0;
        DPRE_upper = 254;
        
        /* Randomized feature */ 
        CAN_InitData.CAN_Mode = CAN_Mode_Normal;
        CAN_InitData.CAN_RT = rand() % 4;
        CAN_InitData.CAN_TSE = rand() % 2;
        CAN_InitData.CAN_EnDBE = rand() % 2;
        
        do
        {
            //speed
            if (ret == 0 || ret == -1)
            {
                NTqs = 0;
                NSamplePer = 0;
                while(NTqs < 8 || NSamplePer < 0.5)
                {
                    CAN_InitData.CAN_NBRP = (rand() % 254) + 2;
                    CAN_InitData.CAN_NProp = (rand() % 64) + 1;
                    CAN_InitData.CAN_NPS1 = (rand() % 32) + 1;
                    CAN_InitData.CAN_NPS2 = (rand() % 30) + 2;
                    CAN_InitData.CAN_NSJW = (rand() % CAN_InitData.CAN_NPS2) + 1;
                    NTqs = Sync_seg + CAN_InitData.CAN_NProp + CAN_InitData.CAN_NPS1 + \
                        CAN_InitData.CAN_NPS2;
                    NSamplePer = (float)(Sync_seg + CAN_InitData.CAN_NProp + \
                        CAN_InitData.CAN_NPS1) / NTqs;
                }
            }
            
            if(ft > 5)
            {
                DTqs = 0;
                DSamplePer = 0;
                DPRE_upper = (DPRE_upper > 2) ? (DPRE_upper - 1): 2; //The higher DBRP will cause checking fail. This decreasement could increase the probability of ckecking pass.
                //BRS
                while(DTqs < 8 || DSamplePer < 0.5)
                {
                    CAN_InitData.CAN_DBRP = (rand() % DPRE_upper) + 2;
                    CAN_InitData.CAN_DProp = (rand() % 9);
                    CAN_InitData.CAN_DPS1 = (rand() % 8) + 1;
                    CAN_InitData.CAN_DPS2 = (rand() % 7) + 2;
                    CAN_InitData.CAN_DSJW = (rand() % CAN_InitData.CAN_DPS2) + 1;
                    DTqs = Sync_seg + CAN_InitData.CAN_DProp + CAN_InitData.CAN_DPS1 + \
                        CAN_InitData.CAN_DPS2;
                    DSamplePer = (float)(Sync_seg + CAN_InitData.CAN_DProp + \
                        CAN_InitData.CAN_DPS1) / DTqs;
                }
            }
            
            ret = CAN_Speed_check(ft, &CAN_InitData);
        }
        while(ret < 0);
        
        /* According frame type to randomize the frame context */
        Generate_Frame(ft, &TxMessage);
        
        /* Randomized the Rx and Tx number */
        rx = rand() % 2;
        tx = rand() % 3; 
        
        /* Dump all settings */
        CAN_DBGPRINTF(DBG_LEVEL_1, "Frame Setting:\n");
        CAN_DBGPRINTF(DBG_LEVEL_1, "  %s format\n", frame_type_table[ft]);
        if(ft == CAN_A_Data || ft == CAN_A_Remote || 
           ft == CAN_FD_A || ft == CAN_FD_A_BRS)
        {
            CAN_DBGPRINTF(DBG_LEVEL_1, "  Based Identifier: %d(0x%08x)\n",
                                        TxMessage.StdId, TxMessage.StdId);
        }
        else
        {
            CAN_DBGPRINTF(DBG_LEVEL_1, "  Extended Identifier: %d(0x%08x)\n",
                (((TxMessage.StdId & 0x7ff) << 18) | (TxMessage.ExtId & 0x3ffff)), 
                (((TxMessage.StdId & 0x7ff) << 18) | (TxMessage.ExtId & 0x3ffff)));
        }
        if(ft != CAN_A_Remote && ft != CAN_B_Remote)
        {
            CAN_DBGPRINTF(DBG_LEVEL_1, "  Data Length: %d\n", DLCtoBytes_table[TxMessage.DLC]);
            if(TxMessage.DLC > 0)
            {
                CAN_DBGPRINTF(DBG_LEVEL_1, "  Data Pattern:");
                for(i = 0; i < DLCtoBytes_table[TxMessage.DLC]; i++)
                {
                    if((i%10) == 0)
                        CAN_DBGPRINTF(DBG_LEVEL_1, "\n\t");
                    
                    CAN_DBGPRINTF(DBG_LEVEL_1, "%02X ", TxMessage.Data[i]);
                }
                 CAN_DBGPRINTF(DBG_LEVEL_1, "\n");
            }
        }
        CAN_DBGPRINTF(DBG_LEVEL_1, "Bit Timing Setting:\n");
        CAN_DBGPRINTF(DBG_LEVEL_1, "  Arbitration phase:\n  %dbps. SPP=%f%%\n  Time Quanta(N):%d Prescaler(N):%d\n", 
                                   (CAN_PCLK / CAN_InitData.CAN_NBRP) / NTqs, 
                                   (NSamplePer * 100), 
                                   NTqs, 
                                   CAN_InitData.CAN_NBRP);
        CAN_DBGPRINTF(DBG_LEVEL_1, "  (Sync,Prop,PS1,PS2) = (1,%d,%d,%d) SJW(N):%d.\n", 
                                   CAN_InitData.CAN_NProp, 
                                   CAN_InitData.CAN_NPS1, 
                                   CAN_InitData.CAN_NPS2, 
                                   CAN_InitData.CAN_NSJW);
        if(ft > 5)
        {
            CAN_DBGPRINTF(DBG_LEVEL_1, "  Data phase:\n  %dbps. SPP=%f%%\n  Time Quanta(D):%d Prescaler(D):%d\n", 
                                       (CAN_PCLK / CAN_InitData.CAN_DBRP) / DTqs, 
                                       (DSamplePer * 100), 
                                       DTqs, 
                                       CAN_InitData.CAN_DBRP);
            CAN_DBGPRINTF(DBG_LEVEL_1, "  (Sync,Prop,PS1,PS2) = (1,%d,%d,%d) SJW(D):%d.\n", 
                                       CAN_InitData.CAN_DProp, 
                                       CAN_InitData.CAN_DPS1, 
                                       CAN_InitData.CAN_DPS2, 
                                       CAN_InitData.CAN_DSJW);
            CAN_DBGPRINTF(DBG_LEVEL_1, "  Sampling interval time:%02fns\n",
                                        ((float)CAN_InitData.CAN_DBRP * 1000000000 / CAN_PCLK) * \
                                         (float)(Sync_seg + CAN_InitData.CAN_DProp + CAN_InitData.CAN_DPS1));
        }
        CAN_DBGPRINTF(DBG_LEVEL_1, "Feature Setting:\n  TimeStamp: %s. Debounded: %s. Retransmission: %s.\n",
                                    boolean_str[CAN_InitData.CAN_TSE], 
                                    boolean_str[CAN_InitData.CAN_EnDBE],
                                    RT_str[CAN_InitData.CAN_RT]);
        CAN_DBGPRINTF(DBG_LEVEL_1, "Tx/Rx Setting:\n  Tx buffer %d -> Rx FIFO %d\n\n", tx, rx);
        
                                   
        /* Reset the CAN1 and CAN2 */
        CAN_Reset(CAN1);
        CAN_Reset(CAN2);
        
        /* Filter and Mask set */
        CAN_FilterGroup(CAN2, 0, DISABLE);
        CAN_FilterGroup(CAN2, 1, DISABLE);
        CAN_MaskNumber(CAN2, 0, DISABLE);
        CAN_MaskNumber(CAN2, 1, DISABLE);
        CAN_FilterConfig.Filter_StdId = TxMessage.StdId;
        CAN_FilterConfig.Filter_ExtId = TxMessage.ExtId;
        CAN_FilterConfig.Filter_frameType = CAN_ALL;
        CAN_MaskConfig.Mask_StdId = TxMessage.StdId;
        CAN_MaskConfig.Mask_ExtId = TxMessage.ExtId;
        if(rx == 0)
        {
            filter = (rand() % 4); 
            CAN_FilterInit(CAN2, &CAN_FilterConfig, filter);
            CAN_FilterGroup(CAN2, 0, ENABLE);
            CAN_MaskInit(CAN2, &CAN_MaskConfig, 0);
            CAN_MaskNumber(CAN2, 0, ENABLE);
        }
        else if(rx == 1)
        {
            filter = (rand() % 2) + 4; 
            CAN_FilterInit(CAN2, &CAN_FilterConfig, filter);
            CAN_FilterGroup(CAN2, 1, ENABLE);
            CAN_MaskInit(CAN2, &CAN_MaskConfig, 1);
            CAN_MaskNumber(CAN2, 1, ENABLE);
        }
        
        /* CAN1 and CAN2 interrupt setting */
        irq_setting(IRQ_CAN1, (PrHandler)CAN1Handler);
        CAN_ITConfig(CAN1, CAN_IRE_EIE, ENABLE); 
        irq_setting(IRQ_CAN2, (PrHandler)CAN2Handler);
        CAN_ITConfig(CAN2, CAN_IRE_EIE, ENABLE); 
        
        /* CAN1 init and standby the config mode */
        CAN_Init(CAN1, &CAN_InitData);
        CAN_Init(CAN2, &CAN_InitData);
        
        /* Each set will run 1000 times */
        for(i = 0; i < 1000; i++)
        {
            CAN_Transmit_TXn(CAN1, &TxMessage, tx);
            while(CAN_GetBTRStatus(CAN1, tx));
            
            /* receive message */
            ret = CAN_Receive_RXn_timeout(CAN2, rx, &RxMessage, CAN_Rx_Timeout);
            if(ret < 0)
            {
                CAN_DBGPRINTF(DBG_LEVEL_0, "FAIL!\n");
                break;
            }
            
            /* Verify pattern */
            if(CAN_InitData.CAN_TSE && TxMessage.DLC >= 2)
            {
                TxMessage.Data[DLCtoBytes_table[TxMessage.DLC] - 2] = 
                    RxMessage.Data[DLCtoBytes_table[TxMessage.DLC] - 2];
                TxMessage.Data[DLCtoBytes_table[TxMessage.DLC] - 1] = 
                    RxMessage.Data[DLCtoBytes_table[TxMessage.DLC] - 1];
            }
            ret = Verify_Pattern(ft, &TxMessage, &RxMessage);
            if(ret != 0)
            {
                CAN_DBGPRINTF(DBG_LEVEL_0, "FAIL! (%s)\n", err_msg[ret]);
                break;
            }
            
            if((i%20) == 0)
                CAN_DBGPRINTF(DBG_LEVEL_2, ">");
        }
        
        CAN_DBGPRINTF(DBG_LEVEL_0, "PASS!\n");
        
    }//while(1) loop
		return 0;
}


/*******************************************************************************
                          APB Register Initial function
*******************************************************************************/
#ifdef APB_REG_TEST 
void APB_Reg_Init(void)
{
    UINT32 i, j, reg_index;
    APB_Wrapper_Reg = 0x50110000;
    for (i = 0; i < Max_Reg_Num; i++) {
        APB_Wrapper_Reg[i].Address_Offset = 0;
        APB_Wrapper_Reg[i].Default_Value = 0;
        for (j = 0; j < 8; j++) {
            APB_Wrapper_Reg[i].Bit_Type[j] = Bit_Type_RO;
        }
    }
    
    reg_index = 0;
    
    /* CR, offset=0x0000 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 0;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
		fLib_printf(" 1 \n");				
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
		fLib_printf("%x size of Register_Definition %x \n",&APB_Wrapper_Reg[reg_index].Bit_Type[6],sizeof(Register_Definition));
		
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
		fLib_printf(" 2 \n");
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
		fLib_printf(" 3 \n");		
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
		fLib_printf(" 4 \n");				
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
		fLib_printf(" 5 \n");				
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
		fLib_printf(" 6 \n");				
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
		fLib_printf(" 7 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
		fLib_printf(" 8 \n");						
    reg_index++;
    /* IRE, offset=0x0001 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 1;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
		fLib_printf(" 9 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
		fLib_printf(" a \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
		fLib_printf(" b \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
		fLib_printf(" c \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
		fLib_printf(" c1 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
		fLib_printf(" c2 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
		fLib_printf(" c3 \n");						
    reg_index++;
    /* CE0, offset=0x0002 */
		fLib_printf(" d \n");						
    APB_Wrapper_Reg[reg_index].Address_Offset = 2;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
		fLib_printf(" e \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_Rsvd;
		fLib_printf(" f \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_Rsvd;
		fLib_printf(" g \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
		fLib_printf(" h \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
		fLib_printf(" i \n");						
    /* CE1, offset=0x0003 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 3;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
		fLib_printf(" j \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_Rsvd;
		fLib_printf(" j1 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
		fLib_printf(" k \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
		fLib_printf(" k1 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
		fLib_printf(" k2 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
		fLib_printf(" l \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
		fLib_printf(" m \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RW;
    reg_index++;
		fLib_printf(" m1 \n");				    
    /* BS, offset=0x0004 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 4;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
		fLib_printf(" m2 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RO;
		fLib_printf(" m21 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RO;
		fLib_printf(" m3 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RO;
		fLib_printf(" m4\n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
		fLib_printf(" m5\n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
		fLib_printf(" m6 \n");						
    /* TRBS, offset=0x0005 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 5;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
		fLib_printf(" n \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_Rsvd;
		fLib_printf(" n0 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_Rsvd;
		fLib_printf(" n1 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_Rsvd;
		fLib_printf(" n11 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RO;
		fLib_printf(" n2 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RO;
		fLib_printf(" n22 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RO;
		fLib_printf(" n3 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RO;
		fLib_printf(" n33 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
		fLib_printf(" n4 \n");						
    reg_index++;
    /* TS, offset=0x0006 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 6;
		fLib_printf(" n5 \n");						
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RO;
		fLib_printf(" n6 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RO;
		fLib_printf(" n7 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RO;
		fLib_printf(" n8 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RO;
		fLib_printf(" n9 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RO;
		fLib_printf(" n9 \n");						
    reg_index++;
    /* Reserved, offset=0x0007 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 7;
		fLib_printf(" n9 \n");						
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
		fLib_printf(" n9 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_Rsvd;
		fLib_printf(" n9 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_Rsvd;
		fLib_printf(" n9 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_Rsvd;
		fLib_printf(" n9 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
		fLib_printf(" n9 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_Rsvd;
		fLib_printf(" n9 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
		fLib_printf(" n9 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
		fLib_printf(" n9 \n");						
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
		fLib_printf(" n9999 \n");						
    reg_index++;
		fLib_printf(" o \n");				    
    for(i = 0; i < 3; i++)
    {
        /* TFDi(i=0,1,2), offset=0x0008,0x0010,0x0018 */
        APB_Wrapper_Reg[reg_index].Address_Offset = 8 + (i*8);
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
        reg_index++;
        /* TREIEi(i=0,1,2), offset=0x0009,0x0011,0x0019 */
        APB_Wrapper_Reg[reg_index].Address_Offset = 9 + (i*8);
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
        reg_index++;
        /* TEILi(i=0,1,2), offset=0x000A,0x0012,0x001A */
        APB_Wrapper_Reg[reg_index].Address_Offset = 10 + (i*8);
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
        reg_index++;
        /* TEIMi(i=0,1,2), offset=0x000B,0x0013,0x001B */
        APB_Wrapper_Reg[reg_index].Address_Offset = 11 + (i*8);
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RW;
        reg_index++;
        
        /* TEIHi(i=0,1,2), offset=0x000C,0x0014,0x001C */
        APB_Wrapper_Reg[reg_index].Address_Offset = 12 + (i*8);
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RW;
        reg_index++;
        /* TBILDLi(i=0,1,2), offset=0x000D,0x0015,0x001D */
        APB_Wrapper_Reg[reg_index].Address_Offset = 13 + (i*8);
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RW;
        reg_index++;
        /* TBIHi(i=0,1,2), offset=0x000E,0x0016,0x001E */
        APB_Wrapper_Reg[reg_index].Address_Offset = 14 + (i*8);
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RW;
        reg_index++;
        /* Reserved, offset=0x000F,0x0017,0x001F */
        APB_Wrapper_Reg[reg_index].Address_Offset = 15 + (i*8);
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
        reg_index++;
    }
    
    for(i = 0; i < 6; i++)
    {
        /* AFEILi, offset=0x0020,0x0024,0x0028,0x002C,0x0030,0x0034 */
        APB_Wrapper_Reg[reg_index].Address_Offset = 32 + (i*4);
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
        reg_index++;
        /* AFEIMi, offset=0x0021,0x0025,0x0029,0x002D,0x0031,0x0035 */
        APB_Wrapper_Reg[reg_index].Address_Offset = 33 + (i*4);
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RW;
        reg_index++;
        /* AFBILEIHi, offset=0x0022,0x0026,0x002A,0x002E,0x0032,0x0036 */
        APB_Wrapper_Reg[reg_index].Address_Offset = 34 + (i*4);
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RW;
        reg_index++;
        /* AFBIHi, offset=0x0023,0x0027,0x002B,0x002F,0x0033,0x0037 */
        APB_Wrapper_Reg[reg_index].Address_Offset = 35 + (i*4);
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RW;
        reg_index++;
    }
    
    /* AFDB00, offset=0x0038 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 56;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RW;
    reg_index++;
    /* AFDB10, offset=0x0039 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 57;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RW;
    reg_index++;
    /* AFDB01, offset=0x003A */
    APB_Wrapper_Reg[reg_index].Address_Offset = 58;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RW;
    reg_index++;
    /* AFDB11, offset=0x003B */
    APB_Wrapper_Reg[reg_index].Address_Offset = 59;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RW;
    reg_index++;
    
    /* AFC0, offset=0x003C */
    APB_Wrapper_Reg[reg_index].Address_Offset = 60;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
    /* AFC1, offset=0x003D */
    APB_Wrapper_Reg[reg_index].Address_Offset = 61;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
    /* Reserved, offset=0x003E */
    APB_Wrapper_Reg[reg_index].Address_Offset = 62;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
    /* Reserved, offset=0x003F */
    APB_Wrapper_Reg[reg_index].Address_Offset = 63;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
    
    for(i = 0; i < 2; i++)
    {
        /* FMEILi, offset=0x0040,0x0044 */
        APB_Wrapper_Reg[reg_index].Address_Offset = 64 + (i*4);
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
        reg_index++;
        /* FMEIMi, offset=0x0041,0x0045 */
        APB_Wrapper_Reg[reg_index].Address_Offset = 65 + (i*4);
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RW;
        reg_index++;
        /* FMBILEIHi, offset=0x0042,0x0046 */
        APB_Wrapper_Reg[reg_index].Address_Offset = 66 + (i*4);
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RW;
        reg_index++;
        /* FMBIHi, offset=0x0043,0x0047 */
        APB_Wrapper_Reg[reg_index].Address_Offset = 67 + (i*4);
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RW;
        reg_index++;
    }
    
    /* FMDB00, offset=0x0048 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 72;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RW;
    reg_index++;
    /* FMDB10, offset=0x0049 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 73;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RW;
    reg_index++;
    /* FMDB01, offset=0x004A */
    APB_Wrapper_Reg[reg_index].Address_Offset = 74;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RW;
    reg_index++;
    /* FMDB11, offset=0x004B */
    APB_Wrapper_Reg[reg_index].Address_Offset = 75;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RW;
    reg_index++;
    
    /* FMC, offset=0x004C */
    APB_Wrapper_Reg[reg_index].Address_Offset = 76;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
    /* Reserved, offset=0x004D */
    APB_Wrapper_Reg[reg_index].Address_Offset = 77;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
    /* Reserved, offset=0x004E */
    APB_Wrapper_Reg[reg_index].Address_Offset = 78;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
    /* Reserved, offset=0x004F */
    APB_Wrapper_Reg[reg_index].Address_Offset = 79;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
    
    for(i = 0; i < 2; i++)
    {
        /* RFDi, offset=0x0050,0x0058 */
        APB_Wrapper_Reg[reg_index].Address_Offset = 80 + (i*8);
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
        reg_index++;
        /* RREIEi, offset=0x0051,0x0059 */
        APB_Wrapper_Reg[reg_index].Address_Offset = 81 + (i*8);
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
        reg_index++;
        /* REILi, offset=0x0052,0x005A */
        APB_Wrapper_Reg[reg_index].Address_Offset = 82 + (i*8);
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
        reg_index++;
        /* REIMi, offset=0x0053,0x005B */
        APB_Wrapper_Reg[reg_index].Address_Offset = 83 + (i*8);
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RO;
        reg_index++;
        /* REIHi, offset=0x0054,0x005C */
        APB_Wrapper_Reg[reg_index].Address_Offset = 84 + (i*8);
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RO;
        reg_index++;
        /* RBILDLi, offset=0x0055,0x005D */
        APB_Wrapper_Reg[reg_index].Address_Offset = 85 + (i*8);
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RO;
        reg_index++;
        /* RBIHi, offset=0x0056,0x005E */
        APB_Wrapper_Reg[reg_index].Address_Offset = 86 + (i*8);
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RO;
        reg_index++;
        /* Reserved, offset=0x0057, 0x005F */
        APB_Wrapper_Reg[reg_index].Address_Offset = 87 + (i*8);
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
        reg_index++;
    }
    
    /* NBTC0, offset=0x0060 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 96;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
    /* NBTC1, offset=0x0061 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 97;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x01;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RW;
    reg_index++;
    /* NBTC2, offset=0x0062 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 98;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
    /* NBTC3, offset=0x0063 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 99;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
    
    /* NBTC4, offset=0x0064 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 100;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
    /* Reserved, offset=0x0065 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 101;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
    /* Reserved, offset=0x0066 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 102;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
    /* Reserved, offset=0x0067 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 103;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
    
    /* DBTC0, offset=0x0068 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 104;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x01;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RW;
    reg_index++;
    /* DBTC1, offset=0x0069 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 105;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
    /* DBTC2, offset=0x006A */
    APB_Wrapper_Reg[reg_index].Address_Offset = 106;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
    /* DBTC3, offset=0x006B */
    APB_Wrapper_Reg[reg_index].Address_Offset = 107;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
    
    /* RECL, offset=0x006C */
    APB_Wrapper_Reg[reg_index].Address_Offset = 108;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RO;
    reg_index++;
    /* RECH, offset=0x006D */
    APB_Wrapper_Reg[reg_index].Address_Offset = 109;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RO;
    reg_index++;
    /* TECL, offset=0x006E */
    APB_Wrapper_Reg[reg_index].Address_Offset = 110;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RO;
    reg_index++;
    /* TECH, offset=0x006F */
    APB_Wrapper_Reg[reg_index].Address_Offset = 111;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RO;
    reg_index++;
    
    /* ET, offset=0x0070 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 112;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
    /* TECH, offset=0x0071 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 113;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
    /* Reserved, offset=0x0072 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 114;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
    /* Reserved, offset=0x0073 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 115;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
    
    /* RBTSL0, offset=0x0074 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 116;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RO;
    reg_index++;
    /* RBTSH0, offset=0x0075 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 117;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RO;
    reg_index++;
    /* RBTSL1, offset=0x0076 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 118;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RO;
    reg_index++;
    /* RBTSH1, offset=0x0077 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 119;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RO;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RO;
    reg_index++;
    
    /* IR, offset=0x0078 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 120;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW1C;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW1C;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW1C;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW1C;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW1C;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW1C;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW1C;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RW1C;
    reg_index++;
    /* Reserved, offset=0x0079 */
    APB_Wrapper_Reg[reg_index].Address_Offset = 121;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
    /* Reserved, offset=0x007A */
    APB_Wrapper_Reg[reg_index].Address_Offset = 122;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
    /* Reserved, offset=0x007B */
    APB_Wrapper_Reg[reg_index].Address_Offset = 123;
    APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
    APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_Rsvd;
    APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_Rsvd;
    reg_index++;
    
    for(i = 0; i < 192; i++)
    {
        APB_Wrapper_Reg[reg_index].Address_Offset = 124 + i;
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RW;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RW;
        reg_index++;
    }
    
    for(i = 0; i < 128; i++)
    {
        APB_Wrapper_Reg[reg_index].Address_Offset = 316 + i;
        APB_Wrapper_Reg[reg_index].Default_Value = 0x00;
        APB_Wrapper_Reg[reg_index].Bit_Type[7] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[6] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[5] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[4] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[3] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[2] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[1] = Bit_Type_RO;
        APB_Wrapper_Reg[reg_index].Bit_Type[0] = Bit_Type_RO;
        reg_index++;
    }
}
#endif