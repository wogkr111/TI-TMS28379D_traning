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
#include "[API]/adc/api_adc.h"


__interrupt void INT_myEPWM1_ISR(void)
{
    EPWM_clearEventTriggerInterruptFlag(myEPWM1_BASE);
    Interrupt_clearACKGroup(INT_myEPWM1_INTERRUPT_ACK_GROUP);
}



#pragma DATA_SECTION(myDmaSrcBuf, "ramgs0");  // map the TX data to memory
#pragma DATA_SECTION(myDmaDstBuf, "ramgs1");  // map the RX data to memory
uint16_t myDmaSrcBuf[128];   // Send data buffer
uint16_t myDmaDstBuf[128];   // Receive data buffer
const void *myDmaSrcAdr = ADCARESULT_BASE;
const void *myDmaDstAdr = myDmaDstBuf;

 __interrupt void INT_myDMA0_ISR(void)
 {
    GPIO_togglePin(DBG_P32);
    Interrupt_clearACKGroup(INT_myDMA0_INTERRUPT_ACK_GROUP);
 }



#if 0
//#pragma DATA_SECTION(sData, "ramgs0");  // map the TX data to memory
//#pragma DATA_SECTION(rData, "ramgs1");  // map the RX data to memory

//
// Defines
//
#define BURST       8       // write 8 to the register for a burst size of 8
#define TRANSFER    16      // [(MEM_BUFFER_SIZE/(BURST)]

//
// Globals
//
uint16_t sData[128];   // Send data buffer
uint16_t rData[128];   // Receive data buffer
volatile uint16_t done;

//
// Function Prototypes
//
__interrupt void dmaCh6ISR(void);
void initDMA(void);
void error();



//
// error - Error Function which will halt the debugger
//
void error(void)
{
    ESTOP0;  //Test failed!! Stop!
    for (;;);
}

//
// dma_init - DMA setup for both TX and RX channels.
//
void initDMA()
{
    //
    // Refer to dma.c for the descriptions of the following functions.
    //

    //
    //Initialize DMA
    //
    DMA_initController();

    const void *destAddr;
    const void *srcAddr;
    srcAddr = (const void *)sData;
    destAddr = (const void *)rData;

    //
    // configure DMA CH6
    //
    DMA_configAddresses(DMA_CH6_BASE, destAddr, srcAddr);
    DMA_configBurst(DMA_CH6_BASE,BURST,1,1);
    DMA_configTransfer(DMA_CH6_BASE,TRANSFER,1,1);
    DMA_configMode(DMA_CH6_BASE,DMA_TRIGGER_SOFTWARE, DMA_CFG_ONESHOT_DISABLE);
    DMA_setInterruptMode(DMA_CH6_BASE,DMA_INT_AT_END);
    DMA_enableTrigger(DMA_CH6_BASE);
    DMA_enableInterrupt(DMA_CH6_BASE);
}

//
// local_D_INTCH6_ISR - DMA Channel6 ISR
//
__interrupt void dmaCh6ISR(void)
{
    uint16_t i;

    DMA_stopChannel(DMA_CH6_BASE);
    // ACK to receive more interrupts from this PIE group
    EALLOW;
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP7);
    EDIS;

    for( i = 0; i < 128; i++ )
    {
        //
        // check for data integrity
        //
        if (rData[i] != i)
        {
            error();
        }
    }

    done = 1; // Test done.
    return;
}
#endif


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


#if 0
 /********************************************************************************************************************************/
    initDMA();  // set up the dma

    //
    // Ensure DMA is connected to Peripheral Frame 2 bridge (EALLOW protected)
    //
    SysCtl_selectSecMaster(0, SYSCTL_SEC_MASTER_DMA);

    //
    // User specific code, enable interrupts:
    // Initialize the data buffers
    //
    for(int i = 0; i < 128; i++)
    {
        sData[i] = i;
        rData[i] = 0;
    }

    //
    // Enable interrupts required for this example
    //
    Interrupt_enable(INT_DMA_CH6);
    EINT;                                // Enable Global Interrupts
    // Start DMA channel
    DMA_startChannel(DMA_CH6_BASE);

    done = 0;           // Test is not done yet

    while(!done)        // wait until the DMA transfer is complete
    {
       DMA_forceTrigger(DMA_CH6_BASE);

       DEVICE_DELAY_US(1000);
    }

    //
    // When the DMA transfer is complete the program will stop here
    //
    ESTOP0;
    while(1)
    {

    }
    /*******************************************************************************************************************************/
#endif

    for(int i = 0; i < ARRAY_LEN(myDmaSrcBuf); i++)
        myDmaSrcBuf[i] = i+1;

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
    ApiTimerStart(&t3, 100,25);


    GPIO_writePin(OPLED_BL, 1);
    EPWM_disableInterrupt(myEPWM1_BASE);


    ApiSrlPrintf("\r\n[%s %s] [%s (%u)] Hellow world\r\n", __DATE__, __TIME__, __FILE__, __LINE__);
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

            ApiSrlPrintf("%6lu\t%u\t%u\t%u\r\n", sysTick, ApiAdcGet(API_ADCA_CH1), ApiAdcGet(API_ADCA_CH2), ApiAdcGet(API_ADCA_CH3));
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
