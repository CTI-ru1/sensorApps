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
  lastPong=millis();

  mqttClient= new PubSubClient(uberdustServer, 1883, callback, *ethernetClient) ;
  sprintf(uid,"arduinoGateway%x",gatewayID);
  if (mqttClient->connect(uid)) {
    //TODO: send a message to declare my existence - if needed
    //client.publish(uid,"hereiam");
    mqttClient->subscribe("arduinoGateway");
    mqttClient->subscribe("heartbeat");
    char reconnectmsg [30] ;
    sprintf(reconnectmsg,"reconnect%s:%d",uid,testbedID);
    mqttClient->publish("heartbeat",reconnectmsg);
  }
}

void UberdustGateway::loop(){
  if (millis()-lastPong>10000){
    pongServer();
    lastPong=millis();
  }
  mqttClient->loop();
}

void UberdustGateway::publish(char * message){
  mqttClient->publish(outTopic,message);
} 

void UberdustGateway::publish(uint16_t address, uint8_t * message,uint8_t length){
  byte data [length+2];
  memcpy(data,&address,2);
  memcpy(data+2,message,length);
  mqttClient->publish(outTopic,data,length+2);
  //mqttClient->publish(outTopic,message,length);
} 

void UberdustGateway::pongServer(){
  mqttClient->publish("heartbeat",uid);
}

