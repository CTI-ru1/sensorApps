#include "UberdustGateway.h"

void UberdustGateway::setTestbedID(uint32_t testbedID){
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
  _uid_count= 1+sizeof(uint16_t) + sizeof(uint32_t); //sprintf(_uid,"%x",gatewayID);
  _uid[0]=1;
  memcpy(_uid+1,&gatewayID, sizeof(uint16_t));
  memcpy(_uid+1+sizeof(uint16_t), &testbedID, sizeof(uint32_t));
  _message_bus_count= sprintf(_message_bus,"ag%s",_uid);
  _reset_id_count= sprintf(_reset_id,"reset%x",gatewayID);
  if (mqttClient->connect(_message_bus)) {
    //TODO: send a message to declare my existence - if needed
    //client.publish(uid,"hereiam");
    mqttClient->subscribe("heartbeat");
    mqttClient->subscribe(_message_bus);
    mqttClient->publish("connect",_uid,_uid_count);
    _uid[0]=0;
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
  mqttClient->publish("connect",_uid,_uid_count);
  byte message [_uid_count + 20];
  memcpy(message,_uid,_uid_count);
  int count = sprintf((char*)message+7,":xbee:%ld",xcounter);  
  mqttClient->publish("stats",message,_uid_count + count);
  xcounter=0;
  count =  sprintf((char*)message+7,":mqtt:%ld",ycounter);
  ycounter=0;
  mqttClient->publish("stats",message,_uid_count + count);
  count =  sprintf((char*)message+7,":addr:%4x",gatewayID);
  ycounter=0;
  mqttClient->publish("stats",message,_uid_count + count);
  count =  sprintf((char*)message+7,":testbed:%4x",testbedID);
  ycounter=0;
  mqttClient->publish("stats",message,_uid_count + count);
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
  //TODO :FIX
  return false;//strncmp(_uid,payload,_uid_count)==0;
};




