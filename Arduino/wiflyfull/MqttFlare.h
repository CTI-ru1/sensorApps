#ifndef MQTT_FLARE
#define MQTT_FLARE

#include <Arduino.h>
#include <SPI.h>
#include <PubSubClient.h>
#include <WiFly.h>
#include <EEPROM.h>
#include "TemperatureSensor.h"
#include "LightSensor.h"
#include "HumiditySensor.h"
//Software Reset, Remote Reset
#include <avr/wdt.h>


#define LED_RED 10
#define LED_GREEN 11

class MqttFlare{

public :
  MqttFlare(){
    scount=0;
    strcpy(server,"150.140.5.11");
    port=1883;
    strcpy (ssid, "myssid");
    strcpy(code,"passphrase");

    //EEPROM.write(0,0);
    //EEPROM.write(1,1);

    
    initFromProgmem();

  }

  void registerSensor(CoapSensor * sensor);

  boolean connect(WiFlyServer* configServer_,void callback(char*, uint8_t*, unsigned int));

  boolean loop();

  void startAdHoc(WiFlyServer * configServer_);

  void checkEthernet();

  void initFromProgmem();

  void action(char * name , char* mess,size_t mess_len);

  void save (int index, String addr){

    int start=0;
    for (int i=0;i<3;i++){
      int mend = addr.indexOf(".",start);
      String partaddrp = addr.substring(start,mend);
      start = mend+1;
      EEPROM.write(index+i,atoi(partaddrp.c_str()));
    }
    String partaddrp = addr.substring(start);
    EEPROM.write(index+3,atoi(partaddrp.c_str()));
  }


  int port;
  byte static_ip[4];
  byte netmask[4];
  byte gateway[4];
  char server[20],ssid[20],code[20];
private :
void connectPubSub();
  //EthernetClient ethClient;
  PubSubClient * client;

  int scount;
  CoapSensor * sensors[28];
  WiFlyServer *configServer;

temperatureSensor * tempSensor;
//lightSensor* liSensor ;
//humiditySensor* humSensor;
  char mac[20];

  char channel[20];
  WiFlyClient *wiflyClient;
};

#endif //MQTT_FLARE


















