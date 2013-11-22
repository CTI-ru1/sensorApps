#include <CoapSensor.h>
#include "CurrentSensor.h"

class WattHourSensor : 
public CoapSensor 
{
public:
  double status[6];
  double interval_factor;
  double ctotal;
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
    this->interval_factor = ((double) report_interval)/3600;
  }
  void get_value( uint8_t* output_data, size_t* output_data_len)
  {
    unsigned long int_part = (unsigned long)this->ctotal;
    double remainder = this->ctotal - (double)int_part;
    int res_dec=remainder*1000;
    *output_data_len = sprintf( (char*)output_data, "%d.%d",int_part,res_dec); 
  }
  void check(void)
  {
    static unsigned long timestamp = 0;
    static unsigned long index = 0;
    if(millis() - timestamp > 5000)
    {
      timestamp = millis();
      double newStatus = monitor->current();  // Calculate Irms only
      this->status[index]=newStatus;
      index = (index+1)%6;
      this->ctotal=total();
    }
  }
  double total(){
    double current_total = 0.0;
    for (int i=0;i<6;i++){
      double partial_total =status[i];
      // P= I * V
      partial_total*=220;
      // move to mWH
      partial_total*=interval_factor;
      partial_total/=6;
      current_total+=partial_total;
    }
    //convert to WH
    return current_total/1000;
  }
};
