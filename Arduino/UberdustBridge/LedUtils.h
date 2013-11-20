#include <Arduino.h>

/**
 * Fancy reboot indicator
 */
void bootblink()
{
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(6, HIGH);
    digitalWrite(2, LOW);
    delay(300);
    digitalWrite(6, LOW);
    digitalWrite(2, HIGH);
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

