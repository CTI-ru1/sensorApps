/**
 * Program that use coap-ethernet to add the hardaware PowerStrip Version I (5 relay, 5 sensor) 
 */
//Uncomment for DEBUG in Serial Port
#define DEBUG

//Ethernet Libraries
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <avr/wdt.h>
byte mac[] = {0xaa, 0xbb, 0xBE, 0xEF, 0xFE, 0xEE};
IPAddress ip(150, 140, 5, 64);
unsigned int localPort = 5683;      // local port to listen on
EthernetUDP EthUDP;

//Coap Library
#include <coap.h>
//Hardware Library
//#include <PowerStrip1.h>
#include "Relay.h"
#include "Sensor.h"
#include "Indicate.h"
//#include "FreeMemory.h"
//#include "UptimeSensor.h"

Coap coap;
int SenA,SenB, SenC, SenD, SenE;
Sensor S[5]={ Sensor("1s\0", A0), Sensor("2s\0", A1),Sensor("3s\0", A2),Sensor("4s\0", A3),Sensor("5s\0", A4)};
Relay R[5]={ Relay("1r\0", 2,&S[0]), Relay("2r\0",3,&S[1]),Relay("3r\0",4,&S[2]),Relay("4r\0",5,&S[3]),Relay("5r\0",6,&S[4])};
//FreeMemory freeMem("mem\0");
//UptimeSensor uptime("up\0");
//Indicate I[5]={Indicate("1i\0",3,&S[0]),Indicate("2i\0",5,&S[1]),Indicate("3i\0",7,&S[2]),Indicate("4i\0",14,&S[3]),Indicate("5i\0",16,&S[4])};

void setup()
{
	DBG(
            Serial.begin(115200);
            Serial.println("");
            Serial.println("Setup...");
            )

	//pinMode(53, OUTPUT);

        //Start UDP server
	Ethernet.begin(mac,ip);
	EthUDP.begin(localPort);

        //Init Coap
        
	coap.init(&Ethernet, &EthUDP);
        //We create the object of the PowerStrip later of has been init the object coap, because is use for the object Pow
        int i;
	float value=0;
	float d[8];
	d[0] =0;
	d[1] =0;
	d[2] =0;
	d[3] =0;
	d[4] =0;
	d[5] =0;
	d[6] =0;
	d[7] =0;
	float minimum=5;
	int num=0;
	value = 0.0050*analogRead(A0);
	d[0]=abs(0-value);
	d[1]=abs(0.92-value);
	d[2]=abs(1.57-value);
	d[3]=abs(2.04-value);    
	d[4]=abs(2.41-value);                
	d[5]=abs(2.70-value);
	d[6]=abs(2.93-value);
	d[7]=abs(3.12-value);
	for(i=0;i<8;i++){
		minimum=min(minimum,d[i]);
	
	}
	int b;	
	for(b=0;b<8;b++){
		if (minimum == d[b]){ 
			num =b;
		}
	}
	num=5;

	
//	coap.add_resource(&freeMem);
	//coap.add_resource(&uptime);
	
	if(num>=1){
           coap.add_resource(&R[0]);
           coap.add_resource(&S[0]);
           //coap.add_resource(&I[0]);
	}
	if(num>=2){
	   coap.add_resource(&R[1]);
           coap.add_resource(&S[1]);
           //coap.add_resource(&I[1]);
	}
	if(num>=3){
           coap.add_resource(&R[2]);
           coap.add_resource(&S[2]);
           //coap.add_resource(&I[2]);
	}
	if(num>=4){
	  coap.add_resource(&R[3]);
          coap.add_resource(&S[3]);
          //coap.add_resource(&I[3]);
	}
	if(num>=5){
          coap.add_resource(&R[4]);
          coap.add_resource(&S[4]);
          //coap.add_resource(&I[4]);
	}
        //digitalWrite(53, HIGH);

	DBG(
            Serial.println("Setup...Done!");
            )
wdt_enable(WDTO_8S);
}

void loop(){
  wdt_reset();
	// nothing else should be done here. CoAP service is running
	// if there is a request for your resource, your callback function will be triggered
        coap.handler();
       SenA=S[0].status;
       SenB=S[1].status; 
       SenC=S[2].status;
       SenD=S[3].status;
       SenE=S[4].status;
}
