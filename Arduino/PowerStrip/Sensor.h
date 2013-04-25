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

  Sensor(String name, int pin): 
  CoapSensor(name) {
    this->pin = pin;
    this->status=10;
    this->lastCheck=0;
    this->maximo = 0;
    this->minimo=40;
  }

  void get_value(uint8_t* output_data, size_t* output_data_len) {
    *output_data_len = sprintf((char*) output_data, "%d", this->status);
  }
  int value(){
    return this->status;
  }



  void check(void) {
    // In every loop read the value of Current sensor and keep some status 
    float valor=0;
    float Amplitudpp=0;
    float I=0;
    int A=0;
    unsigned long contador=millis();
    valor=0.026*analogRead(this->pin);
    maximo=max(valor,maximo);
    minimo=min(valor,minimo);
    //Serial.println(maximo);
    //Serial.println(minimo);
    Amplitudpp= maximo-minimo;
    //Serial.println(Amplitudpp);
    I=(Amplitudpp/5.6568)*1000;//Equation that extract the Irms from Ipp (Irms=Ip/2sqrt(2))
    //Serial.println(I);
    A=int(I);
    //&Sen=this.status;
    this->status= A;
    if(millis() - this->lastCheck > 250){
      maximo = 0;
      minimo=40;
      this->lastCheck =millis();
    }
  }    
};
#endif







