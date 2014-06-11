/*
 Basic MQTT example 
 
 - connects to an MQTT server
 - publishes "hello world" to the topic "outTopic"
 - subscribes to the topic "inTopic"
 */
#define ETH_CONF
//#define USE_INDICATE

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <EEPROM.h>
//Software Reset, Remote Reset
#include <avr/wdt.h>
#include <avr/pgmspace.h>

#include <CoapSensor.h>
#include <UberdustSensors.h>
#include "MyRandomTemperatureSensor.h"

#include "MqttFlare.h"

#define LED_RED 7
#define LED_GREEN 6
#define LED LED_GREEN // any PWM led will do
unsigned long status_breathe_time = millis();
boolean breathe_up = true;
int breathe_i = 15;
int breathe_delay = 10;


MqttFlare * flare;

void callback(char* topic, byte* payload, unsigned int length) {
  size_t topiclen = strlen(topic);
  char newtopic[topiclen];
  boolean foundslash=false;
  int j=0;
  for (int i=0;i<topiclen;i++){
    if (foundslash){
      newtopic[j++]=topic[i];
    }
    if (topic[i]=='/'){
      foundslash=true;
    }
  }
  newtopic[j]='\0';
  // handle message arrived
  Serial.print(newtopic);
  Serial.print(",");
  payload[length]='\0';
  payload[length+1]='\0';
  Serial.println((char*)payload);

  flare->action(newtopic,(char *)payload,length);  


}





void setup()
{
  Serial.begin(9600);
  pinMode(LED_GREEN,OUTPUT);
  pinMode(LED_RED,OUTPUT);

  Serial.println("Started!");
  flare = new MqttFlare();

  uint8_t resp[10]; 
  char comm[5];
  sprintf(comm,"1");
  size_t temp_len;
  if (flare->connect(callback)){
    Serial.println("Connected!");
    char name[10];
    char name1[10];
    sprintf(name,"temp\0");
    flare->registerSensor(new MyRandomTemperatureSensor(name,25,10));
    sprintf(name1,"light\0");
    flare->registerSensor(new MyRandomTemperatureSensor(name1,25,10));
  }
  else{
    wdt_enable(WDTO_2S);
    while(1){
    }
  }

}

void loop(){
  if (flare->loop()){
    nonBlockingBreathe();
  }
  else{
    wdt_enable(WDTO_2S);
    while(1){
    }
  }
}

void nonBlockingBreathe(){

  if( (status_breathe_time + breathe_delay) < millis() ){
    analogWrite(LED, breathe_i);
    status_breathe_time = millis();
    if (breathe_up == true){
      if (breathe_i > 150) {
        breathe_delay = 4;
      }
      if ((breathe_i > 125) && (breathe_i < 151)) {
        breathe_delay = 5;
      }
      if (( breathe_i > 100) && (breathe_i < 126)) {
        breathe_delay = 7;
      }
      if (( breathe_i > 75) && (breathe_i < 101)) {
        breathe_delay = 10;
      }
      if (( breathe_i > 50) && (breathe_i < 76)) {
        breathe_delay = 14;
      }
      if (( breathe_i > 25) && (breathe_i < 51)) {
        breathe_delay = 18;
      }
      if (( breathe_i > 1) && (breathe_i < 26)) {
        breathe_delay = 19;
      }
      breathe_i += 1;
      if( breathe_i >= 255 ){
        breathe_up = false;
      }
    }
    else{
      if (breathe_i > 150) {
        breathe_delay = 4;
      }
      if ((breathe_i > 125) && (breathe_i < 151)) {
        breathe_delay = 5;
      }
      if (( breathe_i > 100) && (breathe_i < 126)) {
        breathe_delay = 7;
      }
      if (( breathe_i > 75) && (breathe_i < 101)) {
        breathe_delay = 10;
      }
      if (( breathe_i > 50) && (breathe_i < 76)) {
        breathe_delay = 14;
      }
      if (( breathe_i > 25) && (breathe_i < 51)) {
        breathe_delay = 18;
      }
      if (( breathe_i > 1) && (breathe_i < 26)) {
        breathe_delay = 19;
      }
      breathe_i -= 1;
      if( breathe_i <= 15 ){
        breathe_up = true;
        breathe_delay = 970;
      }
    }
  }
}









