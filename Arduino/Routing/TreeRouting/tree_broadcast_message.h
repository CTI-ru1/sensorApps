#ifndef TREE_BROADCAST_MESSAGE
#define TREE_BROADCAST_MESSAGE

class TreeBroadcastMessage {
public:
    // --------------------------------------------------------------------
    inline TreeBroadcastMessage();
    inline TreeBroadcastMessage(uint8_t msg, uint8_t hops);
    // --------------------------------------------------------------------

    inline uint8_t msg_id() {
        return buffer[0];
    };
    // --------------------------------------------------------------------

    inline void set_msg_id(uint8_t id) {
        buffer[0] = id;
    }
    // --------------------------------------------------------------------

    inline uint8_t hops() {
        return buffer[HOPS_POS];
    }
    // --------------------------------------------------------------------

    inline void set_hops(uint8_t hops) {
        buffer[HOPS_POS] = hops;
    }
    // --------------------------------------------------------------------

    inline size_t buffer_size() {
        return MSG_END;
    };

private:

    enum data_positions {
        MSG_ID_POS = 0,
        HOPS_POS = 1,
        MSG_END = 4
    };

    byte buffer[MSG_END];
};
// -----------------------------------------------------------------------

TreeBroadcastMessage::
TreeBroadcastMessage() {
    set_msg_id(0);
    set_hops(0);
}
// -----------------------------------------------------------------------

TreeBroadcastMessage::
TreeBroadcastMessage(uint8_t msg, uint8_t hops) {
    set_msg_id(msg);
    set_hops(hops);
}
#endif