#ifndef TREE_ROUTING
#define TREE_ROUTING

#include <XBee.h>
#include <XbeeRadio.h>
#include <BaseRouting.h>
#include "tree_routing_message.h"
#include "tree_broadcast_message.h"
#include <avr/wdt.h>

#define MAX_CHILDREN 20

class TreeRouting : public BaseRouting {
public:

    TreeRouting(XBeeRadio * xbee):
    state_(TrUnconnected),
    lastBeacon(0),
    parent_(0),
    hops_(0xff),
    parent_lqi_(0xff) {
        this->xbee = xbee;
        for (int i = 0; i < MAX_CHILDREN; i++) {
            children[i] = 0xffff;
        }
    }


    uint8_t hops() {
        return hops_;
    };

    uint16_t parent() {
        return parent_;
    };

    /** 
     * \brief Message IDs
     */
    enum TreeRoutingMsgIds {
        TrMsgIdBroadcast = 100, ///< Msg type for broadcasting tree state
        TrMsgIdRouting = 101, ///< Msg type for routing messages
        Tr2MsgIdRouting = 103 ///< Msg type for routing messages
    };



    void loop();

    void set_sink(bool isSink);

    int send(uint16_t receiver, byte *data, uint8_t len);

    void timer_elapsed();

    uint8_t state();

    void set_my_address(uint16_t me);
    void receive(uint16_t from, byte * payload, unsigned int len, uint8_t metric);


    void set_message_received_callback(void (*received_callback)(uint16_t, byte*, unsigned int));

    void add_child(uint16_t child);
    bool is_known(uint16_t from);

    void setXbeeRadio(XBeeRadio * xbee) {
        this->xbee = xbee;
    }

private:
    void (*received_callback_)(uint16_t, byte*, unsigned int);
    void radio_send(uint16_t, byte*, unsigned int);
    uint16_t myAddress;
    XBeeRadio *xbee;
    TreeRoutingState state_;
    uint16_t parent_;
    uint8_t hops_;
    //Reusable response
    Rx16Response rx;
    long lastBeacon;
    uint16_t children[MAX_CHILDREN];
    TreeBroadcastMessage beacon;
    uint8_t parent_lqi_;
};

#endif
