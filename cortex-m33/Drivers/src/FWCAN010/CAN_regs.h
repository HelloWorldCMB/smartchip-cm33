#ifndef __CAN_REGS_H
#define __CAN_REGS_H
//#include "CAN_type.h"
#include "types.h"

/* CAN registers structure */
typedef struct{
    /***************************************************************************
                    Byte 3       Byte 2       Byte 1       Byte 0     Offset
                .....................................................
    CAN_Control |    CE1     |    CE0     |    IRE     |     CR     | 0x0000
                .....................................................
    CAN_Status  |  RESERVED  |     TS     |    TRBS    |     BS     | 0x0004
                .....................................................
    ***************************************************************************/
    union{
        struct{
            volatile UINT8 CRR;        //Command Request Register
            volatile UINT8 IRE;        //Interrupt Request Enable Register
            volatile UINT8 CE0;        //Control Enable 0 Register
            volatile UINT8 CE1;        //Control Enable 1 Register
        }Byte;
        volatile UINT32 All;
    }CAN_Control;
    union{
        struct{
            volatile UINT8 BSR;        //Bus Status Register
            volatile UINT8 TRBS;    //Transmit and Receive Buffer Status Register
            volatile UINT8 TS;        //Transmit Status Register
            volatile UINT8 RESERVED3;
        }Byte;
        volatile UINT32 All;
    }CAN_Status;
    
    /***************************************************************************
                    Byte 3       Byte 2       Byte 1       Byte 0     Offset
                .....................................................
    CAN_TR0     |    TEIM    |    TEIL    |   TREIE    |    TFD     | 0x0008
                .....................................................
                |  RESERVED  |    TBIH    |   TBILDL   |    TEIH    | 0x000C
                .....................................................
    CAN_TR1     |    TEIM    |    TEIL    |   TREIE    |    TFD     | 0x0010
                .....................................................
                |  RESERVED  |    TBIH    |   TBILDL   |    TEIH    | 0x0014
                .....................................................
    CAN_TR2     |    TEIM    |    TEIL    |   TREIE    |    TFD     | 0x0018
                .....................................................
                |  RESERVED  |    TBIH    |   TBILDL   |    TEIH    | 0x001C
                .....................................................
    ***************************************************************************/
    /* Transmit Register 0,1,2 */
    struct{
        union{
            struct{
                volatile UINT8 TFD;        //Transmit Flexible Data-Rate Register
                volatile UINT8 TREIE;        //Transmit Remote and Extended Identifier Enable Register
                volatile UINT8 TEIL;        //Transmit Extended Identifier Low Register
                volatile UINT8 TEIM;        //Transmit Extended Identifier Middle Register
            }Byte;
            volatile UINT32 All;
        }TX_L;
        union{
            struct{
                volatile UINT8 TEIH;        //Transmit Extended Identifier High Register
                volatile UINT8 TBILDL;    //Transmit Base Identifier Low and Data Length Register
                volatile UINT8 TBIH;        //Transmit Base Identifier High Register
                volatile UINT8 RESERVED;
            }Byte;
            volatile UINT32 All;
        }TX_H;
    }CAN_TR[3];
    
    /***************************************************************************
                    Byte 3       Byte 2       Byte 1       Byte 0     Offset
                .....................................................
    CAN_AFIR0   |   AFBIH    |  AFBILEIH  |   AFEIM    |   AFEIL    | 0x0020
                .....................................................
    CAN_AFIR1   |   AFBIH    |  AFBILEIH  |   AFEIM    |   AFEIL    | 0x0024
                .....................................................
    CAN_AFIR2   |   AFBIH    |  AFBILEIH  |   AFEIM    |   AFEIL    | 0x0028
                .....................................................
    CAN_AFIR3   |   AFBIH    |  AFBILEIH  |   AFEIM    |   AFEIL    | 0x002C
                .....................................................
    CAN_AFIR4   |   AFBIH    |  AFBILEIH  |   AFEIM    |   AFEIL    | 0x0030
                .....................................................
    CAN_AFIR5   |   AFBIH    |  AFBILEIH  |   AFEIM    |   AFEIL    | 0x0034
                .....................................................
    AFR_Data    |   AFDB1_1  |   AFDB0_1  |   AFDB1_0  |   AFDB0_0  | 0x0038
                .....................................................
    AFR_Control |  RESERVED  |  RESERVED  |    AFC1    |    AFC0    | 0x003C
                .....................................................
    ***************************************************************************/
    struct{
        /* Acceptance Filter Identifier Register 0,1,2,3,4,5 */
        union{
            struct{
                volatile UINT8 AFEIL;    //Acceptance Filter Extended Identifier Low Register
                volatile UINT8 AFEIM;    //Acceptance Filter Extended Identifier Middle Register
                volatile UINT8 AFBILEIH;//Acceptance Filter Base Identifier Low and Extended Identifier High Register
                volatile UINT8 AFBIH;    //Acceptance Filter Base Identifier High Register
            }Byte;
            volatile UINT32 All;
        }Identifier[6];
        /* Acceptance Filter Data 0,1 Register 0,1 */
        union{
            struct{
                volatile UINT8 AFDB0_0;     //Acceptance Filter Data Byte0 Register
                volatile UINT8 AFDB1_0;     //Acceptance Filter Data Byte1 Register
                volatile UINT8 AFDB0_1;     //Acceptance Filter Data Byte0 Register
                volatile UINT8 AFDB1_1;     //Acceptance Filter Data Byte1 Register
            }Byte;
            volatile UINT32 All;
        }Data;
        /* Acceptance Filter Control Register 0,1 */
        union{
            struct{
                volatile UINT8 AFC0;        //Acceptance Filter Control 0 Register
                volatile UINT8 AFC1;        //Acceptance Filter Control 1 Register
                volatile UINT8 RESERVED2;
                volatile UINT8 RESERVED3;
            }Byte;
            volatile UINT32 All;
        }Control;
    }CAN_Filter;

    /***************************************************************************
                    Byte 3       Byte 2       Byte 1       Byte 0     Offset
                .....................................................
    CAN_AFMIR0  |   FMBIH    |  FMBILEIH  |   FMEIM    |   FMEIL    | 0x0040
                .....................................................
    CAN_AFMIR1  |   FMBIH    |  FMBILEIH  |   FMEIM    |   FMEIL    | 0x0044
                .....................................................
    AFMR_Data   |   FMDB1_1  |   FMDB0_1  |   FMDB1_0  |   FMDB0_0  | 0x0048
                .....................................................
    AFMR_Control│  RESERVED  |  RESERVED  |  RESERVED  |    FMC     | 0x004C
                .....................................................
    ***************************************************************************/
    struct{
        /* Acceptance Filter Mask Register 0,1 */
        union{
            struct{
                volatile UINT8 FMEIL;    //Filter Mask Extended Identifier Low Register
                volatile UINT8 FMEIM;    //Filter Mask Extended Identifier Middle Register 
                volatile UINT8 FMBILEIH;//Filter Mask Base Identifier Low and Extended Identifier High Register
                volatile UINT8 FMBIH;    //Filter Mask Base Identifier High Register
            }Byte;
            volatile UINT32 All;
        }Identifier[2];
        /* Acceptance Filter Mask Data 0,1 Register 0,1 */
        union{
            struct{
                volatile UINT8 FMDB0_0;    //Filter Mask Data Byte0 Register
                volatile UINT8 FMDB1_0;    //Filter Mask Data Byte1 Register
                volatile UINT8 FMDB0_1;    //Filter Mask Data Byte0 Register
                volatile UINT8 FMDB1_1;    //Filter Mask Data Byte1 Register
            }Byte;
            volatile UINT32 All;
        }Data;
        /* Acceptance Filter Mask Control Register */
        union{
            struct{
                volatile UINT8 FMC;        //Filter Mask Control Register
                volatile UINT8 RESERVED1;
                volatile UINT8 RESERVED2;
                volatile UINT8 RESERVED3;
            }Byte;
            volatile UINT32 All;
        }Control;
    }CAN_Mask;

    /***************************************************************************
                    Byte 3       Byte 2       Byte 1       Byte 0     Offset
                .....................................................
    CAN_RR0     |    REIM    |    REIL    |   RREIE    |    RFD     | 0x0050
                .....................................................
                |  RESERVED  |    RBIH    |   RBILDL   |    REIH    | 0x0054
                .....................................................
    CAN_RR1     |    REIM    |    REIL    |   RREIE    |    RFD     | 0x0058
                .....................................................
                |  RESERVED  |    RBIH    |   RBILDL   |    REIH    | 0x005C
                .....................................................
    ***************************************************************************/
    /* Received Register 0,1 */
    struct{
        union{
            struct{
                volatile UINT8 RFD;    //Received Flexible Data-Rate Register
                volatile UINT8 RREIE;    //Received Remote and Extended Identifier Enable Register
                volatile UINT8 REIL;    //Received Extended Identifier Low Register
                volatile UINT8 REIM;    //Received Extended Identifier Middle Register
            }Byte;
            volatile UINT32 All;
        }RX_L;
        union{
            struct{
                volatile UINT8 REIH;        //Received Extended Identifier High Register
                volatile UINT8 RBILDL;    //Received Base Identifier Low and Data Length Register
                volatile UINT8 RBIH;        //Received Base Identifier High Register
                volatile UINT8 RESERVED3;
            }Byte;
            volatile UINT32 All;
        }RX_H;
    }CAN_RR[2];
    
    /***************************************************************************
                    Byte 3       Byte 2       Byte 1       Byte 0     Offset
                .....................................................
    CAN_NBTC    |   NBTC3    |   NBTC2    |   NBTC1    |   NBTC0    | 0x0060
                .....................................................
                |  RESERVED  |  RESERVED  |  RESERVED  |   NBTC4    | 0x0064
                .....................................................
    CAN_DBTC    |   DBTC3    |   DBTC2    |   DBTC1    |   DBTC0    | 0x0068
                .....................................................
    ***************************************************************************/
    /* Bit Timing Register */
    struct{
        union{
            struct{
                volatile UINT8 NBTC0;        //Nominal Bit Timing Configuration 0 Register
                volatile UINT8 NBTC1;        //Nominal Bit Timing Configuration 1 Register
                volatile UINT8 NBTC2;        //Nominal Bit Timing Configuration 2 Register
                volatile UINT8 NBTC3;        //Nominal Bit Timing Configuration 3 Register
            }Byte;
            volatile UINT32 All;
        }Nominal_L;
        union{
            struct{
                volatile UINT8 NBTC4;        //Nominal Bit Timing Configuration 4 Register
                volatile UINT8 RESERVED1;
                volatile UINT8 RESERVED2;
                volatile UINT8 RESERVED3;
            }Byte;
            volatile UINT32 All;
        }Nominal_H;
        union{
            struct{
                volatile UINT8 DBTC0;        //Data Bit Timing Configuration 0 Register
                volatile UINT8 DBTC1;        //Data Bit Timing Configuration 1 Register
                volatile UINT8 DBTC2;        //Data Bit Timing Configuration 2 Register
                volatile UINT8 DBTC3;        //Data Bit Timing Configuration 3 Register
            }Byte;
            volatile UINT32 All;
        }Data;
    }CAN_Bit_Timing;
    
    /***************************************************************************
                    Byte 3       Byte 2       Byte 1       Byte 0     Offset
                .....................................................
    Err_Cntr    |    TECH    |    TECL    |    RECH    |    RECL    | 0x006C
                .....................................................
    Err_Status  |  RESERVED  |  RESERVED  |    TED     |     ET     | 0x0070
                .....................................................
    ***************************************************************************/
    /* Error Monitor Register */
    struct{
        union{
            struct{
                volatile UINT8 RECL;        //Received Error Counter Low 8-bits Register
                volatile UINT8 RECH;        //Received Error Counter High Register
                volatile UINT8 TECL;        //Transmit Error Counter Low Register
                volatile UINT8 TECH;        //Transmit Error Counter High Register
            }Byte;
            volatile UINT32 All;
        }Counter;
        union{
            struct{
                volatile UINT8 ET;        //Error Type Register
                volatile UINT8 TED;        //Transmitter and Receive Error Detected Register
                volatile UINT8 RESERVED2;
                volatile UINT8 RESERVED3;
            }Byte;
            volatile UINT32 All;
        }Status;
    }CAN_Error_Monitor;
    
    /***************************************************************************
                    Byte 3       Byte 2       Byte 1       Byte 0     Offset
                .....................................................
    CAN_TS      |  RBTSH_1   |  RBTSL_1   |  RBTSH_0   |  RBTSL_0   | 0x0074
                .....................................................
    ***************************************************************************/
    /* Time Stamp Register 0,1 */
    union{
        struct{
            volatile UINT8 RBTSL_0;    //Receive FIFO 0 Time Stamp low Register
            volatile UINT8 RBTSH_0;    //Receive FIFO 0 Time Stamp High Register
            volatile UINT8 RBTSL_1;    //Receive FIFO 1 Time Stamp low Register
            volatile UINT8 RBTSH_1;    //Receive FIFO 1 Time Stamp High Register
        }Byte;
        volatile UINT32 All;
    }CAN_Timestamp;
    
    /***************************************************************************
                    Byte 3       Byte 2       Byte 1       Byte 0     Offset
                .....................................................
    CAN_IRR     |  RESERVED  |  RESERVED  |  RESERVED  |     IR     | 0x0078
                .....................................................
    ***************************************************************************/
    /* Interrupt Request Register */
    union{
        struct{
            volatile UINT8 IR;            //Interrupt Request Register
            volatile UINT8 RESERVED1;
            volatile UINT8 RESERVED2;
            volatile UINT8 RESERVED3;
        }Byte;
        volatile UINT32 All;
    }CAN_Interrupt;
    
    /***************************************************************************
                    Byte 3       Byte 2       Byte 1       Byte 0     Offset
                .....................................................
    Tx0_Data0   |   BYTE_3   |   BYTE_2   |   BYTE_1   |   BYTE_0   | 0x007C
                .....................................................
    Tx0_Data1   |   BYTE_7   |   BYTE_6   |   BYTE_5   |   BYTE_4   | 0x0080
                .....................................................
                |    ...     |    ...     |    ...     |    ...     |   ...
                .....................................................
    Tx0_Data15  |   BYTE_63  |   BYTE_62  |   BYTE_61  |   BYTE_60  | 0x00B8
                .....................................................
    Tx1_Data0   |   BYTE_3   |   BYTE_2   |   BYTE_1   |   BYTE_0   | 0x00BC
                .....................................................
    Tx1_Data1   |   BYTE_7   |   BYTE_6   |   BYTE_5   |   BYTE_4   | 0x00C0
                .....................................................
                |    ...     |    ...     |    ...     |    ...     |   ...
                .....................................................
    Tx1_Data15  |   BYTE_63  |   BYTE_62  |   BYTE_61  |   BYTE_60  | 0x00F8
                .....................................................
    Tx2_Data0   |   BYTE_3   |   BYTE_2   |   BYTE_1   |   BYTE_0   | 0x00FC
                .....................................................
    Tx2_Data1   |   BYTE_7   |   BYTE_6   |   BYTE_5   |   BYTE_4   | 0x0100
                .....................................................
                |    ...     |    ...     |    ...     |    ...     |   ...
                .....................................................
    Tx2_Data15  |   BYTE_63  |   BYTE_62  |   BYTE_61  |   BYTE_60  | 0x0138
                .....................................................
    ***************************************************************************/
    /* Transmit Data Register x_n (x=0~63 and n=0,1,2) */
    struct{
        union{
            struct{
                volatile UINT8 BYTE_0;
                volatile UINT8 BYTE_1;
                volatile UINT8 BYTE_2;
                volatile UINT8 BYTE_3;
            }Byte;
            volatile UINT32 All;
        }Data[16];
    }CAN_TD[3];
    
    /***************************************************************************
                    Byte 3       Byte 2       Byte 1       Byte 0     Offset
                .....................................................
    Rx0_Data0   |   BYTE_3   |   BYTE_2   |   BYTE_1   |   BYTE_0   | 0x013C
                .....................................................
    Rx0_Data1   |   BYTE_7   |   BYTE_6   |   BYTE_5   |   BYTE_4   | 0x0140
                .....................................................
                |    ...     |    ...     |    ...     |    ...     |   ...
                .....................................................
    Rx0_Data15  |   BYTE_63  |   BYTE_62  |   BYTE_61  |   BYTE_60  | 0x0178
                .....................................................
    Rx1_Data0   |   BYTE_3   |   BYTE_2   |   BYTE_1   |   BYTE_0   | 0x017C
                .....................................................
    Rx1_Data1   |   BYTE_7   |   BYTE_6   |   BYTE_5   |   BYTE_4   | 0x0180
                .....................................................
                |    ...     |    ...     |    ...     |    ...     |   ...
                .....................................................
    Rx1_Data15  |   BYTE_63  |   BYTE_62  |   BYTE_61  |   BYTE_60  | 0x01B8
                .....................................................
    ***************************************************************************/
    /* Received Data Register x_n (x=0~63 and n=0,1) */
    struct{
        union{
            struct{
                volatile UINT8 BYTE_0;
                volatile UINT8 BYTE_1;
                volatile UINT8 BYTE_2;
                volatile UINT8 BYTE_3;
            }Byte;
            volatile UINT32 All;
        }Data[16];
    }CAN_RD[2];
}CAN_TypeRegs;

/************** The definition of bit and field in CAN registers **************/
/* Bit definition for Control Enable 1 register (CE1)*/
#define  CAN_CE1_OMR_B       ((UINT8)0x03)     //Operation mode request
#define  CAN_CE1_RT_B        ((UINT8)0x18)     //Retransmission
#define  CAN_CE1_TOE_B       ((UINT8)0x20)     //Transmit overload enable
#define  CAN_CE1_RR_B        ((UINT8)0x40)     //Reset request
#define  CAN_CE1_OMR_W       ((UINT32)0x07000000)
#define  CAN_CE1_RT_W        ((UINT32)0x18000000) 
#define  CAN_CE1_TOE_W       ((UINT32)0x20000000) 
#define  CAN_CE1_RR_W        ((UINT32)0x40000000)

/* Bit definition for Control Enable 1 register (CE0)*/
#define  CAN_CE0_TSE_B       ((UINT8)0x80)     //Time stamp enable
#define  CAN_CE0_EnDB_B      ((UINT8)0x40)     //Debounced enable
#define  CAN_CE0_TSE_W       ((UINT32)0x00800000)
#define  CAN_CE0_EnDB_W      ((UINT32)0x00400000)

/* Bit definition for Interrupt Request Enable Register (IRE)*/
#define  CAN_IRE_RIE         ((UINT8)0x08)     //Receive interrupt enable
#define  CAN_IRE_TIE         ((UINT8)0x10)     //Transmit interrupt enable
#define  CAN_IRE_EIE         ((UINT8)0x20)     //Error interrupt enable
#define  CAN_IRE_OIE         ((UINT8)0x40)     //Overrun interrupt enable
#define  CAN_IRE_WIE         ((UINT8)0x80)     //Wake-up interrupt enable
#define  CAN_IRE_RIE_W       ((UINT32)0x00000800) 
#define  CAN_IRE_TIE_W       ((UINT32)0x00001000) 
#define  CAN_IRE_EIE_W       ((UINT32)0x00002000) 
#define  CAN_IRE_OIE_W       ((UINT32)0x00004000) 
#define  CAN_IRE_WIE_W       ((UINT32)0x00008000) 

/* Bit definition for Command Request Register (CR)*/
#define  CAN_CR_RRBA1_B      ((UINT8)0x02)     //Release receive buffer1 all
#define  CAN_CR_RRBA0_B      ((UINT8)0x04)     //Release receive buffer0 all
#define  CAN_CR_BTR2_B       ((UINT8)0x08)     //Buffer2 transmission request
#define  CAN_CR_BTR1_B       ((UINT8)0x10)     //Buffer1 transmission request
#define  CAN_CR_BTR0_B       ((UINT8)0x20)     //Buffer0 transmission request
#define  CAN_CR_RRB1_B       ((UINT8)0x40)     //Release receive buffer1
#define  CAN_CR_RRB0_B       ((UINT8)0x80)     //Release receive buffer0
#define  CAN_CR_RRBA1_W      ((UINT32)0x00000002)    //Release receive buffer1 all
#define  CAN_CR_RRBA0_W      ((UINT32)0x00000004)    //Release receive buffer0 all
#define  CAN_CR_BTR2_W       ((UINT32)0x00000008)
#define  CAN_CR_BTR1_W       ((UINT32)0x00000010)     
#define  CAN_CR_BTR0_W       ((UINT32)0x00000020)     
#define  CAN_CR_RRB1_W       ((UINT32)0x00000040)     
#define  CAN_CR_RRB0_W       ((UINT32)0x00000080)     

/* Bit definition for Transmit Status Register (TS)*/
#define  CAN_TS_BTS          ((UINT8)0x03)     //Buffer transmit status
#define  CAN_TS_BTLA2        ((UINT8)0x04)     //Buffer2 transmit lost arbitration
#define  CAN_TS_BTLA1        ((UINT8)0x08)     //Buffer1 transmit lost arbitration
#define  CAN_TS_BTLA0        ((UINT8)0x10)     //Buffer0 transmit lost arbitration
#define  CAN_TS_BTA2         ((UINT8)0x20)     //Buffer2 transmit abort
#define  CAN_TS_BTA1         ((UINT8)0x40)     //Buffer1 transmit abort
#define  CAN_TS_BTA0         ((UINT8)0x80)     //Buffer0 transmit abort

/* Bit definition for Transmit and Receive Buffer Status Register (TRBS)*/
#define  CAN_TRBS_BRS1_B     ((UINT8)0x06)     //Buffer1 receive status
#define  CAN_TRBS_BRS0_B     ((UINT8)0x18)     //Buffer0 receive status
#define  CAN_TRBS_BRS1_W     ((UINT32)0x00000600)
#define  CAN_TRBS_BRS0_W     ((UINT32)0x00001800)
    

/* Bit definition for Bus Status Register (BS)*/
#define  CAN_BS_DO           ((UINT8)0x0C)     //Data overrun
#define  CAN_BS_RS           ((UINT8)0x10)     //Receive status
#define  CAN_BS_TS           ((UINT8)0x20)     //Transmit status
#define  CAN_BS_EW           ((UINT8)0x40)     //Error warning
#define  CAN_BS_BO           ((UINT8)0x80)     //Bus-Off

/* Bit definition for Transmit Base Identifier High Register (TBIH)*/
#define  CAN_TBIH            ((UINT8)0xFF)     //The high 8-bit of the base transmit identifier

/* Bit definition for Transmit Base Identifier Low and Data Length Register (TBILDL)*/
#define  CAN_TBILDL_TDL      ((UINT8)0x0F)     //The transmit data payload length
#define  CAN_TBILDL_TBIL     ((UINT8)0xE0)     //The low 3-bit of the base transmit identifier

/* Bit definition for Transmit Extended Identifier High Register (TEIH)*/
#define  CAN_TEIH            ((UINT8)0xFF)     //The high 8-bit of the extended transmit identifier

/* Bit definition for Transmit Extended Identifier Middle Register (TEIM)*/
#define  CAN_TEIM            ((UINT8)0xFF)     //The middle 8-bit of the extended transmit identifier

/* Bit definition for Transmit Extended Identifier Low Register (TEIL)*/
#define  CAN_TEIL            ((UINT8)0xC0)     //The low 2-bit of the extended transmit identifier

/* Bit definition for Transmit Remote and Extended Identifier Enable Register (TREIE)*/
#define  CAN_TREIE_TEIR      ((UINT8)0x40)     //Transmit extended  identifier request bit
#define  CAN_TREIE_TRTR      ((UINT8)0x80)     //Transmit remote transmission request bit

/* Bit definition for Transmit Flexible Data-Rate Register (TFD)*/
#define  CAN_TFD_TBRS        ((UINT8)0x40)     //The transmit bit rate switch bit
#define  CAN_TFD_TEDL        ((UINT8)0x80)     //The transmit extended data length bit

/* Bit definition for Acceptance Filter Base Identifier High Register (AFBIH)*/
#define  CAN_AFBIH           ((UINT8)0xFF)     //The high 8-bit of the acceptance filter base received identifier

/* Bit definition for Acceptance Filter Base Identifier Low and Extended Identifier High (AFBILEIH)*/
#define  CAN_AFBILEIH_AFEIH  ((UINT8)0x1F)     //The high 5-bit of the acceptance filter extended identifier
#define  CAN_AFBILEIH_AFBIL  ((UINT8)0xE0)     //The low 3-bit of the acceptance filter base identifier

/* Bit definition for Acceptance Filter Extended Identifier Middle Register (AFEIM)*/
#define  CAN_AFEIM           ((UINT8)0xFF)     //The middle 8-bit of the acceptance filter extended identifier

/* Bit definition for Acceptance Filter Extended Identifier Low Register (AFEIL)*/
#define  CAN_AFEIL           ((UINT8)0xF8)     //The low 5-bit of the acceptance filter extended identifier

/* Bit definition for Acceptance Filter register in 32-bit (Combine the AFBIH, AFBILEIH, AFEIM, and AFEIL)*/
#define  CAN_AFBI_W          ((UINT32)0xFFE00000)
#define  CAN_AFEI_W          ((UINT32)0x001FFFF8)

/* Bit definition for Acceptance Filter Data Byte0  Register (AFDB0)*/
#define  CAN_AFDB0           ((UINT8)0xFF)     //The byte0 of the acceptance data filter

/* Bit definition for Acceptance Filter Data Byte1  Register (AFDB1)*/
#define  CAN_AFDB1           ((UINT8)0xFF)     //The byte1 of the acceptance data filter

/* Bit definition for Acceptance Filter Data Byte 0 and 1 register in 32-bit */
#define  CAN_FIFO0_AFDB0_W   ((UINT32)0x000000FF)
#define  CAN_FIFO0_AFDB1_W   ((UINT32)0x0000FF00)
#define  CAN_FIFO1_AFDB0_W   ((UINT32)0x00FF0000)
#define  CAN_FIFO1_AFDB1_W   ((UINT32)0xFF000000)

/* Bit definition for Acceptance Filter Control Register 0 (AFC0)*/
#define  CAN_AFC0_RB1DFB1E_B ((UINT8)0x04)     //Receive buffer1 data filter byte1 enable
#define  CAN_AFC0_RB1DFB0E_B ((UINT8)0x08)     //Receive buffer1 data filter byte0 enable
#define  CAN_AFC0_RB0DFB1E_B ((UINT8)0x10)     //Receive buffer0 data filter byte1 enable
#define  CAN_AFC0_RB0DFB0E_B ((UINT8)0x20)     //Receive buffer0 data filter byte0 enable
#define  CAN_AFC0_RBAFG1E_B  ((UINT8)0x40)     //Receive buffer1 acceptance filter group enable
#define  CAN_AFC0_RBAFG0E_B  ((UINT8)0x80)     //Receive buffer0 acceptance filter group enable
#define  CAN_AFC0_RB1DFB1E_W ((UINT32)0x00000004) 
#define  CAN_AFC0_RB1DFB0E_W ((UINT32)0x00000008) 
#define  CAN_AFC0_RB0DFB1E_W ((UINT32)0x00000010) 
#define  CAN_AFC0_RB0DFB0E_W ((UINT32)0x00000020) 
#define  CAN_AFC0_RBAFG1E_W  ((UINT32)0x00000040) 
#define  CAN_AFC0_RBAFG0E_W  ((UINT32)0x00000080)     

/* Bit definition for Acceptance Filter Control Register 1 (AFC1)*/
#define  CAN_AFC1_AFT2_B     ((UINT8)0x0C)     //Acceptance frame type 2
#define  CAN_AFC1_AFT1_B     ((UINT8)0x30)     //Acceptance frame type 1
#define  CAN_AFC1_AFT0_B     ((UINT8)0xC0)     //Acceptance frame type 0
#define  CAN_AFC1_AFT2_W     ((UINT32)0x00000C00)     
#define  CAN_AFC1_AFT1_W     ((UINT32)0x00003000)     
#define  CAN_AFC1_AFT0_W     ((UINT32)0x0000C000)     
#define  CAN_AFC1_AFT_W      ((UINT32)0x0000FC00)

/* Bit definition for Filter Mask Base Identifier High Register (FMBIH)*/
#define  CAN_FMBIH           ((UINT8)0xFF)     //Mask bits for the high 8-bit of the acceptance filter base received identifier

/* Bit definition for Filter Mask Base Identifier Low and Extended Identifier High Register (FMBILEIH)*/
#define  CAN_FMBILEIH_FMEIH  ((UINT8)0x1F)     //Mask bits for the high 5-bit of the acceptance filter extended identifier
#define  CAN_FMBILEIH_FMBIL  ((UINT8)0xE0)     //Mask bits for the low 3-bit of the acceptance filter base identifier

/* Bit definition for Filter Mask Extended Identifier Middle Register (FMEIM)*/
#define  CAN_FMBIH           ((UINT8)0xFF)     //Mask bits for the middle 8-bit of the acceptance filter extended identifier

/* Bit definition for Filter Mask Extended Identifier Low Register (FMEIL)*/
#define  CAN_FMEIL           ((UINT8)0xF8)     //Mask bits for the low 5-bit of the acceptance filter extended identifier

/* Bit definition for Filter Mask register in 32-bit (Combine the FMBIH, FMBILEIH, FMEIM, and FMEIL)*/
#define  CAN_FMBI_W          ((UINT32)0xFFE00000)
#define  CAN_FMEI_W          ((UINT32)0x001FFFF8)

/* Bit definition for Filter Mask Data Byte0 Register (FMDB0)*/
#define  CAN_FMDB0           ((UINT8)0xFF)     //Mask bits for the acceptance data byte 0 filter

/* Bit definition for Filter Mask Data Byte1 Register (FMDB1)*/
#define  CAN_FMDB1           ((UINT8)0xFF)     //Mask bits for the acceptance data byte 1 filter

/* Bit definition for Filter Mask Data Byte 0 and 1 register in 32-bit */
#define  CAN_FIFO0_FMDB0_W   ((UINT32)0x000000FF)
#define  CAN_FIFO0_FMDB1_W   ((UINT32)0x0000FF00)
#define  CAN_FIFO1_FMDB0_W   ((UINT32)0x00FF0000)
#define  CAN_FIFO1_FMDB1_W   ((UINT32)0xFF000000)

/* Bit definition for Filter Mask Control Register (FMC)*/
#define  CAN_FMC_RB1DFMB1E   ((UINT8)0x02)     //Receive buffer1 data filter mask byte1 enable
#define  CAN_FMC_RB1DFMB0E   ((UINT8)0x04)     //Receive buffer1 data filter mask byte0 enable
#define  CAN_FMC_RB0DFMB1E   ((UINT8)0x08)     //Receive buffer0 data filter mask byte1 enable
#define  CAN_FMC_RB0DFMB0E   ((UINT8)0x10)     //Receive buffer0 data filter mask byte0 enable
#define  CAN_FMC_RBAFM1E     ((UINT8)0x20)     //Receive buffer1 acceptance filter mask enable
#define  CAN_FMC_RBAFM0E     ((UINT8)0x40)     //Receive buffer0 acceptance filter mask enable
#define  CAN_FMC_FMA         ((UINT8)0x80)     //Filter mask all
#define  CAN_FMC_RB1DFMB1E_W ((UINT32)0x00000002)     
#define  CAN_FMC_RB1DFMB0E_W ((UINT32)0x00000004)     
#define  CAN_FMC_RB0DFMB1E_W ((UINT32)0x00000008)     
#define  CAN_FMC_RB0DFMB0E_W ((UINT32)0x00000010)     
#define  CAN_FMC_RBAFM1E_W   ((UINT32)0x00000020)     
#define  CAN_FMC_RBAFM0E_W   ((UINT32)0x00000040)     
#define  CAN_FMC_FMA_W       ((UINT32)0x00000080)     

/* Bit definition for Received Base Identifier High Register (RBIH)*/
#define  CAN_RBIH            ((UINT8)0xFF)     //The high 8-bit of the base received identifier

/* Bit definition for Received Base Identifier Low and Data Length Register (RBILDL)*/
#define  CAN_RBILDL_RBIL     ((UINT8)0xE0)     //The low 3-bit of the base received identifier
#define  CAN_RBILDL_RDL_B    ((UINT8)0x0F)     //The received data payload length
#define  CAN_RBILDL_RDL_W    ((UINT32)0x00000F00)

/* Bit definition for Received Extended Identifier High Register (REIH)*/
#define  CAN_REIH            ((UINT8)0xFF)     //The high 8-bit of the extended received identifier

/* Bit definition for Received Extended Identifier Middle Register (REIM)*/
#define  CAN_REIM            ((UINT8)0xFF)     //The middle 8-bit of the received extended identifier

/* Bit definition for Received Extended Identifier Low Register (REIL)*/
#define  CAN_REIL            ((UINT8)0xC0)     //The low 2-bit of the received extended identifier

/* Bit definition for Received Extended and Based Identifier register in 32-bit */
#define  CAN_RBI_W           ((UINT32)0x00FFE000)
#define  CAN_REIH_W          ((UINT32)0x000000FF)
#define  CAN_REIL_W          ((UINT32)0xFFC00000)

/* Bit definition for Received Remote and Extended Identifier Enable Register (RREIE)*/
#define  CAN_RREIE_REIE_B    ((UINT8)0x40)     //Received extended identifier enable bit
#define  CAN_RREIE_RRTR_B    ((UINT8)0x80)     //Received remote transmission request bit
#define  CAN_RREIE_REIE_W    ((UINT32)0x00004000)
#define  CAN_RREIE_RRTR_W    ((UINT32)0x00008000)

/* Bit definition for Received Flexible Data-Rate Register (RFD)*/
#define  CAN_RFD_RESI_B      ((UINT8)0x20)     //The received error state indicator bit
#define  CAN_RFD_RBRS_B      ((UINT8)0x40)     //The received bit rate switch bit
#define  CAN_RFD_REDL_B      ((UINT8)0x80)     //The received extended data length bit
#define  CAN_RFD_RESI_W      ((UINT32)0x00000020)
#define  CAN_RFD_RBRS_W      ((UINT32)0x00000040)
#define  CAN_RFD_REDL_W      ((UINT32)0x00000080)

/* Bit definition for Nominal Bit Timing Configuration 0 Register (NBTC0)*/
#define  CAN_NBTC0_NSJW      ((UINT8)0xF8)     //The nominal synchronization jump width length bits

/* Bit definition for Nominal Bit Timing Configuration 1 Register (NBTC1)*/
#define  CAN_NBTC1_NBRP      ((UINT8)0xFF)     //Nominal baud rate prescaler

/* Bit definition for Nominal Bit Timing Configuration 2 Register (NBTC2)*/
#define  CAN_NBTC2_NPRSEG    ((UINT8)0x7E)     //The bits of nominal Prop_seg

/* Bit definition for Nominal Bit Timing Configuration 3 Register (NBTC3)*/
#define  CAN_NBTC3_NPSEG1    ((UINT8)0xF8)     //The bits of nominal Phase_seg1

/* Bit definition for Nominal Bit Timing Configuration 4 Register (NBTC4)*/
#define  CAN_NBTC4_NPSEG2    ((UINT8)0xF8)     //The bits of nominal Phase_seg2

/* Bit definition for Data Bit Timing Configuration 0 Register (DBTC0)*/
#define  CAN_DBTC0_DBRP      ((UINT8)0xFF)     //Data baud rate prescaler

/* Bit definition for Data Bit Timing Configuration 1 Register (DBTC1)*/
#define  CAN_DBTC1_DPRSEG    ((UINT8)0x78)     //The bits of data Prop_seg
#define  CAN_DBTC1_DSJW      ((UINT8)0x06)     //The bits of data SJW

/* Bit definition for Data Bit Timing Configuration 2 Register (DBTC2)*/
#define  CAN_DBTC2_DPSEG1    ((UINT8)0xE0)     //The bits of data Phase_seg1

/* Bit definition for Data Bit Timing Configuration 3 Register (DBTC2)*/
#define  CAN_DBTC3_DPSEG2    ((UINT8)0xE0)     //The bits of data Phase_seg2

/* Bit definition for Transmit and Receive Error Counter Register (TED)*/
#define  CAN_TED_BRE1        ((UINT8)0x08)     //Buffer1 Receive error
#define  CAN_TED_BRE0        ((UINT8)0x10)     //Buffer0 Receive error
#define  CAN_TED_BTE2        ((UINT8)0x20)     //Buffer2 Transmitter error
#define  CAN_TED_BTE1        ((UINT8)0x40)     //Buffer1 Transmitter error
#define  CAN_TED_BTE0        ((UINT8)0x80)     //Buffer0 Transmitter error

/* Bit definition for Transmit Error Counter High Register (TECH)*/
#define  CAN_TECH            ((UINT8)0xFF)     //Transmitter error counter high 8-bit register

/* Bit definition for Transmit Error Counter Low Register (TECL)*/
#define  CAN_TECL            ((UINT8)0xFF)     //Transmitter error counter low 8-bit register

/* Bit definition for Received Error Counter High Register (RECH)*/
#define  CAN_RECH            ((UINT8)0xFF)     //Receiver error counter high 8-bits register

/* Bit definition for Received Error Counter Low Register (RECL)*/
#define  CAN_RECL            ((UINT8)0xFF)     //Receiver error counter low 8-bits register

/* Bit definition for Error Type Register (ET)*/
#define  CAN_ET_OE           ((UINT8)0x04)     //Overrun Error
#define  CAN_ET_AE           ((UINT8)0x08)     //Acknowledgement error
#define  CAN_ET_FE           ((UINT8)0x10)     //Form error
#define  CAN_ET_CE           ((UINT8)0x20)     //CRC error
#define  CAN_ET_SE           ((UINT8)0x40)     //Stuff error
#define  CAN_ET_BE           ((UINT8)0x80)     //Bit error

/* Bit definition for Receive Buffer Time Stamp High Register(RBTSH)*/
#define  CAN_RBTSH           ((UINT8)0xFF)     //Receive buffer time stamp high 8-bit

/* Bit definition for Receive Buffer Time Stamp Low Register(RBTSL)*/
#define  CAN_RBTSL           ((UINT8)0xFF)     //Receive buffer time stamp low 8-bit

/* Bit definition for Interrupt Request Register (IR)*/
#define  CAN_IR_RB           ((UINT8)0x03)     //Receive buffer field
#define  CAN_IR_TB           ((UINT8)0x1C)     //Transmit buffer field
#define  CAN_IR_RBI1         ((UINT8)0x01)     //Receive buffer1 interrupt
#define  CAN_IR_RBI0         ((UINT8)0x02)     //Receive buffer0 interrupt
#define  CAN_IR_TBI2         ((UINT8)0x04)     //Transmit buffer2 interrupt
#define  CAN_IR_TBI1         ((UINT8)0x08)     //Transmit buffer1 interrupt
#define  CAN_IR_TBI0         ((UINT8)0x10)     //Transmit buffer0 interrupt
#define  CAN_IR_EIR          ((UINT8)0x20)     //Error interrupt request
#define  CAN_IR_OIR          ((UINT8)0x40)     //Overrun interrupt request
#define  CAN_IR_WIR          ((UINT8)0x80)     //Wake-up interrupt request


#endif