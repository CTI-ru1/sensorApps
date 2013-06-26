
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
  gatewayID(0){
    this->ethernetClient =ethernetClient ; 
  };

  void setTestbedID(int testbedID);
  void setUberdustServer(byte * uberdustServer);
  void setGatewayID(uint16_t gatewayID);
  void connect( void callback(char*, uint8_t*, unsigned int));
  void loop();

  void publish(char * message);
  void publish(uint16_t address, uint8_t * message,uint8_t length);

  void pongServer();

protected:

private:
  int testbedID;
  int gatewayID;
  long lastPong;
  byte * uberdustServer;
  char outTopic[20] ;
  char uid[20];
  EthernetClient *ethernetClient;
  PubSubClient *mqttClient;
  //XBeeRadio * xbee;

};















