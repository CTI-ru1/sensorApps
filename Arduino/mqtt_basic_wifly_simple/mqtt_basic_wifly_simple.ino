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

//#define USE_DEBUG_WF
#ifdef USE_DEBUG_WF
#define DBGWFL(x)    Serial.println(x);
#define DBGWF(x)    Serial.print(x);
#else
#define DBGWFL(x)   
#define DBGWF(x)    
#endif


//#define SEND_IP

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

//#define WIFLY_SETTINGS_STORED 42
#define WIFLY_STATUS_POS 201
#define MAX_RETRIES 1


char textbuffer[80];
char sensname[30];


WiFly wifly;
MqttFlare *flare ;
PubSubClient *client;

boolean received=true;

boolean setup_mode=false;

//#define USE_BREATH
#ifdef USE_BREATH
#define LED LED_GREEN // any PWM led will do
//unsigned long status_breathe_time = millis();
//boolean breathe_up = true;
//int breathe_i = 15;
//int breathe_delay = 10;
#endif 

#include "utils.h"

boolean configMode =false;
boolean current_line_is_blank = false;

void callback(char* topic, byte* payload, unsigned int length) {
  wdt_reset();
  blinkFast(LED_GREEN);
  received=true;
  char *newtopic = strchr(topic,'/');
  newtopic++;
  if (strcmp("rs",newtopic)==0){
    wdt_enable(WDTO_2S);
    while(1){
    };
  }
  else{
    size_t rlength;
    char response[10];
    flare->action(newtopic,(char *)payload,length,response,&rlength);  
    client->publish(&topic[1],response);
  }
}


void setup()
{
  monitor1.current(A3, 30);      // Current: input pin, calibration.
  monitor1.calcIrms(1480);  // Calculate Irms only
  monitor2.current(A2, 30);      // Current: input pin, calibration.
  monitor2.calcIrms(1480);  // Calculate Irms only
  monitor3.current(A1, 30);      // Current: input pin, calibration.
  monitor3.calcIrms(1480);  // Calculate Irms only

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

#ifdef USE_DEBUG
  //DEBUG SERIAL
  Serial.begin(9600);   // Start hardware Serial for the RN-XV
#endif

#if defined(__AVR_ATmega32U4__)
#warning "pro-micro"
  Serial1.begin(9600);
  wifly.begin(&Serial1);
#else
#warning "uno"
  wifly.begin(&Serial);
#endif


  DBG("Arduino Reset");
  establishConnection();
  wdt_reset();

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
  //Serial.println("Ready!");
}



void loop()
{
  if (setup_mode){
    if (wifly.available() > 0) {
      //Serial.println("client");
      /* See if there is a request */
      if (wifly.gets(textbuffer, sizeof(textbuffer))) {
        if (strncmp_P(textbuffer, PSTR("GET"), 3) == 0) {
          //Serial.println("get");
          /* GET request */
          wifly.flushRx();		// discard rest of input
          sendIndex();
        } 
        else
          if (strncmp_P(textbuffer, PSTR("POS"), 3) == 0) {
            //Serial.println("post");
            /* Form POST */
            /* Get posted field value */
            if (wifly.match(F("s="))) {
              wifly.gets(textbuffer, sizeof(textbuffer));
            }
            wifly.flushRx();		// discard rest of input
            sendGreeting(textbuffer);
          }
        else{
          wifly.flushRx();		// discard rest of input
          send404();

        }
      }
    }
    wdt_reset();
  }
  else{
    //nonBlockingBreathe();
    wdt_reset();
    static unsigned long timestamp = 0;
    client->loop();
    sensors_loop();
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

void sendIndex()
{
  /* Send the header direclty with print */
  wifly.println(F("HTTP/1.1 200 OK"));//TODO:check this with F()
  wifly.println(F("Content-Type: text/html"));
  wifly.println(F("Transfer-Encoding: chunked"));
  wifly.println();
  //  wifly.sendChunkln(F("<form action='/save' method='post'><table>"));
  //  wifly.sendChunkln(F("<tr><td>SSID<td><input type='text' name='ssid'>"));
  //  wifly.sendChunkln(F("<tr><td>Phase<td><input type='text' name='passphrase'>"));
  //  wifly.sendChunkln(F("<tr><td><input type='submit' value='Save'>"));
  //  wifly.sendChunkln(F("</table></form>"));
  wifly.sendChunkln("<form method='post'>");
  wifly.sendChunkln("SSID<input type='text' name='s'>");
  wifly.sendChunkln("Key<input type='text' name='p'>");
  wifly.sendChunkln("<input type='submit' value='OK'>");
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

  //EEPROM.write(200,WIFLY_SETTINGS_STORED);
  EEPROM.write(300,strlen(ssid));
  for (int i=0;i<strlen(ssid);i++){
    EEPROM.write(301+i,ssid[i]);
  }
  EEPROM.write(400,strlen(phrase));
  for (int i=0;i<strlen(phrase);i++){
    EEPROM.write(401+i,phrase[i]);
  }

  wifly.sendChunk(F("Ok!"));
  wifly.sendChunkln();

  EEPROM.write(WIFLY_STATUS_POS,2);

  wdt_enable(WDTO_2S);
  while(1){
  };
}



void send404()
{
  wifly.println(F("HTTP/1.1 404 Not Found"));
  wifly.println();
}















