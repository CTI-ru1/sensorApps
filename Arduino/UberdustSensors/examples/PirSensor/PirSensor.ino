#include <UberdustSensors.h>

pirSensor * sensor;

void setup() {
  Serial.begin(9600);
  sensor = new pirSensor("pir",2);
  delay(5000);
}
 
void loop() {

  sensor->check();

  Serial.println("==============");
  Serial.print(sensor->get_name());
  Serial.print("=");
  Serial.println(sensor->status);
  
  delay(5000);
}
