// Feather9x_TX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (transmitter)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_RX
 

#include <RH_RF95.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>


// for feather32u4 
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

 
// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 868.0
 
// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

#define LED 12
#define ROT 19
// Choose two Arduino pins to use for software serial
int RXPin = 10;
int TXPin = 11;

int GPSBaud = 9600;

// Create a TinyGPS++ object
TinyGPSPlus gps;

// Create a software serial port called "gpsSerial"
SoftwareSerial gpsSerial(RXPin, TXPin);

//%%%%%%%%%%%%%%%%%%%%%%DataStruct%%%%%%%%%%%%%%
struct dataStruct{
  double latitude;
  double longitude;
  uint8_t stunde;
  uint8_t minut;
  uint8_t sekunde;
  uint8_t millisekunde;
  unsigned long counter;
 
}myData;
byte tx_buf[sizeof(myData)]={0};

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void setup() 
{
  pinMode(ROT, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  digitalWrite(ROT, HIGH);
 
  Serial.begin(9600);
  //while (!Serial)  //Warten bis Serial Monitor möglich, auskommentiert wenn im Batteriebetrieb
  {
    delay(1);
  }
 //%%%%%%%%%%%%%%%%%%%%%%%%%%%%GPS%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  // Start the software serial port at the GPS's default baud
  gpsSerial.begin(GPSBaud);
  
 //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  delay(100);
 
  Serial.println("Feather LoRa TX Test!");
 
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
 
int16_t packetnum = 0;  // packet counter, we increment per xmission
void displayInfo()
{
  delay(200);
  if (gps.location.isValid())
  {
    Serial.print("Latitude: ");
    Serial.println(gps.location.lat(), 6);
    myData.latitude=(gps.location.lat());
    Serial.print("Longitude: ");
    Serial.println(gps.location.lng(), 6);
    myData.longitude=(gps.location.lng());
  }
  else
  {
    Serial.println("Location: Not Available");
  }

  
  Serial.print("Time: ");
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    myData.stunde=(gps.time.hour());
    Serial.print(":");
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    myData.minut=(gps.time.minute());
    Serial.print(":");
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    myData.sekunde=(gps.time.second());
   Serial.print(".");
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
   Serial.println(gps.time.centisecond());
    myData.millisekunde=(gps.time.centisecond());
  }
  else
  {
    Serial.println("Not Available");
  }

  Serial.println();
  Serial.println();
  delay(100);
}

void loop()
{
  delay(2500); // Wait 1 second between transmits, could also 'sleep' here!
  

    // This sketch displays information every time a new sentence is correctly encoded.
  while (gpsSerial.available() > 0)
    if (gps.encode(gpsSerial.read()))
      displayInfo();

  // If 5000 milliseconds pass and there are no characters coming in
  // over the software serial port, show a "No GPS detected" error
  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println("No GPS detected");
    while(true);
  }

  Serial.println("Transmitting..."); // Send a message to rf95_server

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */ 
  memcpy(tx_buf, &myData, sizeof(myData));
  byte zize = sizeof(myData);
  
  rf95.send(tx_buf, sizeof(myData));
  rf95.waitPacketSent();
 
 
 
  //Serial.println("Waiting for packet to complete..."); 
  //delay(10);
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
 
  //Serial.println("Waiting for reply...");
   if (rf95.waitAvailableTimeout(5000))
  { 
    // Should be a reply message for us now   
    if (rf95.recv(buf, &len))
   {
      digitalWrite(LED, HIGH);
      Serial.print("  Got reply: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);    
    }
    else
    {
      Serial.println("\n Receive failed");
    }
  }
  else
  {
    Serial.println("\n No reply, is there a listener around?");
  }
  delay(100);
 digitalWrite(LED, LOW);

 
}
