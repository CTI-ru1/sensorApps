#include <CoapSensor.h>

class temperatureSensor : 
public CoapSensor 
{
public:
  int pin, status;
  temperatureSensor(char * name, int pin): 
  CoapSensor(name)
  {
    this->pin = pin;
    this->status = 0;
    this->set_notify_time(300);
  }
  void get_value( uint8_t* output_data, size_t* output_data_len)
  {

    *output_data_len = sprintf( (char*)output_data, "%d", this->status ); 
  }
  void check(void)
  {
    static unsigned long timestamp = 0;
    if(millis() - timestamp > 500)
    {
      int newStatus = analogRead(this->pin)*100;  // read the value from the sensor

      newStatus = ((newStatus/1024)* 5000)/10/100;  
      if(newStatus != this->status)
      {
        this->status = newStatus;
      }
      timestamp = millis();
    }
  }
};
