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
  unsigned long timestamp;
  CurrentSensor (char * name,   EnergyMonitor *monitor): 
  CoapSensor(name)
  {    
    timestamp = 0;
    this->monitor = monitor;
    monitor->calcIrms(1480);  // Calculate Irms only
    this->status=0.0;
    this->set_notify_time(30);
  }

  void get_value( uint8_t* output_data, size_t* output_data_len)
  {
    int val=monitor->calcIrms(1430)*1000;
    *output_data_len = sprintf( (char*)output_data, "%d", this->status); 
  }

  void check(void)
  {
    if(millis() - timestamp > 5000)
    {
      timestamp = millis();
      int val=monitor->calcIrms(1430)*1000;
      this->status = val;  // Calculate Irms only
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