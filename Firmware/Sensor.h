#include "global.h"
#include "VL53L1X.h"
#include "SHT3x.h"
#include "STM32L0.h"
#include "TimerMillis.h"
#include "Adafruit_ADS1015.h"


typedef struct txData{
      byte dataToSend[51];
      int dataToSendIndex;
};
  

class Sensor
{
  
  public:

    Sensor();

    void begin();
    txData transmit();
    
    void getRange();   
    int16_t getTemperature();
    int16_t getRelHumidity(); 
    uint16_t getBatteryAsInt();
    uint16_t getOP1AsInt();
    uint16_t getOP2AsInt();
    uint16_t getGas();

    uint16_t range;
  
    TimerMillis enableTimer;
    static void callbackEnable();


  private:

    VL53L1X sensor;

    SHT3x sht;

    Adafruit_ADS1115 ads;

    float voltage_adc0 = 0.0;
    float voltage_adc1 = 0.0;
    
    int16_t temperature;
    int16_t humidity;
    int16_t maxRange;
    int16_t minRange;
    int16_t rangeArray[3];

    uint8_t successReadings;
    uint8_t rangeCount;

    float VDDA;
    float VBUS;
    float Temperature;
    
    float DACvalue = 0.0f; 
    float ADCvalue = 0.0f;

   
};
