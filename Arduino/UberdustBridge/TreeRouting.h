#include <XBee.h>
#include <XbeeRadio.h>

class TreeRouting{
public:
  TreeRouting(XBeeRadio * xbee,bool isGateway);
  void loop();

private : 
  XBeeRadio *xbee;
  bool isGateway;
  long lastGatewayHeartbeat;
};

