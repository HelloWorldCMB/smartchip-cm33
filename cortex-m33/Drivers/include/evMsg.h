/* ------------------------------------------------------------
 evMsg.h
 Header File

 Copyright (c) 2011 ARM Ltd.  All rights reserved.
------------------------------------------------------------
*/

#if !defined(EVMSG_H)
/* Prevents multiple includes */
#define EVMSG_H

#include "types.h"

/*
 * Holds the message instance data
 */
typedef struct
{
    /*
     * Max length for message buffer 
     */
    int           m_MaxMsgLen;        
    /*
     *  Cursor into tx data stream 
     */
    uint8*        m_pTx;              
    /*
     * The raw tx stream data 
     */
    uint8*        m_pTxData;       
} EVMSG;

/* msgType for a system message */
#define evMsg_SYS (uint8)0
/* msgType for a variable values message */
#define evMsg_VAR (uint8)1
/* msgType for a printf output message */
#define evMsg_PRINTF (uint8)2
/* msgType for a event message */
#define evMsg_EVENT (uint8)3

EVMSG* evMsg_create(int maxMsgLen);
void   evMsg_writeBlock(EVMSG* pMsg, int BCount, const uint8* pData);
void   evMsg_writeWordBlock(EVMSG* pMsg, int WCount, const uint32* pData);
void   evMsg_reset(EVMSG* pMsg, uint8 msgType);
int    evMsg_getWriteCount(EVMSG* pMsg);
int    evMsg_canWrite(EVMSG* pMsg, int byteCount);
uint8* evMsg_getRawMsg(EVMSG* pMsg, int* pMsgLen);

#endif
