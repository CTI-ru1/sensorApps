#include <Arduino.h>
//Software Reset
#include <avr/wdt.h>





void resetWiFly(int pin){
  DBGL("resetWiFly()");
  pinMode(pin,OUTPUT);
  digitalWrite(pin,LOW);
  delay(3000);
  digitalWrite(pin,HIGH);

  //give some time to the wifly to start-up
  static unsigned long initdelay = millis();
  DBG("Waiting to initialize");
  while (millis()-initdelay<3000){
    DBG(".");
    unsigned long diff =millis()-initdelay;
    delay(1000);
  }
#if defined(__AVR_ATmega32U4__)
  //set serials for WiFly -- only for pro micro is Serial1
  //Serial1.begin(9600);   // Start hardware Serial for the RN-XV
  wifly.begin(&Serial1,&Serial);
#else
  wifly.begin(&Serial);
#endif

  DBG(".");
  DBGL("initialized!");
}
int connect2WiFi(){

  DBGL("connect2WiFi()");
  DBG("SSID:");
  DBGL(ssid);
  DBG("Passphrase:");
  DBGL(passphrase);
  DBG("WiFi Association...");
  // Join the WiFi network
  while(!wifly.join(ssid,passphrase)) {
    DBGL("failed!");
    resetWiFly(WIFLY_PIN);
    // Hang on failure.
    digitalWrite(LED_RED, HIGH);
    delay(2000);
    digitalWrite(LED_RED, LOW);
    delay(2000);
    DBG("Attempting again...");
  }
  DBGL("ok!");
}

int connect2MQTT(){
  DBGL("connect2MQTT()");
  DBG("Connecting to MQTT");
  client = new PubSubClient("console.sensorflare.com", 1883, callback,&wifly);
  delay(1000);
  DBG(".");
  digitalWrite(LED_GREEN,LOW);
  digitalWrite(LED_RED,HIGH);
  DBG(".");
  int retries=0;
  //  if (reconnect){
  wdt_enable(WDTO_8S);
  //  }
  while(!client->connect(flare->mac())) {
    DBG(".");
    wdt_reset();
    retries++;
    DBGL("failed!");
    digitalWrite(LED_RED, HIGH);
    delay(1000);
    digitalWrite(LED_RED, LOW);
    delay(1000);
    digitalWrite(LED_RED, HIGH);
    delay(1000);
    digitalWrite(LED_RED, LOW);
    delay(1000);
    DBG("Connecting to MQTT...");
  }
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


    client->publish("connect",flare->connect(0));
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
  flare->registerSensor(new InvertedZoneSensor("r/1\0",2));  //2  
  flare->registerSensor(new InvertedZoneSensor("r/2\0",3));  //3
  flare->registerSensor(new InvertedZoneSensor("r/3\0",4));  //4  
  flare->registerSensor(new InvertedZoneSensor("r/4\0",5));  //5

  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
  pinMode(A3,INPUT);

  DBGL("add_sensors()");
  CurrentSensor * current1 = new CurrentSensor("cur/1\0",&monitor1);
  flare->registerSensor(current1);
  flare->registerSensor(new WattHourSensor("con/1\0",30,current1));  

  CurrentSensor * current2 = new CurrentSensor("cur/2\0",&monitor2);
  flare->registerSensor(current2);
  flare->registerSensor(new WattHourSensor("con/2\0",30,current2));  

  CurrentSensor * current3 = new CurrentSensor("cur/3\0",&monitor3);
  flare->registerSensor(current3);
  flare->registerSensor(new WattHourSensor("con/3\0",30,current3));  
}



void establishConnection(){
  //RESET WIFLY
  resetWiFly(WIFLY_PIN);

  char wifly_mac[50];
  wifly.getMAC(wifly_mac, sizeof(wifly_mac));
  flare->setMac(wifly_mac);

  //connect to the wifi network
  connect2WiFi();
  //get the wifly unique mac address

  //  char wifly_ip[50];
  //  wifly.getIP(wifly_ip,sizeof(wifly_ip));
  //  flare->setIP(wifly_ip);
  //  DBG("WiFi:");
  //  DBGL(flare->ip());
  //if (!setup_mode){
    //connect to the mqtt broker
    int retries = connect2MQTT();

    //publish and subscribe
    client->publish("connect",flare->connect(1));
    delay(10);
    client->publish("retries",flare->retries(retries));
    delay(10);
    //  sprintf(textbuffer,"%s",flare->ip());
    //  client->publish("ip",textbuffer);
    //  delay(10);
    client->subscribe("heartbeat");
    delay(10);
    client->subscribe(flare->channel());
  //}
  //else{

  //}
}



















