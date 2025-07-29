#include "board.h"

#include "api_srl.h"

#include <stdint.h>
#include <stdio.h>
#include "[COMMON]/common.h"

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
    stSwFifo * const swFifo = swTxFifo + API_SRLB;
    const uint32_t sciBase = sciBaseArr[API_SRLB];
    const uint16_t isrAckGrp = sciTxAckGrpArr[API_SRLB];
    
    if(swFifo->tail != swFifo->head) // is next data
    {
        uint16_t txBuf[15], len;

        for(len = 0; len < ARRAY_LEN(txBuf); len++)
        {
            txBuf[len] = swFifo->buf[swFifo->tail];
            swFifo->tail = (swFifo->tail + 1) % ARRAY_LEN(swFifo->buf);
            if(swFifo->tail == swFifo->head)
                break;
        }
        SCI_writeCharArray(sciBase, txBuf, len);
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
    stSwFifo * const swFifo = swRxFifo + API_SRLB;
    const uint32_t sciBase = sciBaseArr[API_SRLB];
    const uint16_t isrAckGrp = sciRxAckGrpArr[API_SRLB];
    
    uint16_t rxArr[16];
    uint16_t cnt = 0, readLen = SCI_getRxFIFOStatus(sciBase);
    SCI_readCharArray(sciBase, rxArr, readLen);

    while(1)
    {
        uint16_t nextHead = (swFifo->head + 1) % ARRAY_LEN(swFifo->buf);

        if((nextHead == swFifo->tail) || (cnt == readLen))
            break;
        
        swFifo->buf[swFifo->head] = rxArr[cnt++];
        swFifo->head = nextHead;
    }

    SCI_clearOverflowStatus(sciBase);
    SCI_clearInterruptStatus(sciBase, SCI_INT_RXFF);
    Interrupt_clearACKGroup(isrAckGrp);
}

int ApiSrlWrite(eApiSrlCh ch,  const uint8_t* src, int len)
{
    stSwFifo * const swFifo = swTxFifo + ch;
    int writeCnt = 0;
    bool isTxRun;

    DINT;
    isTxRun = swFifo->head != swFifo->tail;
    while(1)
    {
        uint16_t nextHead = (swFifo->head + 1) % ARRAY_LEN(swFifo->buf);

        if((nextHead == swFifo->tail) || (writeCnt == len))
            break;
        
        swFifo->buf[swFifo->head] = src[writeCnt++];
        swFifo->head = nextHead;
    }
    EINT;

    if(isTxRun == false)
        SCI_enableInterrupt(sciBaseArr[ch], SCI_INT_TXFF);

    return writeCnt;
}

int ApiSrlRead(eApiSrlCh ch, uint8_t * const  dst, int len)
{
    stSwFifo * const swFifo = swRxFifo + ch;
    bool rxFifoEmpty;

    DINT;
    rxFifoEmpty = (swFifo->tail) == (swFifo->head);
    EINT;

    if(rxFifoEmpty == true)
        return 0;

    int readCnt = 0;
    DINT;
    while(1)
    {
        if((readCnt >= len) || ((swFifo->tail) == (swFifo->head)))
            break;
        dst[readCnt++] = swFifo->buf[swFifo->tail];
        swFifo->tail = (swFifo->tail + 1) % ARRAY_LEN(swFifo->buf);
    }
    EINT;

    return readCnt;
}

bool ApiSrlChkRxEmpty(eApiSrlCh ch) // true : rx is empty
{
    stSwFifo * const swFifo = swRxFifo + ch;
    bool rxFifoEmpty;

    DINT;
    rxFifoEmpty = (swFifo->tail) == (swFifo->head);
    EINT;

    return rxFifoEmpty;
}

#pragma DATA_SECTION(printfBuf, "ramgsRarge")
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
