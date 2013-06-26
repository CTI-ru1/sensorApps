#ifndef TREE_ROUTING_MESSAGE
#define TREE_ROUTING_MESSAGE

class TreeRoutingMessage {
public:
    // --------------------------------------------------------------------
    inline TreeRoutingMessage();
    inline TreeRoutingMessage(uint8_t id, uint16_t src);
    // --------------------------------------------------------------------

    inline uint8_t msg_id() {
        return buffer[0];
    };
    // --------------------------------------------------------------------

    inline void set_msg_id(uint8_t id) {
        buffer[0] = id;
    }
    // --------------------------------------------------------------------

    inline uint8_t payload_size() {
        return buffer[PAYLOAD_POS];
    }
    // --------------------------------------------------------------------

    inline uint16_t source() {
        uint16_t src;
        memcpy(&src, buffer + SOURCE_POS + 1, 1);
        memcpy((uint8_t*) & src + 1, buffer + SOURCE_POS, 1);
        return src;
    }
    // --------------------------------------------------------------------

    inline void set_source(uint16_t src) {
        uint8_t * src_p = (uint8_t *) & src;
        buffer[SOURCE_POS + 1] = *src_p;
        buffer[SOURCE_POS] = *(src_p + 1);
    }

    inline void set_payload(uint8_t len, byte* data) {
        buffer[PAYLOAD_POS] = len;
        if (len == 0) return;
        memcpy((void *) (buffer + PAYLOAD_POS + 1), (void *) data, len);
    }
    // -----------------------------------------------------------------------

    inline byte* payload(void) {
        return buffer + PAYLOAD_POS + 1;
    }
    // --------------------------------------------------------------------

    inline size_t buffer_size() {
        return PAYLOAD_POS + 1 + payload_size();
    };

    enum data_positions {
        MSG_ID_POS = 0,
        SOURCE_POS = 1,
        PAYLOAD_POS = 3
    };
private:

    inline void set_payload_size(uint8_t len) {
        buffer[PAYLOAD_POS] = len;
    }

    byte buffer[100];
};
// -----------------------------------------------------------------------

TreeRoutingMessage::
TreeRoutingMessage() {
    set_msg_id(0);
    set_payload_size(0);
    set_source(0);
}
// -----------------------------------------------------------------------

TreeRoutingMessage::
TreeRoutingMessage(uint8_t id, uint16_t src) {
    set_msg_id(id);
    set_payload_size(0);
    set_source(src);
}
#endif