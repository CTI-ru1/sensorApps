#include <CoapSensor.h>

class RandomSensor : 
public CoapSensor 
{
public:
  RandomSensor(char * name,int limit):   
  CoapSensor(name)
  {  
      this->limit = limit;
      this->status = 0;
      this->set_notify_time(10);
  }
  void get_value( uint8_t* output_data, size_t* output_data_len)
  {
    *output_data_len = sprintf( (char*)output_data, "%d", this->status); 
  }
  void check(){
    static unsigned long timestamp = 0;
    if(millis() - timestamp > 5000)
    {
      this->status=rand()%limit;
    }
  }
  int limit;
  int status;
};
