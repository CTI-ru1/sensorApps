/* 
 * File:   NonRouting.h
 * Author: amaxilatis
 *
 * Created on June 26, 2013, 4:02 PM
 */

#ifndef NONROUTING_H
#define	NONROUTING_H

#include <XBee.h>
#include <XbeeRadio.h>
#include <BaseRouting.h>

class NonRouting : public BaseRouting {
public:

    NonRouting(XBeeRadio * xbee)
    :
    state_(TrConnected) {
        this->xbee = xbee;
    }
    
    void loop();

    void set_sink(bool isSink);

    int send(uint16_t receiver, byte *data, uint8_t len);

    uint8_t state();

    void set_my_address(uint16_t me);
    
    void receive(uint16_t from, byte * payload, unsigned int len, uint8_t metric);

    void set_message_received_callback(void (*received_callback)(uint16_t, byte*, unsigned int));

private:
    void (*received_callback_)(uint16_t, byte*, unsigned int);
    void radio_send(uint16_t, byte*, unsigned int);
    XBeeRadio *xbee;
    TreeRoutingState state_;
    //Reusable response
    Rx16Response rx;
    uint16_t myAddress;
};

#endif	/* NONROUTING_H */

