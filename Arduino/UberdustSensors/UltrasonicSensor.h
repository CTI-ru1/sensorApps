#include <CoapSensor.h>

class UltrasonicSensor : 
public CoapSensor 
{
public:
  int triggerPin, echoPin, status;
  int maximumRange; // Maximum range needed
  int minimumRange; // Minimum range needed
  long duration;
    
    
  UltrasonicSensor(char * name, int triggerPin, int echoPin): 
  CoapSensor(name)
  {
    pinMode(triggerPin, OUTPUT);
    pinMode(echoPin, INPUT);
    this->triggerPin= triggerPin;
    this->echoPin= echoPin;
    this->status = 0;
    this->maximumRange=200; // Maximum range needed
    this->minimumRange=0; // Minimum range needed
  
  }
  void get_value( uint8_t* output_data, size_t* output_data_len)
  {
	check();
    *output_data_len = sprintf( (char*)output_data, "%d", this->status ); 
  }

  void check(void)
  {
    static unsigned long timestamp = 0;
    if(millis() - timestamp > 500)
    {
      int newStatus = checkPresence();
      if(newStatus != this->status)
      {
        this->changed = true;
        this->status = newStatus;
      }
      timestamp = millis();
    }
  }
  
    
  int checkPresence(){
    int distance  = getDistance();
    while (distance==-1 ){ 
      distance = getDistance();
    }

    if (distance >= maximumRange || distance <= minimumRange){
      /* Send a negative number to computer and Turn LED ON 
      to indicate "out of range" */
      return 0;
    }
    else if (distance>=100) {
      return 0;
    }
    else{
      return 1;
    }
  }

  int getDistance(){
    /* The following triggerPin/echoPin cycle is used to determine the
    distance of the nearest object by bouncing soundwaves off of it. */
    digitalWrite(triggerPin, LOW); 
    delayMicroseconds(2); 

    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10); 

    digitalWrite(triggerPin, LOW);
    duration = pulseIn(echoPin, HIGH);

    //Calculate the distance (in cm) based on the speed of sound.
    int distance = duration/58.2;

    if (distance >= maximumRange || distance <= minimumRange){
      /* Send a negative number to computer and Turn LED ON 
      to indicate "out of range" */
      return -1;
    }
    else {
      return distance;
    }
  }
};
