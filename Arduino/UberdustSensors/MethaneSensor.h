#include <CoapSensor.h>

class methaneSensor : public CoapSensor 
{
  public:
    int pin, status;
  methaneSensor(char * name, int pin): CoapSensor(name)
  {
    this->pin = pin;
    this->status = LOW;
  }
  void get_value( uint8_t* output_data, size_t* output_data_len)
  {
    *output_data_len = sprintf( (char*)output_data, "%d", this->status ); 
  }
  void check(void)
  {
    static unsigned long timestamp = 0;
    if(millis() - timestamp > 150000)
    {
      this->status = analogRead(pin);  // read the value from the sensor
      this->changed = true;
      timestamp = millis();
    }
  }
};

