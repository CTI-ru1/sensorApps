#include "UberdustGateway.h"

void UberdustGateway::setTestbedID(int testbedID){
  this->testbedID=testbedID;
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
  _uid_count= sprintf(_uid,"%x",gatewayID);
  _message_bus_count= sprintf(_message_bus,"ag%s",_uid);
  _reset_id_count= sprintf(_reset_id,"reset%x",gatewayID);
  if (mqttClient->connect(_message_bus)) {
    //TODO: send a message to declare my existence - if needed
    //client.publish(uid,"hereiam");
    mqttClient->subscribe("heartbeat");
    mqttClient->subscribe(_message_bus);
    mqttClient->publish("connect",_uid);
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
  //messages are published to the uid. associations are done using the uid of the publisher.
  mqttClient->publish(_message_bus,message);
} 

void UberdustGateway::publish(uint16_t address, uint8_t * message,uint8_t length){
  byte data [length+2];
  memcpy(data,&address,2);
  memcpy(data+2,message,length);
  //messages are published to the uid. associations are done using the uid of the publisher.
  mqttClient->publish(_message_bus,data,length+2);
} 

void UberdustGateway::pongServer(){
  mqttClient->publish("connect",_uid);
  char counter[20];
  sprintf(counter,"stats:%s:x:%ld",_uid,xcounter);
  mqttClient->publish("heartbeat",counter);
  xcounter=0;
  sprintf(counter,"stats:%s:y:%ld",_uid,ycounter);
  ycounter=0;
  mqttClient->publish("heartbeat",counter);
}

char * UberdustGateway::resetCode(){
  return _reset_id;
}

void UberdustGateway::incx(){
  xcounter++;
};
void UberdustGateway::incy(){
  ycounter++;
};

boolean UberdustGateway::checkReset(char * payload){
  return strncmp(_reset_id,payload,_reset_id_count)==0;
};

boolean UberdustGateway::checkForMe(char * payload){
  return strncmp(_uid,payload,_uid_count)==0;
};



