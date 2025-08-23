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


#include "common.h"
#include "[API]/timer/api_tim.h"
#include "[API]/srl/api_srl.h"
#include "[API]/flash/api_flash.h"


__interrupt void INT_myEPWM1_ISR(void)
{
    EPWM_clearEventTriggerInterruptFlag(myEPWM1_BASE);
    Interrupt_clearACKGroup(INT_myEPWM1_INTERRUPT_ACK_GROUP);
}



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

    //USER CODE
    //FLASH_init();

    stApiTimer t1,t2, t3;

    ApiTimerStart(&t1, 100,200);
    ApiTimerStart(&t2, 500,2);
    ApiTimerStart(&t3, 100,200);


    GPIO_writePin(OPLED_BL, 1);
    EPWM_disableInterrupt(myEPWM1_BASE);


    ApiSrlPrintf("[%s %s] [%s (%u)] Hellow world\r\n", __DATE__, __TIME__, __FILE__, __LINE__);
    DEVICE_DELAY_US(10000);

    while(1)
    {
        if(ApiTimerGetExpire(&t1))
        {
            if(ApiSrlRxBytesToRead(API_SRLB) > 0)
            {
                int readCntTot = 0;
                int readCnt;
                uint8_t dat[256];
                while(readCnt = ApiSrlRead(API_SRLB, dat, ARRAY_LEN(dat)))
                    readCntTot += readCnt;
                ApiSrlPrintf("RxCnt: %u \r\n", readCntTot);
            }
        }

        if(ApiTimerGetExpire(&t2))
        {
            static int prd = 0;
            prd = (prd+1)%1000;
            EPWM_setCounterCompareValue(myEPWM1_BASE, EPWM_COUNTER_COMPARE_B, prd);	
        }

        if(ApiTimerGetExpire(&t3))
        {
            static int c = 0;
            GPIO_togglePin(DBG_P32);
            ApiSrlPrintf("%u\r\n", c++);
        }


#if USE_FORCE_IDS_DISABLE_BUSY == true
        {   
            #warning FORCED_BUSY_ENABLE
            static stApiTimer tForceBusy;
            if(tForceBusy.mode != API_TIMER_CONTINUE)
            {
                ApiTimerStart(&tForceBusy, 500,5);
            }
            else if(ApiTimerGetExpire(&tForceBusy))
            {
                DINT;
                DEVICE_DELAY_US(500);
                EINT;
            }
        }
#endif
    }
}

//
// End of File
//
