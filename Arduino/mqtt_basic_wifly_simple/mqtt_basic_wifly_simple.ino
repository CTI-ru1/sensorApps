#include <SPI.h>
#include <Server.h>
#include <PubSubClient.h>
#include "Credentials.h"
#include <UberdustSensors.h>
#include "MqttFlare.h"



#include <WiFlyHQ.h>

//#define USE_DEBUG
#ifdef USE_DEBUG
#define DBGL(x)    Serial.println(x);
#define DBG(x)    Serial.print(x);
#else
#define DBGL(x)   
#define DBG(x)    
#endif

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

#define WIFLY_SETTINGS_STORED 42

MqttFlare *flare ;

char textbuffer[30];
char sensname[30];

char buf[80];

WiFly wifly;
PubSubClient *client;

boolean setup_mode=false;

//#define USE_BREATH
#ifdef USE_BREATH
#define LED LED_GREEN // any PWM led will do
unsigned long status_breathe_time = millis();
boolean breathe_up = true;
int breathe_i = 15;
int breathe_delay = 10;
#endif 

#include "utils.h"

boolean configMode =false;


boolean current_line_is_blank = false;

void callback(char* topic, byte* payload, unsigned int length) {

  //if (strncmp("heartbeat",topic,9)==0){
  //  wdt_reset();
  //}  
  //else{
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
  //}
}


void setup()
{

  monitor1.current(A3, 30);      // Current: input pin, calibration.
  monitor1.calcIrms(1480);  // Calculate Irms only
  monitor2.current(A2, 30);      // Current: input pin, calibration.
  monitor2.calcIrms(1480);  // Calculate Irms only
  monitor3.current(A1, 30);      // Current: input pin, calibration.
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

  resetWiFly(WIFLY_PIN);

  //DEBUG SERIAL
  Serial.begin(9600);   // Start hardware Serial for the RN-XV

#if defined(__AVR_ATmega32U4__)
#warning "is pro micro"
  Serial1.begin(9600);
  wifly.begin(&Serial1);
#else
#warning "is arduino uno"
  wifly.begin(&Serial);
#endif


  DBG("Arduino Reset");

  establishConnection();

  DBG("Started in ");
  DBG(millis());
  DBGL(" ms");

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
#ifdef USE_BREATH
  breathe_up=true;
#endif

  Serial.println("Ready!");

  //server = new WiFlyServer(80);
  //server->begin();
}



void loop()
{
  if (setup_mode){
    if (wifly.available() > 0) {

      /* See if there is a request */
      if (wifly.gets(buf, sizeof(buf))) {
        if (strncmp_P(buf, PSTR("GET / "), 6) == 0) {
          /* GET request */
          while (wifly.gets(buf, sizeof(buf)) > 0) {
            /* Skip rest of request */
          }
          sendIndex();
        } 
        else
          if (strncmp_P(buf, PSTR("POST /save"), 10) == 0) {
            /* Form POST */
            char args[70];

            /* Get posted field value */
            if (wifly.match(F("ssid="))) {
              wifly.gets(args, sizeof(args));
            }
            wifly.flushRx();		// discard rest of input
            sendGreeting(args);
          }
      }
    }
    //wdt_reset();
  }
  else{
    //nonBlockingBreathe();

    static unsigned long timestamp = 0;
    if(!client->loop()) {
      DBGL("Client Disconnected.");
#ifdef USE_BREATH
      breathe_up=false;
#endif
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
#ifdef USE_BREATH
      breathe_up=true;
#endif

    }
    else{
      if(millis() - timestamp > 1000) {
        timestamp = millis();
        sensors_loop();
      }
    }
  }
}

#ifdef USE_BREATH
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
#endif











//
//
//
void sendIndex()
{
  /* Send the header direclty with print */
  wifly.println(F("HTTP/1.1 200 OK"));
  wifly.println(F("Content-Type: text/html"));
  wifly.println(F("Transfer-Encoding: chunked"));
  wifly.println();
  wifly.sendChunkln(F("<form action='/save' method='post'><table>"));
  wifly.sendChunkln(F("<tr><td>SSID<td><input type='text' name='ssid'>"));
  wifly.sendChunkln(F("<tr><td>Phase<td><input type='text' name='passphrase'>"));
  wifly.sendChunkln(F("<tr><td><input type='submit' value='Save'>"));
  wifly.sendChunkln(F("</table></form>"));

  //wifly.sendChunkln("<form action='/save' method='post'><table><tr><td>SSID<td><input type='text' name='ssid'><tr><td>Phase<td><input type='text' name='passphrase'><tr><td><input type='submit' value='Save'></table></form>");

  //wifly.sendChunkln("<form action='/save' method='post'>SSID<input type='text' name='ssid'>Phase<input type='text' name='passphrase'><input type='submit' value='Save'>");
  wifly.sendChunkln();
}


/** Send a greeting HTML page with the user's name and an analog reading */
void sendGreeting(char *args)
{
  /* Send the header directly with print */
  wifly.println(F("HTTP/1.1 200 OK"));
  wifly.println(F("Content-Type: text/html"));
  wifly.println(F("Transfer-Encoding: chunked"));
  wifly.println();

  char *ssid = strtok(args,"&");
  char *rest = strtok(NULL,"&");
  strtok(rest,"=");
  char *phrase = strtok(NULL,"=");

  EEPROM.write(200,WIFLY_SETTINGS_STORED);
  EEPROM.write(300,strlen(ssid));
  for (int i=0;i<strlen(ssid);i++){
    EEPROM.write(301+i,ssid[i]);
  }
  EEPROM.write(400,strlen(phrase));
  for (int i=0;i<strlen(phrase);i++){
    EEPROM.write(401+i,phrase[i]);
  }

  wifly.sendChunk(F("Stored!"));
  wifly.sendChunkln();

  wdt_enable(WDTO_2S);
  while(1){
  };
}










