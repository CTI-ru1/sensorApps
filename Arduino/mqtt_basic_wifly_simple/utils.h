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
  unsigned long initdelay = millis();
  while (millis()-initdelay<500){
    DBG(".");
    unsigned long diff =millis()-initdelay;
    delay(100);
  }

  DBG(".");
  DBGL("ok!");
}
int connect2WiFi(){

  if (EEPROM.read(200)!=WIFLY_SETTINGS_STORED){
    DBGL(F("setup_mode"));
    strcpy(ssid,"wifly");
    strcpy(passphrase,"");
    setup_mode=true;
  }
  else{
    int ssid_len= EEPROM.read(300);
    for (int i=0;i<ssid_len;i++){
      ssid[i]=EEPROM.read(301+i);
    }
    ssid[ssid_len]='\0';
    int phrase_len= EEPROM.read(400);
    for (int i=0;i<phrase_len;i++){
      passphrase[i]=EEPROM.read(401+i);
    }
    passphrase[phrase_len]='\0';
  }

  DBGL("connect2WiFi()");
  int retries = 0;
  // Join the WiFi network
  do{
    if (retries>0){
      DBGL(F("failed!"));
      // Hang on failure.
      digitalWrite(LED_RED, HIGH);
      delay(200);
      digitalWrite(LED_RED, LOW);
      delay(200);
      digitalWrite(LED_RED, HIGH);
    }
    else{
      DBG(F("S:"));
      DBGL(ssid);
      DBG(F("P:"));
      DBGL(passphrase);
    }

    delay(1000);
    retries++;
    if (retries>4){
      wdt_enable(WDTO_2S);
      while(1);
    }
    DBG(F("WiFi Association..."));
  }
  while(!wifly.join(ssid,passphrase));
  DBGL(F("ok!"));

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
      digitalWrite(LED_RED, HIGH);
      delay(100);
      digitalWrite(LED_RED, LOW);
      delay(100);
      digitalWrite(LED_RED, HIGH);
      delay(100);
      digitalWrite(LED_RED, LOW);
      delay(100);
      DBG("Connecting to MQTT...");
    }
    retries++;
    if (retries>4){
      wdt_enable(WDTO_2S);
      while(1);
    }
  }
  while (!client->connect(flare->mac()));
  DBG(".");
  wdt_disable();
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

    DBGL("sensors_send()");
    //    sprintf(sensname,"%s/test",flare->mac());
    //    int val=monitor.calcIrms(1430)*1000;
    //    sprintf(textbuffer,"%d",val);
    //    client->publish(sensname,textbuffer);


    client->publish(str_connect,flare->connect(0));
    //broadcast readings
    for (int i=0;i<flare->scount();i++){
      sprintf(sensname,"%s/%s",flare->mac(),flare->sensor(i)->get_name());
      //sprintf(textbuffer,"%d",rand());
      size_t len = 0;
      flare->sensor(i)->get_value((uint8_t*)textbuffer,&len);
      client->publish(sensname,textbuffer);
      delay(10);
    }
  }
}


void add_sensors() {
  //flare->registerSensor(new InvertedZoneSensor("r/1\0",2));  //2  
  //flare->registerSensor(new InvertedZoneSensor("r/2\0",3));  //3
  //flare->registerSensor(new InvertedZoneSensor("r/3\0",4));  //4  
  //  flare->registerSensor(new InvertedZoneSensor("r/4\0",5));  //5

  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
  pinMode(A3,INPUT);

  DBGL("add_sensors()");
  CurrentSensor * current1 = new CurrentSensor("cur/1\0",&monitor1);
  flare->registerSensor(current1);
  //flare->registerSensor(new WattHourSensor("con/1\0",30,current1));  

  CurrentSensor * current2 = new CurrentSensor("cur/2\0",&monitor2);
  flare->registerSensor(current2);
  //flare->registerSensor(new WattHourSensor("con/2\0",30,current2));  

  CurrentSensor * current3 = new CurrentSensor("cur/3\0",&monitor3);
  flare->registerSensor(current3);
  //flare->registerSensor(new WattHourSensor("con/3\0",30,current3));  
}



void establishConnection(){

  //get the wifly unique mac address
  char wifly_mac[50];
  wifly.getMAC(wifly_mac, sizeof(wifly_mac));
  flare->setMac(wifly_mac);

  //connect to the wifi network
  connect2WiFi();

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
    client->publish(str_connect,flare->connect(1));
    delay(10);
    //client->publish(str_retries,flare->retries(retries));
    //delay(10);
#ifdef SEND_IP
    sprintf(textbuffer,"%s",flare->ip());
    client->publish(str_ip,textbuffer);
    delay(10);
#endif
    //client->subscribe("heartbeat");
    //delay(10);
    client->subscribe(flare->channel());
  }
  //else{

  //}
}

































