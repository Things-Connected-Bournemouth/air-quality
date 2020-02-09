
#ifndef _LORA_H_
#define _LORA_H_

#include "config.h"
#include "STM32L0.h"

class Lora
{
  
  public:
  
    Lora();
  
    struct cred {
      char devEui[17];
      char appEui[17];
      char appKey[33];
    } cred;
  
    void begin();
    void tx(byte packet[51], uint8_t packetLength);
    void wait(long delaytime);
   
  private:
    char devEui[32]; 
    static void joinCallback();
    static void linkCheckCallback();
    static void txDoneCallback();
    static void receiveCallback();
    int xVal;

};

#endif