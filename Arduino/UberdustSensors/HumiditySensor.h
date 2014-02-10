#include <CoapSensor.h>
#include <TemperatureSensor.h>
class humiditySensor: 
public CoapSensor 
{
public:
  int pin, status;
 temperatureSensor* temp;
  humiditySensor(char * name, int pin, temperatureSensor* temp): 
  CoapSensor(name)
  {
    this->temp=temp;
    this->pin = pin;
    this->status = 0;
    this->set_notify_time(30);
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
	//To properly caculate relative humidity, we need the temperature.
  	float temperature =this->temp->status ; //replace with a thermometer reading if you have it
  	//caculate relative humidity
 	 float supplyVolt = 5.0;

  	// read the value from the sensor:
  	int HIH4030_Value = analogRead(this->pin);
  	float voltage = HIH4030_Value/1023. * supplyVolt; // convert to voltage value

  	// convert the voltage to a relative humidity
  	// - the equation is derived from the HIH-4030/31 datasheet
 	 // - it is not calibrated to your individual sensor
 	 //  Table 2 of the sheet shows the may deviate from this line
 	 float sensorRH = 161.0 * voltage / supplyVolt - 25.8;
 	 float trueRH = sensorRH / (1.0546 - 0.0026 * temperature); //temperature adjustment 

	this->status = trueRH;
      timestamp = millis();
    }
  }
};

