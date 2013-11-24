### UberdustSensors



* `RandomSensor` an easy to use Sensor that provides random integer values that can be used for testing during the initial development of a CoAP application. Random values are generated using the Arduino `rand()` function and a limit number used to limit the generated numbers by `rand()%limit`  
* `ParentSensor` a Sensor device used to provide debugging information to the developers for routing information used to propagate the information to the gateway devices.
* `PirSensor`  ( Link to Hardware ) 
*	`CarbonSensor`  ( Link to Hardware ) 
*	`LightSensor`  ( Link to Hardware ) 
*	`MethaneSensor`  ( Link to Hardware ) 
*	`SwitchSensor`  ( Link to Hardware ) 
*	`TemperatureSensor`  ( Link to Hardware ) 
*	`Temperaturesensor`  ( Link to Hardware ) 
*	`UltrasonicSensor`  ( Based on [HC-SRO4](http://users.ece.utexas.edu/~valvano/Datasheets/HCSR04b.pdf) Reads the input from the attached sensor and reports its measurement as a `TRUE` or `FALSE` presence measurement.
*	`ZoneSensor`  ( No Hardware ) Allows to control a single Arduino Digital Pin. `1` sets the pin to `HIGH` and `0` to `LOW`.
*	`InvertedZoneSensor`  ( No Hardware ) Allows to control a single Arduino Digital Pin. `0` sets the pin to `HIGH` and `1` to `LOW`.
