#include <CoapSensor.h>

class carbonSensor : public CoapSensor 
{
  public:
    int pin, heater_pin, status, heater;
  carbonSensor(char * name, int pin, int heater_pin): CoapSensor(name)
  {
    this->pin = pin;
    this->heater_pin = heater_pin;
    pinMode(heater_pin, OUTPUT);
    digitalWrite(heater_pin, LOW);
    this->status = LOW;
    this->heater = LOW;
  }
  void get_value( uint8_t* output_data, size_t* output_data_len)
  {
    *output_data_len = sprintf( (char*)output_data, "%d", this->status ); 
  }

  void check(void)
  {
    static unsigned long timestamp = 0;
    if(millis() - timestamp > 30000)
    {
      static unsigned int count = 0;
      if(count == 0)
      {
	analogWrite(heater_pin, 56);
      }
      else if(count == 3)
      {
	digitalWrite(heater_pin, HIGH);
      }
      else if(count == 5)
      {
	this->status = analogRead(pin);  // read the value from the sensor
	this->changed = true;
	count = -1;
      }
      count++;
      timestamp = millis();
    }
  }
};