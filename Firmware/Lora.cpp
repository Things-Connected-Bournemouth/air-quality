#include "Lora.h"
#include "LoRaWAN.h"

Lora::Lora(){

    strncpy(cred.appEui, APP_EUI,  sizeof(cred.appEui));    
    strncpy(cred.appKey, APP_KEY,  sizeof(cred.appKey));    
    LoRaWAN.getDevEui(cred.devEui,18);
  
}
void Lora::begin(){

    LoRaWAN.begin(EU868);
    LoRaWAN.setDutyCycle(false);
    
    LoRaWAN.setAntennaGain(0);
    
    LoRaWAN.setLinkCheckLimit(10);
    LoRaWAN.setLinkCheckDelay(8);
    LoRaWAN.setLinkCheckThreshold(5);

    LoRaWAN.onJoin(joinCallback);
    LoRaWAN.onLinkCheck(linkCheckCallback);
    LoRaWAN.onTransmit(txDoneCallback);
    LoRaWAN.onReceive(receiveCallback);

    LoRaWAN.setSaveSession(false);

    Serial.println("JOIN( )");
    LoRaWAN.joinOTAA(cred.appEui, cred.appKey, cred.devEui); 
   
}
void Lora::tx(byte packet[51], uint8_t packetLength){
  
    if (!LoRaWAN.busy())
    {
        if (!LoRaWAN.linkGateways())
        {
            Serial.println("REJOIN( )");
            LoRaWAN.rejoinOTAA();
        }
        
        if (LoRaWAN.joined())
        {
            digitalWrite(LED_BUILTIN, 1);

            Serial.print("TRANSMIT( ");
            Serial.print("TimeOnAir: ");
            Serial.print(LoRaWAN.getTimeOnAir());
            Serial.print(", NextTxTime: ");
            Serial.print(LoRaWAN.getNextTxTime());
            Serial.print(", MaxPayloadSize: ");
            Serial.print(LoRaWAN.getMaxPayloadSize());
            Serial.print(", DR: ");
            Serial.print(LoRaWAN.getDataRate());
            Serial.print(", TxPower: ");
            Serial.print(LoRaWAN.getTxPower(), 1);
            Serial.print("dbm, UpLinkCounter: ");
            Serial.print(LoRaWAN.getUpLinkCounter());
            Serial.print(", DownLinkCounter: ");
            Serial.print(LoRaWAN.getDownLinkCounter());
            Serial.println(" )");
            
            LoRaWAN.beginPacket();
            for(int i=0;i<packetLength;i++){
                LoRaWAN.write(packet[i]);
            }
            LoRaWAN.endPacket(); 
            digitalWrite(LED_BUILTIN, 0);
        }
    }  
}
void Lora::wait(long delaytime){
    long delayMillis = millis();
    while (millis() < (delayMillis + delaytime)) {}
}
void Lora::joinCallback(){
   STM32L0.wakeup();
   if (LoRaWAN.joined())
   {
       Serial.println("JOINED( )");
   }
   else
   {
       Serial.println("REJOIN( )");
       LoRaWAN.rejoinOTAA();
   }  
}


void Lora::linkCheckCallback(){
    STM32L0.wakeup();
    Serial.println("LINK CHECK( ");
    Serial.print("RSSI: ");
    Serial.print(LoRaWAN.lastRSSI());
    Serial.print(", SNR: ");
    Serial.print(LoRaWAN.lastSNR());
    Serial.print(", Margin: ");
    Serial.print(LoRaWAN.linkMargin());
    Serial.print(", Gateways: ");
    Serial.print(LoRaWAN.linkGateways());
    Serial.println(" )");
}
void Lora::txDoneCallback(){
    STM32L0.wakeup();
    
    if (!LoRaWAN.linkGateways()){
        Serial.println("DISCONNECTED");
    } 
    else{
        Serial.println("TX DONE( )");
    }
}
void Lora::receiveCallback(){
    STM32L0.wakeup();
    Serial.print("RECEIVE( ");
    Serial.print("RSSI: ");
    Serial.print(LoRaWAN.lastRSSI());
    Serial.print(", SNR: ");
    Serial.print(LoRaWAN.lastSNR());

   if (LoRaWAN.parsePacket())
   {
       uint32_t size;
       uint8_t data[256];

       size = LoRaWAN.read(&data[0], sizeof(data));

       if (size)
       {
           data[size] = '\0';

           Serial.print(", PORT: ");
           Serial.print(LoRaWAN.remotePort());
           Serial.print(", DATA: \"");
           Serial.print((const char*)&data[0]);
           Serial.println("\"");
       }
   }
   Serial.println(" )");

}
