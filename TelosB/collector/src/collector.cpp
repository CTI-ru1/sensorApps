/************************************************************************
 ** This file is part of the the iSense project.
 ** Copyright (C) 2006 coalesenses GmbH (http://www.coalesenses.com)
 ** ALL RIGHTS RESERVED.
 ************************************************************************/
#include <isense/application.h>
#include <isense/os.h>
#include <isense/dispatcher.h>
#include <isense/radio.h>
#include <isense/hardware_radio.h>
#include <isense/task.h>
#include <isense/timeout_handler.h>
#include <isense/isense.h>
#include <isense/uart.h>
#include <isense/time.h>
#include <isense/timer.h>
#include <isense/button_handler.h>
#include <isense/sleep_handler.h>
#include <isense/util/util.h>
#include <isense/modules/telosb_module/telosb_module.h>

#include <isense/platforms/msp430/msp430_os.h>
#include <isense/platforms/msp430/msp430_macros.h>

#include <io.h>
#include <signal.h>

//----------------------------------------------------------------------------
/**
 */

using namespace isense;

/**
 * An application that collects periodically readings
 * from 
 * <a href='http://www.willow.co.uk/html/telosb_mote_platform.html'>
 * TelosB : Light,Infrared,Humidity and Temperature sensors </a>
 * and reports them to the 
 * <a href='https://github.com/organizations/Uberdust'> Uberdust Backend</a>
 */
class TelosBCollectorApp :
public isense::Application,
public isense::Receiver,
public isense::Sender,
public isense::Task,
public isense::TimeoutHandler,
public isense::ButtonHandler,
public isense::UartPacketHandler {
public:
    /**
     * Constructor
     * @param os a pointer to the os
     */
    TelosBCollectorApp(isense::Os& os);

    /**
     * Destructor
     */
    ~TelosBCollectorApp();

    /**
     * Boot function executed when device is powered
     */
    void boot(void);

    /**
     * Executed when a button is pressed
     * @param button the button pressed
     */
    void button_down(uint8 button);

    /**
     * Executed when a button is released
     * @param button the button released
     */
    void button_up(uint8 button);

    /**
     * Called when a new message is received.
     * @param len length of the payload received
     * @param buf a buffer containing the payload
     * @param src_addr the source of the payload
     * @param dest_addr the destination of the payload
     * @param signal_strength the strenght of the received signal
     * @param signal_quality the quality of the received signal
     * @param seq_no sequence number of the message
     * @param interface id of the radio received from
     * @param time the time of the receive event
     */
    void receive(uint8 len, const uint8 * buf, ISENSE_RADIO_ADDR_TYPE src_addr, ISENSE_RADIO_ADDR_TYPE dest_addr, uint16 signal_strength, uint16 signal_quality, uint8 seq_no, uint8 interface, Time time);

    void confirm(uint8 state, uint8 tries, isense::Time time);

    /**
     * Executed preriodically to report new readings to the backend
     * @param userdata unused
     */
    void execute(void* userdata);

    /**
     * unused
     * @param userdata unused
     */
    void timeout(void* userdata);

    /**
     * Handler for uart messages
     * @param type the type of the message
     * @param buf the buffer containing the data
     * @param length the size of the payload
     */
    void handle_uart_packet(uint8 type, uint8* buf, uint8 length);

private:
    TelosbModule *telos;
};

//----------------------------------------------------------------------------

TelosBCollectorApp::
TelosBCollectorApp(isense::Os& os)
: isense::Application(os),
telos(NULL) {
}

//----------------------------------------------------------------------------

TelosBCollectorApp::
~TelosBCollectorApp() {
}

//----------------------------------------------------------------------------

void
TelosBCollectorApp::
handle_uart_packet(uint8 type, uint8 * mess, uint8 length) {

}

//----------------------------------------------------------------------------

void
TelosBCollectorApp::
boot(void) {
    os().debug("App::boot ");

    os().allow_doze(false);
    os().allow_sleep(false);

    telos = new TelosbModule(os_);
    telos->init();
    telos->led_on(1);

    os().debug("my id is %x", os().id());

    //    telos->add_button_handler(this);

    os().add_task_in(Time(5, 0), this, NULL);

    //    os().uart(1).set_packet_handler(11, this);

    //    os().dispatcher().add_receiver(this);

    telos->led_off(1);
}

//----------------------------------------------------------------------------

void
TelosBCollectorApp::
button_down(uint8 button) {
    telos->led_on(0);
    os().debug("BUTTON");
}

void
TelosBCollectorApp::
button_up(uint8 button) {
    telos->led_off(0);
}

//----------------------------------------------------------------------------

void
TelosBCollectorApp::
execute(void* userdata) {
    telos->led_on(1);

    os().add_task_in(Time(60, 0), this, NULL);
    int16 temp = telos->temperature();
    int8 humid = telos->humidity();
    int16 light = telos->light();
    int16 inflight = telos->infrared();

    os().debug("node::%x temperature %d ", os().id(), temp / 10);
    os().debug("node::%x humidity %d ", os().id(), humid);
    os().debug("node::%x ir %d ", os().id(), inflight);
    os().debug("node::%x light %d ", os().id(), light);

    telos->led_off(1);

}

//----------------------------------------------------------------------------

void
TelosBCollectorApp::
receive(uint8 len, const uint8* buf, ISENSE_RADIO_ADDR_TYPE src_addr, ISENSE_RADIO_ADDR_TYPE dest_addr, uint16 signal_strength, uint16 signal_quality, uint8 seq_no, uint8 interface, Time time) {
}

//----------------------------------------------------------------------------

void
TelosBCollectorApp::
confirm(uint8 state, uint8 tries, isense::Time time) {
}

//----------------------------------------------------------------------------

void
TelosBCollectorApp::
timeout(void* userdata) {
}

//----------------------------------------------------------------------------

/**
 */
isense::Application * application_factory(isense::Os & os) {
    return new TelosBCollectorApp(os);
}


/*-----------------------------------------------------------------------
 * Source  $Source: $
 * Version $Revision: 1.24 $
 * Date    $Date: 2006/10/19 12:37:49 $
 *-----------------------------------------------------------------------
 * $Log$
 *-----------------------------------------------------------------------*/
