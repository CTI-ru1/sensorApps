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
#include <isense/modules/cc_weather_module/ms55xx.h>

typedef wiselib::OSMODEL Os;

//ND
#include "algorithms/neighbor_discovery/echo.h"
typedef wiselib::Echo<Os, Os::TxRadio, Os::Timer, Os::Debug> nb_t;

//MESSAGE_TYPES
#include "../../messages/collector_message_new.h"
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
#define REVISION 4

#define REPORTING_INTERVAL 180

#undef WEATHER_MODULE
#undef ENVIRONMENTAL_MODULE
#undef SECURITY_MODULE
//#define WEATHER_MODULE
#define ENVIRONMENTAL_MODULE
#define SECURITY_MODULE

class Application
:
public isense::SensorHandler,
public isense::Int8DataHandler,
public isense::Uint32DataHandler {
public:

    virtual uint16_t application_id() {
        return 1;
    }

    virtual uint8_t software_revision(void) {
        return REVISION;
    }

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
        ospointer = &value;
        radio_ = &wiselib::FacetProvider<Os, Os::TxRadio>::get_facet(value);
        timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet(value);
        debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet(value);
        debug_->debug("*Boot*");
        uart_ = &wiselib::FacetProvider<Os, Os::Uart>::get_facet(value);
        clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet(value);

        cm_ = new isense::CoreModule(value);

        mygateway_ = 0xffff;

#ifdef WEATHER_MODULE
        init_weather_module(value);
#endif
#ifdef ENVIRONMENTAL_MODULE
        init_environmental_module(value);
#endif
#ifdef SECURITY_MODULE
        init_security_module(value);
#endif




        radio_->reg_recv_callback<Application, &Application::receive > (this);
        radio_->set_channel(12);

        uart_->reg_read_callback<Application, &Application::handle_uart_msg > (this);
        uart_->enable_serial_comm();

        //        debug_->debug("INIT ");

        nb_.init(*radio_, *clock_, *timer_, *debug_, 2000, 16000, 250, 255);
        nb_.enable();
        nb_. reg_event_callback<Application, &Application::ND_callback > ((uint8) 2, nb_t::NEW_NB | nb_t::NEW_NB_BIDI | nb_t::LOST_NB_BIDI | nb_t::DROPPED_NB, this);

#ifdef WEATHER_MODULE
        timer_->set_timer<Application, &Application::read_weather_sensors > (10000, this, (void*) 0);
#endif
#ifdef ENVIRONMENTAL_MODULE
        timer_->set_timer<Application, &Application::read_environmental_sensors > (10000, this, (void*) 0);
#endif
        if (is_gateway()) {
            // register task to be called in a minute for periodic sensor readings
            timer_->set_timer<Application, &Application::broadcast_gateway > (1000, this, (void*) 0);
            //            timer_->set_timer<Application, &Application::execute > (5000, this, (void*) TASK_TEST);
        }
    }
    // --------------------------------------------------------------------

    void init_weather_module(Os::AppMainParameter& value) {
        ms_ = new isense::Ms55xx(value);


    }

    void init_environmental_module(Os::AppMainParameter& value) {
        em_ = new isense::EnvironmentModule(value);
        if (em_ != NULL) {
            em_->enable(true);
            if (em_->light_sensor()->enable()) {
                em_->light_sensor()->set_data_handler(this);
                //os().add_task_in(Time(10, 0), this, (void*) TASK_SET_LIGHT_THRESHOLD);
                debug_->debug("em light");
            }
            if (em_->temp_sensor()->enable()) {
                em_->temp_sensor()->set_data_handler(this);
                debug_->debug("em temp");
            }
        }

    }

    void init_security_module(Os::AppMainParameter& value) {
        pir_ = new isense::PirSensor(value);
        pir_->set_sensor_handler(this);
        pir_->set_pir_sensor_int_interval(2000);
        if (pir_->enable()) {
            pir_sensor_ = true;
            debug_->debug("id::%x em pir", radio_->id());
        }

        //        accelerometer_ = new isense::LisAccelerometer(value);
        //        if (accelerometer_ != NULL) {
        //            accelerometer_->set_mode(MODE_THRESHOLD);
        //            accelerometer_->set_threshold(25);
        //            accelerometer_->set_handler(this);
        //            accelerometer_->enable();
        //        }
    }

    /**
     * Periodically read sensor values and report them
     * @param userdata unused
     */
    void read_environmental_sensors(void* userdata) {
        // Get the Temperature and Luminance from sensors and debug them
        if (radio_->id() != 0xddba) {
            timer_->set_timer<Application, &Application::read_environmental_sensors > (REPORTING_INTERVAL * 1000, this, (void*) 0);
        }

        if (!is_gateway()) {

            int16 temp = em_->temp_sensor()->temperature();
            if (temp < 100) {
                send_reading(0xffff, "temperature", temp);
            }
            uint32 lux = em_->light_sensor()->luminance();
            if (lux < 20000) {
                send_reading(0xffff, "light", lux);
            }

        } else {
            int16 temp = em_->temp_sensor()->temperature();
            if (temp < 100) {
                debug_->debug("node::%x temperature %d ", radio_->id(), temp);

            }
            uint32 lux = em_->light_sensor()->luminance();
            if (lux < 20000) {
                debug_->debug("node::%x light %d ", radio_->id(), lux);
            }


        }

    }

    /**
     * Periodically read sensor values from weather module and report them
     * @param userdata unused
     */
    void read_weather_sensors(void* userdata) {
        // Get the Temperature and Luminance from sensors and debug them
        if (radio_->id() != 0xddba) {
            timer_->set_timer<Application, &Application::read_weather_sensors > (REPORTING_INTERVAL * 1000, this, (void*) 0);
        }

        ms_ = new isense::Ms55xx(*ospointer);
        ms_->reset();


        int16 temp = ms_->get_temperature();
        int16 bpressure = ms_->read_pressure();
        if (!is_gateway()) {
            send_reading(0xffff, "temperature", temp / 10);

            send_reading(0xffff, "barometricpressure", bpressure / 10);
        } else {
            debug_->debug("node::%x temperature %d ", radio_->id(), temp / 10);
            debug_->debug("node::%x barometricpressure %d ", radio_->id(), bpressure / 10);
        }
    }

    /**
     * Periodically broadcasts the gateway node message beacon
     * @param userdata TASK to perform
     */
    void broadcast_gateway(void* value) {
        //        debug_->debug("broadcast_gateway");
        broadcastMsg_t msg;
        radio_->send(0xffff, msg.length(), (block_data_t*) & msg);
        timer_->set_timer<Application, &Application::broadcast_gateway > (20 * 1000, this, (void*) 0);
    }

protected:

    /**
     * Handles a new sensor reading
     */
    virtual void handle_sensor() {
        //        debug_->debug("pir event");
        if (!is_gateway()) {
            send_reading(0xffff, "pir", 1);
        } else {
            debug_->debug("node::%x pir 1 ", radio_->id());
        }
    }

    /**
     * Handles a new neighborhood event
     * @param event event type
     * @param from neighbor id
     * @param len unused
     * @param data unused
     */
    void ND_callback(uint8 event, uint16 from, uint8 len, uint8 * data) {
        if (event == nb_t::NEW_NB_BIDI) {
            if (!is_gateway()) {
                send_reading(from, "status", 1);

                send_reading(from, "lqi", nb_.get_lqi(from));
            } else {
                debug_->debug("node::%x,%x status %d ", radio_->id(), from, 1);

            }
        } else if ((event == nb_t::LOST_NB_BIDI) || (event == nb_t::DROPPED_NB)) {
            if (!is_gateway()) {
                send_reading(from, "status", 0);
            } else {
                debug_->debug("node::%x,%x status %d ", radio_->id(), from, 0);
            }
        }
    }

    /**
     * Handle a new uart event
     * @param len payload length
     * @param mess payload buffer
     */
    void handle_uart_msg(Os::Uart::size_t len, Os::Uart::block_data_t *mess) {
        node_id_t node;
        memcpy(&node, mess, sizeof (node_id_t));
        radio_->send(node, len - 2, (uint8*) mess + 2);
        debug_command(mess + 2, 13, node);
        if (len > 8) {
            char buffer[100];
            int bytes_written = 0;
            for (int i = 8; i < len; i++) {
                bytes_written += sprintf(buffer + bytes_written, "%d", mess[i]);
            }
            buffer[bytes_written] = '\0';
            debug_->debug("FORWARDING to %x %s", node, buffer);
        }
    }

    /**
     * Handle a new incoming message
     * @param src_addr
     * @param len
     * @param buf
     */
    void receive(node_id_t src_addr, Os::TxRadio::size_t len, block_data_t * buf) {
        //        debug_payload((uint8_t*) buf, len, src_addr);
        if (check_led(src_addr, len, buf)) return;

        if (!is_gateway()) {
            //Executed by nonGateway nodes
            if (check_gateway(src_addr, len, buf)) return;
        } else {
            //Executed by Gateway nodes
            if ((radio_->id() == 0x1ccd) && (src_addr == 0x42f)) {
                debug_->debug("case1");
                return;
            }
            if (check_air_quality(src_addr, len, buf)) {
                debug_->debug("check_air_quality");
                return;
            }

            check_collector(src_addr, len, buf);
        }
    }

    bool check_led(node_id_t src_addr, Os::TxRadio::size_t len, block_data_t * buf) {
        if (buf[0] == 0x7f && buf[1] == 0x69 && buf[2] == 0x70 && buf[3] == 0x1) {
            if (buf[4] == 0x1) {
                if (buf[5] == 1) {
                    send_reading(0xffff, "led", 1);
                    cm_->led_on();
                    return true;
                } else if (buf[5] == 0) {
                    send_reading(0xffff, "led", 0);
                    cm_->led_off();
                    return true;
                }
            }
        }
        return false;
    }

    bool check_gateway(node_id_t src_addr, Os::TxRadio::size_t len, block_data_t * buf) {
        if ((len == 10) && (buf[0] == 0)) {
            bool sGmsg = true;
            for (int i = 1; i < 10; i++) {
                sGmsg = sGmsg && (buf[i] == i);
            }
            if (sGmsg) {
                mygateway_ = src_addr;
                debug_->debug("mygateway_->%x", mygateway_);
                return true;
            }
        }
        return false;
    }

    bool check_air_quality(node_id_t src_addr, Os::TxRadio::size_t len, block_data_t * buf) {
        if ((src_addr == 0x2c41) && (buf[0] == 0x43) && (0x9979 == radio_->id())) {
            uint8 mess[len];
            memcpy(mess, buf, len);
            mess[len - 1] = '\0';

            if ((buf[1] == 0x4f) && (buf[2] == 0x32)) {
                debug_->debug("node::%x co %s ", src_addr, mess + 5);
            } else if (buf[1] == 0x4f) {
                debug_->debug("node::%x co2 %s ", src_addr, mess + 4);
            } else if (buf[1] == 0x48) {
                debug_->debug("node::%x ch4 %s ", src_addr, mess + 5);
            }
            return true;
        }
        return false;
    }

    void check_collector(node_id_t src_addr, Os::TxRadio::size_t len, block_data_t * buf) {

        collectorMsg_t * mess = (collectorMsg_t *) buf;

        if (mess->msg_id() == collectorMsg_t::COLLECTOR_MSG_TYPE) {
            if ((src_addr == 0xcb5) || (src_addr == 0x786a)) {
                debug_payload((uint8_t*) mess, len, src_addr);
            }
            if (mess->target() == 0xffff) {
                debug_->debug("node::%x %s %s ", mess->source(), mess->capability(), mess->value());
            } else {
                debug_->debug("node::%x,%x %s %s ", mess->source(), mess->target(), mess->capability(), mess->value());
            }
        }
    }


private:

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

    void send_reading(node_id_t destination, const char * capability, int value, int decimals) {
        collectorMsg_t mess;
        mess.set_source(radio_->id());
        mess.set_target(destination);
        char temp_string[30];
        sprintf(temp_string, "%s", capability);
        mess.set_capability(temp_string);
        sprintf(temp_string, "%d.%d", value, decimals);
        mess.set_value(temp_string);
        debug_->debug("Contains bidi %s -> %s ", mess.capability(), mess.value());
        radio_->send(mygateway_, mess.length(), (uint8*) & mess);
    }

    bool is_gateway() {
        switch (radio_->id()) {
            case 0x6699: //2.3
            case 0x0498: //2.1
            case 0x1b7f: //3.3
            case 0x1ccd: //0.1
            case 0xc7a: //0.2
            case 0x99ad: //3,1
            case 0x8978: //1.1
                //            case 0x181: //1.1
                return true;
            default:
                return false;
        }
        return true;
    }

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

    void debug_command(const uint8_t * payload, size_t length, node_id_t dest) {
        //TEMPLATE : "id::%x dest::%x command=1|2|3"
        char buffer[1024];
        int bytes_written = 0;
        bytes_written += sprintf(buffer + bytes_written, "id::%x dest::%x command=", radio_->id(), dest);
        for (size_t i = 0; i < length; i++) {
            bytes_written += sprintf(buffer + bytes_written, "%x|", payload[i]);
        }
        bytes_written += sprintf(buffer + bytes_written, " ");
        buffer[bytes_written] = '\0';
        debug_->debug("%s", buffer);
    }

    node_id_t mygateway_;
    nb_t nb_;
    bool pir_sensor_;
    Os::AppMainParameter* ospointer;
    isense::EnvironmentModule* em_;
    //    isense::LisAccelerometer* accelerometer_;
    isense::PirSensor* pir_;
    isense::Ms55xx* ms_;
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
