//XBee Libraries
#include <XBee.h>
#include <XbeeRadio.h>
//Create the XbeeRadio object we'll be using
XBeeRadio xbee;
// create a reusable response object for responses we expect to handle
XBeeRadioResponse response;
//Reusable response
Rx16Response rx;

#include <TreeRouting.h>
TreeRouting * routing;

const int ledReceiveMessage = 8;
const int ledRoutingstatus = 7;
long previousMillis = 0; // will store last time a message was sent to gateway

/**
 */
void radio_callback(uint16_t topic, byte* payload, unsigned int length) {
    digitalWrite(ledReceiveMessage, HIGH);
    delay(10);
    digitalWrite(ledReceiveMessage, LOW);
}

void setup() {
    pinMode(ledReceiveMessage, OUTPUT);
    pinMode(ledRoutingstatus, OUTPUT);


    //Connect to XBee
    xbee.initialize_xbee_module();
    xbee.begin(38400);
    //Initialize our XBee module with the correct values using channel 12
    xbee.init(12);

    routing = new TreeRouting(&xbee);
    routing->set_sink(false);

    uint16_t address = xbee.getMyAddress(); //fix 4hex digit address
    uint8_t * bit = ((uint8_t*) & address);
    uint8_t mbyte = bit[1];
    uint8_t lbyte = bit[0];
    bit[0] = mbyte;
    bit[1] = lbyte;
    routing->set_my_address(address);

    routing->set_message_received_callback(radio_callback);

}

void loop() {

    routing->loop();

    switch (routing->state()) {
        case 0:
        case 1:
            digitalWrite(ledRoutingstatus, LOW);
            break;
        case 2:
            digitalWrite(ledRoutingstatus, HIGH);
            break;
    }

    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis > 5000) {
        previousMillis = currentMillis;
        uint8_t counter=0;
        routing->sendGateway((byte*) & counter, 1);
    }
}
