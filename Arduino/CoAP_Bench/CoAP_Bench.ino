/**
 * Arduino Coap Benchmark Application.
 *
 * This Example creates a CoAP server with 7 Simulated GET Resources.
 *
 * All resources are of TEXT_PLAIN content type.
 * size: 22926 Bytes
 */

//Comment out to use no arduino-routing
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

/**
 */
void radio_callback(uint16_t sender, byte* payload, unsigned int length) {
  coap.receiver(payload, sender, length);
}

//Runs only once
void setup() {
  wdt_disable();
  pinMode(9,OUTPUT);
  digitalWrite(9,HIGH);
  blinkTime=millis();

  //Connect to XBee
  //xbee.initialize_xbee_module();
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
  digitalWrite(9,LOW);

  uint16_t address = xbee.getMyAddress(); //fix 4hex digit address
  uint8_t * bit = ((uint8_t*) & address);
  uint8_t mbyte = bit[1];
  uint8_t lbyte = bit[0];
  bit[0] = mbyte;
  bit[1] = lbyte;
  routing->set_my_address(address);
  routing->set_message_received_callback(radio_callback);
  // init coap service 
  coap.init(address, routing);

  add_sensors();

  wdt_disable();
  wdt_enable(WDTO_8S);
}

void loop() {
  //run the handler on each loop to respond to incoming requests
  coap.handler();

  routing->loop();
  wdt_reset();
}

void add_sensors() {

  parentSensor* parent = new parentSensor("r",routing);
  coap.add_resource(parent);
  RandomSensor* rand1 = new RandomSensor("temp", 40);
  coap.add_resource(rand1);
  RandomSensor* rand2 = new RandomSensor("light", 400);
  coap.add_resource(rand2);
  RandomSensor* rand3 = new RandomSensor("hum", 100);
  coap.add_resource(rand3);
  RandomSensor* rand4 = new RandomSensor("sound", 2000);
  coap.add_resource(rand4);
  RandomSensor* rand5 = new RandomSensor("co", 100);
  coap.add_resource(rand5);
  RandomSensor* rand6 = new RandomSensor("ch4", 100);
  coap.add_resource(rand6);
  RandomSensor* rand7 = new RandomSensor("pir", 1);
  coap.add_resource(rand7);
 
}


