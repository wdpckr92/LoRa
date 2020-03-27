// Feather9x_TX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (transmitter)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_RX
 
#include <SPI.h>
#include <RH_RF95.h>
#include <OneWire.h>
#include <DallasTemperature.h>


// for feather32u4 
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

//LED
#define LED 12
#define ROT 19

 
// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 868.0
 
// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Data wire is conntec to the Arduino digital pin 10
#define ONE_WIRE_BUS 10
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);
String datastring="";
char databuf[10];
uint8_t dataoutgoing[10];
int numberOfDevices; // Number of temperature devices found
DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address

// function to print a device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
}

//%%%%%%%%%%%%%%%%%%%%%%DataStruct%%%%%%%%%%%%%%
struct dataStruct{
  uint8_t node;
  float temp01;
  float temp02;
  float temp03;
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
 //%%%%%%%%%%%%%%%%%%%%%%%%%%%~~DeviceCounting~~%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 sensors.begin();
 numberOfDevices = sensors.getDeviceCount();

 // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(numberOfDevices, DEC);
  Serial.println(" devices.");

  
  // Loop through each device, print out address
  for(int i=0;i<numberOfDevices; i++) {
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i)) {
      Serial.print("Found device ");
      Serial.print(i, DEC);
      Serial.print(" with address: ");
      Serial.print("Found ghost device at ");
      Serial.print(i, DEC);
      Serial.print(" but could not detect address. Check power and cabling");
    }
  }
//%%%%%%%%%%%%%%%%Getting Sensor Data%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  sensors.requestTemperatures();
 // Loop through each device, print out temperature data
  for(int i=0;i<numberOfDevices; i++) {
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i)){
   
    for(int i=0;i<numberOfDevices; i++){
  if(sensors.getAddress(tempDeviceAddress, 0)){
  myData.temp01= sensors.getTempC(tempDeviceAddress);}
  if(sensors.getAddress(tempDeviceAddress, 1)){
  myData.temp02= sensors.getTempC(tempDeviceAddress);}
  if(sensors.getAddress(tempDeviceAddress, 2)){
  myData.temp03= sensors.getTempC(tempDeviceAddress);
} 
  myData.node=1;
    }
    }
  }

     /* Serial.println("");
      Serial.print("temp01: ");
      Serial.print(myData.temp01);
      Serial.print("\t temp02: ");
      Serial.print(myData.temp02);
      Serial.print("\t temp03: ");
      Serial.print(myData.temp03);
      */
    

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
  
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


void loop()
{
  delay(4000); // Wait 1 second between transmits, could also 'sleep' here!
  Serial.println("Transmitting..."); // Send a message to rf95_server
  
//%%%%%%%%%%%%%%%%Getting Sensor Data%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  sensors.requestTemperatures();
 // Loop through each device, print out temperature data
  for(int i=0;i<numberOfDevices; i++) {
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i)){
   
    for(int i=0;i<numberOfDevices; i++){
  if(sensors.getAddress(tempDeviceAddress, 0)){
  myData.temp01= sensors.getTempC(tempDeviceAddress);}
  if(sensors.getAddress(tempDeviceAddress, 1)){
  myData.temp02= sensors.getTempC(tempDeviceAddress);}
  if(sensors.getAddress(tempDeviceAddress, 2)){
  myData.temp03= sensors.getTempC(tempDeviceAddress);
} 
    }
      //Serial.println("");
      Serial.print("temp01: ");
      Serial.print(myData.temp01);
      Serial.print("\t temp02: ");
      Serial.print(myData.temp02);
      Serial.print("\t temp03: ");
      Serial.print(myData.temp03);

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
   if (rf95.waitAvailableTimeout(1000))
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
 
}
  delay(100);
 digitalWrite(LED, LOW);
  }}
