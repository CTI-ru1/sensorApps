#include <CoapSensor.h>

class RandomSensor : 
public CoapSensor 
{
public:
  RandomSensor(char * name,int limit):   
  CoapSensor(name)
  {  
      this->limit = limit;
      this->set_notify_time(10);
  }
  void get_value( uint8_t* output_data, size_t* output_data_len)
  {
    *output_data_len = sprintf( (char*)output_data, "%d", rand()%limit); 
  }
  uint16_t parent_;
  int limit;
};
