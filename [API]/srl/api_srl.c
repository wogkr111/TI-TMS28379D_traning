#include "[COMMON]/common.h"
#include "api_srl.h"
#include <stdio.h>


const uint32_t sciBaseArr[API_SRL_END] =  {mySCIB_BASE};
const uint32_t sciTxAckGrpArr[API_SRL_END] =  {INT_mySCIB_TX_INTERRUPT_ACK_GROUP};
const uint32_t sciRxAckGrpArr[API_SRL_END] =  {INT_mySCIB_RX_INTERRUPT_ACK_GROUP};

typedef struct
{
    uint16_t buf[512];
    uint16_t head;
    uint16_t tail;
}stSwFifo;

#pragma DATA_SECTION(swTxFifo, "ramgsRarge")
#pragma DATA_SECTION(swRxFifo, "ramgsRarge")
stSwFifo swTxFifo[API_SRL_END] = {{0,}};
stSwFifo swRxFifo[API_SRL_END] = {{0,}};



inline static int getTxUsedSize(eApiSrlCh ch)
{
    uint16_t used_size = (swTxFifo[ch].head - swTxFifo[ch].tail) & (ARRAY_LEN(swTxFifo[0].buf) - 1);
    return used_size;
}
inline static int getTxFreeSize(eApiSrlCh ch)
{
    return (ARRAY_LEN(swTxFifo[0].buf) - 1) - getTxUsedSize(ch);
}
inline static int getRxUsedSize(eApiSrlCh ch)
{
    uint16_t used_size = (swRxFifo[ch].head - swRxFifo[ch].tail) & (ARRAY_LEN(swRxFifo[0].buf) - 1);
    return used_size;
}
inline static int getRxFreeSize(eApiSrlCh ch)
{
    return (ARRAY_LEN(swRxFifo[0].buf) - 1) - getRxUsedSize(ch);
}

inline static void My_SCI_writeCharArray(uint32_t base, const uint16_t * const array, uint16_t length)
{
    uint16_t i;
    for(i = 0U; i < length; i++)
        HWREGH(base + SCI_O_TXBUF) = array[i];
}

// Warning: This function doesn't consider 'len' and the FIFO free size.
inline static void RxFifoPush(eApiSrlCh ch,  uint8_t* src, int len)
{
    for(int i = 0; i < len; i++)
    {
        swRxFifo[ch].buf[swRxFifo[ch].head] = src[i];
        swRxFifo[ch].head = (swRxFifo[ch].head + 1) & (ARRAY_LEN(swRxFifo[0].buf) - 1);
    }
}


// TX COMPLETE
__interrupt void INT_myECAP0_ISR(void)
{
    ECAP_stopCounter(myECAP0_BASE);
    ECAP_clearInterrupt(myECAP0_BASE, ECAP_ISR_SOURCE_COUNTER_PERIOD);
    ECAP_clearGlobalInterrupt(myECAP0_BASE);
    Interrupt_clearACKGroup(INT_myECAP0_INTERRUPT_ACK_GROUP);
}


 __interrupt void INT_mySCIB_TX_ISR(void)
{
    const uint32_t sciBase = sciBaseArr[API_SRLB];
    const uint32_t isrAckGrp = sciTxAckGrpArr[API_SRLB];
    
    int usedSize;
    if((usedSize = getTxUsedSize(API_SRLB)) > 0) // is next data
    {
        // SCI_writeCharArray 는 매우 느림!!
        // SCI_writeCharArray 를 사용한다면, 최대 길이를 15로 설정할것
        // My_SCI_writeCharArray 는 SCI FIFO 설정이 옳바르며, 이 ISR 진입시점에서 TI FIFO가 완전히 비어있음을 가정하고 있음 
        int len = (usedSize > 16)? 16 : usedSize;

        if((ARRAY_LEN(swRxFifo[API_SRLB].buf) - swTxFifo[API_SRLB].tail) <= len)
        {
            int fstLen = ARRAY_LEN(swTxFifo[API_SRLB].buf) - swTxFifo[API_SRLB].tail;
            My_SCI_writeCharArray(sciBase, swTxFifo[API_SRLB].buf + swTxFifo[API_SRLB].tail, fstLen);
            swTxFifo[API_SRLB].tail = 0;
            len -= fstLen;
        }

        My_SCI_writeCharArray(sciBase, swTxFifo[API_SRLB].buf + swTxFifo[API_SRLB].tail, len);
        swTxFifo[API_SRLB].tail += len;
    }
    else // sw fifo end
    {
        SCI_disableInterrupt(sciBase, SCI_INT_TXFF);
        HWREGH(myECAP0_BASE + ECAP_O_TSCTR) = 0; // ECAP Counter Clear
        ECAP_startCounter(myECAP0_BASE); // for TX COMPLETE EVENT (RS485 DE PIN CLEAR)
    }

    SCI_clearInterruptStatus(sciBase, SCI_INT_TXFF);
    Interrupt_clearACKGroup(isrAckGrp);
}

 __interrupt void INT_mySCIB_RX_ISR(void)
{
    const uint32_t sciBase = sciBaseArr[API_SRLB];
    const uint16_t isrAckGrp = sciRxAckGrpArr[API_SRLB];
    
    uint16_t rxArr[16];
    uint16_t readLen = SCI_getRxFIFOStatus(sciBase);
    SCI_readCharArray(sciBase, rxArr, readLen);

    if(readLen <= getRxFreeSize(API_SRLB))
        RxFifoPush(API_SRLB,  rxArr, readLen);

    SCI_clearOverflowStatus(sciBase);
    SCI_clearInterruptStatus(sciBase, SCI_INT_RXFF);
    Interrupt_clearACKGroup(isrAckGrp);
}

int ApiSrlWrite(eApiSrlCh ch,  const uint8_t* src, int len)
{
    DINT;
    int head_old = swTxFifo[ch].head;
    int freeSize = getTxFreeSize(ch);
    EINT;

    if(len > freeSize)
        return -1; // 공간 부족

    int head = head_old;
    uint16_t* buf = swTxFifo[ch].buf;
    for(int i = 0; i < len; i++)
    {
        buf[head] = src[i];
        head = (head + 1) & (ARRAY_LEN(swTxFifo[0].buf) - 1);
    }

    DINT;
    int tail = swTxFifo[ch].tail;
    swTxFifo[ch].head = head;
    EINT;

    if(tail == head_old) // TX FIFO start
        SCI_enableInterrupt(sciBaseArr[ch], SCI_INT_TXFF);

    return 0;
}




int ApiSrlRead(eApiSrlCh ch, uint8_t * const  dst, int _len)
{
    DINT;
    int used_size = getRxUsedSize(ch);
    int tail = swRxFifo[ch].tail;
    EINT;

    if(used_size == 0)
        return 0;

    int len = used_size < _len? used_size : _len;
    uint16_t* buf = swRxFifo[ch].buf;

    for(int i = 0; i < len; i++)
    {
        dst[i] = buf[tail];
        tail = (tail + 1) & (ARRAY_LEN(swTxFifo[0].buf) - 1);
    }

    DINT;
    swRxFifo[ch].tail = tail;
    EINT;

    return len;
}

int ApiSrlRxBytesToRead(eApiSrlCh ch) // true : rx is empty
{
    return getRxUsedSize(ch);
}


#ifndef _FLASH
#pragma DATA_SECTION(printfBuf, "ramgsRarge")
#endif
static char printfBuf[512];

int ApiSrlPrintf(const char* str, ...) // only SRLB
{
    va_list ap;
    uint16_t len;

    va_start(ap, str);
    len = vsprintf(printfBuf, str, ap);
    va_end(ap);

    ApiSrlWrite(API_SRLB, (uint16_t *)printfBuf, len);

    return len;
}
