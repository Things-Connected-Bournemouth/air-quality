#include "Sensor.h"


#define BATTERY_MON A4
#define LIDAR_ENABLE 3

#define OP1_ADC A1
#define TURBIDITY_POWER A2


#define OP2_ADC A0
#define PRESSURE_POWER A3

#define SAMPLES 100



Sensor::Sensor(){}

void Sensor::begin(){
    
  // i2c configuration and initialisation
    Wire.begin();
    Wire.setClock(400000); // I2C frequency at 400 kHz 

    sht.SetUpdateInterval(1000);
    sht.Begin();

    ads.begin();

    // // ads.setGain(GAIN_TWOTHIRDS);  +/- 6.144V  1 bit = 0.1875mV (default)
    // // ads.setGain(GAIN_ONE);        +/- 4.096V  1 bit = 0.125mV
    // // ads.setGain(GAIN_TWO);        +/- 2.048V  1 bit = 0.0625mV
    // // ads.setGain(GAIN_FOUR);       +/- 1.024V  1 bit = 0.03125mV
    ads.setGain(GAIN_EIGHT);      //+/- 0.512V  1 bit = 0.015625mV
    // // ads.setGain(GAIN_SIXTEEN);    +/- 0.256V  1 bit = 0.0078125mV 
    

    int16_t temp = getTemperature();
    int16_t humidity = getRelHumidity();
  
    // battery monitor configuration and initialisation
    pinMode(BATTERY_MON, INPUT);

    pinMode(LED_BUILTIN, OUTPUT);
   
    pinMode(TURBIDITY_POWER, OUTPUT);
    pinMode(OP1_ADC, INPUT);

    pinMode(PRESSURE_POWER, OUTPUT);
    pinMode(OP2_ADC, INPUT);

    analogReadResolution(12);  // set ADC resolution to 12 bit 
  

    #ifdef ENABLE_RADAR
    pinMode(LIDAR_ENABLE, OUTPUT);
    digitalWrite(LIDAR_ENABLE, HIGH); 
    #endif
  
    maxRange = 2000;
    minRange = 40;
    rangeCount = 0;

}

void Sensor::getRange(){

   #ifdef ENABLE_RADAR

  pinMode(LIDAR_ENABLE, OUTPUT);
  digitalWrite(LIDAR_ENABLE, HIGH);

    if (!sensor.init())
    {
        Serial.println("Failed to detect and initialize sensor!");
    }
    else{

      
    }
    // Use long distance mode and allow up to 50000 us (50 ms) for a measurement.
    // You can change these settings to adjust the performance of the sensor, but
    // the minimum timing budget is 20 ms for short distance mode and 33 ms for
    // medium and long distance modes. See the VL53L1X datasheet for more
    // information on range and timing limits.
    sensor.setDistanceMode(VL53L1X::Short);
    sensor.setMeasurementTimingBudget(20000);
    sensor.startContinuous(21);
    sensor.read(true);
    int count = 0;
    
    // stop readings before sensor has started and stabilised
    while (sensor.ranging_data.range_mm < 20 || count < 5){
       delay(21);
       sensor.read(true);
       count++;
    }

    range = sensor.ranging_data.range_mm;

  digitalWrite(LIDAR_ENABLE, LOW); 

  // fix for having incorrect pullup on lidar enable
  // turn gpio into input  
  pinMode(LIDAR_ENABLE, INPUT);
  
  #endif
  
  
  bRangeEvent = true; 
    
    

}

txData Sensor::transmit(){

  byte dataToSend[51];

  // set all bytes to 0x00
  for(int i=0;i<sizeof(dataToSend);i++){
    dataToSend[i] = 0x00;
  }

  int16_t battery = getBatteryAsInt();
  int16_t temp = getTemperature();
  int16_t humidity = getRelHumidity();
  gas gas_result = getGas();
  
  int dataToSendIndex = 0;

  dataToSend[dataToSendIndex]= 0x01;  // put reason code at beginning of the message
  dataToSendIndex++;

  dataToSend[dataToSendIndex] =   (byte)(range >> 8) & 0xFF;        // x(MSB)
  dataToSendIndex++;
  
  dataToSend[dataToSendIndex] =   (byte) range & 0xFF;              // x(LSB)
  dataToSendIndex++;
  
  dataToSend[dataToSendIndex] =  (byte)(temp >> 8) & 0xFF;                // x(MSB)
  dataToSendIndex++;

  dataToSend[dataToSendIndex] =  (byte) temp & 0xFF;                      // x(LSB)
  dataToSendIndex++;

  dataToSend[dataToSendIndex] =  (byte)(humidity >> 8) & 0xFF;                // x(MSB)
  dataToSendIndex++;

  dataToSend[dataToSendIndex] =  (byte) humidity & 0xFF;                      // x(LSB)
  dataToSendIndex++;

  dataToSend[dataToSendIndex] =   (byte)(battery >> 8) & 0xFF;            // x(MSB)
  dataToSendIndex++;
  
  dataToSend[dataToSendIndex] =   (byte) battery & 0xFF;                  // x(LSB)
  dataToSendIndex++;

  dataToSend[dataToSendIndex] =   (byte)(gas_result.working >> 8) & 0xFF;            // x(MSB)
  dataToSendIndex++;

  dataToSend[dataToSendIndex] =   (byte) gas_result.working & 0xFF;                  // x(LSB)
  dataToSendIndex++;

  dataToSend[dataToSendIndex] =   (byte)(gas_result.auxiliary >> 8) & 0xFF;            // x(MSB)
  dataToSendIndex++;

  dataToSend[dataToSendIndex] =   (byte) gas_result.auxiliary & 0xFF;                  // x(LSB)
  dataToSendIndex++;

  txData data;

  memcpy(data.dataToSend, dataToSend, sizeof data.dataToSend);
  data.dataToSendIndex = dataToSendIndex;

  return data;
  
}

int16_t Sensor::getTemperature(){
  sht.UpdateData();
  float t = (float)sht.GetTemperature();
  Serial.print("Temp: "); Serial.print(t);Serial.println(" DEG C");
  return (int16_t)(t*100);
}

int16_t Sensor::getRelHumidity(){
  sht.UpdateData();
  float h = (float)sht.GetRelHumidity();
  Serial.print("Relative humidity: "); Serial.print(h);Serial.println("%");
  return (int16_t)(h*100);
}

uint16_t Sensor::getBatteryAsInt(){
  
  VDDA = STM32L0.getVDDA();
  VBUS = STM32L0.getVBUS();
  
  int aValue = analogRead(BATTERY_MON);
  ADCvalue = (VDDA * ((float) aValue)/4095.0f) * 2; // is actually read

  #ifdef MY_DEBUG
    Serial.print(" VDDA: "); Serial.print(VDDA);Serial.println("V");
    Serial.print(" ADC: "); Serial.print(aValue);Serial.println();
    Serial.print("ADC input is :"); Serial.print(ADCvalue, 3);Serial.println("V");
  # endif 

  uint16_t result = (uint16_t)(ADCvalue * 100);
  Serial.print("Battery Voltage: "); Serial.println(ADCvalue);

  return result;
 
}

gas Sensor::getGas(){

  int32_t adc0 = 0;  // we read from the ADC, we have a sixteen bit integer as a result
  int32_t adc1 = 0;  // we read from the ADC, we have a sixteen bit integer as a result

  gas retVal;

  // digital averaging across 100 readings spaced at 10ms apart
  for (int i = 0; i < 100; ++i)
  {

     adc0 = adc0 + ads.readADC_SingleEnded(0);
     adc1 = adc1 + ads.readADC_SingleEnded(1);
     delay(10);
     
  }

  adc0 = adc0/100;
  adc1 = adc1/100;
  
  voltage_adc0 = (adc0 * 0.015625);
  voltage_adc1 = (adc1 * 0.015625);

  retVal.working = (int16_t)(voltage_adc0 * 100);
  retVal.auxiliary = (int16_t)(voltage_adc1 * 100);

  Serial.print("Working Electrode  Voltage ADC0: "); Serial.println(voltage_adc0);
  Serial.print("Auxiliary Electrode Voltage ADC1: "); Serial.println(voltage_adc1);

  Serial.println();
  return gas;

}



