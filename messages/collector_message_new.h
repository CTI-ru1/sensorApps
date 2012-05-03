/* 
 * File:   collector_msg.h
 * Author: Amaxilatis
 */

#ifndef COLLECTOR_MSG_NEW_H
#define	COLLECTOR_MSG_NEW_H

namespace wiselib {

    template
    < typename OsModel_P, typename Radio_P>
    class CollectorMsg {
    public:
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;

        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::message_id_t message_id_t;
        // message ids

        enum {
            COLLECTOR_MSG_TYPE = 103,
        };

        enum data_positions {
            MSG_ID_POS = 3, // message id position inside the message [uint8]            
            CAPABILITY_LEN_POS = 4,
            VALUE_LEN_POS = 5,
            SOURCE_NODE = 6,
            TARGET_NODE = 8,
            CAPABILITY_STR_POS = 10
        };

        // --------------------------------------------------------------------

        CollectorMsg() {
            buffer[0]=0x7f;
            buffer[1]=0x69;
            buffer[2]=0x70;
            set_msg_id(COLLECTOR_MSG_TYPE);
            set_source(0xffff);
            set_target(0xffff);
        };
        // --------------------------------------------------------------------

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
         * Get the capability reported.
         * @return a char array containing the capability name.
         */
        inline node_id_t source() {
            return read<OsModel, block_data_t, node_id_t > (buffer + SOURCE_NODE);
        };

        /**
         * Sets the capabity of the message.
         * @param capability_ the capability as a char *
         */
        inline void set_source(node_id_t source_) {
            write<OsModel, block_data_t, node_id_t > (buffer + SOURCE_NODE, source_);
        };

        // --------------------------------------------------------------------

        /**
         * Get the capability reported.
         * @return a char array containing the capability name.
         */
        inline node_id_t target() {
            return read<OsModel, block_data_t, node_id_t > (buffer + TARGET_NODE);
        };

        /**
         * Sets the capabity of the message.
         * @param capability_ the capability as a char *
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

    template
    <typename OsModel_P, typename Radio_P>
    class BroadcastMsg {
    public:
        typedef typename Radio_P::size_t size_t;
        typedef typename OsModel_P::block_data_t block_data_t;
        // --------------------------------------------------------------------

        BroadcastMsg() {
            for (int i = 0; i < 10; i++) {
                buffer[i] = i;
            }
        }

        size_t length() {
            return 10;
        }

    private:
        block_data_t buffer[10]; // buffer for the message data

    };

}

#endif	/* COLLECTOR_MSG_NEW_H */



