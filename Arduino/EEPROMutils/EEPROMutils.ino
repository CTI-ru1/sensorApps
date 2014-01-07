/**
 * EEPROMutils
 * Clear, Read or Write EEPROM contents.
 */
#include <EEPROM.h>

#define EECLEAR
#define EEREAD
#define EEWRITE

//XBee Libraries
#include <XBee.h>
#include <XbeeRadio.h>
//Create the XbeeRadio object we'll be using
XBeeRadio xbee;

void setup(){

#ifdef EECLEAR
  Serial.begin(38400);
  EEPROM.write(0,0);  
#endif

#ifdef EEWRITE
  char testbedHash[17];

  //Connect to XBee
  xbee.begin(38400);
  //Initialize our XBee module with the correct values using CHANNEL
  xbee.init();

  uint32_t addr64h = xbee.getMyAddress64High();
  uint32_t addr64l = xbee.getMyAddress64Low();

  uint16_t * addr64hp =  ( uint16_t * ) &addr64h;
  uint16_t * addr64lp =  ( uint16_t * ) &addr64l;

  sprintf(testbedHash, "%4x%4x%4x%4x",addr64hp[0],addr64hp[1],addr64lp[0],addr64lp[1]);

  for (int i=0;i<64;i++){
    if (testbedHash[i]==' '){
      testbedHash[i]='0';
    }
  }

  EEPROM.write(0, 67);
  for (int i = 0; i < 17; i++){
    EEPROM.write(1+i, testbedHash[i]);
  }

#endif

#ifdef EEREAD
  Serial.begin(38400);
  Serial.println("");  
  Serial.print("EEPROM_indicator:");
  Serial.println(EEPROM.read(0));  
  Serial.print("EEPROM_address:");  
  for (int i = 1; i < 18; i++){
    Serial.print((char)EEPROM.read(i));  
  }
  Serial.println("");  
#endif

}

void loop(){
}











