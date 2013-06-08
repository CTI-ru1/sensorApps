#include "TreeRouting.h"

TreeRouting::TreeRouting(XBeeRadio * xbee,bool isGateway):
lastGatewayHeartbeat(0){
  this->isGateway=isGateway;
}
void TreeRouting::loop(){

  if (this->isGateway){
    if ((millis()-lastGatewayHeartbeat>10000) //check if interval has passed
    ||(millis()<lastGatewayHeartbeat) //check for overflow
    ){
      //TODO: implement the heart beat

        lastGatewayHeartbeat = millis();
    }
  }
  else{
    //TODO: implement routing checks
  }
}








