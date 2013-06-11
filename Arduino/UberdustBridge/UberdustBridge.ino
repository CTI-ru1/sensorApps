/*
 Basic MQTT example 
 
 - connects to an MQTT server
 - publishes "hello world" to the topic "outTopic"
 - subscribes to the topic "inTopic"
 */

#include <SPI.h>
#include <Ethernet.h>
//Include Libraries
#include <XBee.h>
#include <XbeeRadio.h>
#include <PubSubClient.h>
#include <avr/wdt.h>
#include "TreeRouting.h"

#include "UberdustGateway.h"
EthernetClient ethernetClient ;
UberdustGateway gateway(&ethernetClient);
TreeRouting * routing;

//Create the XbeeRadio object we'll be using
XBeeRadio xbee = XBeeRadio();
// create a reusable response object for responses we expect to handle 
XBeeRadioResponse response = XBeeRadioResponse();
// create a reusable rx16 response object to get the address


// Update these with values suitable for your network.
byte mac[]    = { 
  0xAE, 0xED, 0xBA, 0xFE, 0xFF, 0xFF };
byte uberdustServer[] = { 
  150,140,5,20 };

char address[20];
//get the response
Rx16Response rx;
int lastReceivedStatus;
bool receivedAny;
long lastReceived;

long lastCheck;

void callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic,"heartbeat")==0){
    lastCheck=millis();
  }
  else{
    //TODO: forward topic commands to the routing protocol.
    Tx16Request tx = Tx16Request(*((uint16_t*)payload), payload+2, length);
    xbee.send(tx, 112);
  }
}


void setup()
{
  pinMode(9,OUTPUT);
  pinMode(8,OUTPUT);
  digitalWrite(9,HIGH);
  digitalWrite(8,HIGH);
  //start our XbeeRadio object and set our baudrate to 38400.
  xbee.begin(38400);
  //Initialize our XBee module with the correct values using channel 13
  xbee.init(12);
  digitalWrite(8,LOW);
  uint16_t my_address = xbee.getMyAddress();
  mac[4] = ((byte*)&my_address)[0];
  mac[5] = ((byte*)&my_address)[1];
  routing = new TreeRouting(&xbee,true);
  if (Ethernet.begin(mac)==0){
    watchdogReset();
  }
  else{
    digitalWrite(9,LOW);
    gateway.setXbeeRadio(&xbee);
    gateway.setUberdustServer(uberdustServer);
    gateway.setGatewayID(xbee.getMyAddress());
    gateway.setTestbedID(1);

    gateway.connect(callback);
    lastCheck=millis();

  }
  lastReceivedStatus=LOW;
  lastReceived=millis();
  receivedAny=false;
}

void loop()
{
  if (millis()-lastCheck >30000){
    //watchdogReset();
    digitalWrite(9,HIGH);
  }
  else{
    digitalWrite(9,LOW);
  }
  //checks for incoming packets
  gateway.loop();

  routing->loop();

  //returns true if there is a packet for us on port 112
  if(xbee.checkForData(112))
  {
    xbee.getResponse().getRx16Response(rx);
    receivedAny=true;
    sprintf(address,"%x",rx.getRemoteAddress16());

    gateway.publish(rx.getRemoteAddress16(),xbee.getResponse().getData(),xbee.getResponse().getDataLength()); 
  }

  if (millis()-lastReceived>5000){
    if (receivedAny){
      if (lastReceivedStatus==HIGH){
        lastReceivedStatus=LOW;
      }
      else{
        lastReceivedStatus=HIGH;
      }
      receivedAny=false;
      digitalWrite(8,lastReceivedStatus);
    }
    lastReceived=millis();
  }
}



void watchdogReset()
{
  delay(10);
  ethernetClient.stop();
  wdt_disable();
  wdt_enable(WDTO_2S);
  while(1);
}















