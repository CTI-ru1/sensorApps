/**
 * Arduino Coap Application for CTI Sensor and Actuator Boards.
 *
 *
 */


//Include XBEE Libraries
#include <XBee.h>
#include <XbeeRadio.h>

//Include CoAP Libraries
#include <coap.h>

//Create the XbeeRadio object we'll be using
XBeeRadio xbee = XBeeRadio();
//Create a reusable response object for responses we expect to handle
XBeeRadioResponse response = XBeeRadioResponse();
//Create a reusable rx16 response object to get the address
Rx16Response rx = Rx16Response();

//CoAP object
Coap coap;

//Application Specific Variables
uint8_t lampStatuses[5];
uint8_t ledState;

//Relay Board Variables
uint8_t relayCheckPin;
uint8_t numOfRelays;

//Sensor Board Variables
uint8_t sensorsCheckPin ;
bool sensorsExist;
uint8_t pirPin, pirStatus;
uint8_t tempPin ;
int tempValue;
uint8_t lightPin ;
int lightValue;
uint8_t methanePin;
int methaneValue;
uint8_t carbonPin;
int carbonValue;

//Runs only once
void setup()
{
  //pinMode(10, OUTPUT);

  pinMode(12, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);

  digitalWrite( 2, LOW);
  digitalWrite( 3, LOW );
  digitalWrite( 4, LOW );
  digitalWrite( 5, LOW );
  digitalWrite( 6, LOW );  
  // comment out for debuging
  xbee.initialize_xbee_module();
  //start our XbeeRadio object and set our baudrate to 38400.
  xbee.begin( 38400 );
  //Initialize our XBee module with the correct values (using the default channel, channel 12)h
  xbee.init(12);
  // set coap object for callback functions
  //Wrapper::setObj(coap);
  lampStatuses[0] = 0;
  lampStatuses[1] = 0;
  lampStatuses[2] = 0;
  lampStatuses[3] = 0;
  lampStatuses[4] = 0;

  // init coap service 
  coap.init( &xbee, &response, &rx );

  coap.add_resource("pir"    , GET,  &pir  , true, 20, TEXT_PLAIN);
  coap.add_resource("lz1"    , GET | POST, &lz1, true, 20, TEXT_PLAIN);
  coap.add_resource("lz2"    , GET | POST, &lz2, true, 20, TEXT_PLAIN);
  coap.add_resource("lz3"    , GET | POST, &lz3, true, 20, TEXT_PLAIN);
  coap.add_resource("lz4"    , GET | POST, &lz4, true, 20, TEXT_PLAIN);
  coap.add_resource("lz5"    , GET | POST, &lz5, true, 20, TEXT_PLAIN);


  // resource id 0 is reserved for built in resource-discovery
  // init test resource, with resource id 1
  // inside init you must register a callback function
}


coap_status_t pir(uint8_t method, uint8_t* input_data, size_t input_data_len, uint8_t* output_data, size_t* output_data_len, queries_t queries);


void loop()
{
  coap.handler();
}


coap_status_t pir(uint8_t method, uint8_t* input_data, size_t input_data_len, uint8_t* output_data, size_t* output_data_len, queries_t queries){
  if( method == GET )  {
    *output_data_len = sprintf( (char*)output_data, "%d",pirStatus ); 
    return CONTENT;
  }
}


void setLamp(int lamp,uint8_t state)
{
  lampStatuses[lamp-1]=state;
  digitalWrite(lamp+1, state);  
}

coap_status_t lightZones(int zone,uint8_t method, uint8_t* input_data, uint8_t* output_data, size_t* output_data_len){
  if( method == GET )  {
    output_data[0] = 0x30 + lampStatuses[zone-1];
    *output_data_len = 1;
    return CONTENT;
  }
  else if ( method == POST ) {
    setLamp( zone , *input_data-0x30);
    output_data[0] = 0x30 + lampStatuses[zone-1];
    *output_data_len = 1;
    return CHANGED;
  }
}


coap_status_t lz1( uint8_t method, uint8_t* input_data, size_t input_data_len, uint8_t* output_data, size_t* output_data_len, queries_t queries){
  return lightZones( 1, method, input_data,output_data,  output_data_len);
}
coap_status_t lz2( uint8_t method, uint8_t* input_data, size_t input_data_len, uint8_t* output_data, size_t* output_data_len, queries_t queries){
  return lightZones( 2, method, input_data,output_data,  output_data_len);
}
coap_status_t lz3( uint8_t method, uint8_t* input_data, size_t input_data_len, uint8_t* output_data, size_t* output_data_len, queries_t queries){
  return lightZones( 3, method, input_data,output_data,  output_data_len);
}
coap_status_t lz4( uint8_t method, uint8_t* input_data, size_t input_data_len, uint8_t* output_data, size_t* output_data_len, queries_t queries){
  return lightZones( 4, method, input_data,output_data,  output_data_len);
}
coap_status_t lz5( uint8_t method, uint8_t* input_data, size_t input_data_len, uint8_t* output_data, size_t* output_data_len, queries_t queries){
  return lightZones( 5, method, input_data,output_data,  output_data_len);
}

