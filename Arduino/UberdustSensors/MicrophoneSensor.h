#include <CoapSensor.h>
#include "Arduino.h"

class microphoneSensor: 
public CoapSensor 
{
public:
int pin;
int status;
int numberOfSample;
int value;
int maximum;
int minimum;
int sensorValue;
microphoneSensor(char * name, int pin): 
  CoapSensor(name)
  {
    this->pin = pin;
    this->status;
    this->numberOfSample=128;
    this->set_notify_time(10);
    this->maximum=1024; 
    this->minimum=0;
   this->value=0;
   this->sensorValue=0;
  }
  void get_value( uint8_t* output_data, size_t* output_data_len)
  {
    *output_data_len = sprintf( (char*)output_data, "%d", this->status ); 
  }
  void check(void)
  {
	static unsigned long timestamp2=0;
	static unsigned long timestamp = 0;
	    if(millis() - timestamp > 50)
	    {
		        value=analogRead(this->pin)-512;
			sensorValue = abs(value);
			if (sensorValue>maximum){maximum=sensorValue;}
		        //if (value<minimum){minimum=value;}
			//for(int i = 0; i < numberOfSample ; i++){
			//value=analogRead(this->pin);
			//if (value>maximum){maximum=value;}
                        //if (value<minimum){minimum=value;}
	  		//}
		int newStatus = maximum;
		this->status=newStatus;		
		timestamp = millis();
		}
	    if (millis()-timestamp2>10000){
		minimum = 1024;
		maximum = 0;
		timestamp2=millis();
		}

	    
	}
};

