#include "common.h"

#include "api_flash.h"

#include "F021_F2837xD_C28x.h"


#pragma DATA_ALIGN(write8WordBuf, 8)
uint16_t write8WordBuf[8];

#ifdef _FLASH
#pragma CODE_SECTION(ApiFlashInit, ".TI.ramfunc")
#endif
int ApiFlashInit(void)
{
    Fapi_StatusType oReturnCheck;
    int ret = 0;

    EALLOW;

    oReturnCheck = Fapi_initializeAPI(F021_CPU0_BASE_ADDRESS, (DEVICE_SYSCLK_FREQ/1000000ul));
    if(oReturnCheck != Fapi_Status_Success)
        ret = - 1;

    if(ret == 0)
    {
        oReturnCheck = Fapi_setActiveFlashBank(Fapi_FlashBank0);
        if(oReturnCheck != Fapi_Status_Success)
            ret = - 1;
    }

    EDIS;

    return ret;
}

#ifdef _FLASH
#pragma CODE_SECTION(ApiFlashSectorErase, ".TI.ramfunc")
#endif
int ApiFlashSectorErase(eApiFlashSector sector)
{
    Fapi_StatusType oReturnCheck;
    Fapi_FlashStatusType oFlashStatus;
    int ret = 0;

    EALLOW;

    // Erase a Sector
    oReturnCheck = Fapi_issueAsyncCommandWithAddress(Fapi_EraseSector, (uint32 *)sector);
    // Wait until the erase operation is over
    while (Fapi_checkFsmForReady() != Fapi_Status_FsmReady){}
    if(oReturnCheck != Fapi_Status_Success)
        ret = - 1;

    if(ret == 0)
    {
        // Read FMSTAT register contents to know the status of FSM after erase command to see if there are any erase operation related errors
        oFlashStatus = Fapi_getFsmStatus();
        if (oFlashStatus!=0)
            ret = - 1;
    }

    EDIS;

    return ret;
}

void ApiFlashRead(uint16_t* readAdr, uint16_t*datBuf, uint32_t len)
{
    for (int i = 0; i < len; i++)
        datBuf[i] = ((volatile uint16_t*)readAdr)[i];
}

#ifdef _FLASH
#pragma CODE_SECTION(ApiFlashWrite, ".TI.ramfunc")
#endif
int ApiFlashWrite(uint16_t* writeAdr, uint16_t*datBuf, uint32_t len)
{
    Fapi_StatusType oReturnCheck;
    Fapi_FlashStatusType oFlashStatus;
    int ret = 0;

    EALLOW;

    while(len != 0)
    {
        int i, writeLen = (len > 8)? 8 : len;
        for(i = 0; i < writeLen; i++)   write8WordBuf[i] = *datBuf++;
        for(; i < 8; i++)               write8WordBuf[i] = 0xFFFF;

        // Issue program command
        oReturnCheck = Fapi_issueProgrammingCommand((uint32 *)writeAdr, write8WordBuf, 8, 0, 0, Fapi_AutoEccGeneration);

        // Wait until the Flash program operation is over
        while (Fapi_checkFsmForReady() != Fapi_Status_FsmReady){}
        if(oReturnCheck != Fapi_Status_Success)
        {
            ret = -1;
            break;
        }

        // Read FMSTAT register contents to know the status of FSM after program command to see if there are any program operation related errors
        oFlashStatus = Fapi_getFsmStatus();
        if(oFlashStatus != 0)
        {
            ret = -1;
            break;
        }

        len -= writeLen;
        writeAdr += writeLen;
    }

    EDIS;

    return ret;
}
