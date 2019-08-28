![ov1](/img/overview.jpg)

# MQTTDevice

## General Introduction
### What is is?

The MQTTDevice is an Arduino Sketch based on the ESP8266 to enable stable communication between the CraftBeerPi and wireless actors and sensors.

### What does it offer?

* Web Interface for configuration

* Sensors
  * Search for OneWire addresses
  * value is read in a configurable period[sec], corrected by an optional offset and sent to CraftBeerPi
* Actors
  * Choose PIN
  * Used PINS are not shown
  * Inverted GPIO
  * Power Percentage: If a Value between 0 and 100% is sent, the ESP "pulses" with a duty cycle of 1000ms
* Induction
  * Control of a GGM Induction Cooker via serial communication


Installation: https://hobbybrauer.de/forum/viewtopic.php?f=58&t=19036&p=309196#p309196 (german)


### Requirements: (2019.08)

- Arduino IDE 1.8.9 
- ESP8266 by ESP8266 Community version 2.5.2
- download lib folder from repository

-- EventManager

-- ESP8266HTTPUpdateServer

- modify MQTTDevice.ino as you prefer:

-- set useDisplay true or false

-- VSCode: change path as your need #include "C:/Arduino/git/MQTTDevice/icons.h". 

-- Arduino IDE: simply use #include "icons.h".

- download and install the following libs in your Arduino IDE:  

NTPClient by Fabrice Weinberg Version 3.1.0

Adafruit GFX Library by Adafruit Version 1.5.6

Adafruit SSD1306 by Adafruit Version 1.3.0

ArduinoJSON by Benoit Blanchon Version 5.13.4 (only use this version! Do not update this lib!)

DallasTemperature by Miles Burton Version 3.8.0

OneWire By Jim Studt Version 2.3.4

PubSubClient by Nick O'Leary Version 2.7.0

Time by Michael Margolis Version 1.5.0

Timezone by Jack Christensen Version 1.2.2

WiFiManager by tzapu Version 0.14.0

TimeZone lib: open file library.properties and change the line architectures=avr into architectures=*

Please note: starting with Version 1.03 you need to install Adafruit SSD1306 and Adafruit GFX in your Arduino IDE. Libs are removed from repository!


### Main Functions

- Add, edit and remove sensors, actors and induction
- Configure OLED display
- Configure misc settings
- Firmware and SPIFFS Over the Air Update
- Firmware and SPIFFS update by file upload 
- Filebrowser for easy file management (eg backup and restore config.json)
- DS18B20 temperature offset

### Misc Menu:
In misc menu you can
- configure time period to update sensor, actor, induction, system and display data
- reset WiFi settings		-> ESP device will reboot in AP mode!
- clear all settings		-> ESP device will reboot in AP mode!
- edit MQTT broker IP address
- configure event handling (actors and induction on/off with delay)
- configure Debug output serial monitor
- configure mDNS

### EventManager:
Configured are 4 event queues: system, sensors, actors and induction. For example everything regarding the system will be thrown into the system queue, telling the eventmanager to proccess them by FIFO.

#define SYS_UPDATE  10		-> System update events should be queued approx. every 10ms

#define SEN_UPDATE  5000	-> Sensor data read should be queued approx. every 5s

#define ACT_UPDATE  10000	-> Actor data read/write should be queued approx. every 10s

#define IND_UPDATE  10000	-> Induction data read/write should be queued approx. every 10s

#define DISP_UPDATE 10000	-> Display screen update queued approx. every 10s

Beside those read and write events (normally handled by loop) also error events are queued. For example a sensors fails. If this event is queued you can if enabled in webif automatically switch off all actors and/or induction. Enter a value for delay on error as you want to delay the event switch off. The logic behind this delay is, that a single error event should not immediately turn off your brewery, but if an error event is still queued after some time, then you might prefer a turn off. 

CAUTION: If you do not know, what these time periods will effect, leave all of them on default! 
CAUTION: Setting SYS_UPDATE to a higher value will slow down device! Prefered value should be 0 (zero) which means: start all system handles ín any loop (no delay) 

### FileBrowser:
You can browse, down- an dupload files from/to spiffs. This makes it very easy to safe or restore configuration (config.json)

### Over the Air Updates:
ArduinoOTA can be activated on webif. Keep in mind to start OTA on ESP before you open Arduino-IDE.

### Debug information:
You can enable debug output on serial monitor for testing purpose or to find out working settings for your equipment.

### OLED Display: (not yet ready)
You can use OLED display. Up now only an OLED monochrome display 128x64 I2C is tested.
OLED display is activated in WebIf Display menu. Pins D1 and D2 are used for OLED.
WLAN and MQTT icon will automatically disappear, if an error raises up.
Sensors, actors and induction are displayed with their configured number. In an error event the number is replaced with "Er".


![oled1](/img/display3.jpg)
![oled2](/img/display2.jpg)
![oled3](/img/display.jpg)
![oled4](/img/display1.jpg)

Wiring ESP8266 D1 Mini, AZ-Delivery 0.96 i2c 128x64 OLED display

VCC -> 3.3V

GND -> GND

SCL -> D1

SDA -> D2

This repo is based on https://github.com/matschie1/MQTTDevice Main work is done by matschie! 
