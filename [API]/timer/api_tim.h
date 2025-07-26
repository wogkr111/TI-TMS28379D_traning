#ifndef API_TIM_H_
#define API_TIM_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    API_TIMER_STOP,
    API_TIMER_SINGLE,
    API_TIMER_CONTINUE,
}eApiTimerMode;

typedef struct
{
    eApiTimerMode mode;
    uint32_t period;
    uint32_t befTick;
}stApiTimer;


extern uint32_t sysTick; // do not edit !!

// fstPrd   : The first period in milliseconds (0 = immediate expiration).
// period   : The recurring period in milliseconds (0 = single timer).
void ApiTimerStart(stApiTimer* tim,uint32_t fstPrd, uint32_t period);

// return : false if not expired, true if expired.
bool ApiTimerGetExpire(stApiTimer* tim);

void ApiTimerStop(stApiTimer* tim);

// return : The remaining time in ms (0 = not expired or stopped).
uint32_t ApiTimerGetRemainigTick(stApiTimer *tim);


#endif /* API_TIM_H_ */
