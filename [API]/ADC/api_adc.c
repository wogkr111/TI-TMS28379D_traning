#include "common.h"

#include "api_adc.h"

static uint16_t adcBuf[API_ADC_END];

#ifdef INT_myADC0_1_INTERRUPT_ACK_GROUP
 __interrupt void INT_myADC0_1_ISR(void)
{
    //Read the round-robin pointer to determine which burst just completed
    uint16_t rrPointer = (HWREGH(ADCA_BASE + ADC_O_SOCPRICTL) & 0x03E0) >> 5;
    ADC_IntNumber burstIntSource;

    // (rrPointer = burst mode step / 15)
    // burst mode 기준
    // ADCINTn 에 등록한 SOCn 변환이 끝(or시작) 될 때 인터럽트 이벤트 발생
    // burst 단위가 ADCINTn 에 등록한 SOCn 과 일치하지 않으면 ISR 에서 round robin 읽었을 때 의도한 SOCn 보다 더 뒤에 있을 수 있음
    // 총 16ch 중 burst mode 단위가 4 라면 ADCINTn 에 등록는 SOCn 은 3,7,11,15 로 설정 해야 함
    switch(rrPointer)
    {
    case 15:
        adcBuf[0] = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0);
        adcBuf[1] = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER1);
        adcBuf[2] = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER2);
        burstIntSource = ADC_INT_NUMBER1; // user define
        break;
    default:
        //ESTOP0; //handle error for unexpected RR pointer value
        break;
    }
#if 0 //'Continuous Interrupt Mode' 체크 하면 clear 안해도 인터럽트 계속 수행됨
    // Clear the interrupt flag
    ADC_clearInterruptStatus(myADC0_BASE, burstIntSource);
#endif

    // Check if overflow has occurred
    if(ADC_getInterruptOverflowStatus(myADC0_BASE, burstIntSource) == true)
    {
        ADC_clearInterruptOverflowStatus(myADC0_BASE, burstIntSource);
        ADC_clearInterruptStatus(myADC0_BASE, burstIntSource);
    }

    // Acknowledge the interrupt
    Interrupt_clearACKGroup(INT_myADC0_1_INTERRUPT_ACK_GROUP);
}
#endif





inline uint16_t ApiAdcGet(eApiAdcCh ch)
{
#if 0 // SW mullty channel start 
    ADC_forceMultipleSOC(myADC0_BASE, (ADC_FORCE_SOC0 | ADC_FORCE_SOC1 | ADC_FORCE_SOC2));
#elif 0 // SW single channel start
    ADC_forceSOC(myADC0_BASE, ADC_SOC_NUMBER0);
    ADC_forceSOC(myADC0_BASE, ADC_SOC_NUMBER1);
    ADC_forceSOC(myADC0_BASE, ADC_SOC_NUMBER2);
#elif 0 // SW burst mode start
    ADC_forceSOC(myADC0_BASE, ADC_SOC_NUMBER0);
#endif
    // DEVICE_DELAY_US(100);
    // adcBuf[0] = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0);
    // adcBuf[1] = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER1);
    // adcBuf[2] = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER2);

    return adcBuf[ch];
}
