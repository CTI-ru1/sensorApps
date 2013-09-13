#include <CoapSensor.h>
#ifndef __SENSOR_H_
#define __SENSOR_H_
//extern int Sen;
class Sensor : 
public CoapSensor {
public:
  int pin, status;
  long lastCheck,everyCheck;
  float maximo;
  float minimo;

  Sensor(char * name, int pin): 
  CoapSensor(name,60) {
    this->pin = pin;
    this->status=10;
    this->lastCheck=0;
    this->maximo = 0;
    this->minimo=40;
  }

  void get_value(uint8_t* output_data, size_t* output_data_len) {
    my_check();
    *output_data_len = sprintf((char*) output_data, "%d", this->status);
  }
  int value(){
    return this->status;
  }



      void my_check(void) {
	
       // In every loop read the value of Current sensor and keep some status 
        float valor=0;
    float maximo = 0;
    float minimo=40;
    float Amplitudpp=0;
    float I=0;
    int A=0;
    unsigned long contador=millis();
    while((millis()-contador < 250)) //Read the value during one periodo of the Analogic signal for can find maximum and minimum
    {
        valor=0.026*analogRead(this->pin);
        maximo=max(valor,maximo);
        minimo=min(valor,minimo);
    }
    //Serial.println(maximo);
    //Serial.println(minimo);
    Amplitudpp= maximo-minimo;
    //Serial.println(Amplitudpp);
    I=(Amplitudpp/5.6568)*1000;//Equation that extract the Irms from Ipp (Irms=Ip/2sqrt(2))
    //Serial.println(I);
    A=int(I);
        //&Sen=this.status;
    this->status= A;
   } 
};
#endif







