
#define RH_MESH_MAX_MESSAGE_LEN 50
#include <RHMesh.h>
#include <EEPROM.h>
#include <RHRouter.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>


#define RH_HAVE_SERIAL
#define LED 9
#define N_NODES 4

// for feather32u4 
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

// Data wire is conntec to the Arduino digital pin 10
#define ONE_WIRE_BUS 10
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

uint8_t nodeId;
uint8_t routes[N_NODES]; // full routing table for mesh
int16_t rssi[N_NODES]; // signal strength info
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
  int ident=1;
  float temp01;
  float temp02;
  float temp03;
  
  unsigned long counter;
 
}myData;
byte tx_buf[sizeof(myData)]={0};

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);
// Class to manage message delivery and receipt, using the driver declared above
RHMesh *manager;
void setup() 
{
  randomSeed(analogRead(0));
  pinMode(LED, OUTPUT);
  Serial.begin(115200);
  //while (!Serial) ; // Wait for serial port to be available

  nodeId = EEPROM.read(0);
  if (nodeId > 10) {
    Serial.print(F("EEPROM nodeId invalid: "));
    Serial.println(nodeId);
    nodeId = 1;
  }
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
      printAddress(tempDeviceAddress);
      Serial.println();
    } else {
      Serial.print("Found ghost device at ");
      Serial.print(i, DEC);
      Serial.print(" but could not detect address. Check power and cabling");
    }
  }
 
}



// Dont put this on the stack:
uint8_t buf[RH_MESH_MAX_MESSAGE_LEN];



//%%%%%%%%%%%%SENDING LOOP%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void loop()
{
 
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
    }
  }


  
  delay(4000);
      Serial.println("Sending to Server on Node 4");
      Serial.print("SensorNode:");
      Serial.print(myData.ident);
      Serial.print("\t temp01: ");
      Serial.print(myData.temp01);
      Serial.print("\t temp02: ");
      Serial.print(myData.temp02);
      Serial.print("\t temp03: ");
      Serial.print(myData.temp03);
  memcpy(tx_buf, &myData, sizeof(myData));
  byte zize = sizeof(myData);
  
  
    
  // Send a message to a rf95_mesh_server
  // A route to the destination will be automatically discovered.
  if (manager->sendtoWait(tx_buf, sizeof(myData), nodeId=4) == RH_ROUTER_ERROR_NONE)
  {
    // It has been reliably delivered to the next node.
    // Now wait for a reply from the ultimate server
    uint8_t len = sizeof(buf);
    uint8_t from;    
    if (manager->recvfromAckTimeout(buf, &len, 3000, &from))
    {
      Serial.print("got OK : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);
    }
    else
    {
      Serial.println("No reply, is rf22_mesh_server1, rf22_mesh_server2 and rf22_mesh_server3 running?");
    }
  }
  else
     Serial.println("sendtoWait failed. Are the intermediate mesh servers running?");
}
