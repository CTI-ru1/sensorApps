#include <CoapSensor.h>

class parentSensor : 
public CoapSensor 
{
public:
  parentSensor(char * name,BaseRouting* routing):   
  CoapSensor(name)
  {  
      this->routing = routing;
      this->set_notify_time(200);
  }
  void get_value( uint8_t* output_data, size_t* output_data_len)
  {
    *output_data_len = sprintf( (char*)output_data, "0x%x", routing->parent()); 
  }
  void check(){
    static unsigned long timestamp = 0;
    static uint16_t parent = 0;
    if(millis() - timestamp > 5000)
    {
      timestamp = millis();
      uint16_t newParent = routing->parent();
      if (parent!=newParent){
	parent=newParent;
	this->changed=true;
      }
    }
  }
  BaseRouting * routing;
};
