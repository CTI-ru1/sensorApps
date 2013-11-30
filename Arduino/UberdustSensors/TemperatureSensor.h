#include <CoapSensor.h>
#ifndef TEMPERATURESENSOR
#define TEMPERATURESENSOR 

class temperatureSensor : 
public CoapSensor 
{
public:
  int pin, status;
  temperatureSensor(char * name, int pin): 
  CoapSensor(name)
  {
    this->pin = pin;
    this->status = 0;
    this->set_notify_time(10);
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
  	//getting the voltage reading from the temperature sensor
 	int reading = analogRead(this->pin);  
 
 	// converting that reading to voltage, for 3.3v arduino use 3.3
 	float voltage = reading * 5.0;
 	voltage /= 1024.0; 

	 // now print out the temperature
 	float temperatureC = (voltage - 0.5) * 100 ;  //converting from 10 mv per degree wit 500 mV offset
                                               //to degrees ((voltage - 500mV) times 100)
 	int newStatus=temperatureC;
       if(newStatus != this->status)
      {
        this->status = newStatus;
      }
      timestamp = millis();
    }
  }
};
#endif
