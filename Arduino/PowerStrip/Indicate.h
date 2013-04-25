#include <CoapSensor.h>
#include "Sensor.h"
//extern int SenA, SenB, SenC, SenD, SenE, SenF, SenG;
class Indicate : public CoapSensor {
public:
    int pin, status;
    Sensor *Sen ;

    Indicate(String name, int pin, Sensor *Sen) : CoapSensor(name) {
        this->pin = pin;
        pinMode(pin, OUTPUT);
	this->status = 0;
       this->Sen = Sen;
         //digitalWrite(this->pin,this->status);
        
    }

    void get_value(uint8_t* output_data, size_t* output_data_len) {
      check();
        *output_data_len = sprintf((char*) output_data, "%d", this->status);
    }
    void check(){
        if (this->Sen->value()>70){
	  this->status = HIGH;
          
	}else{
	  this->status=LOW;
	}	 
        //this->status = HIGH;
       digitalWrite(this->pin,this->status);
    }
};
