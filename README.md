This is just another fork of MQTTDevice https://github.com/matschie1/MQTTDevice
Main work is done by matschie. Please use origin repository!

For internal use

OLED Display:
You can use OLED display. Up now only an OLED monochrome display 128x64 I2C is tested.
OLED display is activated in WebIf Display menu.
![oled1](/img/display3.jpg)
![oled2](/img/display2.jpg)
![oled3](/img/display.jpg)
![oled4](/img/display1.jpg)

WLAN and MQTT icon will automatically disappear, if an error raises up.
Sensors, actors and induction are displayed with their configured number. In an error event the number is replaced with "Er".

Misc Menu:
In misc menu you can
- reset WiFi settings		-> ESP device will reboot in AP mode!
- clear all settings		-> ESP device will reboot in AP mode!
- edit MQTT broker IP address
- configure event handling (actors and induction on/off with delay)
- configure Debug output serial monitor

EventManager:
Download EventManager (Copyright (c) 2016 Igor Mikolic-Torreira) from (https://github.com/InnuendoPi/arduino-EventManager) 

How does it works?
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
