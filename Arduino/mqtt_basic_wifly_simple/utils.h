#include <Arduino.h>
//Software Reset
#include <avr/wdt.h>


void resetWiFly(int pin){
  DBG("resetWiFly()");
  pinMode(pin,OUTPUT);
  digitalWrite(pin,LOW);
  delay(1000);
  digitalWrite(pin,HIGH);

  //give some time to the wifly to start-up
  //  unsigned long initdelay = millis();
  //  while (millis()-initdelay<500){
  //    DBG(".");
  //    unsigned long diff =millis()-initdelay;
  //    delay(100);
  //  }

  DBG(".");
  DBGL("ok!");
}
int connect2WiFi(){

  int wifly_status = EEPROM.read(WIFLY_STATUS_POS);
  int ssid_len= EEPROM.read(300);

  //Serial.println(wifly_status);
  //if ((EEPROM.read(200)!=WIFLY_SETTINGS_STORED) || (wifly_status==1)){
  if (wifly_status==1||wifly_status==255||!ssid_len){
    DBGL(F("setup_mode"));
    strcpy(ssid,"wifly");
    strcpy(p,"");
    setup_mode=true;
  }
  else{
    for (int i=0;i<ssid_len;i++){
      ssid[i]=EEPROM.read(301+i);
    }
    ssid[ssid_len]='\0';
    int phrase_len= EEPROM.read(400);
    for (int i=0;i<phrase_len;i++){
      p[i]=EEPROM.read(401+i);
    }
    p[phrase_len]='\0';
  }

  DBGL("connect2WiFi()");
  // Join the WiFi network
  //  DBG(F("S:"));
  //Serial.println(ssid);
  //  DBG(F("P:"));
  //Serial.println(p);


  --wifly_status;
  if (wifly_status==0){
    wifly_status=2;
  }
  EEPROM.write(WIFLY_STATUS_POS,wifly_status);

  DBG(F("WiFi Association..."));
  wifly.setSSID(ssid);
  wifly.setPassphrase(p);
  wifly.enableDHCP();
  wifly.save();

  //  delay(1000);
  wdt_reset();
  if(!wifly.join()){
    wdt_enable(WDTO_8S);
    while(1){
      blinkFast(LED_RED);
      delay(1000);
    }
  }
  wifly.setProtocol(WIFLY_PROTOCOL_TCP);
  if (wifly.getPort() != 80) {
    wifly.setPort(80);
    /* local port does not take effect until the WiFly has rebooted (2.32) */
    wifly.save();
    //Serial.println(F("Set port to 80, rebooting to make it work"));
    wifly.reboot();
    wdt_reset();
    delay(3000);
  }
  DBGL(F("ok!"));
  if (setup_mode){
  }
  else{
    EEPROM.write(WIFLY_STATUS_POS,3);
  }

  //  if (setup_mode){
  //      wifly.setProtocol(WIFLY_PROTOCOL_TCP);
  //      wifly.setPort(80);
  //      /* local port does not take effect until the WiFly has rebooted (2.32) */
  //      wifly.save();
  //      wifly.reboot();
  //  }
}

int connect2MQTT(){
  DBG("connect2MQTT()");
  client = new PubSubClient("console.sensorflare.com", 1883, callback,&wifly);
  wdt_reset();

  //client = new PubSubClient("150.140.5.11", 1883, callback,&wifly);
  delay(1000);
  DBG(".");
  digitalWrite(LED_GREEN,LOW);
  digitalWrite(LED_RED,HIGH);
  DBG(".");
  int retries=0;
  //  if (reconnect){
  //wdt_enable(WDTO_8S);
  //  }
  do{
    wdt_reset();
    if (retries>0){
      DBGL("failed!");
      //blinkSlow(LED_RED);
      DBG("Connecting to MQTT...");
    }
    retries++;
    if (retries>MAX_RETRIES){
      wdt_enable(WDTO_2S);
      while(1);
    }
  }
  while (!client->connect(flare->mac()));
  DBG(".");
  wdt_reset();
  DBGL("ok!");
  return retries;
}

void sensors_loop()
{
  DBGL("sensors_loop()");

  flare->check();

  static long sendmillis = millis(); 

  if (millis()-sendmillis>30000){
    sendmillis=millis();
    if (!received){
      digitalWrite(LED_RED,HIGH);
      while(1){
      }
    }
    received=false;

    DBGL("sensors_send()");
    //    sprintf(sensname,"%s/test",flare->mac());
    //    int val=monitor.calcIrms(1430)*1000;
    //    sprintf(textbuffer,"%d",val);
    //    client->publish(sensname,textbuffer);


    //client->publish("connect",flare->connect(0));
    //broadcast readings
    for (int i=0;i<flare->scount();i++){
      sprintf(sensname,"%s/%s",flare->mac(),flare->sensor(i)->get_name());
      //sprintf(textbuffer,"%d",rand());
      size_t len = 0;
      char response[10];
      flare->sensor(i)->get_value((uint8_t*)response,&len);
      client->publish(sensname,response);
      delay(10);
    }
  }
}


void add_sensors() {
  flare->registerSensor(new InvertedZoneSensor("r/1\0",2));  //2  
  flare->registerSensor(new InvertedZoneSensor("r/2\0",3));  //3
  flare->registerSensor(new InvertedZoneSensor("r/3\0",4));  //4  
  flare->registerSensor(new InvertedZoneSensor("r/4\0",5));  //5

    //    DBGL("add_sensors()");
  //  CurrentSensor * current1 = ;
  flare->registerSensor(new CurrentSensor("cur/1\0",&monitor1));
  //  //flare->registerSensor(new WattHourSensor("con/1\0",30,current1));  
  //
  //  CurrentSensor * current2 = new CurrentSensor("cur/2\0",&monitor2);
  flare->registerSensor(new CurrentSensor("cur/2\0",&monitor2));
  //  //flare->registerSensor(new WattHourSensor("con/2\0",30,current2));  
  //
  //  CurrentSensor * current3 = new CurrentSensor("cur/3\0",&monitor3);
  flare->registerSensor(new CurrentSensor("cur/3\0",&monitor3));
  //  //flare->registerSensor(new WattHourSensor("con/3\0",30,current3));  
}



void establishConnection(){

  //get the wifly unique mac address
  wifly.getMAC(textbuffer, sizeof(textbuffer));
  wdt_reset();
  flare->setMac(textbuffer);

  //connect to the wifi network
  connect2WiFi();
  wdt_reset();
  wdt_enable(WDTO_8S);

  if(!setup_mode){
#ifdef SEND_IP
    char wifly_ip[50];
    wifly.getIP(wifly_ip,sizeof(wifly_ip));
    flare->setIP(wifly_ip);
    DBG("WiFi:");
    DBGL(flare->ip());
#endif
    //if (!setup_mode){
    //connect to the mqtt broker
    //int retries = connect2MQTT();
    connect2MQTT();

    //publish and subscribe
    client->publish("c",textbuffer);
    //delay(10);
    //client->publish(str_retries,flare->retries(retries));
    //delay(10);
#ifdef SEND_IP
    sprintf(textbuffer,"%s",flare->ip());
    client->publish(str_ip,textbuffer);
    delay(10);
#endif
    client->subscribe("heartbeat");
    delay(10);
    client->subscribe(flare->channel());
  }
  //else{

  //}
}


















































