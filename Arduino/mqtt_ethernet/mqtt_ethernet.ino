/*
 Basic MQTT example 
 
 - connects to an MQTT server
 - publishes "hello world" to the topic "outTopic"
 - subscribes to the topic "inTopic"
 */

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

#include "MqttFlare.h"

// Update these with values suitable for your network.
byte mac[]    = {  
  0xDE, 0xED, 0xBA, 0xFE, 0xbb, 0xaa };
byte server[] = { 
  192,168,1,5 };
byte ip[] = { 
  192,168,1,6 };

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  Serial.print(topic);
  Serial.print("/");
  Serial.println((char*)payload);
}




MqttFlare * flare;

void setup()
{
  Serial.begin(9600);
  Serial.println("Started!");
  if (Ethernet.begin(mac)==0){
    Serial.println("Dhcp failed!");
  }
  else{  
    Serial.print("Ethernet IP : ");
    Serial.println(Ethernet.localIP());
    Serial.println("Connected to Ethernet");  


    flare = new MqttFlare();
    if (flare->connect(server,1883,callback)){
    }
  }

}

void loop(){


  flare->loop();


}












