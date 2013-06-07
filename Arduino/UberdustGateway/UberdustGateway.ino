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

#include "UberdustGateway.h"
UberdustGateway gateway;

//Create the XbeeRadio object we'll be using
XBeeRadio xbee = XBeeRadio();
// create a reusable response object for responses we expect to handle 
XBeeRadioResponse response = XBeeRadioResponse();
// create a reusable rx16 response object to get the address


// Update these with values suitable for your network.
byte mac[]    = { 
  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
byte uberdustServer[] = { 
  150,140,5,11 };



void callback(char* topic, byte* payload, unsigned int length) {
  //TODO: forward topic commands to the routing protocol.
  Tx16Request tx = Tx16Request(0xffff, payload, length);
  blink(8);
  xbee.send(tx, 112);
}


void setup()
{
  pinMode(9,OUTPUT);
  pinMode(8,OUTPUT);
  //start our XbeeRadio object and set our baudrate to 38400.
  xbee.begin(38400);
  //Initialize our XBee module with the correct values using channel 13
  xbee.init(12);
  Ethernet.begin(mac);

  gateway.setXbeeRadio(&xbee);
  gateway.setUberdustServer(uberdustServer);
  gateway.setGatewayID(xbee.getMyAddress());
  gateway.setTestbedID(1);

  gateway.connect(callback);

}

void blink(int pin){
  digitalWrite(pin,HIGH);
  delay(100);
  digitalWrite(pin,LOW);
  delay(100);
}

void loop()
{
  //checks for incoming packets
  gateway.loop();

  //returns true if there is a packet for us on port 112
  if(xbee.checkForData(112))
  {
    //get the response
    Rx16Response rx;
    xbee.getResponse().getRx16Response(rx);

    char address[20];
    sprintf(address,"%x",rx.getRemoteAddress16());

    gateway.publish(rx.getRemoteAddress16(),xbee.getResponse().getData(),xbee.getResponse().getDataLength()); 
    blink(9);
  } 

}









