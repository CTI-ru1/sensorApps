/**
 * Arduino Coap Example Application.
 *
 * This Example creates a CoAP server with 2 resources.
 * resGET : A resource that contains an integer and the GET method is only available.
 *          A GET request returns the value of the value_get variable.
 * resGET-POST : A resource that contains an integer and the GET-POST methods are available.
 *               A GET request returns the value of the value_post variable.
 *               A POST request sets the value of value_post to the sent integer.
 * Both resources are of TEXT_PLAIN content type.
 * size: 25036Byte
 */

#define USE_TREE_ROUTING
#define REMOTE_RESET


//Include XBEE Libraries
#include <XBee.h>
#include <XbeeRadio.h>
#include <BaseRouting.h>
#include <TreeRouting.h>
#include <NonRouting.h>
//Software Reset
#include <avr/wdt.h>
//Include CoAP Libraries
#include <coap.h>
#include <UberdustSensors.h>

//Create the XbeeRadio object we'll be using
XBeeRadio xbee = XBeeRadio();


//CoAP object
Coap coap;

//Message Routing
BaseRouting * routing;
long blinkTime;
//parentSensor* parent;

/**
 */
void radio_callback(uint16_t sender, byte* payload, unsigned int length) {
  coap.receiver(payload, sender, length);
}

//Runs only once
void setup() {
  wdt_disable();
  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);
  blinkTime=millis();

  //Connect to XBee
  xbee.initialize_xbee_module();
  //start our XbeeRadio object and set our baudrate to 38400.
  xbee.begin(38400);
  //Initialize our XBee module with the correct values (using the default channel, channel 12)h
  xbee.init(12);

#ifdef USE_TREE_ROUTING
  routing = new TreeRouting(&xbee);
#else 
  routing = new NonRouting(&xbee);
#endif 
  routing->set_sink(false);
  digitalWrite(13,LOW);

  uint16_t address = xbee.getMyAddress(); //fix 4hex digit address
  uint8_t * bit = ((uint8_t*) & address);
  uint8_t mbyte = bit[1];
  uint8_t lbyte = bit[0];
  bit[0] = mbyte;
  bit[1] = lbyte;
  routing->set_my_address(address);
  routing->set_message_received_callback(radio_callback);
  //routing->setXbeeRadio(&xbee);
  // init coap service 
  coap.init(address, routing,"ctiac2");

  add_relays();

  wdt_disable();
  wdt_enable(WDTO_8S);
}

void loop() {
  //run the handler on each loop to respond to incoming requests
  coap.handler();

  routing->loop();
  wdt_reset();
}

void add_relays() {
    char name1 [4];
    sprintf(name1,"ac/1");
    zoneSensor * acZone1  = new zoneSensor(name1, 2);
    coap.add_resource(acZone1);
    char name2 [4];
    sprintf(name2,"ac/2");
    zoneSensor * acZone2  = new zoneSensor(name2, 3);
    coap.add_resource(acZone2);
}
