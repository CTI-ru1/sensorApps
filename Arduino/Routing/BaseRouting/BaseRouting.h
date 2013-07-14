/* 
 * File:   BaseRouting.h
 * Author: amaxilatis
 *
 * Created on June 26, 2013, 3:42 PM
 */

#ifndef BASEROUTING_H
#define	BASEROUTING_H

class BaseRouting {
public:

    BaseRouting() {
    }

    enum StateValues {
        READY,
        NO_VALUE,
        INACTIVE
    };
    // --------------------------------------------------------------------
#define NULL_NODE_ID      0      ///< Unknown/No node id

    // --------------------------------------------------------------------

    enum Restrictions {
        MAX_MESSAGE_LENGTH = 100 ///< Maximal number of bytes in payload
    };


    enum TreeRoutingState {
        TrGateway,
        TrConnected,
        TrUnconnected
    };

    virtual void set_sink(bool);
    virtual void set_my_address(uint16_t);

    virtual void set_message_received_callback(void (*received_callback)(uint16_t, byte*, unsigned int));
    virtual int send(uint16_t receiver, byte *data, uint8_t len);
    virtual void loop();
    virtual uint8_t state();
    virtual uint16_t parent();
};

#endif	/* BASEROUTING_H */

