![ov1](/img/fw104x.jpg)

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


### Requirements: (2019.10)

* Arduino IDE 1.8.10
* ESP8266 by ESP8266 Community version 2.5.2
* download lib folder from repository
  * EventManager
  * ESP8266HTTPUpdateServer
* modify MQTTDevice.ino as you prefer:
  * set useDisplay true or false
  * VSCode change path: #include "C:/Arduino/git/MQTTDevice/icons.h". 
  * Arduino IDE: simply use #include "icons.h".
* download and install the following libs in your Arduino IDE:  

  * NTPClient by Fabrice Weinberg Version 3.2.0
  * Adafruit GFX Library by Adafruit Version 1.5.7
  * Adafruit SSD1306 by Adafruit Version 1.3.0
  * ArduinoJSON by Benoit Blanchon Version 5.13.5 (only use this version! Do not update this lib!)
  * DallasTemperature by Miles Burton Version 3.8.0
  * OneWire By Jim Studt Version 2.3.5
  * PubSubClient by Nick O'Leary Version 2.7.0
  * Time by Michael Margolis Version 1.5.0
  * Timezone by Jack Christensen Version 1.2.2
  * WiFiManager by tzapu Version 0.15.0
  * TimeZone lib: open file library.properties and change the line architectures=avr into architectures=*

### How to flash

Download binary files (build folder) and use esptool.exe to flash
Example ESP8266 D1 mini 4MB flash size connected to COM3
* open cmd.exe
* navigate into build folder
	* flash firmware: esptool.exe -ca 0x000000 -cd nodemcu -cp COM3 -cb 921600 -cf MQTTDevice.ino.bin
	* flash SPIFFS: esptool.exe -ca 0x100000 -cd nodemcu -cp COM3 -cb 921600 -cf MQTTDevice.spiffs.bin
* reset ESP8266 device


### Main Functions

* Add, edit and remove sensors, actors and induction
* Configure OLED display
* Configure misc settings
* Firmware and SPIFFS Over the Air Update
* Firmware and SPIFFS update by file upload 
* Filebrowser for easy file management (eg backup and restore config.json)
* DS18B20 temperature offset
* Serial output via Telnet (Putty)
* Simulation

### Misc Menu:
In misc menu you can
* configure time period to update sensor, actor, induction, system and display data
* reset WiFi settings		-> ESP device will reboot in AP mode!
* clear all settings		-> ESP device will reboot in AP mode!
* edit MQTT broker IP address
* configure event handling (actors and induction on/off with delay)
* configure Debug output serial monitor
* activate Telnet
* configure mDNS. Recommended: 

### EventManager:
Configured are 4 event queues: system, sensors, actors and induction. For example everything regarding the system will be thrown into the system queue, telling the eventmanager to proccess them by FIFO.

* SYS_UPDATE  0		-> System update events should be queued with no delay (every loop). Recommended: 0
* SEN_UPDATE  5000	-> Sensor data read should be queued approx. every 5s.  Recommended: values between 2000-5000 (2-5sec)
* ACT_UPDATE  10000	-> Actor data read/write should be queued approx. every 10s.  Recommended: values between 2000-10000 (2-10sec)
* IND_UPDATE  10000	-> Induction data read/write should be queued approx. every 10s. Recommended: values between 2000-10000 (2-10sec)
* DISP_UPDATE 2000	-> Display screen update queued approx. every 2s.  Recommended: values between 1000-5000 (1-5sec)

Beside those read and write events (normally handled by loop) also error events are queued. For example a sensors fails. If this event is queued you can if enabled in webif automatically switch off all actors and/or induction. Enter a value for delay on error as you want to delay the event switch off. The logic behind this delay is, that a single error event should not immediately turn off your brewery, but if an error event is still queued after some time, then you might prefer a turn off. 

### FileBrowser:
You can browse, down- and upload files from or into spiffs. This makes it very easy to safe or restore configuration (config.json)

### Over the Air Updates:
ArduinoOTA can be activated on webif. Keep in mind to start OTA on ESP before you open Arduino-IDE.
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

This repo is based on https://github.com/matschie1/MQTTDevice Main work is done by matschie! 
