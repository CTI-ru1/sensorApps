/************************************************************************
 ** This file is part of the the iSense project.
 ** Copyright (C) 2006 coalesenses GmbH (http://www.coalesenses.com)
 ** ALL RIGHTS RESERVED.
 ************************************************************************/
#include <isense/application.h>
#include <isense/os.h>
#include <isense/dispatcher.h>
#include <isense/radio.h>
#include <isense/task.h>
#include <isense/timeout_handler.h>
#include <isense/isense.h>
#include <isense/uart.h>
#include <isense/dispatcher.h>
#include <isense/time.h>
#include <isense/button_handler.h>
#include <isense/sleep_handler.h>
#include <isense/modules/pacemate_module/pacemate_module.h>
#include <isense/util/util.h>
#include "external_interface/isense/isense_os.h"
#include "external_interface/isense/isense_radio.h"
#include "external_interface/isense/isense_timer.h"
#include "external_interface/isense/isense_debug.h"
typedef wiselib::iSenseOsModel WiselibOs;
#define USE_ECHO
#ifdef USE_ECHO
#include "algorithms/neighbor_discovery/echo.h"
typedef wiselib::Echo<WiselibOs, WiselibOs::TxRadio, WiselibOs::Timer, WiselibOs::Debug> nb_t;
#endif

#define TASK_READ_SENSORS 2

#define MILLISECONDS 1000
#define READING_INTERVAL 15
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
	public isense::SleepHandler,
	public ButtonHandler
{
public:
	iSenseDemoApplication(isense::Os& os);
	
	virtual ~iSenseDemoApplication() ;
	
	///From isense::Application
	virtual void boot (void) ;

	///From isense::SleepHandler
	virtual bool stand_by (void) ; 	// Memory held

	///From isense::SleepHandler
	virtual bool hibernate (void) ;  // Memory not held

	///From isense::SleepHandler
	virtual void wake_up (bool memory_held) ;

	///From isense::ButtonHandler
	virtual void button_down( uint8 button );	
	
	///From isense::Receiver
	virtual void receive (uint8 len, const uint8 * buf, ISENSE_RADIO_ADDR_TYPE src_addr, ISENSE_RADIO_ADDR_TYPE dest_addr, uint16 signal_strength, uint16 signal_quality, uint8 seq_no, uint8 interface, Time rx_time) ;
	
	///From isense::Sender 
	virtual void confirm (uint8 state, uint8 tries, isense::Time time) ;

	///From isense::Task
	virtual void execute( void* userdata ) ;

	///From isense::TimeoutHandler
	virtual void timeout( void* userdata ) ;

#ifdef USE_ECHO

    void ND_callback(uint8 event, uint16 from, uint8 len, uint8 * data) {
                os().debug("nd-call");
	/*
        if (event == nb_t::NEW_NB_BIDI) {
            if (!is_gateway()) {
                uint16 id1, id2;
                id1 = os().id();
                id2 = from;
                collectorMsg_t mess;
                mess.set_collector_type_id(collectorMsg_t::LINK_UP);
                mess.set_link(id1, id2);

                os().radio().send(mygateway_, mess.buffer_size(), (uint8*) & mess, 0, 0);
            } else {
                os().debug("id::%x LINK_UP %x ", os().id(), from);

            }
        } else if ((event == nb_t::LOST_NB_BIDI) || (event == nb_t::DROPPED_NB)) {
            if (!is_gateway()) {
                uint16 id1, id2;
                id1 = os().id();
                id2 = from;
                collectorMsg_t mess;
                mess.set_collector_type_id(collectorMsg_t::LINK_DOWN);
                mess.set_link(id1, id2);

                os().radio().send(mygateway_, mess.buffer_size(), (uint8*) & mess, 0, 0);
            } else {
                os().debug("id::%x LINK_DOWN %x ", os().id(), from);
            }
        }
	*/
    }
#endif


private:

    bool is_gateway() {
        switch (os().id()) {
            case 0x6699: //2.3
            case 0x0498: //2.1
            case 0x1b7f: //3.3
            case 0x1ccd: //0.1
            case 0xc7a: //0.2
            case 0x99ad: //3,1
            case 0x8978: //1.1
                return true;
            default:
                return false;
        }
        return true;
    }


    WiselibOs::TxRadio radio_;
    WiselibOs::Debug debug_;
    WiselibOs::Clock clock_;
    WiselibOs::Timer timer_;


    int channel;
#ifdef USE_ECHO
    nb_t nb_;
#endif


};

//----------------------------------------------------------------------------
iSenseDemoApplication::
	iSenseDemoApplication(isense::Os& os)
	: isense::Application(os),
	radio_(os),
	debug_(os),
	clock_(os),
	timer_(os)
	{
	}

//----------------------------------------------------------------------------
iSenseDemoApplication::
	~iSenseDemoApplication()
		
	{
	}

//----------------------------------------------------------------------------
void 
	iSenseDemoApplication::
	boot(void)
	{
	os_.debug("ON-%d", READING_INTERVAL);
        os_.allow_sleep(false);

#ifdef USE_ECHO
	nb_.init(radio_, clock_, timer_, debug_, 2000, 16000, 190, 210);
        nb_.enable();
        uint8_t flags = nb_t::LOST_NB_BIDI | nb_t::DROPPED_NB;
        nb_.reg_event_callback<iSenseDemoApplication, &iSenseDemoApplication::ND_callback > ((uint8) CONTROLL, flags, this);

#endif


        os_.add_task_in(Time(READING_INTERVAL*MILLISECONDS), this, (void *)TASK_READ_SENSORS);
 	}
	
//----------------------------------------------------------------------------
bool 
	iSenseDemoApplication::
	stand_by (void)
	{
		os_.debug("App::sleep");
		return true;
	}
	
//----------------------------------------------------------------------------
bool 
	iSenseDemoApplication::
	hibernate (void)
	{
		os_.debug("App::hibernate");
		return false;
	}
	
//----------------------------------------------------------------------------
void 
	iSenseDemoApplication::
	wake_up (bool memory_held)	
	{
		os_.debug("App::Wakeup");
	}

void
	iSenseDemoApplication::
	button_down( uint8 button )
	{
		
	}
//----------------------------------------------------------------------------
void 
	iSenseDemoApplication::
	execute( void* userdata )
	{
	if (userdata==(void*)TASK_READ_SENSORS){
		os_.add_task_in(Time(READING_INTERVAL*MILLISECONDS), this, (void *)TASK_READ_SENSORS);
		os_.debug("TASK_READ_SENSORS");

	}
	}

//----------------------------------------------------------------------------
void 
	iSenseDemoApplication::
	receive (uint8 len, const uint8 * buf, ISENSE_RADIO_ADDR_TYPE src_addr, ISENSE_RADIO_ADDR_TYPE dest_addr, uint16 signal_strength, uint16 signal_quality, uint8 seq_no, uint8 interface, Time rx_time)
	{
	}
	
//----------------------------------------------------------------------------
void 
	iSenseDemoApplication::
	confirm (uint8 state, uint8 tries, isense::Time time)
	{
	}

//----------------------------------------------------------------------------
void 
	iSenseDemoApplication::
	timeout( void* userdata )
	{
		//os_.add_task( this, NULL);
		//os_.add_timeout_in(Time(MILLISECONDS), this, NULL);	 
	}

//----------------------------------------------------------------------------
/**
  */	
isense::Application* application_factory(isense::Os& os)
{
	return new iSenseDemoApplication(os);
}


/*-----------------------------------------------------------------------
* Source  $Source: $
* Version $Revision: 1.24 $
* Date    $Date: 2006/10/19 12:37:49 $
*-----------------------------------------------------------------------
* $Log$
*-----------------------------------------------------------------------*/
