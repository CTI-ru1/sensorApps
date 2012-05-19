/* 
 * File:   collector_message_new.h
 * Author: amaxilat
 */

#ifndef COLLECTOR_MSG_NEW_H
#define	COLLECTOR_MSG_NEW_H

/**
 * The Wiselib is an algorithms library for networked embedded devices. 
 * It contains various algorithm classes (for instance, localization or routing) 
 * that can be compiled for several platforms such as iSense or Contiki, 
 * or the sensor network simulator Shawn. It is completely written in C++, 
 * and uses templates in the same way as Boost and CGAL. This makes it possible 
 * to write generic and platform independent code that is very efficiently 
 * compiled for the various platforms.
 */
namespace wiselib {

    /**
     * A Wiselib Message class for using reporting new Readings to the backend
     * Contains a MessageID, the sender node id , the 2nd node id for link readings,
     * the capability (as a string) and the value of the reading (as a string)
     */
    template < typename OsModel_P, typename Radio_P> class CollectorMsg {
    public:
        /**
         * The type of the Wiselib Os
         */
        typedef OsModel_P OsModel;
        /**
         * The type of the Wiselib Radio
         */
        typedef Radio_P Radio;
        /**
         * The type of the Wiselib node ids
         */
        typedef typename Radio::node_id_t node_id_t;
        /**
         * The type of the Wiselib size
         */
        typedef typename Radio::size_t size_t;
        /**
         * The type of the Wiselib block of data
         */
        typedef typename Radio::block_data_t block_data_t;
        /**
         * The type of the Wiselib message ids
         */
        typedef typename Radio::message_id_t message_id_t;
        // message ids

        /**
         * contains the Wiselib 
         * <a href='https://github.com/ibr-alg/wiselib/wiki/Reserved-message-ids'>
         * Unique Message ID </a> for Collector messages         
         */
        enum {
            COLLECTOR_MSG_TYPE = 103,
        };

        // --------------------------------------------------------------------

        /**
         * Default constructor
         * Initializes the 
         * <a href='https://github.com/mksense'>mkSense</a> 
         * prefix and the message id, source and target address
         */
        CollectorMsg() {
            buffer[0] = 0x7f;
            buffer[1] = 0x69;
            buffer[2] = 0x70;
            set_msg_id(COLLECTOR_MSG_TYPE);
            set_source(0xffff);
            set_target(0xffff);
        };
        // --------------------------------------------------------------------

        /**
         * Destructor
         */
        ~CollectorMsg() {
        };

        /**
         * Get the Message id.
         * @return the 1st byte of the payload as Message id.
         */
        inline message_id_t msg_id() {
            return buffer[MSG_ID_POS];

        };

        /**
         * Set the message id to the default id.
         * @param id the id to set to.
         */
        inline void set_msg_id(message_id_t id) {
            write<OsModel, block_data_t, uint8_t > (buffer + MSG_ID_POS, id);
        };
        // --------------------------------------------------------------------

        /**
         * Get the sender node id.
         * @return the node id as a node_id_t.
         */
        inline node_id_t source() {
            return read<OsModel, block_data_t, node_id_t > (buffer + SOURCE_NODE);
        };

        /**
         * Sets the sender id to the given id.
         * @param source_ the id of the source node
         */
        inline void set_source(node_id_t source_) {
            write<OsModel, block_data_t, node_id_t > (buffer + SOURCE_NODE, source_);
        };

        // --------------------------------------------------------------------

        /**
         * Get the target node id.
         * @return the node id as a node_id_t.
         */
        inline node_id_t target() {
            return read<OsModel, block_data_t, node_id_t > (buffer + TARGET_NODE);
        };

        /**
         * Sets the target id to the given id.
         * @param target_ the id of the target node
         */
        inline void set_target(node_id_t target_) {
            write<OsModel, block_data_t, node_id_t > (buffer + TARGET_NODE, target_);
        };

        // --------------------------------------------------------------------

        /**
         * Get the capability reported.
         * @return a char array containing the capability name.
         */
        inline char * capability() {
            return (char*) (buffer + CAPABILITY_STR_POS);
        };

        /**
         * Sets the capabity of the message.
         * @param capability_ the capability as a char *
         */
        inline void set_capability(const char * capability_) {
            uint8_t len = sprintf(capability(), "%s", capability_);
            buffer[CAPABILITY_LEN_POS] = len + 1;
        };

        // --------------------------------------------------------------------

        /**
         * Get the value reported.
         * @return a char array containing the value.
         */
        inline char * value() {
            return (char*) (buffer + CAPABILITY_STR_POS + capability_length());
        };

        /**
         * Sets the value of the message.
         * @param value_ the value as a char *
         */
        inline void set_value(const char* value_) {
            uint8_t len = sprintf(value(), "%s", value_);
            buffer[VALUE_LEN_POS] = len + 1;
        };
        // --------------------------------------------------------------------

        /**
         * Gives the length of the message to send.
         * @return the length of the message buffer.
         */
        inline uint8_t length() {
            return CAPABILITY_STR_POS + capability_length() + value_length();
        };

    private:

        enum data_positions {
            MSG_ID_POS = 3, // message id position inside the message [uint8]            
            CAPABILITY_LEN_POS = 4,
            VALUE_LEN_POS = 5,
            SOURCE_NODE = 6,
            TARGET_NODE = 8,
            CAPABILITY_STR_POS = 10
        };

        /**
         * Length of the contained capability.
         * @return the length as unit8_t.
         */
        inline uint8_t capability_length() {
            return buffer[CAPABILITY_LEN_POS];
        };

        /**
         * Length of the contained value.
         * @return the length as unit8_t.
         */
        inline uint8_t value_length() {
            return buffer[VALUE_LEN_POS];
        };

        block_data_t buffer[Radio::MAX_MESSAGE_LENGTH]; // buffer for the message data
    };
}

#endif	/* COLLECTOR_MSG_NEW_H */



