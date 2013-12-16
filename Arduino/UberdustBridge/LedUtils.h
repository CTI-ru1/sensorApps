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

/**
 * Fast Blink operation. (The led has to be already set to OUTPUT)
 * @param pin the pin to blink
 */
void blinkFast(int pin){
  digitalWrite(pin,HIGH);
  delay(10);
  digitalWrite(pin,LOW);
}

/**
 * Slow Blink operation. (The led has to be already set to OUTPUT)
 * @param pin the pin to blink
 */
void blinkSlow(int pin){
  digitalWrite(pin,HIGH);
  delay(300);
  digitalWrite(pin,LOW);
}


