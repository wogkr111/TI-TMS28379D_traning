#ifndef API_ADC_H_
#define API_ADC_H_

typedef enum
{
    API_ADCA_CH1 = 0,
    API_ADCA_CH2,
    API_ADCA_CH3,
    API_ADC_END,
}eApiAdcCh;

inline uint16_t ApiAdcGet(eApiAdcCh ch);

#endif /* API_ADC_H_ */
