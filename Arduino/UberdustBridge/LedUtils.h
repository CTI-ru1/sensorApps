#include <Arduino.h>

/**
 * Fancy reboot indicator
 */
void bootblink()
{
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(9, HIGH);
    digitalWrite(8, LOW);
    delay(300);
    digitalWrite(9, LOW);
    digitalWrite(8, HIGH);
    delay(300);
  }
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

