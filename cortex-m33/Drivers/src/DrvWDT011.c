/***************************************************************************
* Copyright  Faraday Technology Corp 2002-2003.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:DrvWDT011.c                                                         *
* Description: Watch Dog Timer library routine                             *
* Author: Zerget                                                     *
****************************************************************************/
#include "common_include.h"
#include "Driver_Common.h"
#include "leo_cm33.h"
#include "DrvWDT011.h"
extern uint32_t WDT_BASE[];
extern uint32_t WDT_BASE_IDX;

void fLib_WatchDog_Enable(DRVIWDT iwdt)
{
    outw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_KR, START_KEY);
}

void fLib_WatchDog_ReProg(DRVIWDT iwdt)
{
	  /* Disable written access protection */
    outw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_KR, REPROG_KEY);

    /* wait for watchdog timer to stop */
    while(inw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_CR) & WdEnable);
}

void fLib_WatchDog_KickDog(DRVIWDT iwdt)
{
    outw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_KR, KICKDOG_KEY);
}

void fLib_WatchDog_SetPrescaler(DRVIWDT iwdt, UINT8 value)
{
    fLib_WatchDog_ReProg(iwdt);
    outw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_PR, value);

    while (inw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_SR) & WdPVU);
}

void fLib_WatchDog_SetAutoReLoad(DRVIWDT iwdt, UINT32 value)
{
    fLib_WatchDog_ReProg(iwdt);
    outw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_RLR, value);

    while (inw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_SR) & WdRVU);
}

void fLib_WatchDog_SetUnderflowValue(DRVIWDT iwdt, UINT32 value)
{
    fLib_WatchDog_ReProg(iwdt);
    outw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_UDF, value);
}

void fLib_WatchDog_SetSignalLength(DRVIWDT iwdt, UINT8 value)
{
    fLib_WatchDog_ReProg(iwdt);
    outw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_INTRLEN, value);
}

void fLib_WatchDog_ExtClockSrcEnable(void)
{
    UINT32 tmp;

    fLib_WatchDog_ReProg((DRVIWDT) 0);

    tmp = inw(WDT_BASE[WDT_BASE_IDX] + IWDT_CR);
    tmp &= 0xDF;
    tmp |= WdClockSrc;

    outw(WDT_BASE[WDT_BASE_IDX] + IWDT_CR, tmp);
}

void fLib_WatchDog_IntClockSrcEnable(void)
{
    UINT32 tmp;

    fLib_WatchDog_ReProg((DRVIWDT) 0);

    tmp = inw(WDT_BASE[WDT_BASE_IDX] + IWDT_CR);
    tmp &= 0xDF;
    tmp &= (~WdClockSrc);

    outw(WDT_BASE[WDT_BASE_IDX] + IWDT_CR, tmp);
}

void fLib_WatchDog_ExtSignalEnable(DRVIWDT iwdt)
{
    UINT32 tmp;

    fLib_WatchDog_ReProg(iwdt);

    tmp = inw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_CR);
    tmp &= 0xDF;
    tmp |= WdExtEn;

    outw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_CR, tmp);
}

void fLib_WatchDog_ExtSignalDisable(DRVIWDT iwdt)
{
    UINT32 tmp;

    fLib_WatchDog_ReProg(iwdt);

    tmp = inw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_CR);
    tmp &= 0xDF;
    tmp &= (~WdExtEn);
    outw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_CR, tmp);
}

void fLib_WatchDog_SysIntEnable(DRVIWDT iwdt)
{
    UINT32 tmp;

    tmp = inw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_CR);
    tmp &= 0xDF;
    tmp |= WdIntrEn;

    outw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_CR, tmp);
}

void fLib_WatchDog_SysIntDisable(DRVIWDT iwdt)
{
    UINT32 tmp;

    tmp = inw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_CR);
    tmp &= 0xDF;
    tmp &= (~WdIntrEn);
    outw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_CR, tmp);
}

void fLib_WatchDog_UnderflowIntEnable(DRVIWDT iwdt)
{
    UINT32 tmp;

    tmp = inw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_CR);
    tmp &= 0xDF;
    tmp |= WdUDFIntrEn;

    outw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_CR, tmp);
}

void fLib_WatchDog_UnderflowIntDisable(DRVIWDT iwdt)
{
    UINT32 tmp;

    tmp = inw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_CR);
    tmp &= 0xDF;
    tmp &= (~WdUDFIntrEn);
    outw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_CR, tmp);
}

void fLib_WatchDog_UnderflowEnable(DRVIWDT iwdt)
{
    UINT32 tmp;

    tmp = inw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_CR);
    tmp &= 0xDF;
    tmp |= WdUDFEn;

    outw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_CR, tmp);
}

void fLib_WatchDog_UnderflowDisable(DRVIWDT iwdt)
{
    UINT32 tmp;

    tmp = inw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_CR);
    tmp &= 0xDF;
    tmp &= (~WdUDFEn);
    outw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_CR, tmp);
}

void fLib_WatchDog_SysResetEnable(DRVIWDT iwdt)
{
    UINT32 tmp;

    fLib_WatchDog_ReProg(iwdt);

    tmp = inw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_CR);
    tmp &= 0xDF;
    tmp |= WdRstEn ;
    outw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_CR,tmp);
}

void fLib_WatchDog_SysResetDisable(DRVIWDT iwdt)
{
    UINT32 tmp;

    fLib_WatchDog_ReProg(iwdt);

    tmp = inw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_CR);
    tmp &= 0xDF;
    tmp &= (~WdRstEn);
    outw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_CR, tmp);
}

BOOL fLib_WatchDog_IsCounterZero(DRVIWDT iwdt)
{
    UINT32 tmp;

    tmp = inw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_INTSR);
    if (tmp == 1)
        return TRUE;
    else
        return FALSE;
}

void fLib_WatchDog_ClearOverflowStatus(DRVIWDT iwdt)
{
    outw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_INTSR, 0x1);
}

void fLib_WatchDog_ClearUnderflowStatus(DRVIWDT iwdt)
{
    outw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_INTSR, 0x2);
}

void fLib_WatchDog_SetUnderflowVaule(DRVIWDT iwdt, UINT32 value)
{
    outw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_UDF, value);
}

UINT32 fLib_WatchDog_GetCurrentCounter(DRVIWDT iwdt)
{
    return inw(WDT_BASE[WDT_BASE_IDX] + (iwdt * IWDT_OFFSET) + IWDT_CURR);
}

UINT32 fLib_WatchDog_GetGlobalIntrStatus(void)
{
    return inw(WDT_BASE[WDT_BASE_IDX] + IWDT_INTSTS);
}

void fLib_WatchDog_ClearGlobalIntrStatus(UINT32 value)
{
    outw(WDT_BASE[WDT_BASE_IDX] + IWDT_INTSTS, value);
}
