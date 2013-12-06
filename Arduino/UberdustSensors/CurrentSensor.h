#ifndef CURRENT_SENSOR
#define CURRENT_SENSOR

#include <CoapSensor.h>
#include <EmonLib.h>                   // Include Emon Library

class CurrentSensor: 
public CoapSensor 
{
public:
  int status;
  EnergyMonitor *monitor;
  CurrentSensor (char * name,   EnergyMonitor *monitor): 
  CoapSensor(name)
  {    
    this->monitor = monitor;
    this->status=0.0;
    this->set_notify_time(30);
  }

  void get_value( uint8_t* output_data, size_t* output_data_len)
  {
    *output_data_len = sprintf( (char*)output_data, "%d", this->status); 
  }

  void check(void)
  {
    static unsigned long timestamp = 0;
    if(millis() - timestamp > 5000)
    {
      timestamp = millis();
      this->status = monitor->calcIrms(1480)*1000;  // Calculate Irms only
      if (this->status<100){
	this->status=0;
      }
    }
  }

  int current(){
    return this->status;
  }
};
#endif