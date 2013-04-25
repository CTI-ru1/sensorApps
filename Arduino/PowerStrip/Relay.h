#include <CoapSensor.h>
#include "Sensor.h"
class Relay : public CoapSensor
{

public:
int pin, status;
Sensor * sensor;
  Relay(String name,int pin, Sensor * sensor): CoapSensor(name)
  {
    this->pin = pin;
    pinMode(pin, OUTPUT);
    this->status = 0;
    this->sensor = sensor;
  }
  void get_value( uint8_t* output_data, size_t* output_data_len)
  {
    *output_data_len = sprintf( (char*)output_data, "%d", this->status );
  }
  void set_value(uint8_t* input_data, size_t input_data_len, uint8_t* output_data, size_t* output_data_len)
  {
    this->set(*input_data-0x30);
    output_data[0] = 0x30 + status;
    *output_data_len = 1;
    this->sensor->check();
  }
  void set(uint8_t value)
  {
    this->status = value;
    //Rel->state=value;
  }
  void check(){
    if (this->status==HIGH){
      this->status = HIGH;
    }
    if(this->status==LOW){
      this->status=LOW;
    } 
    digitalWrite(this->pin,this->status);
  }

};
