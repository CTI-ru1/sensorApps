/**
 * UberdustBridge
 * - connects to an MQTT server
 * - publishes coap messages to the topics "connect/#", "xbeeaddress/#" and "stats/#"
 * - subscribes to the topic "heartbeat/#" to receive keep-alive messages from the server
 * @author Dimitrios Amaxilatis
 */

//Operational Parameters
#define USE_TREE_ROUTING
#define CHANNEL 13

#include <EEPROM.h>

//Leds
#include "LedUtils.h"

//Software Reset, Remote Reset
#include <avr/wdt.h>

//XBee Libraries
#include <XBee.h>
#include <XbeeRadio.h>
//Create the XbeeRadio object we'll be re-using
XBeeRadio xbee;
// create a reusable response object for responses we expect to handle
XBeeRadioResponse response;
//Reusable response
Rx16Response rx;

//Message Routing
#include <BaseRouting.h>
#include <TreeRouting.h>
#include <NonRouting.h>
BaseRouting * radio;

//Ethernet
#include <SPI.h>
#include <Ethernet.h>
EthernetClient ethernetClient ;

//MQTT Library
#include <PubSubClient.h>

//Helper Class
#include "UberdustGateway.h"
UberdustGateway gateway(&ethernetClient);

//Update these with values suitable for your network/broker.
byte mac[] = {  
  0xAE, 0xED, 0xBA, 0xFc, 0xaa, 0xaa};

byte uberdustServer[] ={  
  150, 140, 5, 20};

// Address of the XBee
char address[5];
// Indicator of XBee communication
bool receivedAny;
// Indicator of MQTT communication
int lastReceivedStatus;
// Timestamp of the last MQTT message received
unsigned long lastReceived;

/**
 * Callaback to the MQTT connection.
 * @param topic the topic the message was published on
 * @param payload the message published
 * @param length the size of the message
 */
void callback(char* topic, byte* payload, unsigned int length)
{
  gateway.incy();
  check_heartbeat(topic,payload,length);
  check_xbee(topic,payload,length);
}

/**
 * Check incoming MQTT message for the heartbeat and reset message.
 * @param topic the topic the message was published on
 * @param payload the message published
 * @param length the size of the message
 */
void check_heartbeat(char* topic, byte* payload, unsigned int length)
{
  if (strcmp(topic, "heartbeat") == 0){
    if (strncmp((char *)payload, "reset",5)==0){
      wdt_reset();
      blinkFast(6);
    }
  }
  else{
    check_reset(topic,payload,length);
  }
}

/**
 * Check incoming MQTT message for the reset message.
 * @param topic the topic the message was published on
 * @param payload the message published
 * @param length the size of the message
 */
void check_reset(char* topic, byte* payload, unsigned int length)
{
  if (strncmp((char*)payload,"reset",5)==0){
    watchdogReset();
  }
}

/**
 * Check incoming MQTT message for xbee messages.
 * @param topic the topic the message was published on
 * @param payload the message published
 * @param length the size of the message
 */
void check_xbee(char* topic, byte* payload, unsigned int length)
{
  blinkFast(2);
  radio->send( *((uint16_t*)payload) , &(payload[2]),length-2);
}

/**
 * Callaback to the XBee radio.
 * @param sender origin device of the message
 * @param payload the bytes of the message received
 * @param length the size of the payload
 */
void radio_callback(uint16_t sender, byte* payload, unsigned int length) {
  gateway.incx();
  receivedAny = true;
  //add_device(sender);
  sprintf(address, "%x", sender);
  gateway.publish(sender, payload, length);
}

/**
 * Initializes UberdustBridge.
 * -Connect to XBee
 * -Generate Unique mac based on xbee address
 * -Connect to Network
 * -Connect to MQTT broker
 * -Initialize variables
 */
void setup()
{
  pinMode(6, OUTPUT);
  pinMode(2, OUTPUT);
  bootblink();
  ledState(STATE_BOOT);


  //wdt_enable(WDTO_8S);
  //wdt_reset();
  //Ethernet.begin(mac,ip);
  //Connect to Network
  if (Ethernet.begin(mac)==0){  
    //if ( 1==0 ){      
    //Software Reset
    ledState(STATE_ERROR);
    watchdogReset();
  }
  else
  {
  wdt_enable(WDTO_8S);
  wdt_reset();
    //Connect to MQTT broker
    ledState(STATE_ETH);
    gateway.setUberdustServer(uberdustServer);

    //create the unique id based on the xbee mac address
    char testbedHash[17];

    if ( EEPROM.read(0) == 67 ){
      for (int i = 0; i < 17; i++){
        testbedHash[i]= EEPROM.read(1+i);
      }
      gateway.setTestbedID(testbedHash);
      gateway.connect(callback);
      gateway.publish("connect","xbee-connected");
      connectXbee();

    }
    else{

      connectXbee();

      uint32_t addr64h = xbee.getMyAddress64High();
      uint32_t addr64l = xbee.getMyAddress64Low();

      uint16_t * addr64hp =  ( uint16_t * ) &addr64h;
      uint16_t * addr64lp =  ( uint16_t * ) &addr64l;

      if (addr64hp[0]!=0x0013){
        watchdogReset();
      }
      if (addr64hp[0]==addr64lp[0]){
        watchdogReset();
      }

      sprintf(testbedHash, "%4x%4x%4x%4x",addr64hp[0],addr64hp[1],addr64lp[0],addr64lp[1]);

      for (int i=0;i<64;i++){
        if (testbedHash[i]==' '){
          testbedHash[i]='0';
        }
      }

      EEPROM.write(0, 67);
      for (int i = 0; i < 17; i++){
        EEPROM.write(1+i, testbedHash[i]);
      }  

      gateway.setTestbedID(testbedHash);
      gateway.connect(callback);
      gateway.publish("connect","xbee-connected2");
    }



  }

  wdt_reset();
  wdt_disable();

  //Initialize variables
  lastReceivedStatus = false;
  lastReceived = millis();
  receivedAny = false;
  wdt_enable(WDTO_8S);
}

/**
 * Runs continously
 * -Check server connection
 * -Check MQTT messages
 * -Check XBee messages
 */
void loop()
{
  //Check MQTT messages
  gateway.loop();
  radio->loop();

  //Blink on network traffic
  if (millis() - lastReceived > 5000)
  {
    if (receivedAny)
    {
      lastReceivedStatus = !lastReceivedStatus;
      receivedAny = false;
    }
    lastReceived = millis();
  }
}

/**
 * Sofrware Reset using watchdogTimer
 */
void watchdogReset()
{
  wdt_disable();
  wdt_enable(WDTO_30MS);
  while(1);
}

/**
 * Software 2color led implementation
 * @param theStatus the status to report
 */
void ledState(int theStatus)
{
  if (theStatus == 2)
  {
    digitalWrite(6, HIGH);
    digitalWrite(2, HIGH);
  }
  else if (theStatus == 1)
  {
    digitalWrite(6, HIGH);
    digitalWrite(2, lastReceivedStatus ? HIGH : LOW);
  }
  else if (theStatus == 0)
  {
    digitalWrite(6, LOW);
    digitalWrite(2, lastReceivedStatus ? HIGH : LOW);
  }
}


void connectXbee(){

  //Auto-reset if something goes bad
  wdt_reset();
  wdt_disable();
  wdt_enable(WDTO_8S);

  //Connect to XBee
  wdt_reset();
  wdt_disable();
  //Initialize our XBee module with the correct values using CHANNEL
  xbee.initialize_xbee_module();
  int value = xbee.init(CHANNEL,38400);
  wdt_reset();
  char mess[20];
  sprintf(mess, "xbee-init-%d",value);
  gateway.publish("connect",mess);
  wdt_reset();
  ledState(STATE_XBEE);

  //Setup radio using the xbee
  lastReceivedStatus = false;
#ifdef USE_TREE_ROUTING
  radio = new TreeRouting(&xbee);
#else 
  radio = new NonRouting(&xbee);
#endif 
  radio->set_sink(true);
  wdt_reset();

  uint16_t address = xbee.getMyAddress(); //fix 4hex digit address
  uint8_t * bit = ((uint8_t*) & address);
  uint8_t mbyte = bit[1];
  uint8_t lbyte = bit[0];
  bit[0] = mbyte;
  bit[1] = lbyte;
  radio->set_my_address(address);
  radio->set_message_received_callback(radio_callback);
  wdt_reset();


  //Generate Unique mac based on xbee address
  uint16_t my_address = address;
  mac[4] = (&my_address)[1];
  mac[5] = (&my_address)[0];
}



