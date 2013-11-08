#include "TreeRouting.h"

void TreeRouting::loop() {
    if (xbee->checkForData(112)) {
        //Create a rx16 response object to get the address
        //wdt_reset();
        Rx16Response rx = Rx16Response();
        xbee->getResponse().getRx16Response(rx);
        uint8_t rssi = rx.getRssi();
        uint16_t from = rx.getRemoteAddress16();
        uint8_t len = xbee->getResponse().getDataLength();
        byte payload[len];
        memcpy(payload, xbee->getResponse().getData(), len);
        receive(from, payload, len, rssi);
    }

    if (state_ == TrGateway) {

        if (millis() - lastBeacon > 3000) {
            timer_elapsed();
            lastBeacon = millis();
        }
    } else {
        if (millis() - lastBeacon > 10000) {
            state_ = TrUnconnected;
            parent_ = 0;
            hops_ = 0xff;
            for (int i = 0; i < MAX_CHILDREN; i++) {
                children[i] = 0xffff;
            }
        }
    }

}

void TreeRouting::add_child(uint16_t child) {
    if (!is_known(child)){
      for (int i = 0; i < MAX_CHILDREN; i++) {
	  if (children[i] == 0xffff) {
	      children[i] = child;
	      return;
	  }
      }
    }
}

bool TreeRouting::is_known(uint16_t from) {
    for (int i = 0; i < MAX_CHILDREN; i++) {
        if (children[i] == from) {
            return true;
        }
    }
    return false;
}

void TreeRouting::receive(uint16_t from, byte * payload, unsigned int len, uint8_t metric) {
    //if (from == 0x58e)return;
    uint8_t msg_id = payload[0];
    if (msg_id == TrMsgIdBroadcast) {
        TreeBroadcastMessage *message = reinterpret_cast<TreeBroadcastMessage*> (payload);
        if (!is_known(from)) {

            if (state_ != TrGateway) {
                //set new or better gateway
                if ((hops_ == 0xff) || (parent_lqi_ > metric)) {
                    hops_ = message->hops() + 1;
                    parent_ = from;
                    state_ = TrConnected;
                    lastBeacon = millis();
                    parent_lqi_ = metric;
                    message->set_hops(hops_);
                    radio_send(0xffff, (byte*) message, message->buffer_size());
                }
                //update the gateway's life indicator
                if ((state_ == TrConnected) && (parent_ == from)) {
                    lastBeacon = millis();
                    message->set_hops(hops_);
                    radio_send(0xffff, (byte*) (message), message->buffer_size());
                }
            }
        }
    } else if (msg_id == TrMsgIdRouting) {
        TreeRoutingMessage *message = reinterpret_cast<TreeRoutingMessage*> (payload);
        if (state_ == TrGateway) {
            //message reached its destination
            received_callback_(message->source(), message->payload(), message->payload_size());
        } else if (parent_ != 0) {
            add_child(message->source());
            radio_send(parent_, payload, len);
        }
    } else if (msg_id == Tr2MsgIdRouting) {
        TreeRoutingMessage *message = reinterpret_cast<TreeRoutingMessage*> (payload);
        if (message->source() == this->myAddress) {
            //message reached its destination
            received_callback_(message->source(), message->payload(), message->payload_size());
        } else if (state_ != TrGateway) {
            if (from == parent_) {
                if (is_known(message->source())) {

                    radio_send(0xffff, payload, len);
                }
            }
        }
    }
}

void TreeRouting::set_sink(bool isSink) {
    if (isSink) {
        hops_ = 0;
        state_ = TrGateway;
    } else {
        hops_ = 0xff;
        state_ = TrUnconnected;
    }
}

int TreeRouting::send(uint16_t receiver, byte *data, uint8_t len) {
    if (state_ == TrGateway) {
        TreeRoutingMessage message(Tr2MsgIdRouting, this->myAddress);
        message.set_payload(len, data);
        message.set_source(receiver);
        radio_send(0xffff, (byte*) (&message), message.buffer_size());
        return SUCCESS;
    } else {
        TreeRoutingMessage message(TrMsgIdRouting, this->myAddress);
        message.set_payload(len, data);
        radio_send(parent_, (byte*) (&message), message.buffer_size());
    }
}

void TreeRouting::timer_elapsed() {
    switch (state_) {
        case TrGateway:
        {
            beacon.set_msg_id(TrMsgIdBroadcast);
            beacon.set_hops(hops_);
            radio_send(0xffff, (byte*) (&beacon), beacon.buffer_size());

            break;
        }
        case TrConnected:
        case TrUnconnected:
            break;
        default:
            break;
    }
}

uint8_t TreeRouting::state() {
    return state_;
}

void TreeRouting::set_my_address(uint16_t me) {
    this->myAddress = me;
}

void TreeRouting::set_message_received_callback(void (*received_callback)(uint16_t, byte*, unsigned int)) {
    this->received_callback_ = received_callback;
}

void TreeRouting::radio_send(uint16_t to, byte* payload, unsigned int size) {
    Tx16Request tx = Tx16Request(to, payload, size);
    xbee->send(tx, 112);
    delay(10);
}
