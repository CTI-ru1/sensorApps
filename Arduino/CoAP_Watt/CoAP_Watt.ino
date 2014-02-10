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
#include "EmonLib.h"  
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

//parentSensor* parent;
//For make blink the led without delay(coiap can not acept a delay of 1 second)
int ledState = LOW;             
long previousMillis = millis();  
long interval = 1000;
/**
 */
void radio_callback(uint16_t sender, byte* payload, unsigned int length) {
  coap.receiver(payload, sender, length);
}

//Runs only once
void setup() {
  //Indicate
  pinMode(10,OUTPUT);//on red
  pinMode(11,OUTPUT);//on green
  //We put constantly red in the indicate until be ready the set up, to be sure that the hardware work
  digitalWrite(10,HIGH);
  digitalWrite(11,LOW);

  wdt_disable();

  //Connect to XBee
  //xbee.initialize_xbee_module();
  //start our XbeeRadio object and set our baudrate to 38400.
  //xbee.begin(38400);
  //Initialize our XBee module with the correct values (using the default channel, channel 12)h
  xbee.init(12,38400);

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
  coap.init(address, routing,"nis1\0");

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

  if(millis() - previousMillis > interval) {
    // save the last time you blinked the LED 
    previousMillis = millis();   
    if(coap.coap_has_observers()==0){


      // if the LED is off turn it on and vice-versa:
      if (ledState == LOW)
        ledState = HIGH;
      else
        ledState = LOW;

      // set the LED with the ledState of the variable:
      digitalWrite(10, ledState);
      digitalWrite(11,LOW);
    }

    else{
      digitalWrite(10,LOW);
      digitalWrite(11,HIGH);
    }
  }
}


void add_sensors() {
  EnergyMonitor  * monitor = new EnergyMonitor();
  monitor->current(A1, 30);      // Current: input pin, calibration.
  monitor->calcIrms(1480)*1000;  // Calculate Irms only

  CurrentSensor * current = new CurrentSensor("cur/1",monitor);
  coap.add_resource(current);

  WattHourSensor * cons = new WattHourSensor("con/1",30,current);
  coap.add_resource(cons);  
}



