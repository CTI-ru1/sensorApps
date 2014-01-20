#include "MqttFlare.h"

prog_char string_0[] PROGMEM = "s=150.140.5.11&p=1883&u=0000000000000001&i=0&n=255.255.0.0&g=192.168.1.1&";
prog_char string_1[] PROGMEM = "HTTP/1.1 200 OK\nConnection: close\n\n<html><body><center><p>Stored!<br><a href='/'>Go Back</a></p></center></body></html>";
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

  pubsubclient->loop();

  for (int i=0;i<scount;i++){
    sensors[i]->check();
  }

  static long sendmillis = millis(); 
  char sensname[30];

  if (millis()-sendmillis>30000){
    sendmillis=millis();

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
}


void MqttFlare::initFromProgmem(){

  byte isSet = EEPROM.read(0);

  if (isSet==42){
    //Serial.println("Reading Connection information...");
    int index = 4;
    String current = "";
    int argIndex=0;
    while(index<1024){
      char c = EEPROM.read(index);
      if (c=='&'){
        switch(argIndex){
        case 0:
          sprintf(server,"%s",current.c_str());
          break;
        case 1:
          port = current.toInt();
          break;
        case 2:
          sprintf(testbedHash,"%s",current.c_str());
          break;
        case 3:
          sprintf(static_ip,"%s",current.c_str());
          break;
          //        case 4:
          //          sprintf(netmask,"%s",current.c_str());
          //          break;
          //        case 4:
          //          sprintf(gateway,"%s",current.c_str());
          //          break;

        }
        current="";
        index+=3;
        argIndex++;
      }
      else{
        current+=c;
        index++;
      }
    }
  }  
  else{
    //Serial.println("Saving Default Settings...");
    EEPROM.write(0,42);
    EEPROM.write(1,1);
    for (int i=0;i<73;i++){
      EEPROM.write(2+i,pgm_read_byte_near(string_0+i));
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
    char request[90];
    int index =0 ;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (index<90){
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
          client.println("'/><br/>Broker Port<input type='text' class='form-control' name='p' value='");
          client.print(port);
          client.println("'/><br/>Device ID<input type='text' class='form-control' name='u' value='");
          client.print(testbedHash);
          client.println("'/><br/>IP (0 for DHCP)<input type='text' class='form-control' name='i' value='");
          client.print(static_ip);
          client.println("'/><br/>");
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
          if (request[5]=='s' && request[6]=='a' && request[7]=='v' && request[8]=='e') {  // see if checkbox was clicked

            //Serial.println(request);
            //Serial.println(request.indexOf(" HTTP"));
            //String rest = request.substring(10,request.indexOf(" HTTP"));

            //rest+="&";

            for (int i=0;i<index;i++){
              EEPROM.write(2+i,request[10+i]);
            }
            EEPROM.write(2+index,'&');

            initFromProgmem();
            for (int i=0;i<119;i++){
              client.print((char)pgm_read_byte_near(string_1+i));
            }
            client.println("");
            EEPROM.write(1,42);
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

































