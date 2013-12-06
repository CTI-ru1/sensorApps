#include <CoapSensor.h>
#include "CurrentSensor.h"

class DescriptionSensor : 
public CoapSensor 
{
public:
  char status[40];
  DescriptionSensor(char * name, char * status): 
  CoapSensor(name)
  {    
    strcpy(this->status,status);
    this->set_notify_time(60);
  }
  void get_value( uint8_t* output_data, size_t* output_data_len)
  {
    *output_data_len = sprintf( (char*)output_data, "%s",this->status); 
  }
};
