#include <CoapSensor.h>

class microphoneSensor: 
public CoapSensor 
{
public:
  int pin, status;
  microphoneSensor(char * name, int pin): 
  CoapSensor(name)
  {
    this->pin = pin;
    this->status = 0;
    this->set_notify_time(10);
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
      int newStatus = abs(analogRead(this->pin)-760);  // read the value from the sensor
      this->status = newStatus;
      timestamp = millis();
    }
  }
};
