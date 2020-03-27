// Feather9x_RX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (receiver)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_TX
 
#include <SPI.h>
#include <RH_RF95.h>
#include "SD.h"
#include "RTClib.h"
 
//for Feather32u4 RFM9x
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7
 
// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 868.0
 
// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);
 
// Blinky on receipt
#define LED 11
#define AN 12
//#define LOG_INTERVAL 1000 // mills between entries (reduce to take more/faster data)
//#define SYNC_INTERVAL 1000 // mills between calls to flush() - to write data to the card
//uint32_t syncTime = 0; // time of last sync()
RTC_PCF8523 RTC;

const int chipSelect = 10;
// the logging file
File logfile;

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
  
  
  while(1);
}

struct dataStruct{
  double latitude;
  double longitude;
  uint8_t stunde;
  uint8_t minut;
  uint8_t sekunde;
  uint8_t millisekunde;
  unsigned long counter;
}myData;


 
void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(AN, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  digitalWrite(AN, HIGH);
 
  Serial.begin(9600);
 
  //while (!Serial)
  {
    delay(1);
  }
  delay(100);

    // initialize the SD card
  //Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  SD.begin(chipSelect);
    // create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE); 
      break;  // leave the loop!
    }
  }
      // connect to RTC
   
  if (!RTC.begin()) 
  {
      Serial.println("RTC failed");
  }
  
     logfile.println("Date,Time(GMT+2),Latitude,Logitude,GPS-Time(UTC),RSSI"); 
 
  Serial.println("Feather LoRa RX Test!");
 
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
 
  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");
 
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
 
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
 
  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
}
 
void loop()
{
      DateTime now;

  // delay for the amount of time we want between readings
  //delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL));
  
  // log milliseconds since starting
  //  uint32_t m = millis();
  //logfile.print(m);           // milliseconds since start
  //logfile.print(", "); 
/*
  // fetch the time
  now = RTC.now();
  // log time
  //logfile.print(now.unixtime()); // seconds since 1/1/1970
  logfile.print(now.year(), DEC);
  logfile.print("/");
  logfile.print(now.month(), DEC);
  logfile.print("/");
  logfile.print(now.day(), DEC);
  
  logfile.print(", ");
  logfile.print(now.hour(), DEC);
  logfile.print(":");
  logfile.print(now.minute(), DEC);
  logfile.print(":");
  logfile.print(now.second(), DEC);
  logfile.print('"');
  logfile.print(", ");
*/

  
  if (rf95.available())
  {
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    
    if (rf95.recv(buf, &len))
    {
      digitalWrite(LED, HIGH);
      //RH_RF95::printBuffer("Received: ", buf, len);
      memcpy(&myData,buf,sizeof(myData));

        // fetch the time
  now = RTC.now();
  // log time
  //logfile.print(now.unixtime()); // seconds since 1/1/1970
  logfile.print(now.year(), DEC);
  logfile.print("/");
  logfile.print(now.month(), DEC);
  logfile.print("/");
  logfile.print(now.day(), DEC);
  
  logfile.print(", ");
  logfile.print(now.hour(), DEC);
  logfile.print(":");
  logfile.print(now.minute(), DEC);
  logfile.print(":");
  logfile.print(now.second(), DEC);
  logfile.print('"');
  //logfile.print(", ");
     
      
      Serial.println("");
      Serial.print("latitude: ");
      logfile.print(", ");
      logfile.print(myData.latitude,6);
      Serial.print(myData.latitude,6);
      
      Serial.print("\t longitude: ");
      logfile.print(", ");
      logfile.print(myData.longitude,6);
      Serial.print(myData.longitude,6);
      
      Serial.print("\t Time: ");
      logfile.print(", ");
       if (myData.stunde < 10) Serial.print(F("0"));
       if (myData.stunde < 10) logfile.print(F("0"));
      logfile.print(myData.stunde);
      logfile.print(":");
      Serial.print(myData.stunde);
      Serial.print(":");
       if (myData.minut < 10) Serial.print(F("0"));
       if (myData.minut < 10) logfile.print(F("0"));
       logfile.print(myData.minut);
       logfile.print(":");
      Serial.print(myData.minut);
      Serial.print(":");
       if (myData.sekunde < 10) Serial.print(F("0"));
       if (myData.sekunde < 10) logfile.print(F("0"));
       logfile.print(myData.sekunde);
       logfile.print(".");
      Serial.print(myData.sekunde);
      Serial.print(".");
      if (myData.millisekunde < 10) Serial.print(F("0"));
      if (myData.millisekunde < 10) logfile.print(F("0"));
      logfile.print(myData.millisekunde);
      logfile.print(",");
      Serial.print(myData.millisekunde);
      Serial.println("");
      

      
      
      //Serial.println((char*)buf);
       Serial.print("RSSI: ");
       logfile.print(rf95.lastRssi(),DEC);
       logfile.print(", \n");
      Serial.println(rf95.lastRssi(), DEC);
 
      // Send a reply
      uint8_t data[] = "OK";
     
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      
      //Serial.println("Sent a reply");
      delay(100);
      digitalWrite(LED, LOW);
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
       // if ((millis() - syncTime) < SYNC_INTERVAL) return;
      //syncTime = millis();
    
      logfile.flush();
}
