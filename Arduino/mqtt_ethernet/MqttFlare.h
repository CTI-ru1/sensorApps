#ifndef MQTT_FLARE
#define MQTT_FLARE

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <CoapSensor.h>
//Software Reset, Remote Reset
#include <avr/wdt.h>
#include <avr/pgmspace.h>


#define ETH_CONF
// Update these with values suitable for your network.

    
 


class MqttFlare{

public :
  MqttFlare(){

    // Then set up a table to refer to your strings.

//    PROGMEM const char *string_table[] = 	   // change "string_table" name to suit
//    {   
//      string_0     };
//

    strcpy(server,"150.140.5.11");
    strcpy(static_ip,"192.168.1.7");
    strcpy(testbedHash,"0000000000000001");
    //    strcpy(netmask,"255.255.0.0");
    //    strcpy(gateway,"192.168.1.1");
    port=1883;
    scount=0;
    static byte mac[] = { 
      0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED            };
    //EEPROM.write(0,0);

    initFromProgmem();


    Serial.println(server);
    Serial.println(port);
    Serial.println(testbedHash);
    Serial.println(static_ip);
    //    Serial.println(netmask);
    //    Serial.println(gateway);

    //EEPROM.write(0,0);
    if (static_ip[0]=='0'){
      //Serial.println("Using DHCP");
      if (Ethernet.begin(mac)==0){
        Serial.println("DHCP failed!");
      }
      else{  
        Serial.println(Ethernet.localIP());
      }
    }
    else{
      byte ip [4];
      String part="";
      int index=0;
      for (int i=0;i<strlen(static_ip);i++){  
        if (static_ip[i]=='.'){
          ip[index++]=part.toInt();
          part="";
        }
        else{
          part+=(char)static_ip[i];
        }
      }
      ip[index++]=part.toInt();
      //
      //      byte gw [4];
      //       part="";
      //       index=0;
      //      for (int i=0;i<strlen(static_ip);i++){  
      //        if (gateway[i]=='.'){
      //          gw[index++]=part.toInt();
      //        }
      //        else{
      //          part+=(char)static_ip[i];
      //        }
      //      }
      //      byte ip [4];
      //      String part="";
      //      int index=0;
      //      for (int i=0;i<strlen(static_ip);i++){  
      //        if (static_ip[i]=='.'){
      //          ip[index++]=part.toInt();
      //        }
      //        else{
      //          part+=(char)static_ip[i];
      //        }
      //      }
      Ethernet.begin(mac,ip);
    }

  }

  void registerSensor(CoapSensor * sensor);

  boolean connect(void callback(char*, uint8_t*, unsigned int));

  boolean loop();

#ifdef ETH_CONF
  void checkEthernet();
#endif

  void initFromProgmem();

  void action(char * name , char* mess,size_t mess_len);

  void save (int index, String addr){

    int start=0;
    for (int i=0;i<3;i++){
      int mend = addr.indexOf(".",start);
      String partaddrp = addr.substring(start,mend);
      start = mend+1;
      EEPROM.write(index+i,partaddrp.toInt());
    }
    String partaddrp = addr.substring(start);
    EEPROM.write(index+3,partaddrp.toInt());
  }


private :
  void doPublish(int i,uint8_t * message,size_t len){
    size_t len1;
    sensors[i]->set_value(message,len,(uint8_t *)textbuffer,&len1);
    char sensname[30];
    sprintf(sensname,"%s/%s",testbedHash,sensors[i]->get_name());
    textbuffer[len1]='\0';
    pubsubclient->publish(sensname,textbuffer);
  }

  EthernetClient ethClient;
  PubSubClient * pubsubclient;
#ifdef ETH_CONF
  EthernetServer * ethServer;
#endif
  char server[20];
  int port;
  char static_ip[20];
  //  char netmask[20];
  //  char gateway[20];

  char textbuffer[30];
  char testbedHash[17];

  int scount;
  CoapSensor * sensors[6];



};

#endif //MQTT_FLARE




















