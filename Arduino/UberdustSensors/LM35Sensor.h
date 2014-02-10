#include <CoapSensor.h>

class LM35Sensor : 
public CoapSensor 
{
public:
  int pin, status;

  LM35Sensor(char * name, int pin):
  CoapSensor(name)
  {
    this->pin = pin;
    this->status = 0;
    this->set_notify_time(180);
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
      //int newStatus = analogRead(this->pin)*100;  // read the value from the sensor

     // newStatus = ((newStatus/1024)* 5000)/10/100;  
	float temperature = analogRead(this->pin)*0.0049;
	temperature=temperature*100;  
	int newStatus=temperature;    

	if(newStatus != this->status)
      {
        this->status = newStatus;
      }
      timestamp = millis();
    }
  }
};
