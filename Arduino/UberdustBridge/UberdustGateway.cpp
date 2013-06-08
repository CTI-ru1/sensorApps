#include "UberdustGateway.h"

void UberdustGateway::setTestbedID(int testbedID){
  this->testbedID=testbedID;
  sprintf(this->outTopic,"testbed%d",testbedID);
}
void UberdustGateway::setUberdustServer(byte * uberdustServer){
  this->uberdustServer = uberdustServer;  
}
void UberdustGateway::setGatewayID(uint16_t gatewayID){
  this->gatewayID = gatewayID;  
}

void UberdustGateway::connect( void callback(char*, uint8_t*, unsigned int)){
  ethernetClient = new EthernetClient();
  mqttClient= new PubSubClient(uberdustServer, 1883, callback, *ethernetClient) ;
  sprintf(uid,"arduinoGateway",gatewayID);
  if (mqttClient->connect(uid)) {
    //TODO: send a message to declare my existence - if needed
    //client.publish(uid,"hereiam");
    mqttClient->subscribe(uid);
  }
}





