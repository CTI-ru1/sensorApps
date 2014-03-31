#ifndef MQTT_FLARE
#define MQTT_FLARE

#include <CoapSensor.h>
#include <WiFlyHQ.h>

class MqttFlare{

public :
  MqttFlare():
  _scount(0){
  }

  void setMac(char * wifly_mac){

    int j=0;
    for (int i =0 ; i<strlen(wifly_mac) ; i++){
      if (wifly_mac[i]!=':'){
        _mac[j++]=wifly_mac[i];
      }
    }
    _mac[j]='\0';

    sprintf(_channel, "s%s/#",_mac);
  }
  
  char * mac(){
    return _mac;
  }  
  char * channel(){
    return _channel;
  }

  char * connect(int i){
    char msg[20];
    sprintf(msg,"%d-%s",i,mac());
    return msg;
  }

  char * retries(int i){
    char msg[20];
    sprintf(msg,"%d-%s",i,mac());
    return msg;
  }

  CoapSensor *  sensor(int index){
    return sensors[index];
  }

  void check();

  void registerSensor(CoapSensor * sensor);

  int scount () {
    return _scount;
  }

  void action(char * name ,char * message , size_t len,char*resp,size_t*len1){
    for (int i=0;i<_scount;i++){
      if (strcmp ( sensors[i]->get_name() , name ) == 0 ){

    size_t len1;
    sensors[i]->set_value((uint8_t *)message,len,(uint8_t *)resp,&len1);
    char sensname[30];


      }
    }
  }

private :
  char _mac[20];				//wifly mac address
  char _channel[30];		//topic to subscribe to (s + mac)
  char textbuffer[20];

  int _scount;
  CoapSensor * sensors[10];


};

#endif








