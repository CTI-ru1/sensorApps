#include <CoapSensor.h>

class lightSensor : 
public CoapSensor 
{
public:
  int pin, status;
  lightSensor(char * name, int pin): 
  CoapSensor(name)
  {
    this->pin = pin;
    this->status = 0;
    this->set_notify_time(180);
  }
  void get_value( uint8_t* output_data, size_t* output_data_len)
  {
    *output_data_len = sprintf( (char*)output_data, "%d", this->status ); 
  }
  void check(void)
  {
    static unsigned long timestamp = 0;
    if(millis() - timestamp > 5000)
    {
      int newStatus = analogRead(this->pin);  // read the value from the sensor
      this->status = newStatus;
      timestamp = millis();
    }
  }
};
