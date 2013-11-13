#include "UberdustGateway.h"

void UberdustGateway::setTestbedID(char* testbedID){
  this->testbedID=testbedID;
}
void UberdustGateway::setUberdustServer(byte * uberdustServer){
  this->uberdustServer = uberdustServer;  
}
void UberdustGateway::setGatewayID(uint16_t gatewayID){
  this->gatewayID = gatewayID;  
}

void UberdustGateway::connect( void callback(char*, uint8_t*, unsigned int)){
  mqttClient= new PubSubClient(uberdustServer, 1883, callback, *ethernetClient) ;

  _message_bus_count= sprintf(_message_bus,"%s-%d",testbedID,gatewayID);

  char receive_bus[50];
  sprintf(receive_bus,"s%s-%d",testbedID,gatewayID);


  if (mqttClient->connect(_message_bus)) {

    //subscribe to heartbeats
    mqttClient->subscribe("heartbeat");
    //subscribe to messages
    mqttClient->subscribe(receive_bus);

    _connect_count = sprintf(firstConnect,"1-%s-%d",testbedID,gatewayID);
    mqttClient->publish("connect",(uint8_t*)firstConnect,_connect_count);
    firstConnect[0]='0';
    lastPong=millis();
  }
}

void UberdustGateway::loop(){
  if (millis()-lastPong>10000){
    pongServer();
    lastPong=millis();
  }
  mqttClient->loop();
}

//void UberdustGateway::publish(char * message){
//  //messages are published to the uid. associations are done using the uid of the publisher.
//  mqttClient->publish(_message_bus,message);
//
//} 

void UberdustGateway::publish(uint16_t address, uint8_t * message,uint8_t length){
  byte data [length+2];
  memcpy(data,&address,2);
  memcpy(data+2,message,length);
  //messages are published to the uid. associations are done using the uid of the publisher.
  mqttClient->publish(_message_bus,data,length+2);

} 




void UberdustGateway::pongServer(){

  mqttClient->publish("connect",(uint8_t*)firstConnect,_connect_count);
  lastPong=millis();

  {//sendConnect
    //    char stats[50];
    //    uint8_t len = sprintf(stats,"%s,%d,xbee,%ld",testbedID,gatewayID,xcounter);
    //    mqttClient->publish("stats",(uint8_t*)stats,len);
    //    xcounter=0;
    //    len = sprintf(stats,"%s,%d,mqtt,%ld",testbedID,gatewayID,ycounter);
    //    mqttClient->publish("stats",(uint8_t*)stats,len);
    //    ycounter=0;
  }
}

//void UberdustGateway::incx(){
//  xcounter++;
//};
//void UberdustGateway::incy(){
//  ycounter++;
//};









