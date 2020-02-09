
//iot-solutions-lora-bin-sensor

#include "config.h"
#include "STM32L0.h"
#include "Lora.h"
#include "Timer.h"
#include "Sensor.h"


#define LED_BUILTIN 13


Lora lora;
Sensor sensor; 
Timer tRange; volatile bool bRange; 

float ADCvalue = 0.0f;

void cRange(){
  STM32L0.wakeup();
  bRange = true;
}

void startTimers(){
   tRange.begin(READ_INTERVAL*1000 ,cRange); 
}


void setup( void ){
  

    pinMode(10, INPUT);
   
    digitalWrite(LED_BUILTIN, 1);
      
    Serial.begin(9600);
    //while(!Serial){}

    delay(10000);
    digitalWrite(LED_BUILTIN, 0);

    sensor.begin(); 

    Serial.print("AppEui : ");Serial.println(lora.cred.appEui);
    Serial.print("AppKey : ");Serial.println(lora.cred.appKey);
    Serial.print("DevEui : ");Serial.println(lora.cred.devEui);
       
    lora.begin();

    startTimers();
    bRange = true;
    
}

void loop( void ){ 
  if (bRange) {sensor.getRange();  bRange = false;}
  if (bRangeEvent) {
    //Serial.print("Range detected : ");Serial.print(sensor.range);Serial.println("mm");
    bRangeEvent = false;
    
    txData txdata = sensor.transmit();
    lora.tx(txdata.dataToSend,txdata.dataToSendIndex);

  }
  
  #ifndef DEBUG
    STM32L0.stop();
  #endif
}



