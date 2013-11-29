#include <CoapSensor.h>

class pirSensor : 
public CoapSensor 
{
public:
  int pin, status;
  pirSensor(char * name, int pin): 
  CoapSensor(name)
  {
    this->pin = pin;
    this->status = LOW;
    pinMode(pin, INPUT);
    this->set_notify_time(30);

  }
  void get_value( uint8_t* output_data, size_t* output_data_len)
  {
    *output_data_len = sprintf( (char*)output_data, "%d", this->status ); 
  }

  void check(void)
  {
    static unsigned long timestamp = 0;
    if(millis() - timestamp > 1000)
    {
      timestamp = millis();
      int newStatus = !digitalRead(this->pin); // read the value from the sensor
      if(newStatus != this->status)
      {
        this->status = newStatus;
	this->changed = true;
      }
    }
}
};
