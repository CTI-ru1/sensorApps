#include "MqttFlare.h"


//void MqttFlare::action(char * name ,char * message , size_t len){
//  for (int i=0;i<scount;i++){
//    if (strcmp ( sensors[i]->get_name() , name ) == 0 ){
//      char localnetip[20];
//      size_t len1;
//      sensors[i]->set_value((uint8_t *)message,len,(uint8_t *)localnetip,&len1);
//      char sensname[30];
//      sprintf(sensname,"myethernetarduino/%s",sensors[i]->get_name());
//      localnetip[len1]='\0';
//      pubsubclient->publish(sensname,localnetip);
//    }
//  }
//}

void  MqttFlare::startAdHoc(WiFlyServer * configServer_){
  WiFly.setConfiguration(true);

  if(!WiFly.createAdHocNetwork("wifly")) {
    Serial.println("Failed to create network, handle error properly?");
    digitalWrite(LED_GREEN,LOW);
    while(1) {
      digitalWrite(LED_RED,HIGH);
      delay(100);
      digitalWrite(LED_RED,LOW);
      delay(100);
    }
  }
  else{
    Serial.println("Network Created");
    Serial.print("IP: ");
    Serial.println(WiFly.ip());
    configServer = configServer_;
    configServer->begin();


    while(1){
      checkEthernet();
    }

  }
}

boolean MqttFlare::connect(WiFlyServer * configServer_,void callback(char*, uint8_t*, unsigned int)){
  //#define SENSORS_CHECK_PIN 12
  //#define SECURITY_PIN 11
#define TEMP_PIN A3
#define LIGHT_PIN A2
#define METH_PIN A1
#define CARBON_PIN A0
#define HEATER_PIN 2
#define PIR_PIN 3
#define MICRO_PIN A5
#define HUM_PIN A4
  tempSensor = new temperatureSensor("temp", TEMP_PIN);
  //liSensor = new lightSensor("light", LIGHT_PIN);
  ///  humSensor= new humiditySensor("humid", HUM_PIN,tempSensor);

  Serial.begin(38400);


  char localnetip[30];
  initFromProgmem();

  Serial.println(server);
  Serial.println(port);
  Serial.println(ssid);
  Serial.println(code);

  WiFly.setUart(&Serial); // Tell the WiFly library that we are not using the SPIUart

  int adHocMode = EEPROM.read(1);

  if (adHocMode>0){
    Serial.println("droping to adhoc mode");
    // RUN THIS TO GO TO ADHOC MODE
    WiFly.begin(true);
    //#ifdef ETH_CONF
    configServer = configServer_;
    //#endif
    startAdHoc(configServer);
  }
  else{
    Serial.println("trying to connect to wifi netowrk");
    // RUN THIS TO GO TO NORMAL MODE
    WiFly.begin(false);
    int retries = 0;
    // Join the WiFi network
    while (!WiFly.join(ssid, code, true)) {
      retries++;
      if (retries==5){
        EEPROM.write(1,1);
        //TODO :RESTART with WDT
        wdt_enable(WDTO_2S);
      }
      digitalWrite(LED_GREEN,LOW);
      digitalWrite(LED_RED,HIGH);
      delay(2000);
      digitalWrite(LED_RED,LOW);
      delay(2000);
    }
    digitalWrite(LED_RED,LOW);
    char localmac  [20]  ;
    size_t len  = sprintf (localmac,"%s",WiFly.getMAC());

    int j=0;
    for (int i =0 ; i<len ; i++){
      if (localmac[i]!=':'){
        mac[j++]=localmac[i];
      }
    }
    mac[j]='\0';

    sprintf(channel,"1-%s",mac);



    wiflyClient = new WiFlyClient();
    client = new PubSubClient(server, 1883, callback,*wiflyClient);

    connectPubSub();

    client->publish("connect",channel);
    digitalWrite(LED_RED,LOW);
    digitalWrite(LED_GREEN,HIGH);
  }      
}


void MqttFlare::registerSensor(CoapSensor * sensor){
  sensors[scount++]=sensor;
}


boolean MqttFlare::loop(){

  tempSensor->check();
  //  liSensor->check();
  //  humSensor->check();

  if (!client->loop()){
    connectPubSub();
  }

  //  for (int i=1;i<scount;i++){
  //    sensors[i]->check();
  //  }
  //
  static long timestamp = millis(); 
  if (millis()-timestamp>30000){
    timestamp = millis();
    digitalWrite(LED_RED,LOW);
    digitalWrite(LED_GREEN,LOW);
    delay(100);
    digitalWrite(LED_GREEN,HIGH);
    delay(100);
    {    
      char sensname[30];
      char localnetip[10];
      sprintf(sensname,"%s/%s/1",mac , tempSensor->get_name());
      //sprintf(localnetip,"%d",rand());
      size_t len = 0;
      tempSensor->get_value((uint8_t*)localnetip,&len);
      client->publish(sensname,localnetip);
    }
    //    {    
    //      char sensname[30];
    //      char localnetip[10];
    //      sprintf(sensname,"%s/%s/1",mac , liSensor->get_name());
    //      //sprintf(localnetip,"%d",rand());
    //      size_t len = 0;
    //      liSensor->get_value((uint8_t*)localnetip,&len);
    //      client->publish(sensname,localnetip);
    //    }
    //    {    
    //      char sensname[30];
    //      char localnetip[10];
    //      sprintf(sensname,"%s/%s",mac , humSensor->get_name());
    //      //sprintf(localnetip,"%d",rand());
    //      size_t len = 0;
    //      humSensor->get_value((uint8_t*)localnetip,&len);
    //      client->publish(sensname,localnetip);
    //    }
  }
}


void MqttFlare::initFromProgmem(){

  byte isSet = EEPROM.read(0);

  if (isSet==42){
    int index = 4;
    String current = "";
    int argIndex=0;
    while(index<100){
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
          sprintf(ssid,"%s",current.c_str());
          break;
        case 3:
          sprintf(code,"%s",current.c_str());
          break;
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
    Serial.println("Saving Default Settings");
    String defaultSettings = "s=150.140.5.11&p=1883&w=ssid&c=default&";
    EEPROM.write(0,42);
    EEPROM.write(1,1);
    for (int i=0;i<defaultSettings.length();i++){
      EEPROM.write(2+i,defaultSettings[i]);
    }
  }
}

void MqttFlare::checkEthernet(){
  static unsigned long blinkTime = millis();
  static int ledState = LOW;
  if (millis()-blinkTime>500){
    if (ledState==LOW){
      ledState=HIGH;
    }else{
      ledState=LOW;
    }
    digitalWrite(LED_RED,ledState);
    blinkTime=millis();
  }

  // listen for incoming clients
  //
  // Code from the WiFly Server Example
  //
  WiFlyClient client = configServer->available();
  if (client) {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    String request="";
    int index =0 ;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request +=c;

        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of each analog input pin
          client.println("<body>");
          /*client.println("<form role='form' method='get'>");
           client.println("<div class='form-group'> <label class='col-sm-2'> MQTT Broker </label> <input type='col-sm-10  text' class='form-control' value='150.140.5.11'/> </div>");
           client.println("<div class='form-group'> <label class='col-sm-2'> IP </label> <input type='col-sm-10  text' class='form-control' value='150.140.5.11'/> </div>");
           //client.println("<div class='form-group'> <label class='col-sm-2'> IP </label> <select class='col-sm-10 form-control'><option>DHCP</option><option>STATIC</option></select> </div>");
           client.println("<div class='form-group'> <label class='col-sm-2'> Static IP </label> <input type='col-sm-10  text' class='form-control' value='192.168.1.10'/> </div>");
           client.println("<div class='form-group'> <label class='col-sm-2'> Netmask </label> <input type='col-sm-10  text' class='form-control' value='255.255.0.0'/> </div>");
           client.println("<div class='form-group'> <label class='col-sm-2'> Gateway </label> <input type='col-sm-10  text' class='form-control' value='192.168.1.254'/> </div>");
           client.println("<div class='form-group'> <input type='submit' class='btn btn-primary' value='Save and Restart' class='col-sm-2'/> </div>");
           client.println("</form>");   
           client.println("</div>");*/
          client.println("<center>");
          client.println("<form role='form' method='get' action='save'>");
          client.print("Broker:<input type='text' name='s' value='");
          client.print(server);
          client.println("'/><br/>");
          client.print("Port:<input type='text' name='p' value='");
          client.print(port);
          client.println("'/><br/>");
          //          client.println("IP:<input type='text' name='i' value='");
          //          client.print(flare->static_ip[0]);
          //          client.print(".");
          //          client.print(flare->static_ip[1]);
          //          client.print(".");
          //          client.print(flare->static_ip[2]);
          //          client.print(".");
          //          client.print(flare->static_ip[3]);
          //          client.println("'/><br/>");
          //          client.println("Netmask:<input type='text' name='n' value='");
          //          client.print(flare->netmask[0]);
          //          client.print(".");
          //          client.print(flare->netmask[1]);
          //          client.print(".");
          //          client.print(flare->netmask[2]);
          //          client.print(".");
          //          client.print(flare->netmask[3]);
          //          client.println("'/><br/>");
          //          client.println("Gateway:<input type='text' name='g' value='");
          //          client.print(flare->gateway[0]);
          //          client.print(".");
          //          client.print(flare->gateway[1]);
          //          client.print(".");
          //          client.print(flare->gateway[2]);
          //          client.print(".");
          //          client.print(flare->gateway[3]);
          //          client.println("'/><br/>");
          client.print("SSID:<input type='text' name='w' value='");
          client.print(ssid);
          client.println("'/><br/>");
          client.print("Phrase:<input type='text' name='c' value='");
          client.print(code);
          client.println("'/><br/>");
          client.println("<input type='submit' value='Save and Restart'/>");
          client.println("</form>"); 
          client.println("</center>");  
          client.println("</body>");   
          client.println("</html>");   
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

            for (int i=0;i<request.length();i++){
              EEPROM.write(2+i,request[10+i]);
            }
            EEPROM.write(2+request.indexOf(" HTTP")-10,'&');

            initFromProgmem();
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");  // the connection will be closed after completion of the response
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html><body><center><p>Done!<br>");
            client.println(server);
            client.println("<br>");
            client.println(port);
            client.println("<br>");            
            client.println(ssid);
            client.println("<br>");
            client.println(code);
            client.println("<br>");
            client.println("</p></center></body></html>");
            EEPROM.write(1,0);
            wdt_enable(WDTO_2S);
            break;
          }
          request="";

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
    client.stop();
  }
}
void  MqttFlare::connectPubSub(){
  int retries=0;
  while (!client->connect(mac)) {
    retries++;
    if (retries==5){
      EEPROM.write(1,1);
      //TODO :RESTART with WDT
      wdt_enable(WDTO_2S);
    }
    digitalWrite(LED_RED,LOW);
    digitalWrite(LED_GREEN,HIGH);
    delay(2000);
    digitalWrite(LED_GREEN,LOW);
    delay(2000);
  }
  delay(5000);
}





