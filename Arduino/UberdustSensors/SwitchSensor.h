#include <CoapSensor.h>

class switchSensor : 
public CoapSensor 
{
public:
  int pin, status;
  switchSensor(char * name, int pin): 
  CoapSensor(name)
  {
    this->pin = pin;
    pinMode(pin, INPUT);
    this->status = 0;
  }
  switchSensor(char *name, int pin, int pullup): 
  CoapSensor(name)
  {
    this->pin = pin;
    pinMode(pin, INPUT);
    digitalWrite(pin, pullup);
    this->status = 0;
    this->set_notify_time(300);
  }
  void get_value( uint8_t* output_data, size_t* output_data_len)
  {
    *output_data_len = sprintf( (char*)output_data, "%d", this->status ); 
  }

  void check()
  {
    int newStatus = digitalRead(this->pin);
    if(newStatus != this->status)
    {
      this->changed = true;
      this->status = newStatus;
    }
  }
};
