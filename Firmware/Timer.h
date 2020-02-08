#include "global.h"
#include "STM32L0.h"
#include "TimerMillis.h"

class Timer
{ 
  
  public:

    Timer();
    typedef void (*CallbackFunction)(void);    
    CallbackFunction timerTick;
    void begin(long intervalMilliSeconds,CallbackFunction cbTimerTick);

  private:

      TimerMillis oTimer;
      static void callback_oTimer();
      volatile bool oTimeout = false;
      

};
