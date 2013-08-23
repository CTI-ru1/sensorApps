#include <CoapSensor.h>

class zoneSensor : 
public CoapSensor 
{
public:
  int pin, status;
  zoneSensor(): 
  CoapSensor()
  {
  }

  zoneSensor(char * name, int pin): 
  CoapSensor(name)
  {
    this->pin = pin;
    this->status = 0;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    this->set_notify_time(300);
  }
  void get_value( uint8_t* output_data, size_t* output_data_len)
  {
    this->status = digitalRead(this->pin);
    if (this->status){
      strcpy((char *)output_data,"1");
    }
    else{
      strcpy((char *)output_data,"0");
    }
    *output_data_len = 1; 
  }
  void set_value(uint8_t* input_data, size_t input_data_len, uint8_t* output_data, size_t* output_data_len)
  {
    this->set(*input_data-0x30);
    this->changed = true;
    output_data[0] = *input_data;
    *output_data_len = 1;
  }
  inline void set(uint8_t value)
  {
    this->status = value;
    digitalWrite(pin, status);
  }
};
