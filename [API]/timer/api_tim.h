#ifndef API_TIM_H_
#define API_TIM_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    API_TIMER_STOP = 0,
    API_TIMER_SINGLE,
    API_TIMER_CONTINUE,
}eApiTimerMode;

typedef struct
{
    eApiTimerMode mode;
    uint32_t nextTick;
    uint32_t period;
}stApiTimer;


extern uint32_t sysTick; // do not edit !!

// fst      : The first period in milliseconds (0 = immediate expiration, fst < 0x80000000).
// period   : The recurring period in milliseconds (if <=0, single timer).
void ApiTimerStart(stApiTimer* tim, int32_t fst, int32_t period);

// return : false if not expired, true if expired.
bool ApiTimerGetExpire(stApiTimer* tim);

void ApiTimerStop(stApiTimer* tim);

// return : The remaining time in ms (0 = not expired or stopped).
uint32_t ApiTimerGetRemainigTick(stApiTimer *tim);


#endif /* API_TIM_H_ */
