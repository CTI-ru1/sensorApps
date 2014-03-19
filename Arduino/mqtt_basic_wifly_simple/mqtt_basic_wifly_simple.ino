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


#include "utils.h"

void callback(char* topic, byte* payload, unsigned int length) {
  digitalWrite(LED_GREEN, LOW);
  delay(10);
  digitalWrite(LED_GREEN, HIGH);
  delay(10);

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

  //RESET WIFLY
  resetWiFly(WIFLY_PIN);

  //connect to the wifi network
  connect2WiFi();

  //get the wifly unique mac address
  flare->setMac(WiFly.getMAC());
  Serial.print("WiFi mac is:");
  Serial.println(WiFly.getMAC());
  //connect to the mqtt broker
  int retries = connect2MQTT();

  //publish and subscribe
  client->publish("connect",flare->connect(1));
  delay(10);
  client->publish("retries",flare->retries(retries));
  delay(10);
  client->subscribe("heartbeat");
  delay(10);
  client->subscribe(flare->channel());

  Serial.print("Started in ");
  Serial.print(millis());
  Serial.println(" ms");
  
  //all set -- change the leds!
  digitalWrite(LED_RED,LOW);
  digitalWrite(LED_GREEN,HIGH);

}



void loop()
{

  static unsigned long timestamp = 0;
  if(!client->loop()) {
    Serial.println("Client Disconnected.");
    /*
    digitalWrite(LED_RED, HIGH);
     delay(1000);
     digitalWrite(LED_RED, LOW);
     delay(1000);
     digitalWrite(LED_RED, HIGH);
     delay(1000);
     digitalWrite(LED_RED, LOW);
     delay(1000);
     int retries = connect2MQTT();
     Serial.println("With retries");
     //all set again -- change the leds!
     digitalWrite(LED_RED,LOW);
     digitalWrite(LED_GREEN,HIGH);
     */

  }
  else{
    if(millis() - timestamp > 1000) {
      timestamp = millis();
      sensors_loop();
    }
  }
}





