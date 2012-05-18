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

class iSenseDemoApplication :
public isense::Application,
public isense::Receiver,
public isense::Sender,
public isense::Task,
public isense::TimeoutHandler,
public isense::ButtonHandler,
public isense::UartPacketHandler {
public:
    iSenseDemoApplication(isense::Os& os);

    ~iSenseDemoApplication();

    ///From isense::Application
    void boot(void);

    ///From isense::ButtonHandler
    void button_down(uint8 button);

    ///From isense::ButtonHandler
    void button_up(uint8 button);

    ///From isense::Receiver
    void receive(uint8 len, const uint8 * buf, ISENSE_RADIO_ADDR_TYPE src_addr, ISENSE_RADIO_ADDR_TYPE dest_addr, uint16 signal_strength, uint16 signal_quality, uint8 seq_no, uint8 interface, Time time);

    ///From isense::Sender
    void confirm(uint8 state, uint8 tries, isense::Time time);

    ///From isense::Task
    void execute(void* userdata);

    ///From isense::TimeoutHandler
    void timeout(void* userdata);

    ///From isense::UartPacketHandler
    void handle_uart_packet(uint8 type, uint8* buf, uint8 length);

private:
    TelosbModule *telos;
};

//----------------------------------------------------------------------------

iSenseDemoApplication::
iSenseDemoApplication(isense::Os& os)
: isense::Application(os),
telos(NULL) {
}

//----------------------------------------------------------------------------

iSenseDemoApplication::
~iSenseDemoApplication() {
}

//----------------------------------------------------------------------------

void
iSenseDemoApplication::
handle_uart_packet(uint8 type, uint8 * mess, uint8 length) {

}

//----------------------------------------------------------------------------

void
iSenseDemoApplication::
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
iSenseDemoApplication::
button_down(uint8 button) {
    telos->led_on(0);
    os().debug("BUTTON");
}

void
iSenseDemoApplication::
button_up(uint8 button) {
    telos->led_off(0);
}

//----------------------------------------------------------------------------

void
iSenseDemoApplication::
execute(void* userdata) {
    telos->led_on(1);

    os().add_task_in(Time(60, 0), this, NULL);
    int16 temp = telos->temperature();
    int8 humid = telos->humidity();
    int16 light = telos->light();
    int16 inflight = telos->infrared();

    os().debug("node::%x temperature %d ", os().id(), temp/10);
    os().debug("node::%x humidity %d ", os().id(), humid);
    os().debug("node::%x ir %d ", os().id(), inflight);
    os().debug("node::%x light %d ", os().id(), light);

    telos->led_off(1);

}

//----------------------------------------------------------------------------

void
iSenseDemoApplication::
receive(uint8 len, const uint8* buf, ISENSE_RADIO_ADDR_TYPE src_addr, ISENSE_RADIO_ADDR_TYPE dest_addr, uint16 signal_strength, uint16 signal_quality, uint8 seq_no, uint8 interface, Time time) {
}

//----------------------------------------------------------------------------

void
iSenseDemoApplication::
confirm(uint8 state, uint8 tries, isense::Time time) {
}

//----------------------------------------------------------------------------

void
iSenseDemoApplication::
timeout(void* userdata) {
}

//----------------------------------------------------------------------------

/**
 */
isense::Application * application_factory(isense::Os & os) {
    return new iSenseDemoApplication(os);
}


/*-----------------------------------------------------------------------
 * Source  $Source: $
 * Version $Revision: 1.24 $
 * Date    $Date: 2006/10/19 12:37:49 $
 *-----------------------------------------------------------------------
 * $Log$
 *-----------------------------------------------------------------------*/
