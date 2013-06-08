#include <String.h>

//mqtt
#include <PubSubClient.h>
//ethernet
#include <SPI.h>
#include <Ethernet.h>
#include <XBee.h>
#include <XbeeRadio.h>


class UberdustGateway {

public:
  UberdustGateway(){
  };

  void setTestbedID(int testbedID);
  void setUberdustServer(byte * uberdustServer);
  void setGatewayID(uint16_t gatewayID);
  void setXbeeRadio(XBeeRadio * xbee){
    this->xbee = xbee;
  }

  void connect( void callback(char*, uint8_t*, unsigned int));
  void loop(){
    mqttClient->loop();
  }
  void publish(char * message){
    mqttClient->publish(outTopic,message);
  } 
  void publish(uint16_t address, uint8_t * message,uint8_t length){
    byte data [length+2];
    memcpy(data,&address,2);
    memcpy(data+2,message,length);
    mqttClient->publish(outTopic,data,length+2);
    //mqttClient->publish(outTopic,message,length);
  } 


protected:
  //  void callback(char* topic, byte* payload, unsigned int length) {
  //    //TODO: forward topic commands to the routing protocol.
  //    Tx16Request tx = Tx16Request(0xffff, payload, length);
  //    mqttClient->publish(outTopic,payload,length);
  //    blink();
  //    xbee->send(tx, 112);
  //
  //  }






private:
  int testbedID;
  int gatewayID;
  byte * uberdustServer;
  char outTopic[20] ;
  char uid[20];
  EthernetClient *ethernetClient;
  PubSubClient *mqttClient;
  XBeeRadio * xbee;

};









