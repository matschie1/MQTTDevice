This is just another fork for personal dev - please use origin repository https://github.com/matschie1/MQTTDevice

OLED Display:
You can use OLED display. Up now only an OLED monochrome display 128x64 I2C is tested.
To activate an OLED display uncomment the following line
#define DISPLAY                         // Uncomment this line if you have an OLED display connected
and check, if width and height fits with your hardware
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
There are defined a couple of predefined function to manage screen output
dispAPMode          	// show in AP Mode
dispSTAMode         	// Start screen in station mode
showDispClear       	// Clear Display
showDispDisplay		// SHow
showDispVal		// Display a String or an int value
showDispWlan		// Show WLAN icon
showDispMqtt		// Show MQTT icon
showDispCbpi		// Show CBPI icon
showDispLines		// Draw Lines
showDispSen		// Show sensor status
showDispAct		// show actor status
showDispInd		// show induction cooker status
showDispTime		// Show time (NTP enabled)
showDispIP		// Show IP address

These functions are just a basic set and useable everywhere you want eg while processing events. Customize as you want or use my predefined collection.

![oled](/img/oled2.jpg)
![AP-mode](/img/AP-Mode.jpg)
![config](/img/oled.jpg)

Misc Menu:
In misc menu you can
- reset WiFi settings		-> ESP device will reboot in AP mode!
- clear all settings		-> ESP device will reboot in AP mode!
- edit MQTT broker IP address

![Misc](/img/misc.jpg)

EventManager:
Download EventManager (Copyright (c) 2016 Igor Mikolic-Torreira) from (https://github.com/InnuendoPi/arduino-EventManager) 

How does it works?
The EventManager is the central processing logic. Every task, every decision .. anything will be queued and processed. 
Configured are 4 event queues: system, sensors, actors and induction and every queue has a listener. For example everything regarding the system will be thrown into the system queue, telling the eventmanager to proccess them by FIFO.
- cbpiEventSystem		-> everything around is handled in this queue

	-- WLAN 				event id 20
	
	-- Over the Air update	event id 21
	
	-- MQTT 				event id 22
	
	-- Webserver				event id 23
	
	-- mDNS					event id 24
	
	-- Display				event id 30
	
- cbpiEventSensors			-> get date from sensors 
- cbpiEventActors			-> get and send data from/to actors
- cbpiEventInduction		-> get and send data to induction cooker

In every loop events in all queues are processed. You can manage how often special events should be queued

#define SYS_UPDATE  100		-> System update events should be queued approx. every 100ms

#define SEN_UPDATE  2000	-> Sensor data read should be queued approx. every 2s

#define ACT_UPDATE  5000	-> Actor data read/write should be queued approx. every 5s

#define IND_UPDATE  5000	-> Induction data read/write should be queued approx. every 5s

#define DISP_UPDATE 10000	-> Display screen update queued approx. every 10s

Let's say you want your temperature sensor datas every 5 seconds. Change SEN_UPDATE to 5000 (ms) and every approx 5sec a new event read sensors in generated in th Sensor queue.  
Beside those read and write events (normally handled by loop) also errors events are queued. For example a temperature sensors fails. If this event is queued you can automatically switch off all actors and induction. Define a value for WAIT_ON_ERROR as you need how long you want to delay the event switch off. For example WAIT_ON_ERROR 30000 will wait approx. 30sec before switch off. The logic behind this delay is, that a single error event should not immediately turn off your brewery, but if an error event is still queued after WAIT_ON_ERROR time, then you might prefer a turn off. 

Activate or deactive the following lines to manage the main behavior of Eventmanager

//#define StopActorsOnSensorError       // Uncomment this line, if you want to stop all actors on error after WAIT_ON_ERROR ms

#define StopInductionOnSensorError      // Uncomment this line, if you want to stop InductionCooker on error after WAIT_ON_ERROR ms

With these two main switches you can decide, that an actor like a pump should not be turn off, if your sensor fails. Also if you share a single sensor between your tuns, you will not use this automatic.

The following sensor attributes are checked periodically: CRC valid, is connected (0 or 1), valid address, valid temperature.

Debug information:

#define DEBUG                 			// Uncomment this line for debug output on serial monitor

for testing purpose and check output in serial monitor to find out working settings for your equipment.
