/*
  UberdustBridge 
 
 - connects to an MQTT server
 - publishes coap messages to the topic "testbed#"
 - subscribes to the topic "heartbeat" to receive keep-alive messages from the server
 */

//The TestbedID to use for the connection
#define TESTBED_ID 5

//Software Reset
#include <avr/wdt.h>

//Ethernet Libraries
#include <SPI.h>
#include <Ethernet.h>
EthernetClient ethernetClient ;

//XBee Libraries
#include <XBee.h>
#include <XbeeRadio.h>
//Create the XbeeRadio object we'll be using
XBeeRadio xbee;
// create a reusable response object for responses we expect to handle 
XBeeRadioResponse response;
//Reusable response
Rx16Response rx;

//MQTT Library
#include <PubSubClient.h>

//Message Routing
//#include "TreeRouting.h"
//TreeRouting * routing;

//Helper Class
#include "UberdustGateway.h"
UberdustGateway gateway(&ethernetClient);


// Update these with values suitable for your network/broker.
byte mac[]    = { 
  0xAE, 0xED, 0xBA, 0xFE, 0xaa, 0xaa };
byte uberdustServer[] = { 
  150,140,5,20 };

// global variables
char address[20];
bool receivedAny;
int lastReceivedStatus;
long lastReceived;
long lastCheck;

/**
 * Callaback to the MQTT connection. 
 * Topic: heartbeat - used to keep-alive the connection to the server
 * Topic: arduinoGateway - used to receive commands from the uberdust server
 */
void callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic,"heartbeat")==0){
    lastCheck=millis();
  }
  else{
    //TODO: actually check for topic : arduinoGateway
    //TODO: forward topic commands to the routing protocol.
    Tx16Request tx = Tx16Request(*((uint16_t*)payload), payload+2, length);
    xbee.send(tx, 112);
  }
}

/**
 * Software 2color led implementation
 */
void ledState(int led1){
  if (led1==2){
    digitalWrite(9,HIGH);
    digitalWrite(8,lastReceivedStatus);
  }
  else if (led1==1){
    digitalWrite(9,HIGH);
    digitalWrite(8,lastReceivedStatus?HIGH:LOW);
  }
  else if (led1==0){
    digitalWrite(9,LOW);
    digitalWrite(8,lastReceivedStatus?HIGH:LOW);
  }
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
  pinMode(9,OUTPUT);
  pinMode(8,OUTPUT);
  bootblink();
  ledState(2);

  //Connect to XBee
  xbee.initialize_xbee_module();
  xbee.begin(38400);
  //Initialize our XBee module with the correct values using channel 12
  xbee.init(13);
  lastReceivedStatus=false;
  ledState(1);

  //Generate Unique mac based on xbee address
  uint16_t my_address = xbee.getMyAddress();
  //TODO:fix endianness
  memcpy(mac+4,&my_address,2);

  //routing = new TreeRouting(&xbee,true);
  //Connect to Network
  if (Ethernet.begin(mac)==0){
    //Software Reset
    ledState(2);
    watchdogReset();
  }
  else{
    //Connect to MQTT broker
    ledState(0);
    gateway.setUberdustServer(uberdustServer);
    gateway.setGatewayID(xbee.getMyAddress());
    gateway.setTestbedID(TESTBED_ID);
    gateway.connect(callback);
    gateway.pongServer();

  }
  //Initialize variables
  lastCheck=millis();  
  lastReceivedStatus=false;
  lastReceived=millis();
  receivedAny=false;

}

/**
 * Runs continously
 * -Check server connection
 * -Check MQTT messages
 * -Check XBee messages
 */
void loop()
{
  //Check server connection
  if (millis()-lastCheck >30000){
    ledState(1);
    watchdogReset();
  }
  else{
    ledState(0);
  }
  //Check MQTT messages
  gateway.loop();

  //Check XBee messages
  if(xbee.checkForData(112))
  {
    xbee.getResponse().getRx16Response(rx);
    receivedAny=true;
    sprintf(address,"%x",rx.getRemoteAddress16());

    gateway.publish(rx.getRemoteAddress16(),xbee.getResponse().getData(),xbee.getResponse().getDataLength()); 
  }

  //Blink on network traffic
  if (millis()-lastReceived>5000){
    if (receivedAny){
      lastReceivedStatus=!lastReceivedStatus;
      receivedAny=false;
    }
    lastReceived=millis();
  }
}


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
 * Fancy reboot indicator
 */
void bootblink(){
  for (int i=0;i<4;i++){
    digitalWrite(9,HIGH);
    digitalWrite(8,LOW);
    delay(300);
    digitalWrite(9,LOW);
    digitalWrite(8,HIGH);
    delay(300);
  }
}





































