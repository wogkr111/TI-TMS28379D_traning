#include "api_tim.h"
#include "board.h"

uint32_t sysTick;

__interrupt void INT_myCPUTIMER0_ISR(void)
{
  sysTick++;
  Interrupt_clearACKGroup(INT_myCPUTIMER0_INTERRUPT_ACK_GROUP);
}


// fstPrd   : The first period in milliseconds (0 = immediate expiration).
// period   : The recurring period in milliseconds (0 = single timer).
void ApiTimerStart(stApiTimer *tim, uint32_t fstPrd, uint32_t period)
{
  tim->mode = (period == 0) ? API_TIMER_SINGLE : API_TIMER_CONTINUE;
  tim->period = period;
  tim->befTick = sysTick - fstPrd;
}


// return : false if not expired, true if expired.
bool ApiTimerGetExpire(stApiTimer *tim)
{
  bool expire;

  if (tim->mode != API_TIMER_STOP)
  {
    uint32_t tick = sysTick;
    uint32_t elapses = (tim->befTick <= tick) ? (tick - tim->befTick) : ((uint32_t)(0x100000000ULL + tick - tim->befTick));

    if (elapses >= tim->period)
    {
      expire = true;
      if (tim->mode == API_TIMER_CONTINUE)
        tim->befTick = tick;
    }
    else
    {
      expire = false;
    }
  }
  else
  {
    expire = false;
  }

  return expire;
}

void ApiTimerStop(stApiTimer *tim)
{
  tim->mode = API_TIMER_STOP;
}


// return : The remaining time in ms (0 = not expired or stopped).
uint32_t ApiTimerGetRemainigTick(stApiTimer *tim) 
{
  uint32_t remainingTick;

  if (tim->mode == API_TIMER_STOP)
  {
    remainingTick = 0;
  }
  else
  {
    uint32_t tick = sysTick;
    uint32_t elapses = (tim->befTick <= tick) ? (tick - tim->befTick) : ((uint32_t)(0x100000000ULL + tick - tim->befTick));

    remainingTick = (elapses >= tim->period)? (0) : (tim->period - elapses);
  }

  return remainingTick;
}
