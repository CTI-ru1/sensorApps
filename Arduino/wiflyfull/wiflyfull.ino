/*
 * based on:
 *
 * WiFly Autoconnect Example
 * Copyright (c) 2010 SparkFun Electronics.  All right reserved.
 * Written by Chris Taylor
 *
 * This code was written to demonstrate the WiFly Shield from SparkFun Electronics
 * 
 * This code will initialize and test the SC16IS750 UART-SPI bridge, and automatically
 * connect to a WiFi network using the parameters given in the global variables.
 *
 * http://www.sparkfun.com
 *
 * Further changes here made by Chris Theberge to demo AdHoc mode.
 */

#define ETH_CONF

#define LED_RED 10
#define LED_GREEN 11

#include <SPI.h>
#include "WiFly.h"
#include <PubSubClient.h>
#include <EEPROM.h>
#include "MqttFlare.h"
#include "UberdustSensors.h"

//Software Reset, Remote Reset
#include <avr/wdt.h>



WiFlyServer configServer(80);

MqttFlare * flare;


void callback(char* topic, uint8_t* payload, unsigned int length) {
  // handle message arrived
  /* topic = part of the variable header:has topic name of the topic where the publish received
   	 * NOTE: variable header does not contain the 2 bytes with the 
   	 *       publish msg ID
   	 * payload = pointer to the first item of the buffer array that
   	 *           contains the message tha was published
   	 *          EXAMPLE of payload: lights,1
   	 * length = the length of the payload, until which index of payload
   	 */
  digitalWrite(LED_GREEN,LOW);
  digitalWrite(LED_RED,HIGH);
  delay(10);
  digitalWrite(LED_RED,LOW);
  delay(10);


}

void setup() {
  pinMode(LED_RED , OUTPUT);
  digitalWrite(LED_RED,LOW);
  pinMode(LED_GREEN , OUTPUT);
  digitalWrite(LED_GREEN,LOW);  
  digitalWrite(LED_RED,HIGH);
  delay(1000);
  digitalWrite(LED_RED,LOW);
  digitalWrite(LED_GREEN,HIGH);  
  delay(1000);
  digitalWrite(LED_GREEN,LOW);  
  digitalWrite(LED_RED,HIGH);

  flare = new MqttFlare();
  flare->initFromProgmem();
  Serial.println("Starting Server");
  flare->connect(&configServer,callback);
}


void loop() {
  flare->loop();
}  





