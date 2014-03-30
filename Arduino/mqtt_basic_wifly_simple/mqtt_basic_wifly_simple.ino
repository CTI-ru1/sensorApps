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

EnergyMonitor monitor1;
EnergyMonitor monitor2;
EnergyMonitor monitor3;


//Software Reset
#include <avr/wdt.h>

#define LED_RED 9
#define LED_GREEN 10
#define WIFLY_PIN 6

MqttFlare *flare ;

char textbuffer[30];
char sensname[30];

WiFlyClient wiFlyClient;

PubSubClient *client;


#define LED LED_GREEN // any PWM led will do
unsigned long status_breathe_time = millis();
boolean breathe_up = true;
int breathe_i = 15;
int breathe_delay = 10;

#include "utils.h"

boolean configMode =false;


//WiFlyServer server(80);
//boolean current_line_is_blank = false;

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
      //Serial.println(&topic[1]);
      //Serial.println(response);
      //delay(10);
    }
  }
}


void setup()
{

  monitor1.current(A1, 30);      // Current: input pin, calibration.
  monitor1.calcIrms(1480);  // Calculate Irms only
  monitor2.current(A2, 30);      // Current: input pin, calibration.
  monitor2.calcIrms(1480);  // Calculate Irms only
  monitor3.current(A3, 30);      // Current: input pin, calibration.
  monitor3.calcIrms(1480);  // Calculate Irms only

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

  //pinMode(A3,INPUT);  
  //EnergyMonitor  * monitor1 = new EnergyMonitor();
  //monitor1->current(A3, 30);      // Current: input pin, calibration.

  //    while(true){
  //
  //  //Serial.print("Irms:");  // Calculate Irms only
  //  //Serial.println(monitor1->calcIrms(1480));  // Calculate Irms only
  //      sensors_loop();
  //      delay(1000);
  //  }


  //all set -- change the leds!
  digitalWrite(LED_RED,LOW);
  digitalWrite(LED_GREEN,HIGH);
  breathe_up=true;


//  Serial.println("Starting Server");
//  server.begin();
}



void loop()
{

//    WiFlyClient configclient = server.available();
//    char header[10];
//    int index = 0;
//
//    if (configclient) {
//      Serial.println("client");
//      // an http request ends with a blank line
//
//      while (configclient.connected()) {
//        wdt_reset();
//        if (configclient.available()) {
//          delay(1);
//          char c = configclient.read();
//          header[index++] = c;
//
//          // 				if we've got to the end of the line (received a newline
//          // 				character) and the line is blank, the http request has ended,
//          // 				so we can send a reply
//          if (c == '\n' && current_line_is_blank) {
//            header[index] = 0;
//            if (strstr(header, "GET") != NULL) {
//              configclient.println("HTTP/1.1 200 OK");
//              configclient.println("Content-Type: text/html");
//              configclient.println();
//
//              configclient.println("<html></html>");
//              break;
//            }
//
//          }
//          if (c == '\n') {
//            // we're starting a new line
//            current_line_is_blank = true;
//          } 
//          else if (c != '\r') {
//            // we've gotten a character on the current line
//            current_line_is_blank = false;
//          }
//        }
//      }
//
//      configclient.stop();
//    }


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


















