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

#include <UberdustSensors.h>
#include "Relay.h"
#include "Sensor.h"
#include "Indicate.h"
#include "WattHourSensor.h"

#include "MqttFlare.h"



//int SenA,SenB, SenC, SenD, SenE, SenF, SenG;
int pinsSensor[7] = {
  A1,A2,A3,A4,A5,A6,A7};
int pins[7] = {
  2,4,6,8,15,17,19};
int pinsIndicate[7] = {
  3,5,7,14,16,18,20};
//Sensor S[7]={ 
//  Sensor("cur/1", A1), Sensor("cur/2", A2),Sensor("cur/3", A3),Sensor("cur/4", A4),Sensor("cur/5", A5),Sensor("cur/6", A6),Sensor("cur/7", A7)};
////WattHourSensor W[7]={  WattHourSensor("con/1",30, &(S[0])), WattHourSensor("con/2", 30, &(S[1])),WattHourSensor("con/3", 30, &(S[2])),WattHourSensor("con/4", 30, &(S[3])),WattHourSensor("con/5", 30,&( S[4])),WattHourSensor("con/6", 30,&( S[5])),WattHourSensor("con/7", 30, &(S[6]))};
//Relay R[7]={ 
//  Relay("r/1", 2), Relay("r/2",4),Relay("r/3",6),Relay("r/4",8),Relay("r/5",15),Relay("r/6",17),Relay("r/7",19)};
//#ifdef USE_INDICATE
//Indicate I[7]={
//  Indicate("i/1",3,&S[0]),Indicate("i/2",5,&S[1]),Indicate("i/3",7,&S[2]),Indicate("i/4",14,&S[3]),Indicate("i/5",16,&S[4]),Indicate("i/6",18,&S[5]),Indicate("i/7",20,&S[6])};
//#endif





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
  Serial.println("Started!");
  flare = new MqttFlare();

  uint8_t resp[10]; 
  char comm[5];
  sprintf(comm,"1");
  size_t temp_len;
  if (flare->connect(callback)){
    char name [6];
    for (int i=0 ; i<1;i++){

      sprintf(name,"cur/%d",i+1);
      Sensor * sens = new Sensor(name, pinsSensor[i]);
      flare->registerSensor(sens);

      sprintf(name,"r/%d",i+1);
      flare->registerSensor(new Relay(name,pins[i]) );

      //      sprintf(name,"i/%d",i+1);
      //      flare->registerSensor(new Indicate(name,pinsIndicate[i],sens) );
      //      sprintf(name,"con/%d",i+1);
      //      flare->registerSensor(new WattHourSensor(name,30 ,sens) );
      //R[i].set_value((uint8_t*)comm,1,resp,&temp_len);
      //        flare->registerSensor(&(W[i]));
#ifdef USE_INDICATE
      //      flare->registerSensor(&(I[i]));
#endif
    }
  }else{
    wdt_enable(WDTO_2S);
    while(1){}
  }

}

void loop(){
  flare->loop();
}



















