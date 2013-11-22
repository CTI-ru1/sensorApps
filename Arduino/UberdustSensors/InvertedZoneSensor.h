#include <CoapSensor.h>

class InvertedZoneSensor : 
public CoapSensor 
{
public:
  int pin, status;
  InvertedZoneSensor(): 
  CoapSensor()
  {
  }

  InvertedZoneSensor(char * name, int pin): 
  CoapSensor(name)
  {
    this->pin = pin;
    this->status = 0;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
    this->set_notify_time(30);
  }
  void get_value( uint8_t* output_data, size_t* output_data_len)
  {
    this->status = (1-digitalRead(this->pin));
    *output_data_len = sprintf((char *)output_data,"%d",this->status);
  }
  void set_value(uint8_t* input_data, size_t input_data_len, uint8_t* output_data, size_t* output_data_len)
  {
    this->set(*input_data-0x30);
    this->changed = true;
    get_value(output_data,output_data_len);
  }
  inline void set(uint8_t value)
  {
    this->status = value;
    digitalWrite(pin, 1-status);
  }
};
