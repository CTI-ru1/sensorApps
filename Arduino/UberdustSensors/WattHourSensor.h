#include <CoapSensor.h>
#include "CurrentSensor.h"

class WattHourSensor : 
public CoapSensor 
{
public:
  unsigned long status[6];
  unsigned long  interval_factor;
  unsigned long  ctotal;
  CurrentSensor * monitor;
  WattHourSensor(char * name, int report_interval,CurrentSensor* monitor): 
  CoapSensor(name)
  {    
    for (int i=0;i<6;i++){
	this->status[i]=0.0;
    }
    this->monitor=monitor;
    this->ctotal=0.0;
    this->set_notify_time(report_interval);
  }
  void get_value( uint8_t* output_data, size_t* output_data_len)
  {
    *output_data_len = sprintf( (char*)output_data, "%ld",this->ctotal); 
  }
  void check(void)
  {
    static unsigned long timestamp = 0;
    static unsigned long index = 0;
    if(millis() - timestamp > 5000)
    {
      timestamp = millis();
      double newStatus = (double)monitor->current();  // Calculate Irms only
      this->status[index]=newStatus;
      index = (index+1)%6;
      this->ctotal=total();
    }
  }
  double total(){
    unsigned long current_total = 0.0;
    for (int i=0;i<6;i++){
      if (status[i]<100)continue;
      unsigned long partial_total =status[i];
      // P= I * V
      partial_total*=220;
      // move to mWH
      partial_total*=5;
      partial_total/=3600;
      current_total+=partial_total;
    }
    //convert to WH
    return current_total;
  }
};
