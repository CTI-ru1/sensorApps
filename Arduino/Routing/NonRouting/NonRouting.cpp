#include "NonRouting.h"

void NonRouting::loop() {
    if (xbee->checkForData(112)) {
        //Create a reusable rx16 response object to get the address
        Rx16Response rx = Rx16Response();
        xbee->getResponse().getRx16Response(rx);
        uint8_t rssi = rx.getRssi();
        uint16_t from = rx.getRemoteAddress16();
        uint8_t len = xbee->getResponse().getDataLength();
        byte payload[len];
        memcpy(payload, xbee->getResponse().getData(), len);
        receive(from, payload, len, rssi);
    }
}

void NonRouting::receive(uint16_t from, byte * payload, unsigned int len, uint8_t metric) {
    //message reached its destination
    received_callback_(from, payload, len);
}

void NonRouting::set_sink(bool isSink) {
}

int NonRouting::send(uint16_t destination, byte *data, uint8_t len) {
    radio_send(destination, data, len);
}

uint8_t NonRouting::state() {
    return state_;
}

void NonRouting::set_my_address(uint16_t me) {

    this->myAddress = me;
}

void NonRouting::set_message_received_callback(void (*received_callback)(uint16_t, byte*, unsigned int)) {

    this->received_callback_ = received_callback;
}

void NonRouting::radio_send(uint16_t to, byte* payload, unsigned int size) {
    Tx16Request tx = Tx16Request(to, payload, size);
    xbee->send(tx, 112);
    delay(10);
}