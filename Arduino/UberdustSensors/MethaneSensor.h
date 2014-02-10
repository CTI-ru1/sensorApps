#include <CoapSensor.h>

class methaneSensor : public CoapSensor 
{
  public:
    int pin, status;
  methaneSensor(char * name, int pin): CoapSensor(name)
  {
    this->pin = pin;
    this->status = 0.0;
    this->set_notify_time(30);
  }
  void get_value( uint8_t* output_data, size_t* output_data_len)
  {
    *output_data_len = sprintf( (char*)output_data, "%d", this->status ); 
  }
  void check(void)
  {
    static unsigned long timestamp = 0;
   /* if(millis() - timestamp > 5000)
    {
      this->status = analogRead(pin);  // read the value from the sensor
      this->changed = true;
      timestamp = millis();
    }*/
	
	if(millis() - timestamp > 5000)
    	{
		//int vrl=(analogRead(this->pin)*5000)/1024;
      		//this->status = (5000-vrl)/vrl;  // read the value from the sensor
		this->status=analogRead(this->pin);     		
		this->changed = true;
      		timestamp = millis();
   	 }
	
  }
};

