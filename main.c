//#############################################################################
//
// FILE:   empty_driverlib_main.c
//
// TITLE:  Empty Project
//
// Empty Project Example
//
// This example is an empty project setup for Driverlib development.
//
//#############################################################################
//
//
// $Copyright:
// Copyright (C) 2013-2025 Texas Instruments Incorporated - http://www.ti.com/
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions 
// are met:
// 
//   Redistributions of source code must retain the above copyright 
//   notice, this list of conditions and the following disclaimer.
// 
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the 
//   documentation and/or other materials provided with the   
//   distribution.
// 
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// $
//#############################################################################

//
// Included Files
//
#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "c2000ware_libraries.h"


#include "[COMMON]/common.h"
#include "[API]/timer/api_tim.h"
#include "[API]/srl/api_srl.h"

#define ARRAY_LEN(x)    (sizeof(x)/sizeof((x)[0]))

struct
{
    int tail;
    int head;
    uint16_t buf[512];
}myFifo = {0,};


#if 0
 __interrupt void INT_mySCIB_TX_ISR(void)
{
    GPIO_togglePin(OPLED_BL);
    Interrupt_clearACKGroup(INT_mySCIB_TX_INTERRUPT_ACK_GROUP);
}

  void TxTest(void)
{
    const uint16_t datArr[] = {'1','2','3','4','5','6','\r','\n'};
    SCI_writeCharArray(mySCIB_BASE, datArr, ARRAY_LEN(datArr));
	SCI_clearInterruptStatus(mySCIB_BASE, SCI_INT_TXFF);	
}

#elif 0
 __interrupt void INT_mySCIB_TX_ISR(void)
{
    GPIO_togglePin(OPLED_BL);

    SCI_clearInterruptStatus(mySCIB_BASE, SCI_INT_TXFF);

    if(myFifo.tail != myFifo.head) // next data
    {
        uint16_t txBuf[15], len;

        for(len = 0; len < ARRAY_LEN(txBuf); len++)
        {
            txBuf[len] = myFifo.buf[myFifo.tail];
            myFifo.tail = (myFifo.tail + 1) % ARRAY_LEN(myFifo.buf);
            if(myFifo.tail == myFifo.head)
                break;
        }
        SCI_writeCharArray(mySCIB_BASE, txBuf, len);
    }
    else // fifo end
    {
        SCI_disableInterrupt(mySCIB_BASE, SCI_INT_TXFF);
    }

    Interrupt_clearACKGroup(INT_mySCIB_TX_INTERRUPT_ACK_GROUP);
}

void TxTest(void)
{
    const uint16_t datArr[] = {'1','2','3','4','5','6','\r','\n'};

    // 인터럽트 예외처리 필요
    for(int j = 0; j < 5; j++)
        for (int i = 0; i < ARRAY_LEN(datArr); i++)
        {
            myFifo.buf[myFifo.head] = datArr[i];
            myFifo.head = (myFifo.head + 1) % ARRAY_LEN(myFifo.buf);
        }
    SCI_enableInterrupt(mySCIB_BASE, SCI_INT_TXFF);
}
#endif


 //const uint16_t datArr[] = {'1','2','3','4','5','6','1','2','3','4','5','6','1','2','3','4','5','6','1','2','3','4','5','6','1','2','3','4','5','6','1','2','3','4','5','6','1','2','3','4','5','6','1','2','3','4','5','6','\r','\n'};
 const uint16_t datArr[] = {'1','2','3','4','5','\r','\n'};
//
// Main
//
void main(void)
{

    //
    // Initialize device clock and peripherals
    //
    Device_init();

    //
    // Disable pin locks and enable internal pull-ups.
    //
    Device_initGPIO();

    //
    // Initialize PIE and clear PIE registers. Disables CPU interrupts.
    //
    Interrupt_initModule();

    //
    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    //
    Interrupt_initVectorTable();

    //
    // PinMux and Peripheral Initialization
    //
    Board_init();

    //
    // C2000Ware Library initialization
    //
    C2000Ware_libraries_init();

    //
    // Enable Global Interrupt (INTM) and real time interrupt (DBGM)
    //
    EINT;
    ERTM;

    stApiTimer t1,t2,tForceBusy;

    ApiTimerStart(&t1, 100,200);
    ApiTimerStart(&t2, 500,2);
    ApiTimerStart(&tForceBusy, 500,5);


    GPIO_writePin(OPLED_BL, 1);

    while(1)
    {
        if(ApiTimerGetExpire(&t1))
        {
            if(ApiSrlChkRxEmpty(API_SRLB) == false)
            {
                int readCntTot = 0;
                int readCnt;
                uint8_t dat[256];
                while(readCnt = ApiSrlRead(API_SRLB, dat, ARRAY_LEN(dat)))
                    readCntTot += readCnt;
                ApiSrlPrintf("RxCnt: %u \r\n", readCntTot);
                ApiSrlPrintf("RX[1-10]: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n\r\n", dat[0], dat[1], dat[2], dat[3], dat[4], dat[5], dat[6], dat[7], dat[8], dat[9]);
            }
        }

        if(ApiTimerGetExpire(&tForceBusy))
        {    
            DINT;
            DEVICE_DELAY_US(500);
            EINT;
        }
    }
}

//
// End of File
//
