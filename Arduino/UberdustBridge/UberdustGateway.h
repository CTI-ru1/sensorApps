//mqtt
#include <PubSubClient.h>
//ethernet
#include <SPI.h>
#include <Ethernet.h>
#include <XBee.h>
#include <XbeeRadio.h>
#include "credentials.h"


class UberdustGateway {

public:
  UberdustGateway(EthernetClient *ethernetClient):    
  xcounter(0),ycounter(0)
  {
    this->ethernetClient =ethernetClient ; 
  };

  void setTestbedID(char* testbedID);
  void connect( void callback(char*, uint8_t*, unsigned int));
  boolean loop();

  //  void publish(char * message);
  void publish(uint16_t address, uint8_t * message,uint8_t length);
  void publish(char * topic,char * message);

  void pongServer();

  //char * resetCode();
  void incx();
  void incy();

protected:

private:
  char testbedID[17];
  long lastPong;
  char _message_bus[50];
  char firstConnect[50];
  int _message_bus_count;
  int _connect_count;

  EthernetClient *ethernetClient;
  PubSubClient *mqttClient;
  unsigned long xcounter,ycounter;
};
