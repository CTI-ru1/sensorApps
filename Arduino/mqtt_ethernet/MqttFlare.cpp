#include "MqttFlare.h"

boolean MqttFlare::connect(byte *mac,int port,void callback(char*, uint8_t*, unsigned int)){
  ethernetServer = new EthernetServer(80);


  pubsubclient = new PubSubClient(mac, port, callback, ethClient);

  if (pubsubclient->connect("arduinoClient")) {
    Serial.println("MQTT to Ethernet");
    pubsubclient->subscribe("inTopic");

    char localnetip[20];
    sprintf(localnetip,"1-myethernetarduino");
    pubsubclient->publish("connect",localnetip);

    return true;
  }
  else{
    Serial.println("MQTT failed!");
    return false;
  }
}



boolean MqttFlare::loop(){
  pubsubclient->loop();

  static long sendmillis = millis(); 
  if (millis()-sendmillis>30000){

    char localnetip[20];
    //sprintf(localnetip,"%s",Ethernet.localIP());
    //pubsubclient.publish("myethernetarduino/ip",localnetip);

    sprintf(localnetip,"0-myethernetarduino");
    pubsubclient->publish("connect",localnetip);

    for (int i=1;i<9;i++){
      char sensname[30];
      sprintf(sensname,"myethernetarduino/sens/%d",i);
      sprintf(localnetip,"%d",rand());
      pubsubclient->publish(sensname,localnetip);
    }
    sendmillis=millis();
  }

  checkEthernet();
}


void MqttFlare::checkEthernet(){

  // listen for incoming clients
  EthernetClient client = ethernetServer->available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
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
          client.println("<link href=\"http://getbootstrap.com/dist/css/bootstrap.min.css\" rel=\"stylesheet\">");
          // output the value of each analog input pin
          client.println("<body>");
          client.println("<div class='container'>");
          client.println("<form role='form' method='get'>");
          client.println("<div class='form-group'> <label class='col-sm-2'> MQTT Broker </label> <input type='col-sm-10  text' class='form-control' value='150.140.5.11'/> </div>");
          client.println("<div class='form-group'> <label class='col-sm-2'> IP </label> <input type='col-sm-10  text' class='form-control' value='150.140.5.11'/> </div>");
          //client.println("<div class='form-group'> <label class='col-sm-2'> IP </label> <select class='col-sm-10 form-control'><option>DHCP</option><option>STATIC</option></select> </div>");
          client.println("<div class='form-group'> <label class='col-sm-2'> Static IP </label> <input type='col-sm-10  text' class='form-control' value='192.168.1.10'/> </div>");
          client.println("<div class='form-group'> <label class='col-sm-2'> Netmask </label> <input type='col-sm-10  text' class='form-control' value='255.255.0.0'/> </div>");
          client.println("<div class='form-group'> <label class='col-sm-2'> Gateway </label> <input type='col-sm-10  text' class='form-control' value='192.168.1.254'/> </div>");
          client.println("<div class='form-group'> <input type='submit' class='btn btn-primary' value='Save and Restart' class='col-sm-2'/> </div>");
          client.println("</form>");   
          client.println("</div>");
          client.println("</body>");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
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
    Serial.println("client disonnected");
  }
}


