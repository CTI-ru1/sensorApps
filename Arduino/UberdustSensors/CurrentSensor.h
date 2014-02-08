#ifndef CURRENT_SENSOR
#define CURRENT_SENSOR

#include <CoapSensor.h>
//#include <EmonLib.h>                   // Include Emon Library
#define ICAL 30
class CurrentSensor: 
public CoapSensor 
{
public:
  //EnergyMonitor *monitor;
  CurrentSensor (char * name,int pin): 
  CoapSensor(name,30)
  {    
    //this->monitor = monitor;
    this->pin = pin;
    this->status=0.0;
  }

  void get_value( uint8_t* output_data, size_t* output_data_len)
  {
    send_value(this->status,output_data,output_data_len);
  }

  void check(void)
  {
    static unsigned long timestamp = 0;
    if(millis() - timestamp > 5000)
    {
      timestamp = millis();
      this->status =0;// monitor->calcIrms(1480)*1000;  // Calculate Irms only
      if (this->status<100){
	this->status=0;
      }
    }
  }

  int current(){
    return this->status;
  }
  
  double calcIrms(int NUMBER_OF_SAMPLES)
{
  	int lastSampleI,sampleI;                      

   #if defined emonTxV3
	int SUPPLYVOLTAGE=3300;
   #else 
	int SUPPLYVOLTAGE = readVcc();
   #endif

  
  for (int n = 0; n < NUMBER_OF_SAMPLES; n++)
  {
    lastSampleI = sampleI;
    sampleI = analogRead(pin);
    lastFilteredI = filteredI;
    filteredI = 0.996*(lastFilteredI+sampleI-lastSampleI);

    // Root-mean-square method current
    // 1) square current values
    sqI = filteredI * filteredI;
    // 2) sum 
    sumI += sqI;
  }

  double I_RATIO = ICAL *((SUPPLYVOLTAGE/1000.0) / 1023.0);
  Irms = I_RATIO * sqrt(sumI / NUMBER_OF_SAMPLES); 

  //Reset accumulators
  sumI = 0;
//--------------------------------------------------------------------------------------       
 
  return Irms;
}


long readVcc() {
  long result;
  
  //not used on emonTx V3 - as Vcc is always 3.3V - eliminates bandgap error and need for calibration http://harizanov.com/2013/09/thoughts-on-avr-adc-accuracy/

  #if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328__) || defined (__AVR_ATmega328P__)
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  
  #elif defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  ADCSRB &= ~_BV(MUX5);   // Without this the function always returns -1 on the ATmega2560 http://openenergymonitor.org/emon/node/2253#comment-11432
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
  ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  ADMUX = _BV(MUX3) | _BV(MUX2);
	
  #endif


  #if defined(__AVR__) 
  delay(2);                                        // Wait for Vref to settle
  ADCSRA |= _BV(ADSC);                             // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result;                     //1100mV*1024 ADC steps http://openenergymonitor.org/emon/node/1186
  return result;
 #elif defined(__arm__)
  return (3300);                                  //Arduino Due
 #else 
  return (3300);                                  //Guess that other un-supported architectures will be running a 3.3V!
 #endif
}

   //Useful value variables
    double realPower,
       apparentPower,
       powerFactor,
       Vrms,
       Irms;
	int lastSampleI,sampleI;                      
	double lastFilteredI, filteredI;                  
	double sqV,sumV,sqI,sumI,instP,sumP;              //sq = squared, sum = Sum, inst = instantaneous

};
#endif