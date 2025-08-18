#include "api_tim.h"
#include "board.h"

uint32_t sysTick;

__interrupt void INT_myCPUTIMER0_ISR(void)
{
  sysTick++;
  Interrupt_clearACKGroup(INT_myCPUTIMER0_INTERRUPT_ACK_GROUP);
}


// fst      : The first period in milliseconds (0 = immediate expiration, fst < 0x80000000).
// period   : The recurring period in milliseconds (if <=0, single timer).
void ApiTimerStart(stApiTimer* tim, int32_t fst, int32_t period)
{
  tim->mode = (period <= 0)? API_TIMER_SINGLE : API_TIMER_CONTINUE;
  tim->nextTick = (uint32_t)(sysTick + fst);
  tim->period = (uint32_t)period;
}


// return : false if not expired, true if expired.
bool ApiTimerGetExpire(stApiTimer *tim)
{
  bool isExpire;

  // 1. overflow 미발생 (nextTick = sysTick + period, nextTick >= sysTick)
  //   1-1. 지정시간 달성 (sysTick >= nextTick)
  //     : (sysTick - nextTick) >= 0,  (비교적 작은 값)
  //     : (sysTick - nextTick) < (UINT32_MAX / 2) is true
  //   1-2. 지정시간 미달성 (nextTick > sysTick)
  //     : (sysTick - nextTick)  < 0,  (연산결과 overflow, 0x80000000ul 이상으로 처리, 매우 큰 값)
  //     : (sysTick - nextTick) < (UINT32_MAX / 2) is false

  // 2. nextTick overflow 발생 (nextTick = sysTick + period, nextTick << sysTick)
  //   2-1. 지정시간 미달성 (sysTick > nextTick 인 경우)
  //     : (sysTick - nextTick) > 0  (매우 큰 값)
  //     : (sysTick - nextTick) < (UINT32_MAX / 2) is false
  //   2-2. 지정시간 미달성 (sysTick < nextTick 인 경우, sysTick overflow)
  //     : (sysTick - nextTick)  < 0,  (연산결과 overflow, 0x80000000ul 이상으로 계산, 매우 큰 값)
  //     : (sysTick - nextTick) < (UINT32_MAX / 2) is false
  //   2-3. 지정시간 달성 (sysTick >= nextTick 인 경우, sysTick overflow)
  //     : (sysTick - nextTick) >= 0,  (비교적 작은 값)
  //     : (sysTick - nextTick) < (UINT32_MAX / 2) is true

  if((tim->mode != API_TIMER_STOP) && ((uint32_t)(sysTick - tim->nextTick) < (UINT32_MAX / 2)))
  {
    isExpire = true;
    if (tim->mode == API_TIMER_CONTINUE)
      tim->nextTick = sysTick + tim->period;
  }
  else
  {
    isExpire = false;
  }
  
  return isExpire;
}


void ApiTimerStop(stApiTimer *tim)
{
  tim->mode = API_TIMER_STOP;
}


// return : The remaining time in ms (0 =  expired or stopped).
uint32_t ApiTimerGetRemainigTick(stApiTimer *tim) 
{
  uint32_t remaningTick;

  if((tim->mode != API_TIMER_STOP) && ((uint32_t)(sysTick - tim->nextTick) >= (UINT32_MAX / 2)))
  {
    remaningTick = tim->nextTick - sysTick;
  }
  else
  {
    remaningTick = 0;
  }
  
  return remaningTick;
}
