#ifndef MQTT_FLARE
#define MQTT_FLARE

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>



class MqttFlare{

public :
  MqttFlare(){
  }
  
  boolean connect(byte *mac,int port,void callback(char*, uint8_t*, unsigned int));

  boolean loop();
  
  void checkEthernet();


private :
  EthernetClient ethClient;
  PubSubClient * pubsubclient;
  EthernetServer * ethernetServer;

};

#endif //MQTT_FLARE
