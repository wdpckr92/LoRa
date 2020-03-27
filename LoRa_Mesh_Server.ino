
#define RH_MESH_MAX_MESSAGE_LEN 50
#include <RHMesh.h>
#include <EEPROM.h>
#include <RHRouter.h>
#include <RH_RF95.h>
#include <SPI.h>

#define RH_HAVE_SERIAL
#define LED 8
#define N_NODES 4
// In this small artifical network of 4 nodes,
//#define CLIENT_ADDRESS 1
//#define SERVER1_ADDRESS 2
//#define SERVER2_ADDRESS 3
//#define SERVER3_ADDRESS 4

// for feather32u4 
//#define RFM95_CS 8
//#define RFM95_RST 4
//#define RFM95_INT 7

uint8_t nodeId;
uint8_t routes[N_NODES]; // full routing table for mesh
int16_t rssi[N_NODES]; // signal strength info

struct dataStruct{
  int ident;
  float temp01;
  float temp02;
  float temp03;
  unsigned long counter;
}myData;

// Singleton instance of the radio driver
RH_RF95 rf95;//(RFM95_CS, RFM95_INT);
// Class to manage message delivery and receipt, using the driver declared above
RHMesh *manager;
void setup() 
{
  randomSeed(analogRead(0));
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available

  nodeId = 4;
  //if (nodeId > 10) {
  //  Serial.print(F("EEPROM nodeId invalid: "));
  //  Serial.println(nodeId);
  //  nodeId = 1;
  //}
  Serial.print(F("initializing node "));

  manager = new RHMesh(rf95, nodeId);

  if (!manager->init()) {
    Serial.println(F("init failed"));
  } else {
    Serial.println("done");
  }
  rf95.setTxPower(23, false);
  rf95.setFrequency(868.0);
  rf95.setCADTimeout(500);

    // long range configuration requires for on-air time
  boolean longRange = false;
  if (longRange) {
    RH_RF95::ModemConfig modem_config = {
      0x78, // Reg 0x1D: BW=125kHz, Coding=4/8, Header=explicit
      0xC4, // Reg 0x1E: Spread=4096chips/symbol, CRC=enable
      0x08  // Reg 0x26: LowDataRate=On, Agc=Off.  0x0C is LowDataRate=ON, ACG=ON
    };
    rf95.setModemRegisters(&modem_config);
    if (!rf95.setModemConfig(RH_RF95::Bw125Cr48Sf4096)) {
      Serial.println(F("set config failed"));
    }
  }

  Serial.println("RF95 ready");

  for(uint8_t n=1;n<=N_NODES;n++) {
    routes[n-1] = 0;
    rssi[n-1] = 0;
  }
}
uint8_t data[] = "OK";
// Dont put this on the stack:
uint8_t buf[RH_MESH_MAX_MESSAGE_LEN];
void loop()
{
  uint8_t len = sizeof(buf);
  uint8_t from;
  if (manager->recvfromAck(buf, &len, &from))
  {
    Serial.print("Node : 0x");
    Serial.print(from, HEX);
    Serial.print(": ");
    Serial.println((char*)buf);
    memcpy(&myData,buf,sizeof(myData))
    ;
      Serial.println("");
      Serial.print("SensorNode: ");
      Serial.print(myData.ident);
      Serial.print("\ttemp01: ");
      Serial.print(myData.temp01);
      Serial.print("\t temp02: ");
      Serial.print(myData.temp02);
      Serial.print("\t temp03: ");
      Serial.print(myData.temp03);
    // Send a reply back to the originator client
    if (manager->sendtoWait(data, sizeof(data), from) != RH_ROUTER_ERROR_NONE)
      Serial.println("sendtoWait failed");
  }
}
