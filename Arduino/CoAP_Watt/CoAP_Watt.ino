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


#include "EmonLib.h"  
//Create the XbeeRadio object we'll be using
XBeeRadio xbee = XBeeRadio();


//CoAP object
Coap coap;

//Message Routing
BaseRouting * routing;

//parentSensor* parent;

/**
 */
void radio_callback(uint16_t sender, byte* payload, unsigned int length) {
  coap.receiver(payload, sender, length);
}

//Runs only once
void setup() {

  wdt_disable();

  //Connect to XBee
  //xbee.initialize_xbee_module();
  //start our XbeeRadio object and set our baudrate to 38400.
  xbee.begin(38400);
  //Initialize our XBee module with the correct values (using the default channel, channel 12)h
  xbee.init(13);

#ifdef USE_TREE_ROUTING
  routing = new TreeRouting(&xbee);
#else 
  routing = new NonRouting(&xbee);
#endif 
  routing->set_sink(false);

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
  coap.init(address, routing);

  add_sensors();

  wdt_disable();
  wdt_enable(WDTO_8S);
}

void loop() {
  static unsigned long observersTimestamp = 0;
  //run the handler on each loop to respond to incoming requests
  coap.handler();
  routing->loop();
  wdt_reset();

  if (millis()-observersTimestamp>5000&&coap.coap_has_observers()){
    observersTimestamp=millis();
  }
}


void add_sensors() {
  EnergyMonitor  * monitor = new EnergyMonitor();
  monitor->current(A1, 30);      // Current: input pin, calibration.
  //NonInvasiveSensor* NonInvasive = new NonInvasiveSensor("1S",A1);
  //coap.add_resource(NonInvasive);  
  CurrentSensor * current = new CurrentSensor("curr:1",monitor);
  coap.add_resource(current);  
  WattHourSensor * cons = new WattHourSensor("cons:1",30,current);
  coap.add_resource(cons);  
  parentSensor * par = new parentSensor("r",routing);
  coap.add_resource(par);  
}






