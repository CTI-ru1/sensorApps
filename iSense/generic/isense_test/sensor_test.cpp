#include "external_interface/external_interface.h"
#include "util/delegates/delegate.hpp"
#include "util/pstl/map_static_vector.h"
#include "util/pstl/static_string.h"

//ISENSE SENSORS
#include <isense/modules/core_module/core_module.h>
#include <isense/modules/environment_module/environment_module.h>
#include <isense/modules/environment_module/temp_sensor.h>
#include <isense/modules/environment_module/light_sensor.h>
#include <isense/modules/security_module/pir_sensor.h>


typedef wiselib::OSMODEL Os;

//MESSAGE_TYPES
#include "./collector_message.h"
typedef wiselib::CollectorMsg<Os, Os::TxRadio> collectorMsg_t;
typedef wiselib::BroadcastMsg<Os, Os::TxRadio> broadcastMsg_t;

//TYPEDEFS
typedef Os::TxRadio::node_id_t node_id_t;
typedef Os::TxRadio::block_data_t block_data_t;

//EVENT IDS
#define TASK_SLEEP 1
#define TASK_WAKE 2
#define TASK_READ_SENSORS 2
#define TASK_SET_LIGHT_THRESHOLD 3
#define TASK_BROADCAST_GATEWAY 4
#define TASK_TEST 5

#define REPORTING_INTERVAL 60

class Application {
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




        radio_->reg_recv_callback<Application, &Application::receive > (this);
        radio_->set_channel(12);



        timer_->set_timer<Application, &Application::execute > (10000, this, (void*) TASK_READ_SENSORS);

    }
    // --------------------------------------------------------------------

    /**
     * Executed periodically
     * @param userdata TASK to perform
     */
    void execute(void* userdata) {

        // Get the Temperature and Luminance from sensors and debug them
        if ((long) userdata == TASK_READ_SENSORS) {
            timer_->set_timer<Application, &Application::execute > (REPORTING_INTERVAL * 1000, this, (void*) TASK_READ_SENSORS);
            collectorMsg_t mess;
            mess.set_collector_type_id(collectorMsg_t::TTEST);
            radio_->send(mygateway_, mess.buffer_size(), (uint8*) & mess);

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



    node_id_t mygateway_;
    bool led;

    isense::CoreModule* cm_;

    Os::TxRadio::self_pointer_t radio_;
    Os::Timer::self_pointer_t timer_;
    Os::Debug::self_pointer_t debug_;
    Os::Uart::self_pointer_t uart_;
    Os::Clock::self_pointer_t clock_;

};

wiselib::WiselibApplication<Os, Application> application;
// --------------------------------------------------------------------------

void application_main(Os::AppMainParameter& value) {
    application.init(value);
}
