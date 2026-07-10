/* ------------------------------------------------------------
 evprintf.h
 Header File

 Copyright (c) 2011 ARM Ltd.  All rights reserved.
------------------------------------------------------------
*/

#ifndef EVPRINTF_H
#define EVPRINTF_H

struct EVMSG;

void evprintf_init(void);
void evprintf_final(void);
int  evprintf(const char * format, ... );
void evprintf_msgGenerator(EVMSG* pMsg);

#endif

