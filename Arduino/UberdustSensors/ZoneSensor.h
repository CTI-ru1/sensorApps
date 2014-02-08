#include <CoapSensor.h>

class zoneSensor : 
public CoapSensor 
{
public:
  zoneSensor(): 
  CoapSensor()
  {
  }

  zoneSensor(char * name, int pin): 
  CoapSensor(name,30)
  {
    this->pin = pin;
    this->status = 0;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
  void get_value( uint8_t* output_data, size_t* output_data_len)
  {
    //this->status = digitalRead(this->pin);
    send_value(this->status,output_data,output_data_len);
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
    digitalWrite(pin, status);
  }
};
