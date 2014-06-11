#include "MqttFlare.h"

prog_char string_1[] PROGMEM = "HTTP/1.1 200 OK\nConnection: close\n\n<html><body><center><p>Saved!<br><a href='/'>Go Back</a></p></center></body></html>";
prog_char string_2[] PROGMEM = "HTTP/1.1 200 OK\nConnection: close\n\n<html><link href=\"http://tinyurl.com/pw33vc2\" rel=\"stylesheet\"><body><div class='container'><form method='get' action='save'>MQTT Broker<input type='text' class='form-control' name='s' value='";
void MqttFlare::action(char * name ,char * message , size_t len){
  for (int i=0;i<scount;i++){
    if (strcmp ( sensors[i]->get_name() , name ) == 0 ){
      doPublish(i,(uint8_t *)message,len);
    }
  }
}

boolean MqttFlare::connect(void callback(char*, uint8_t*, unsigned int)){
  ethServer = new EthernetServer(80);

  //initFromProgmem();

  pubsubclient = new PubSubClient(server, port, callback, ethClient);

  if (pubsubclient->connect(testbedHash)) {
    //Serial.println("MQTT to Ethernet");
    sprintf(textbuffer,"s%s/#",testbedHash);
    pubsubclient->subscribe(textbuffer);

    sprintf(textbuffer,"1-%s",testbedHash);
    pubsubclient->publish("connect",textbuffer);

    return true;
  }
  else{
    Serial.println("MQTT failed!");
    return false;
  }
}


void MqttFlare::registerSensor(CoapSensor * sensor){
  sensors[scount++]=sensor;
}


boolean MqttFlare::loop(){

  boolean loopres = pubsubclient->loop();
  if (!loopres){
    wdt_enable(WDTO_2S);
    while(1){
    }
    return false;
  }

  for (int i=0;i<scount;i++){
    sensors[i]->check();
  }

  static long sendmillis = millis(); 
  char sensname[30];

  if (millis()-sendmillis>30000){
    sendmillis=millis();
    Serial.println("pushing to server...");
    //send connect 0
    sprintf(textbuffer,"0-%s",testbedHash);
    pubsubclient->publish("connect",textbuffer);

    //broadcast readings
    for (int i=0;i<scount;i++){
      sprintf(sensname,"%s/%s",testbedHash,sensors[i]->get_name());
      //sprintf(textbuffer,"%d",rand());
      size_t len = 0;
      sensors[i]->get_value((uint8_t*)textbuffer,&len);
      pubsubclient->publish(sensname,textbuffer);
    }
  }
#ifdef ETH_CONF
  checkEthernet();
#endif
  return true;
}


void MqttFlare::initFromProgmem(){

  byte isSet = EEPROM.read(0);

  if (isSet==42){
    int server_len = EEPROM.read(1);
    for (int i=1;i<=server_len;i++){
      server[i-1]=EEPROM.read(1+i);
    }
    server[server_len]='\0';
    int port_len = EEPROM.read(100);
    String current = "";
    for (int i=1;i<=port_len;i++){
      current+=EEPROM.read(100+i);
    }
    int testbedHash_len = EEPROM.read(200);
    for (int i=1;i<=testbedHash_len;i++){
      testbedHash[i-1]=EEPROM.read(200+i);
    }
    testbedHash[testbedHash_len]='\0';
    int staticIp_len = EEPROM.read(300);
    for (int i=1;i<=staticIp_len;i++){
      static_ip[i-1]=EEPROM.read(300+i);
    }
    static_ip[staticIp_len]='\0';

  }  
  else{
    //Serial.println("Saving Default Settings...");
    EEPROM.write(0,42);
    int server_len = strlen(server);
    EEPROM.write(1, server_len);
    for (int i=1;i<=server_len;i++){
      EEPROM.write(1+i,server[i-1]);
    }
    char port_str[10];
    int port_len = sprintf(port_str,"%d",port);
    EEPROM.write(100,port_len);
    for (int i=1;i<=port_len;i++){
      EEPROM.write(100+i,port_str[i-1]);
    }
    int testbedHash_len = strlen(testbedHash);
    EEPROM.write(200,testbedHash_len);
    for (int i=1;i<=testbedHash_len;i++){
      EEPROM.write(200+i,testbedHash[i-1]);
    }
    int staticIp_len = strlen(static_ip);
    EEPROM.write(300,staticIp_len);
    for (int i=1;i<=staticIp_len;i++){
      EEPROM.write(300+i,static_ip[i-1]);
    }
  }

  delay(2000);
}

void MqttFlare::checkEthernet(){
  //Serial.println("loop");  

  // listen for incoming clients
  EthernetClient client = ethServer->available();
  if (client) {
    //Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    char request[250];
    int index =0 ;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (index<250){
          request[index++]=c;
        }

        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          //client.println("");
          for (int i=0;i<227;i++){
            client.print((char)pgm_read_byte_near(string_2+i));
          }
          //client.println("<html><link href=\"http://tinyurl.com/pw33vc2\" rel=\"stylesheet\"><body><div class='container'><form method='get' action='save'>MQTT Broker<input type='text' class='form-control' name='s' value='");
          client.print(server);
          client.println("'/><br/>");
          client.println("Port<input type='text' class='form-control' name='p' value='");
          client.print(port);
          client.println("'/><br/>ID<input type='text' class='form-control' name='u' value='");
          client.print(testbedHash);
          client.println("'/><br/>IP (0 for DHCP)<input type='text' class='form-control' name='i' value='");
          client.print(static_ip);
          client.println("'/><br/>");
          client.println(millis()/1000/60);
          client.flush();

          //          client.print("Netmask <input type='text' class='form-control' name='n' value='");
          //          client.print(netmask);
          //          client.println("'/>");
          //          client.print("Gateway <input type='text' class='form-control' name='g' value='");
          //          client.print(gateway);
          //          client.println("'/>");
          client.println("<input type='hidden' name='h' value='a'/><input type='submit' class='btn btn-primary' value='Save and Restart'></form></div></body></html>");

          index = 0;    // finished with request, empty string
          break;

        }
        if (c == '\n') {

          // you're starting a new line
          currentLineIsBlank = true;
          //Serial.println(request);
          //Serial.println(request.indexOf("/save"));
          if (request[5]=='s' && request[6]=='a' && request[7]=='v' && request[8]=='e') {  

            // /save?s=xx.xx.xx.xx&p=xxxx&u=xxxx&i=xx.xx.xx.xx&h=a
            char *new_server = strtok(request,"&")+12;  // xx.xx.xx.xx
            char *new_port = strtok(NULL,"&");  // p=xxxx
            char *new_hash = strtok(NULL,"&");  // u=xxxx
            char *new_ip = strtok(NULL,"&");  // i=xx.xx.xx.xx

            new_port=strtok(new_port,"="); // p
            new_port=strtok(NULL,"=");    // xxxx
            new_hash=strtok(new_hash,"="); // u
            new_hash=strtok(NULL,"="); // xxxx
            new_ip=strtok(new_ip,"="); // i
            new_ip=strtok(NULL,"="); //xx.xx.xx.xx

            strcpy(server,new_server);
            String newPortStr("");
            for (int i =0;i<strlen(new_port);i++){
              newPortStr+=new_port[i];
            }
            port=newPortStr.toInt();
            strcpy(testbedHash,testbedHash);
            strcpy(static_ip,new_ip);

            EEPROM.write(0,0);
            initFromProgmem();
            for (int i=0;i<119;i++){
              client.print((char)pgm_read_byte_near(string_1+i));
            }
            client.println("");
            EEPROM.write(0,42);

            Serial.println("New Settings:");
            Serial.print("s:");
            Serial.println(server);
            Serial.print("p:");
            Serial.println(port);
            Serial.print("h:");
            Serial.println(testbedHash);
            Serial.print("i:");
            Serial.println(static_ip);

            wdt_enable(WDTO_2S);
            break;
          }
          index=0;

          //if (strncmp("GET /save",line,8)==0){
          //  Serial.println("Handle Save");
          //}

          //line[index]='\0';
          //index=0;
          //Serial.write(line);
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    //Serial.println("client disonnected");
  }
}


























































