![ov1](/img/fw105x.jpg)

# MQTTDevice

## Changelog
Version 1.055
- Fix:      DeserializationError Json6
- Added:    TCP Server Support


Version 1.050
- Update: 	Update ArduinoJson Version 6
- Update:	bootstrap
- Update:	jquery
- Reworked:	Read/Write configFile

-> before you update backup your config.json!

-> check your config after update! 

-> I recommend to build a new config from scratch!!!

Version 1.048
- Fixed: 	Sensor search
- Reworked:	EventManager (reverted back to original lib)
- Reworked: Sensor handling
- Fixed:	WebIf 
- Cleanup code and minor changes

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
* OLED display integration
* TCP Server Support
![tcp](/img/TCPServer.jpg)


Installation: https://hobbybrauer.de/forum/viewtopic.php?f=58&t=19036&p=309196#p309196 (german)


### Requirements: (2019.10)

* Arduino IDE 1.8.10
* Optional but recommended: Microsoft VSCode + Arduino + ESP8266FS
* ESP8266 by ESP8266 Community version 2.5.2
* download lib folder from repository
  * ESP8266HTTPUpdateServer (modified lib: add web SPIFFS updates https://github.com/esp8266/Arduino/pull/3732/files )

  -> copy ./lib/ESP8266HTTPUpdateServer into your sketchfolder/libraries folder
* download and install the following libs in your Arduino IDE:  
  * Standard libs with Arduino IDE
    * ESP8266WiFi 1.0 
    * ESP8266WebServer 1.0
    * DNSServer 1.1.1
    * ESP8266mDNS Version 1.2
    * SPI 1.0
    * Wire 1.0
  * Check and add the following libs
    * NTPClient by Fabrice Weinberg Version 3.2.0
    * Adafruit GFX Library by Adafruit Version 1.5.7
    * Adafruit SSD1306 by Adafruit Version 1.3.0
    * ArduinoJSON by Benoit Blanchon Version 6.12.0 
    * DallasTemperature by Miles Burton Version 3.8.0
    * OneWire By Jim Studt Version 2.3.5
    * PubSubClient by Nick O'Leary Version 2.7.0
    * Time by Michael Margolis Version 1.5.0
    * Timezone by Jack Christensen Version 1.2.2
    * WiFiManager by tzapu Version 0.15.0
    * TimeZone lib: open file library.properties and change the line architectures=avr into architectures=*

    -> see ./lib/Timezone_library.properties.txt
    * EventManager Download from https://github.com/igormiktor/arduino-EventManager

    -> copy lib into your sketchfolder/libraries folder

    Important note:

	-> starting with firmware 1.050 ArduinoJson 6 required

    -> starting with firmware 1.048 modified EventManager lib reverted to original lib


### How to flash without compile

* Use esptool.exe (see https://github.com/igrr/esptool-ck/releases ) to flash from tools folder

Example ESP8266 D1 mini 4MB flash size connected to COM3
	* open cmd.exe
	* navigate into build folder
		* flash firmware: esptool.exe -ca 0x000000 -cd nodemcu -cp COM3 -cb 921600 -cf MQTTDevice.ino.bin
		* flash SPIFFS: esptool.exe -ca 0x100000 -cd nodemcu -cp COM3 -cb 921600 -cf MQTTDevice.spiffs.bin
        * alternativ: flash SPIFFS after your device is connected to your WLAN using WebIf <ip-address>/update
	* reset ESP8266 device
	* ESP8266 device will reboot in AP mode with IP 192.168.4.1
	* Connect ESP8266 device to your WLAN

* Updates
	To install updates (firmware an SPIFFS) open the buildin WebIf: <ip address esp device>/update

* Backup and restore configuration
    Open the buildin FileBrowser <ip address esp device>/edit and download or upload config.json 

### Main Functions

* Add, edit and remove sensors, actors and induction
* Auto reconnect MQTT
* Auto reconnect WLAN
* Configure OLED display
* Configure misc settings
* Firmware and SPIFFS update by file upload
* Firmware update OTA (Over The Air)
* Filebrowser for easy file management (eg backup and restore config.json)
* DS18B20 temperature offset
* Serial output via Telnet (Putty)
* Simulation

### Misc Menu:

* configure time period to update sensor, actor, induction, system and display data
* reset WiFi settings		-> ESP device will reboot in AP mode!
* clear all settings		-> ESP device will reboot in AP mode!
* edit MQTT broker IP address
* configure event handling (actors and induction on/off with delay)
* configure Debug output serial monitor
* activate Telnet
* configure mDNS 

### EventManager:
Configured are 4 event queues: system, sensors, actors and induction. For example everything regarding the system will be thrown into the system queue, telling the eventmanager to proccess them by FIFO.

* SYS_UPDATE  0		-> System update events should be queued with no delay (every loop). Recommended: values between 0-1000(0-1sec)
* SEN_UPDATE  5000	-> Sensor data read should be queued approx. every 5s.  Recommended: values between 2000-5000 (2-5sec)
* ACT_UPDATE  10000	-> Actor data read/write should be queued approx. every 10s.  Recommended: values between 2000-10000 (2-10sec)
* IND_UPDATE  10000	-> Induction data read/write should be queued approx. every 10s. Recommended: values between 2000-10000 (2-10sec)
* DISP_UPDATE 2000	-> Display screen update queued approx. every 2s.  Recommended: values between 1000-5000 (1-5sec)

Beside those read and write events (normally handled by loop) also error events are queued. For example a sensors fails. If this event is queued you can if enabled in webif automatically switch off all actors and/or induction after a configured delay. Enter a value for delay on error as you want to delay the event switch off. The logic behind this delay is, that a single error event should not immediately turn off your brewery, but if an error event is still queued after some time, then you might prefer a turn off. For induction instead of turn off you can set a power state. For example 15% power to hold actual temperature in your kettle. 

### FileBrowser:
You can browse, down- and upload files from btw. into SPIFFS. This makes it very easy to safe or restore configuration (config.json)

### Updates:
Firmware has a buildin update modul. Open <ip-address esp8266>/update in your browser. 
Alternativly ArduinoOTA can be activated on webif. Keep in mind to start OTA on ESP before you open Arduino-IDE.
Updates by file instead of OTA is recommended.

### Debug information:
You can enable debug output on serial monitor or telnet (eg putty) for testing purpose or to find out working settings for your equipment.

### Simulation:
Simulation will start ALL actors and induction. In loop 0-9 sensors event, in loop 10-19 MQTT and finally in loop 20-29 WLAN events are simulated. Switchable actors and induction are modified. 
Simulation will use the time for delays configured in webif. At the end simulation will switch off all actors and induction. For simulation enable debug and connect serial monitor or your telnet client.
The main purpose of simulation is testing your delays and which actors should be event handling enabled or disabled.  

### OLED Display:
You can use OLED display. This firmware is tested with monochrom OLED 128x64 I2C 0.96".
OLED display is activated in WebIf Display menu. Pins D1 and D2 are used for OLED.
WLAN and MQTT icon will automatically disappear, if an error raises up.
Sensors, actors and induction are displayed with their actual values. Display output "S1 78 | A2 100 | I off" means: sensor1 temperature is 78°C, actor2 is on with power 100% and induction is turned off. 
Every loop (see DISP_UPDATE) next sensor or actor items value will be shown. In a sensor error event the temperature value is replaced with "Err".


![oled1](/img/display4.jpg)

Wiring ESP8266 D1 Mini, AZ-Delivery 0.96 i2c 128x64 OLED display (use this information on your own risk!)

 * VCC -> 3.3V
 * GND -> GND
 * SCL -> D1
 * SDA -> D2

This repo is based on https://github.com/matschie1/MQTTDevice
