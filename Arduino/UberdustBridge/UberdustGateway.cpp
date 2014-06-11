#include "UberdustGateway.h"

void UberdustGateway::setTestbedID(char* testbedID){
  strcpy(this->testbedID,testbedID);
}

void UberdustGateway::connect( void callback(char*, uint8_t*, unsigned int)){
  mqttClient= new PubSubClient("150.140.5.20", 1883, callback, *ethernetClient) ;

  _message_bus_count= sprintf(_message_bus,"%s",testbedID);

  char receive_bus[50];
  sprintf(receive_bus,"s%s",testbedID);

  char deviceID [50];
  sprintf(deviceID,"%s%d",testbedID,rand());

  if (mqttClient->connect(deviceID)) {

    //subscribe to heartbeats
    mqttClient->subscribe("heartbeat");
    //subscribe to messages
    mqttClient->subscribe(receive_bus);

    _connect_count = sprintf(firstConnect,"1-%s",testbedID);
    mqttClient->publish("connect",(uint8_t*)firstConnect,_connect_count);
    firstConnect[0]='0';
    lastPong=millis();
  }
}

boolean UberdustGateway::loop(){
  if (millis()-lastPong>10000){
    pongServer();
    lastPong=millis();
  }
  return mqttClient->loop();
}

void UberdustGateway::publish(uint16_t address, uint8_t * message,uint8_t length){
  byte data [length+2];
  memcpy(data,&address,2);
  memcpy(data+2,message,length);
  //messages are published to the uid. associations are done using the uid of the publisher.
  mqttClient->publish(_message_bus,data,length+2);
} 


void UberdustGateway::publish(char * topic,char * message){
  mqttClient->publish(topic,message);

} 


void UberdustGateway::pongServer(){
  mqttClient->publish("connect",(uint8_t*)firstConnect,_connect_count);
  lastPong=millis();

  {
        char stats[50];
        uint8_t len = sprintf(stats,"%s-xbee-%ld",testbedID,xcounter);
        mqttClient->publish("stats",(uint8_t*)stats,len);
        xcounter=0;
        len = sprintf(stats,"%s-mqtt-%ld",testbedID,ycounter);
        mqttClient->publish("stats",(uint8_t*)stats,len);
        ycounter=0;
  }
}

void UberdustGateway::incx(){
  xcounter++;
};
void UberdustGateway::incy(){
  ycounter++;
};

