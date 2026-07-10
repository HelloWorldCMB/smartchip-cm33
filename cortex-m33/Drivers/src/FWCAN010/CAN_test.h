#ifndef __CAN_TEST_H
#define __CAN_TEST_H

#include <stdarg.h>
#include <stdio.h>
#include "portme.h"
#include "types.h"
#include "CAN_type.h"

#define IRQ_CAN1 58
#define IRQ_CAN2 59
#define IRQ_CAN3 15

/******************* User choose CANs controller and setting ******************/
#define APB_USE
#define CAN1_FTCAN
#define CAN2_FTCAN
//#define CAN3_FTCAN
/******************************************************************************/

#define IPT_Tq 2
#define Sync_seg 1
#define NumberOfRound 1
#define CAN_PCLK 0x5F5E100 //100Mhz, the SYSCLK into CAN controller on FPGA

#define CAN_Tx_Timeout 0x4000 //Tx transmit timeout value
#define CAN_Rx_Timeout 0x4000 //Rx transmit timeout value

typedef enum
{
    CAN_A_Data = 0,
    CAN_A_Remote,
    CAN_B_Data,
    CAN_B_Remote,
    CAN_FD_A,
    CAN_FD_B,
    CAN_FD_A_BRS,
    CAN_FD_B_BRS,
    /* The below definitions are for random frame type test use */
    CAN_Classical_Random = 100,
    CAN_FD_Random
}Frame_Type;

static const char * frame_type_table[] = {
    "2.0A Data",
    "2.0A Remote",
    "2.0B Data",
    "2.0B Remote",
    "FD based noBRS",
    "FD extended noBRS",
    "FD based BRS",
    "FD extended BRS"
};

static const char * boolean_str[] = {
    "DISABLE",
    "ENABLE",
};

static const char * RT_str[] = {
    "Always",
    "1 time",
    "3 times",
    "8 time",
};

/* APB Register definition */
typedef enum
{
    Bit_Type_RO = 0,
    Bit_Type_RW = 1,
    Bit_Type_RW1C = 2,
    Bit_Type_Rsvd = 3,
    Bit_DontTouchMe
}Bit_Attribute;

typedef struct
{
    UINT32            Address_Offset;
    UINT8           Default_Value;
    Bit_Attribute   Bit_Type[8];
	  //UINT8   Bit_Type[8];
}Register_Definition;

#define Max_Reg_Num 443


typedef enum
{
    //BIT_RATE_10kbps
    //BIT_RATE_20kbps 
    //BIT_RATE_50kbps
    //BIT_RATE_100kbps
    BIT_RATE_125kbps = 0,
    BIT_RATE_250kbps,
    BIT_RATE_500kbps,
    BIT_RATE_800kbps,
    BIT_RATE_1Mbps,
    BIT_RATE_NUMBER
}Bit_Rate;

typedef enum
{
    Fast_BIT_RATE_1Mbps = 0,
    Fast_BIT_RATE_2Mbps,
    Fast_BIT_RATE_2_5Mbps,
    Fast_BIT_RATE_5Mbps,
    Fast_BIT_RATE_NUMBER
}Fast_Bit_Rate;


typedef enum
{
    LOOPBACK_MODE = 0,
    NORMAL_MODE,
    LISTEN_MODE,
    SLEEP_MODE
}CAN_MODE;


typedef struct
{
    BOOL FD_mode;
    /* Nominal bit rate */
    int NPRE;
    int NPROP;
    int NPS1;
    int NPS2;
    int NSJW;
    /* Daba bit rate */
    int DPRE;
    int DPROP;
    int DPS1;
    int DPS2;
    int DSJW;
    /* frame type */
    Frame_Type FrameType;
    /* Free run */
    BOOL FreeRun;
}CAN_Param;

/*********************** Debug print function **********************************
    Introduction:
        Because the 'printf' function would cause the delay for UART operation,
        this debug print function can reduce the non-important infomation be
        printed to screen.
        There are three methods for handle the 'CAN_DBGPRINTF'. It can be 
        set by the 'USE_DBG' definition:
            If it was set to 0, no print work.
            If it was set to 1, normal print work (call prints function)
            If it was set to 2, print the info into memory (call snprintf function)
            
        And four levels are provided to decide this 'CAN_DBGPRINTF' should be 
        handled or not:
        ex: CAN_DBGPRINTF(DBG_LEVEL_X, "Hello\n");
        The first parameter DBG_LEVEL_X:
            DBG_LEVEL_0: Important Error infomation
            DBG_LEVEL_1: Some test infomation
            DBG_LEVEL_2: Some loading or tips infomation
            DBG_LEVEL_3: Just for debug use
        User can set 'DBG_LEVEL' to decide which level can be worked.
        ex: If DBG_LEVEL 3, all the DBG_LEVEL_0,1,2,3 are accepted.
            If DBG_LEVEL 1, only the DBG_LEVEL_0,1 are accepted.
        
        For error string, user can use ERRORCODE to print the string from 
        err_msg[]. This number can directly return.
        ex:
            if(ret)
            {
                CAN_DBGPRINTF(DBG_LEVEL_0, "%s",err_msg[ERRORCODE]);
            }
*******************************************************************************/
#define MAX_DBMSG_SIZE 500 //The space of snprintf
#define USE_DBG 1 //0:no printf, 1:normal printf, 2:snprintf
#define DBG_LEVEL 3
typedef enum
{
    DBG_LEVEL_0 = 0,
    DBG_LEVEL_1,
    DBG_LEVEL_2,
    DBG_LEVEL_3
}DBGLEVEL;

#if USE_DBG == 0
    #define CAN_DBGPRINTF(level, f_, ...) 
#elif USE_DBG == 1
    #define CAN_DBGPRINTF(level, f_, ...) \
            if (level <= DBG_LEVEL) { \
                fLib_printf(f_, ##__VA_ARGS__); \
            }
#elif USE_DBG == 2
    #define CAN_DBGPRINTF(level, f_, ...) \
            if (level <= DBG_LEVEL) { \
                snprintf(CAN_DBGMSG + strlen(CAN_DBGMSG), MAX_DBMSG_SIZE, f_, ##__VA_ARGS__); \
            }
    
#endif

/* For testing */
#if USE_DBG == 0
    #define CAN_DBGPRINTF2(level, args) 
#elif USE_DBG == 1
    #define CAN_DBGPRINTF2(level, args) \
            if (level <= DBG_LEVEL) { \
                (fLib_printf args); \
            }
#elif USE_DBG == 2
    #define CAN_DBGPRINTF2(level, f_, ...) \
            if (level <= DBG_LEVEL) { \
                snprintf(CAN_DBGMSG + strlen(CAN_DBGMSG), MAX_DBMSG_SIZE, args); \
            }
    
#endif


/* ERRORCODE and error_message[] are mapped */
typedef enum
{
    Test_Normal = 0,
    /* Register error */
    CAN_Reg_Default_ERROR,
    CAN_REG_RO_ERROR,
    CAN_REG_RW_ERROR,
    CAN_REG_RW1C_ERROR,
    CAN_REG_Rsvd_ERROR,
    /* Timeout error */
    CAN_Rx_TIMEOUT_ERROR,
    /* Verification error */
    CAN_IDE_ERROR,
    CAN_DLC_ERROR,
    CAN_RTR_ERROR,
    CAN_FD_ERROR,
    CAN_ESI_BIT,
    CAN_BRS_ERROR,
    CAN_STDID_ERROR,
    CAN_EXTID_ERROR,
    CAN_DATA_ERROR,
    /* Check status */
    CAN_Mode_ERROR,
    CAN_Loopback_ERROR,
    CAN_Listen_ERROR,
    CAN_ERROR_STATUS,
    CAN_OVERRUN_STATUS,
    CAN_WAKEUP_STATUS,
    /* Overrun test function */
    CAN_BRSn_Uncorrect,
    CAN_DO_Uncorrect,
    CAN_OE_ERROR
}ERRORCODE;
static const char * err_msg[] = {
    "Test\n",
    /* Register error */
    "The regitser default value error.\n",
    "The RO register error.\n",
    "The RW register error.\n",
    "The RW1C register error.\n",
    "The Rsvd register error.\n",
    /* Timeout error */
    "Rx timeout.\n",
    /* Verification error */
    "Rx IDE Error.\n",
    "Rx DLC Error.\n",
    "Rx RTR Error.\n",
    "Rx FD Error.\n",
    "Rx ESI=1.\n",
    "Rx BRS Error.\n",
    "Rx StdID Error.\n",
    "Rx ExtID Error.\n",
    "Rx Data Error.\n",
    /* Check status */
    "Go to wrong mode (OMR).\n",
    "Receive the others message in loopback mode.\n",
    "In listen mode, CAN must not send messages to bus.\n",
    "Error interrupt status.\n",
    "Overrun interrupt status.\n",
    "Wakeup interrupt status.\n",
    /* Overrun test function */
    "The FIFO BRSn field is wrong.\n",
    "The DO field is wrong\n",
    "The OE bit is not cleared.\n"
};
/******************************************************************************/

static const char * errorType_msg[8] = {
    "",
    "", 
    "Overrun Error",
    "Ack Error",
    "Form Error",
    "CRC Error",
    "Stuff Error",
    "Bit Error"
};


/********************************* Global API *********************************/
/* Regitser test functions */
#ifdef APB_REG_TEST
int CAN_Register_default_value_test(void);
int CAN_Register_Attribute_test(void);
#endif
/* Mode test functions */
int CAN_Mode_Change_test(CAN_MODE mode);

/* Simple test functions */
int CAN_Transmission_polling_template(void);
int CAN_Transmission_interrupt_template(void);
int CAN_Simple_Transmission_test(CAN_Param *dev);
int CAN_Simple_Transmission_Interrupt_test(CAN_Param *dev);
int CAN_data_length_test(CAN_Param *dev);

/* Bit timing test functions */
int CAN_Bit_Timing_Classical_test1(UINT32 bitrate, float SPP, char opt1, char opt2, char opt3);
int CAN_Bit_Timing_Classical_test2(void);
int CAN_Bit_Timing_Classical_test3(void);
int CAN_Bit_Timing_FD_test1(UINT32 bitrate1, UINT32 bitrate2, float SPP1, float SPP2, char opt1, char opt2, char opt3);
int CAN_Bit_Timing_FD_test2(void);

/* Special test functions */
int CAN_ACKError_test(void);
int CAN_CRCError_test(void);
int CAN_Overrun_case_burnin_test(CAN_Param *dev);
int CAN_GenError_Transmitter_test(void);
int CAN_GenError_Receiver_test(void);
int CAN_Arbitration_random_test(void); 
int CAN_TimeStamp_test(CAN_Param *dev);
int CAN_Filter_Frame_Type_test(CAN_Param *dev);
int CAN_Random_BurnIn_test(void);

/* Other test functions */
int CAN_DBGMSG_test(void);
float Oscillator_Tolerance_Calc(CAN_Param *dev, float *df);



#endif