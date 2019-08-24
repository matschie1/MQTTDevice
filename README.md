Installation: https://hobbybrauer.de/forum/viewtopic.php?f=58&t=19036&p=309196#p309196 (german)

Requirements:
- Arduino IDE 1.8.8 (Version 1.8.9: 'byte does not name a type'. Try #define byte uint8_t)
- download lib folder
- modify MQTTDevice.ino as you prefer:
-- set useDisplay true or false
-- VSCode: change the path #include "C:/Arduino/git/MQTTDevice/icons.h". 
-- Arduino IDE: simply use #include "icons.h".
- download th follow libs:  
OneWire (by Jim Studt..) version 2.3.4

DallasTemperature (by Miles Burton...) version 3.8.0

PubSubClient (by Nick O''Leary) version 2.6.0

ArduinoJson (by Benoit Bianchon) version 5.13.4 (only this version! Do not update lib!)

WiFiManager (by tzapu) version 0.14.0

NTPClient (by by Fabrice Weinberg) version 3.1.0

Time by Michael Margolis Version 1.5.0

Timezone by Jack Christensen Version 1.2.2

TimeZone lib: open file library.properties and change the line architectures=avr into architectures=*

Main Functions

Misc Menu:
In misc menu you can
- reset WiFi settings		-> ESP device will reboot in AP mode!
- clear all settings		-> ESP device will reboot in AP mode!
- edit MQTT broker IP address
- configure event handling (actors and induction on/off with delay)
- configure Debug output serial monitor

EventManager:
Configured are 4 event queues: system, sensors, actors and induction. For example everything regarding the system will be thrown into the system queue, telling the eventmanager to proccess them by FIFO.

#define SYS_UPDATE  100		-> System update events should be queued approx. every 100ms

#define SEN_UPDATE  5000	-> Sensor data read should be queued approx. every 5s

#define ACT_UPDATE  10000	-> Actor data read/write should be queued approx. every 10s

#define IND_UPDATE  10000	-> Induction data read/write should be queued approx. every 10s

#define DISP_UPDATE 10000	-> Display screen update queued approx. every 10s

Beside those read and write events (normally handled by loop) also error events are queued. For example a sensors fails. If this event is queued you can if enabled in webif automatically switch off all actors and/or induction. Enter a value for delay on error as you want to delay the event switch off. The logic behind this delay is, that a single error event should not immediately turn off your brewery, but if an error event is still queued after some time, then you might prefer a turn off. 

FileBrowser:
You can browse, down- an dupload files from/to spiffs. This makes it very easy to safe or restore configuration (config.json)

Over the Air Updates:
ArduinoOTA can be activated on webif. Keep in mind to start OTA on ESP before you open Arduino-IDE.

mDNS:
You can active mDNS on webif if required.

Debug information:
You can enable debug output on serial monitor for testing purpose or to find out working settings for your equipment.

OLED Display: (not yet ready)
You can use OLED display. Up now only an OLED monochrome display 128x64 I2C is tested.
OLED display is activated in WebIf Display menu. Pins D1 and D2 are used for OLED.

![oled1](/img/display3.jpg)
![oled2](/img/display2.jpg)
![oled3](/img/display.jpg)
![oled4](/img/display1.jpg)

WLAN and MQTT icon will automatically disappear, if an error raises up.
Sensors, actors and induction are displayed with their configured number. In an error event the number is replaced with "Er".

This repo is based on https://github.com/matschie1/MQTTDevice Main work is done by matschie! 
