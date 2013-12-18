#include <UberdustSensors.h>

UltrasonicSensor * sensor;

void setup() {
  Serial.begin(9600);
  sensor = new UltrasonicSensor("pir",2,3);
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