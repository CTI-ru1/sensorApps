#include <SPI.h>
#include <WiFly.h>
#include <PubSubClient.h>
#include "Credentials.h"
#include <UberdustSensors.h>
#include "MqttFlare.h"

#include <EEPROM.h>
#include <EmonLib.h>
#include <CurrentSensor.h>
#include <WattHourSensor.h>



//Software Reset
#include <avr/wdt.h>

#define LED_RED 9
#define LED_GREEN 10
#define WIFLY_PIN 6

MqttFlare *flare ;

char textbuffer[30];

WiFlyClient wiFlyClient;

PubSubClient *client;


#define LED LED_GREEN // any PWM led will do
unsigned long status_breathe_time = millis();
boolean breathe_up = true;
int breathe_i = 15;
int breathe_delay = 10;

#include "utils.h"

void callback(char* topic, byte* payload, unsigned int length) {

  if (strncmp("heartbeat",topic,9)==0){
    wdt_reset();

  }  
  else{
    char *newtopic = strchr(topic,'/');
    newtopic++;
    if (strcmp("reset",newtopic)==0){
      wdt_enable(WDTO_2S);
      while(1){
      };
    }
    else{
      char response[10];
      size_t rlength;
      flare->action(newtopic,(char *)payload,length,response,&rlength);  
      client->publish(&topic[1],response);
      Serial.println(&topic[1]);
      Serial.println(response);
      delay(10);
    }
  }
}


void setup()
{
  //EEPROM.write(0,0);  EEPROM.write(1,0);  EEPROM.write(2,0);  EEPROM.write(3,0);
  wdt_disable();
  //create the sensoflare connector
  flare = new MqttFlare();
  //add the sensors to report
  add_sensors();


  //2colorled
  pinMode(LED_RED,OUTPUT);
  pinMode(LED_GREEN,OUTPUT);
  digitalWrite(LED_GREEN,LOW);
  digitalWrite(LED_RED,HIGH);

  //DEBUG SERIAL
  Serial.begin(38400);   // Start hardware Serial for the RN-XV

  establishConnection();

  Serial.print("Started in ");
  Serial.print(millis());
  Serial.println(" ms");

  //all set -- change the leds!
  digitalWrite(LED_RED,LOW);
  digitalWrite(LED_GREEN,HIGH);
  breathe_up=true;
}



void loop()
{

  nonBlockingBreathe();

  static unsigned long timestamp = 0;
  if(!client->loop()) {
    Serial.println("Client Disconnected.");
    breathe_up=false;
    
    digitalWrite(LED_RED, HIGH);
    delay(1000);
    digitalWrite(LED_RED, LOW);
    delay(1000);
    digitalWrite(LED_RED, HIGH);
    delay(1000);
    digitalWrite(LED_RED, LOW);
    delay(1000);
    digitalWrite(LED_RED, HIGH);

    establishConnection();

    //all set again -- change the leds!
    digitalWrite(LED_RED,LOW);
    digitalWrite(LED_GREEN,HIGH);
    breathe_up=true;


  }
  else{
    if(millis() - timestamp > 1000) {
      timestamp = millis();
      sensors_loop();
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

