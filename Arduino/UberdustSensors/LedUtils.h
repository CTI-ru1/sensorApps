#include <Arduino.h>

/**
 * Fancy reboot indicator
 */
void bootblink(int pin1,int pin2)
{
  pinMode(pin1,OUTPUT);
  pinMode(pin2,OUTPUT);
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(pin1, HIGH);
    digitalWrite(pin2, LOW);
    delay(300);
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, HIGH);
    delay(300);
  }
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, LOW);
}

void blinkFast(int pin){
  digitalWrite(pin,HIGH);
  delay(10);
  digitalWrite(pin,LOW);
}

void blinkSlow(int pin){
  digitalWrite(pin,HIGH);
  delay(300);
  digitalWrite(pin,LOW);
}
