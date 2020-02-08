#include "Timer.h"
#include "TimerMillis.h"

Timer::Timer(){}

void Timer::begin(long intervalMilliSeconds, CallbackFunction cbTimerTick){
  timerTick = cbTimerTick;
  oTimer.start(timerTick, 0, intervalMilliSeconds);
}
