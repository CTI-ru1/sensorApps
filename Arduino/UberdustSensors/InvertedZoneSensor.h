#include <CoapSensor.h>
#include <EEPROM.h>

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
    pinMode(this->pin, OUTPUT);
    set(EEPROM.read(100+this->pin)>0?0:1);
    set_notify_time(30);
  }
  void get_value( uint8_t* output_data, size_t* output_data_len)
  {
    //this->status = (1-digitalRead(this->pin));
    *output_data_len = sprintf((char *)output_data,"%d",1-this->status);
  }
  void set_value(uint8_t* input_data, size_t input_data_len, uint8_t* output_data, size_t* output_data_len)
  {
    set(*input_data-0x30);
    this->changed = true;
    get_value(output_data,output_data_len);
  }
  inline void set(uint8_t value)
  {
    this->status = 1-value;
    EEPROM.write(100+pin,this->status);
    digitalWrite(pin, this->status);
  }
};
