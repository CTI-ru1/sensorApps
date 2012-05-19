#include "external_interface/external_interface.h"
#include "util/delegates/delegate.hpp"
#include "util/pstl/map_static_vector.h"
#include "util/pstl/static_string.h"

//used to blink the led
#include <isense/modules/core_module/core_module.h>

typedef wiselib::OSMODEL Os;

//messages used to report a reading and receive the GatewayBeacons
#include "../../messages/collector_message_new.h"
typedef wiselib::CollectorMsg<Os, Os::TxRadio> collectorMsg_t;
#include "../../messages/gateway_beacon_message.h"
typedef wiselib::GatewayBeaconMsg<Os, Os::TxRadio> broadcastMsg_t;


//TYPEDEFS
typedef Os::TxRadio::node_id_t node_id_t;
typedef Os::TxRadio::block_data_t block_data_t;

//EVENT IDS
#define TASK_READ_SENSORS 2


#define REPORTING_INTERVAL 60

class iSenseTestApp {
public:

    //--------------------------------------------------------------

    /**
     * unused in this context
     * @param value
     */
    void handle_uint32_data(uint32 value) {
        //nothing
    }

    //--------------------------------------------------------------

    /**
     * unused in this context
     * @param value
     */
    void handle_int8_data(int8 value) {
        //nothing
    }

    /**
     * boot function
     * @param value pointer to os
     */
    void init(Os::AppMainParameter& value) {
        radio_ = &wiselib::FacetProvider<Os, Os::TxRadio>::get_facet(value);
        timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet(value);
        debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet(value);
        debug_->debug("*Boot*");
        uart_ = &wiselib::FacetProvider<Os, Os::Uart>::get_facet(value);
        clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet(value);

        cm_ = new isense::CoreModule(value);

        mygateway_ = 0xffff;
        led = false;




        radio_->reg_recv_callback<iSenseTestApp, &iSenseTestApp::receive > (this);
        radio_->set_channel(12);



        timer_->set_timer<iSenseTestApp, &iSenseTestApp::execute > (10000, this, (void*) TASK_READ_SENSORS);

    }
    // --------------------------------------------------------------------

    /**
     * Executed periodically
     * @param userdata TASK to perform
     */
    void execute(void* userdata) {

        // Get the Temperature and Luminance from sensors and debug them
        if ((long) userdata == TASK_READ_SENSORS) {
            timer_->set_timer<iSenseTestApp, &iSenseTestApp::execute > (REPORTING_INTERVAL * 1000, this, (void*) TASK_READ_SENSORS);
            send_reading(0xffff, "test", 1);

        }
    }

protected:

    /**
     * Handle a new incoming message
     * @param src_addr
     * @param len
     * @param buf
     */
    void receive(node_id_t src_addr, Os::TxRadio::size_t len, block_data_t * buf) {

        if (check_gateway(src_addr, len, buf)) return;

        if ((buf[0] == 1) && (buf[1] == 1) && (buf[2] == 1)) {
            if (!led) {
                cm_->led_on();
                led = true;
            } else {
                cm_->led_off();
                led = false;
            }
        }

    }

    bool check_gateway(node_id_t src_addr, Os::TxRadio::size_t len, block_data_t * buf) {
        if ((len == 10) && (buf[0] == 0)) {
            bool sGmsg = true;
            for (int i = 1; i < 10; i++) {
                sGmsg = sGmsg && (buf[i] == i);
            }
            if (sGmsg) {
                mygateway_ = src_addr;
                //                debug_->debug("mygateway_->%x", mygateway_);
                return true;
            }
        }
        return false;
    }




private:

    void debug_payload(const uint8_t * payload, size_t length, node_id_t src) {
        char buffer[1024];
        int bytes_written = 0;
        bytes_written += sprintf(buffer + bytes_written, "pl(%x)(", src);
        for (size_t i = 0; i < length; i++) {
            bytes_written += sprintf(buffer + bytes_written, "%x|", payload[i]);
        }
        bytes_written += sprintf(buffer + bytes_written, ")");
        buffer[bytes_written] = '\0';
        debug_->debug("%s", buffer);
    }

    void send_reading(node_id_t destination, const char * capability, int value) {
        collectorMsg_t mess;
        mess.set_source(radio_->id());
        mess.set_target(destination);
        char temp_string[30];
        sprintf(temp_string, "%s", capability);
        mess.set_capability(temp_string);
        sprintf(temp_string, "%d", value);
        mess.set_value(temp_string);
        debug_->debug("Contains bidi %s -> %s ", mess.capability(), mess.value());
        radio_->send(mygateway_, mess.length(), (uint8*) & mess);
    }

    node_id_t mygateway_;
    bool led;

    isense::CoreModule* cm_;

    Os::TxRadio::self_pointer_t radio_;
    Os::Timer::self_pointer_t timer_;
    Os::Debug::self_pointer_t debug_;
    Os::Uart::self_pointer_t uart_;
    Os::Clock::self_pointer_t clock_;

};

wiselib::WiselibApplication<Os, iSenseTestApp> application;
// --------------------------------------------------------------------------

void application_main(Os::AppMainParameter& value) {
    application.init(value);
}
