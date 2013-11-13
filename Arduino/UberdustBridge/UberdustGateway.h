
//mqtt
#include <PubSubClient.h>
//ethernet
#include <SPI.h>
#include <Ethernet.h>
#include <XBee.h>
#include <XbeeRadio.h>


class UberdustGateway {

public:
  UberdustGateway(EthernetClient *ethernetClient):  
  gatewayID(0)
//    ,xcounter(0)
//      ,ycounter(0)
      {
        this->ethernetClient =ethernetClient ; 
      };

  void setTestbedID(char* testbedID);
  void setUberdustServer(byte * uberdustServer);
  void setGatewayID(uint16_t gatewayID);
  void connect( void callback(char*, uint8_t*, unsigned int));
  void loop();

  //  void publish(char * message);
  void publish(uint16_t address, uint8_t * message,uint8_t length);

  void pongServer();

  //char * resetCode();

//  void incx();
//  void incy();

protected:

private:
  char* testbedID;
  uint16_t gatewayID;  
  long lastPong;
  byte * uberdustServer;
  char _message_bus[50];
  char firstConnect[50];
  int _message_bus_count;
  int _connect_count;

  EthernetClient *ethernetClient;
  PubSubClient *mqttClient;
  //unsigned long xcounter,ycounter;


};

