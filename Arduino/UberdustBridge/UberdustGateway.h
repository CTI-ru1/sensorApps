
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
  gatewayID(0),
  xcounter(0),
  ycounter(0){
    this->ethernetClient =ethernetClient ; 
  };

  void setTestbedID(uint32_t testbedID);
  void setUberdustServer(byte * uberdustServer);
  void setGatewayID(uint16_t gatewayID);
  void connect( void callback(char*, uint8_t*, unsigned int));
  void loop();

  void publish(char * message);
  void publish(uint16_t address, uint8_t * message,uint8_t length);

  void pongServer();

  char * resetCode();
  
  boolean checkReset(char * payload);
  boolean checkForMe(char * payload);
  
  void incx();
  void incy();

protected:

private:
  uint32_t testbedID;
  uint16_t gatewayID;
  long lastPong;
  byte * uberdustServer;
  byte _uid[1+sizeof(uint32_t)+sizeof(uint16_t)];
  char _message_bus[20],_reset_id[23];
  int _uid_count,_message_bus_count,_reset_id_count;
  
  EthernetClient *ethernetClient;
  PubSubClient *mqttClient;
  //XBeeRadio * xbee;
  unsigned long xcounter,ycounter;
 

};
