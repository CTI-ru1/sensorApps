#include "external_interface/external_interface.h"
#include "util/delegates/delegate.hpp"
#include "util/pstl/map_static_vector.h"
#include "util/pstl/static_string.h"

#undef CORE_COLLECTOR
#undef WEATHER_COLLECTOR
#undef ENVIRONMENTAL_COLLECTOR
#undef SECURITY_COLLECTOR
#undef SOLAP_COLLECTOR

//Uncomment to enable the isense module
#define CORE_COLLECTOR
#define ENVIRONMENTAL_COLLECTOR
#define SECURITY_COLLECTOR
//#define SOLAR_COLLECTOR
//#define WEATHER_COLLECTOR

#ifdef CORE_COLLECTOR
#include <isense/modules/core_module/core_module.h>
#endif
#ifdef ENVIRONMENTAL_COLLECTOR
#include <isense/modules/environment_module/environment_module.h>
#include <isense/modules/environment_module/temp_sensor.h>
#include <isense/modules/environment_module/light_sensor.h>
#endif
#ifdef SOLAR_COLLECTOR
#include <isense/modules/solar_module/solar_module.h>
#endif
#ifdef SECURITY_COLLECTOR
#include <isense/modules/security_module/pir_sensor.h>
#endif
#ifdef WEATHER_COLLECTOR
#include <isense/modules/cc_weather_module/ms55xx.h>
#endif

typedef wiselib::OSMODEL Os;

//neighbor discovery algorithm
#include "algorithms/neighbor_discovery/echo.h"
typedef wiselib::Echo<Os, Os::TxRadio, Os::Timer, Os::Debug> nb_t;

//reporting message
#include "../../messages/collector_message_new.h"
typedef wiselib::CollectorMsg<Os, Os::TxRadio> collectorMsg_t;
#include "../../messages/gateway_beacon_message.h"
typedef wiselib::GatewayBeaconMsg<Os, Os::TxRadio> broadcastMsg_t;

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

//defines every how many seconds new readings will be reported to the gateway
#define REPORTING_INTERVAL 180

/**
 * A wiselib application that collects periodically readings
 * from 
 * <a href='http://www.coalesenses.com/index.php?page=hardware-docs-and-demos'>
 * iSense : Environmental, Security, Weather sensors </a>
 * and reports them to the 
 * <a href='https://github.com/organizations/Uberdust'> Uberdust Backend</a>
 * Also collects Link Readings (Neighbor Discovery and Lqis).
 */
class iSenseCollectorApp :
#ifdef SECURITY_COLLECTOR
public isense::SensorHandler,
#endif
public isense::Int8DataHandler,
public isense::Uint32DataHandler {
public:

    /**
     * Returns a unique application id.
     * @return the id of the current application
     */
    uint16_t application_id() {
        return 1;
    }

    /**
     * Returns the current version of the application
     * @return the current version
     */
    uint8_t software_revision(void) {
        return REVISION;
    }

    //--------------------------------------------------------------

    /**
     * Accelerometer sensor, unused in this app
     * @param value
     */
    void handle_uint32_data(uint32 value) {
        //nothing
    }

    /**
     * Accelerometer sensor, unused in this app
     * @param value
     */
    void handle_int8_data(int8 value) {
        //nothing
    }

    //--------------------------------------------------------------

    /**
     * Initializes the application and the enabled sensor modules.
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
#ifdef CORE_COLLECTOR
        cm_ = new isense::CoreModule(value);
#endif
        mygateway_ = 0xffff;

#ifdef WEATHER_COLLECTOR
        init_weather_module(value);
#endif
#ifdef SOLAR_COLLECTOR
        init_solar_module(value);
#endif
#ifdef ENVIRONMENTAL_COLLECTOR
        init_environmental_module(value);
#endif
#ifdef SECURITY_COLLECTOR
        init_security_module(value);
#endif

        radio_->reg_recv_callback<iSenseCollectorApp, &iSenseCollectorApp::receive > (this);
        radio_->set_channel(12);

        uart_->reg_read_callback<iSenseCollectorApp, &iSenseCollectorApp::handle_uart_msg > (this);
        uart_->enable_serial_comm();

        nb_.init(*radio_, *clock_, *timer_, *debug_, 2000, 16000, 250, 255);
        nb_.enable();
        nb_. reg_event_callback<iSenseCollectorApp, &iSenseCollectorApp::ND_callback > ((uint8) 2, nb_t::NEW_NB | nb_t::NEW_NB_BIDI | nb_t::LOST_NB_BIDI | nb_t::DROPPED_NB, this);

#ifdef WEATHER_COLLECTOR
        timer_->set_timer<iSenseCollectorApp, &iSenseCollectorApp::read_weather_sensors > (10000, this, (void*) 0);
#endif
#ifdef SOLAR_COLLECTOR
        debug_->debug("set timer");
        timer_->set_timer<iSenseCollectorApp, &iSenseCollectorApp::read_solar_sensors > (5000, this, (void*) TASK_WAKE);
#else        
#ifdef ENVIRONMENTAL_COLLECTOR
        timer_->set_timer<iSenseCollectorApp, &iSenseCollectorApp::read_environmental_sensors > (10000, this, (void*) 0);
#endif
#endif
        if (is_gateway()) {
            // register task to be called in a minute for periodic sensor readings
            timer_->set_timer<iSenseCollectorApp, &iSenseCollectorApp::broadcast_gateway > (1000, this, (void*) 0);
            //            timer_->set_timer<Application, &Application::execute > (5000, this, (void*) TASK_TEST);
        }
    }

    // --------------------------------------------------------------------
#ifdef WEATHER_COLLECTOR

    /**
     * Initializes the Weather Sensor Module
     * @param value pointer to os
     */
    void init_weather_module(Os::AppMainParameter& value) {
        ms_ = new isense::Ms55xx(value);
    }

    /**
     * Reads sensor values from the Weather Sensor Module
     * and reports them to the Gateway node
     * @param userdata unused, required for wiselib
     */
    void read_weather_sensors(void* userdata) {
        // Get the Temperature and Luminance from sensors and debug them
        if (radio_->id() != 0xddba) {
            timer_->set_timer<iSenseCollectorApp, &iSenseCollectorApp::read_weather_sensors > (REPORTING_INTERVAL * 1000, this, (void*) 0);
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
#endif

#ifdef ENVIRONMENTAL_COLLECTOR

    /**
     * Initializes the Environmental Sensor Module
     * @param value pointer to os
     */
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

    /**
     * Reads sensor values from the Environmental Sensor Module
     * and reports them to the Gateway node
     * @param userdata unused, required for wiselib
     */
    void read_environmental_sensors(void* userdata) {
        // Get the Temperature and Luminance from sensors and debug them
        if (radio_->id() != 0xddba) {
            timer_->set_timer<iSenseCollectorApp, &iSenseCollectorApp::read_environmental_sensors > (REPORTING_INTERVAL * 1000, this, (void*) 0);
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

#endif
#ifdef SOLAR_COLLECTOR

    /**
     * Initializes the Solar Harvesting Module
     * @param value pointer to os
     */
    void init_solar_module(Os::AppMainParameter& value) {
        debug_->debug("init_solar_module");
        // create SolarModule instance
        sm_ = new isense::SolarModule(value);

        // if allocation of SolarModule was successful
        if (sm_ != NULL) {
            debug_->debug("not null");
            // read out the battery state
            isense::BatteryState bs = sm_->battery_state();
            // estimate battery charge from the battery voltage
            uint32 charge = sm_->estimate_charge(bs.voltage);
            // set the estimated battery charge
            sm_->set_battery_charge(charge);
            debug_->debug("initialized");

        }

    }

    /**
     * Reads sensor values from the Solar Harvesting Module
     * and reports them to the Gateway node
     * Also sets the duty cycling of the iSense device 
     * so that battery life is extended
     * @param userdata Used to define a Sleep or wake up task
     */
    void read_solar_sensors(void* userdata) {
        debug_->debug("read_solar_sensors");
        if ((uint32) userdata == TASK_WAKE) {
            // register as a task to wake up again in one minute
            // the below call equals calling
            // add_task_in(Time(60,0), this, (void*)TASK_WAKE);
            timer_->set_timer<iSenseCollectorApp, &iSenseCollectorApp::read_solar_sensors > (60000, this, (void*) TASK_WAKE);

            //initiate control cycle, and read out battery state
            isense::BatteryState bs = sm_->control();

            // prevent sleeping to keep node awake
            ((isense::Os *) ospointer)->allow_sleep(false);
            //            ospointer->allow_sleep(false);

            // adopt duty cycle to the remaining battery charge
            if (bs.charge < 50000) {
                // battery nearly empty -->
                // set ultra-low duty cycle
                // live ~20 days
                duty_cycle_ = 1; // 0.1%
            } else
                if (bs.capacity < 1000000) //1 Ah or less
            {
                //live approx. 9 days out of 1Ah
                // and then another 20 days at 0.1% duty cycle
                duty_cycle_ = 100;
            } else
                if (bs.capacity < 3000000) //3Ah or less
            {
                // live approx. 6 days out of 1Ah
                // and then another 9 days at 10% duty cycle
                // and then another 20 days at 0.1% duty cycle
                duty_cycle_ = 300; // 30%
            } else
                if (bs.capacity < 5000000/*2Ah*/) {
                // live approx. 4 days out of 2Ah
                // and then another 6 days at 30%
                // and then another 9 days at 10% duty cycle
                // and then another 20 days at 0.1% duty cycle
                duty_cycle_ = 500; // 50%
            } else {
                // live approx. 1.5 days out of 1.4Ah
                // and then another 4 days at 40%
                // and then another 6 days at 30%
                // and then another 9 days at 10% duty cycle
                // and then another 20 days at 0.1% duty cycle
                duty_cycle_ = 880; // 88%
            }
            // add task to allow sleeping again
            timer_->set_timer<iSenseCollectorApp, &iSenseCollectorApp::read_solar_sensors > (duty_cycle_ * 60000, this, (void*) TASK_SLEEP);
            send_reading(0xffff, "batterycharge", bs.capacity);
            send_reading(0xffff, "batteryvoltage", bs.voltage);
            send_reading(0xffff, "dutycycle", duty_cycle_);
            send_reading(0xffff, "batterycurrent", bs.current);

            // output battery state and duty cycle
            //            os().debug("voltage=%dmV, charge=%iuAh -> duty cycle=%d, current=%i",                    , , , );
        } else
            if ((uint32) userdata == TASK_SLEEP) {
            // allow sleeping again
            ((isense::Os *) ospointer)->allow_sleep(true);
            //            os().allow_sleep(true);
        }

#ifdef ENVIRONMENTAL_COLLECTOR
        read_environmental_sensors(0);
#endif

    }
#endif

#ifdef SECURITY_COLLECTOR

    /**
     * Initializes the Security Sensor Module
     * @param value pointer to os
     */
    void init_security_module(Os::AppMainParameter & value) {
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
#endif

    /**
     * Broadcasts the Gateway Beacon
     * Used from Gateway nodes to let other devices know him 
     * forward messages to the backend through him
     * @param userdata unused, required for wiselib
     */
    void broadcast_gateway(void* userdata) {
        //        debug_->debug("broadcast_gateway");
        broadcastMsg_t msg;
        radio_->send(0xffff, msg.length(), (block_data_t*) & msg);
        timer_->set_timer<iSenseCollectorApp, &iSenseCollectorApp::broadcast_gateway > (20 * 1000, this, (void*) 0);
    }

protected:

    /**
     * Handles a new Pir Event
     * Reports the Reading to the Gateway
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
     * Handles a new Neighborhood Event
     * @param event event type {NB,NBB,NBL,NBD}
     * @param from the node_id of the other side of the link
     * @param len unused, concerns the beacon payloads
     * @param data unused, concerns the beacon payloads
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
     * Handle a new Uart Event
     * Used to forward Commands from Uberdust to the WSN
     * @param len the length of the payload and node id to forward the command
     * @param mess the buffer containing the payload and the node id as {Node_id,payload}
     */
    void handle_uart_msg(Os::Uart::size_t len, Os::Uart::block_data_t * mess) {
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
     * Handle a new Received Message
     * @param src_addr the source of the message
     * @param len the length of the message
     * @param buf the data of the message
     */
    void receive(node_id_t src_addr, Os::TxRadio::size_t len, block_data_t * buf) {
        //        debug_payload((uint8_t*) buf, len, src_addr);

#ifdef CORE_COLLECTOR
        //check if an actuation command
        if (check_led(src_addr, len, buf)) return;
#endif


        if (!is_gateway()) {
            //if not a gateway check only for a GatewayBeaconMsg
            if (check_gateway(src_addr, len, buf)) return;
        } else {
            //if a gateway check the message for readings to report

            //check for the air quality sensor
            if (check_air_quality(src_addr, len, buf)) {
                debug_->debug("check_air_quality");
                return;
            }

            //check for a CollectorMsg
            check_collector(src_addr, len, buf);
        }
    }
#ifdef CORE_COLLECTOR

    /**
     * Checks the incoming payload for a Command message.
     * If the Payload is a Command message, swithces the led according to the message
     * @param src_addr the source of the message
     * @param len the length of the payload
     * @param buf the contents of the payload
     * @return true if it was the Command message
     */
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
#endif

    /**
     * Checks the incoming payload for a GatewayBeaconMsg.
     * If the Payload is a GatewayBeaconMsg, replaces the old gateway with the new one
     * @param src_addr the source of the message
     * @param len the length of the payload
     * @param buf the contents of the payload
     * @return true if it was the GatewayBeaconMsg
     */
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

    /**
     * Checks the incoming payload for an AirQualityMsg.
     * If the Payload is a AirQualityMsg, and reports it to the backend
     * @param src_addr the source of the message
     * @param len the length of the payload
     * @param buf the contents of the payload
     * @return true if it was the AirQualityMsg
     */
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

    /**
     * Checks the incoming payload for a CollectorMsg.
     * If the Payload is a CollectorMsg, and reports it to the backend
     * @param src_addr the source of the message
     * @param len the length of the payload
     * @param buf the contents of the payload
     * @return true if it was the CollectorMsg
     */
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
#ifdef ENVIRONMENTAL_COLLECTOR
    isense::EnvironmentModule* em_;
#endif
#ifdef SECURITY_COLLECTOR
    isense::PirSensor* pir_;
    //    isense::LisAccelerometer* accelerometer_;
#endif
#ifdef WEATHER_COLLECTOR
    isense::Ms55xx* ms_;
#endif
#ifdef SOLAR_COLLECTOR
    isense::SolarModule* sm_;
    uint16_t duty_cycle_;
#endif 
#ifdef CORE_COLLECTOR
    isense::CoreModule* cm_;
#endif
    Os::TxRadio::self_pointer_t radio_;
    Os::Timer::self_pointer_t timer_;
    Os::Debug::self_pointer_t debug_;
    Os::Uart::self_pointer_t uart_;
    Os::Clock::self_pointer_t clock_;

};

wiselib::WiselibApplication<Os, iSenseCollectorApp> application;
// --------------------------------------------------------------------------

void application_main(Os::AppMainParameter& value) {
    application.init(value);
}
