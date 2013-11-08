/*
  UberdustBridge
 - connects to an MQTT server
 - publishes coap messages to the topic "testbed#"
 - subscribes to the topic "heartbeat" to receive keep-alive messages from the server
 */
#define USE_TREE_ROUTING
#define USE_SD

//The TestbedID to use for the connection
#define TESTBED_ID "urn:wisebed:ctitestbed:"
#define CHANNEL 13

#include "LedUtils.h"

//Software Reset
#include <avr/wdt.h>

//Ethernet Libraries
#include <SPI.h>
#include <Ethernet.h>
EthernetClient ethernetClient ;

//XBee Libraries
#include <XBee.h>
#include <XbeeRadio.h>
//Create the XbeeRadio object we'll be re-using
XBeeRadio xbee;
// create a reusable response object for responses we expect to handle
XBeeRadioResponse response;
//Reusable response
Rx16Response rx;

//MQTT Library
#include <PubSubClient.h>

#ifdef USE_SD
#include <tinyFAT.h>
#endif 

//Message Routing
#include <BaseRouting.h>
#include <TreeRouting.h>
#include <NonRouting.h>
BaseRouting * radio;

//Helper Class
#include "UberdustGateway.h"
UberdustGateway gateway(&ethernetClient);

//uint16_t devices[20];
//void add_device(uint16_t device){
//  for (int i=0;i<20;i++){
//    if (devices[i]==device)return;
//  }
//  for (int i=0;i<20;i++){
//    if (devices[i]==0){
//      devices[i]=device;
//      return;
//    }
//  }
//}

/**
 * Check if the device was registered via this Gateway.
 * TODO: think if this is needed any more.
 */
//boolean check_device(uint16_t device){
//  for (int i=0;i<20;i++){
//    if (devices[i]==device)return true;
//  }
//  return false;
//}

// Update these with values suitable for your network/broker.
byte mac[]    =
{
  0xAE, 0xED, 0xBA, 0xFE, 0xaa, 0xaa
};

byte uberdustServer[] =
{
  150, 140, 5, 11
};

// global variables
char address[20];
bool receivedAny;
int lastReceivedStatus;
long lastReceived;

/**
 * Callaback to the MQTT connection.
 * Topic: heartbeat - used to keep-alive the connection to the server
 * Topic: arduinoGateway - used to receive commands from the uberdust server
 */
void callback(char* topic, byte* payload, unsigned int length)
{
  //gateway.incy();
  check_heartbeat(topic,payload,length);
  check_xbee(topic,payload,length);
}

void check_heartbeat(char* topic, byte* payload, unsigned int length)
{
  if (strcmp(topic, "heartbeat") == 0){
    if (strncmp((char *)payload, "reset",5)==0){
      //lastCheck = millis();
      wdt_reset();
      blinkFast(9);
    }
  }
  else{
    check_reset(topic,payload,length);
  }
}

void check_reset(char* topic, byte* payload, unsigned int length)
{
  if (strncmp((char*)payload,"reset",5)==0){
    watchdogReset();
  }
}
void check_xbee(char* topic, byte* payload, unsigned int length)
{
  blinkFast(8);
  radio->send( *((uint16_t*)payload) , &(payload[2]),length-2);
}

/**
 */
void radio_callback(uint16_t sender, byte* payload, unsigned int length) {
  //gateway.incx();
  receivedAny = true;
  //add_device(sender);
  sprintf(address, "%x", sender);
  gateway.publish(sender, payload, length);
}


/**
 * Initializes UberdustBridge.
 * -Connect to XBee
 * -Generate Unique mac based on xbee address
 * -Connect to Network
 * -Connect to MQTT broker
 * -Initialize variables
 */
void setup()
{
  pinMode(9, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(7, OUTPUT);
  bootblink();
  ledState(2);

  char testbedHash[50];

#ifdef USE_SD
  file.initFAT();
  file.exists("sensorflare.txt");
  file.openFile("sensorflare.txt");
  file.readLn(testbedHash, 80);
  file.closeFile();
#else
  strcpy(testbedHash,TESTBED_ID);
#endif

  Serial.begin(38400);
  Serial.flush();
  Serial.end();

  //Connect to XBee
  //wdt_enable(WDTO_8S);
  wdt_reset();
  wdt_disable();

//  for (int i=0;i<20;i++){
//    devices[i]=0;
//  }

  wdt_enable(WDTO_8S);
  xbee.begin(38400);
  wdt_reset();
  //Initialize our XBee module with the correct values using CHANNEL
  xbee.init(CHANNEL);
  wdt_reset();
  wdt_disable();

  lastReceivedStatus = false;
#ifdef USE_TREE_ROUTING
  radio = new TreeRouting(&xbee);
#else 
  radio = new NonRouting(&xbee);
#endif 
  radio->set_sink(true);

  uint16_t address = xbee.getMyAddress(); //fix 4hex digit address
  uint8_t * bit = ((uint8_t*) & address);
  uint8_t mbyte = bit[1];
  uint8_t lbyte = bit[0];
  bit[0] = mbyte;
  bit[1] = lbyte;
  radio->set_my_address(address);
  radio->set_message_received_callback(radio_callback);

  ledState(1);

  //Generate Unique mac based on xbee address
  uint16_t my_address = address;
  mac[4] = (&my_address)[1];
  mac[5] = (&my_address)[0];
  //memcpy(mac + 4, &my_address, 2);
  //radio = new TreeRouting(&xbee,true);
  wdt_enable(WDTO_8S);
  //Connect to Network
  if (Ethernet.begin(mac)==0){  
    //Software Reset
    ledState(2);
    watchdogReset();
  }
  else
  {
    wdt_reset();
    //Connect to MQTT broker
    ledState(0);
    gateway.setUberdustServer(uberdustServer);
    gateway.setGatewayID(address);
    gateway.setTestbedID(testbedHash);
    gateway.connect(callback);

  }

  wdt_reset();
  wdt_disable();

  //Initialize variables
  lastReceivedStatus = false;
  lastReceived = millis();
  receivedAny = false;
  wdt_enable(WDTO_8S);
}

/**
 * Runs continously
 * -Check server connection
 * -Check MQTT messages
 * -Check XBee messages
 */
void loop()
{
  //Check MQTT messages
  gateway.loop();
  radio->loop();

  //Blink on network traffic
  if (millis() - lastReceived > 5000)
  {
    if (receivedAny)
    {
      lastReceivedStatus = !lastReceivedStatus;
      receivedAny = false;
    }
    lastReceived = millis();
  }
}

//TODO: What?
#warning should use check not loop

/**
 * Sofrware Reset using watchdogTimer
 */
void watchdogReset()
{
  wdt_disable();
  wdt_enable(WDTO_30MS);
  while(1);
}

/**
 * Software 2color led implementation
 */
void ledState(int led1)
{
  if (led1 == 2)
  {
    digitalWrite(9, HIGH);
    digitalWrite(8, HIGH);
  }
  else if (led1 == 1)
  {
    digitalWrite(9, HIGH);
    digitalWrite(8, lastReceivedStatus ? HIGH : LOW);
  }
  else if (led1 == 0)
  {
    digitalWrite(9, LOW);
    digitalWrite(8, lastReceivedStatus ? HIGH : LOW);
  }
}



