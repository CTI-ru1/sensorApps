#include <CoapSensor.h>

class memorySensor : 
public CoapSensor 
{
public:
  memorySensor(char * name):   
  CoapSensor(name)
  {  
      this->set_notify_time(300);
  }
  void get_value( uint8_t* output_data, size_t* output_data_len)
  {
    *output_data_len = sprintf( (char*)output_data, "%d", this->freeRam()); 
  }
  int freeRam () {
    extern int __heap_start, *__brkval; 
    int v; 
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
  }
};



