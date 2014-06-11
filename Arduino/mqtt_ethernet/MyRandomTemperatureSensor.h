#include <CoapSensor.h>

class MyRandomTemperatureSensor : 
public CoapSensor 
{
public:
  MyRandomTemperatureSensor(char * name,int value,int limit):   
  CoapSensor(name)
  {  
    this->limit = limit;      
    this->value = value;
    this->status = 0;
    this->set_notify_time(10);
    this->timestamp = 0;
  }
  void get_value( uint8_t* output_data, size_t* output_data_len)
  {
    *output_data_len = sprintf( (char*)output_data, "%d", this->status); 
  }
  void check(){
    
    if(millis() - timestamp > 5000)
    {
      this->status=value+rand()%limit;
    }
  }
  unsigned long timestamp;
  int limit;
  int value;
  int status;
};







