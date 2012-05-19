/* 
 * File:   gateway_beacon_message.h
 * Author: amaxilat
 */

#ifndef GATEWAY_BEACON_MESSAGE_H
#define	GATEWAY_BEACON_MESSAGE_H

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
     * A Wiselib Message class for using declaring the existence of a Gateway     
     */
    template <typename OsModel_P, typename Radio_P> class GatewayBeaconMsg {
    public:
        /**
         * The type of the Wiselib size
         */
        typedef typename Radio_P::size_t size_t;
        /**
         * The type of the Wiselib block of data
         */
        typedef typename OsModel_P::block_data_t block_data_t;
        // --------------------------------------------------------------------

        /**
         * Constructor that sets the contents of the message to the beacon contents
         */
        GatewayBeaconMsg() {
            for (int i = 0; i < 10; i++) {
                buffer[i] = i;
            }
        }

        /**
         * Gives the length of the message to send.
         * @return the length of the message buffer.
         */
        size_t length() {
            return 10;
        }

    private:
        block_data_t buffer[10]; // buffer for the message data

    };

}

#endif	/* GATEWAY_BEACON_MESSAGE_H */



