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

#define MAC_EEPROM_OFFSET 500




class MqttFlare{

public :
  MqttFlare(){
    strcpy(server,"console.sensorflare.com");
    strcpy(static_ip,"0");
    strcpy(testbedHash,"flare00000000001");
    port=1883;
    scount=0;
    static byte mac[] = { 
      0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED       
    };
    int sensorValue = analogRead(A0);
    randomSeed(sensorValue);
    getMacAddress(mac);
    Serial.println("Default Values loaded!");

    //EEPROM.write(0,0);

    initFromProgmem();
    Serial.println("Initialized from Progmem!");

    Serial.print("s:");
    Serial.println(server);
    Serial.print("p:");
    Serial.println(port);
    Serial.print("h:");
    Serial.println(testbedHash);
    Serial.print("i:");
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
        Serial.print("IP:");
        Serial.println(Ethernet.localIP());
      }
    }
    else{

      //TODO:re-enable staticIp
      //      byte ip [4];
      //      String part="";
      //      int index=0;
      //      for (int i=0;i<strlen(static_ip);i++){  
      //        if (static_ip[i]=='.'){
      //          ip[index++]=part.toInt();
      //          part="";
      //        }
      //        else{
      //          part+=(char)static_ip[i];
      //        }
      //      }
      //      ip[index++]=part.toInt();
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
      //        if (static_ip[i]=='.'){      //          ip[index++]=part.toInt();
      //        }
      //        else{
      //          part+=(char)static_ip[i];
      //        }
      //      }
      Ethernet.begin(mac);
      Serial.print("IP:");
      Serial.println(Ethernet.localIP());

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
  char server[50];
  int port;
  char static_ip[20];
  //  char netmask[20];
  //  char gateway[20];

  char textbuffer[30];
  char testbedHash[64];

  int scount;
  CoapSensor * sensors[6];



  // This function takes a 6 byte array, and will fill every byte that is 0x00 with device unique data.
  // The data is retrieved from a preset place in the Atmega EEPROM.
  // If the particular EEPROM bytes are 0x00 or 0xFF, the data is ranomly generated and stored in EEPROM.

  void getMacAddress(byte* macAddr) {
    int eepromOffset = MAC_EEPROM_OFFSET;
    int b = 0; 
    for (int c = 0; c < 6; c++) {
      b = 0;
      if(macAddr[c] == 0) {
        b = EEPROM.read(eepromOffset + c);
        if(b == 0 || b == 255) {
          b = random(0, 255);
          EEPROM.write(eepromOffset + c, b);
        }
        macAddr[c] = b;
      }
    }
  }


};

#endif //MQTT_FLARE

























