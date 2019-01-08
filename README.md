# MQTTDevice

![Overview Image](/img/Overview.png)
## General Introduction

### What is is?

The MQTTDevice is an Arduino Sketch based on the ESP8266 to enable stable communication between [CraftBeerPi V3](https://github.com/Manuel83/craftbeerpi3) and wireless actors and sensors.

### Why do I need it?

I can only speak for myself. I wanted a centralized CBPi installation that shows me all the information in one place, but needed sensors and actors all over the place. E.g. the fridge in the basement, the fermenter somewhere in the house and the brewery again someplace else.  
This is why I wanted WiFi-Connected Devices. MQTT offers a stable communication which is why I chose it.

### What does it offer?

* Web Interface for configuration
* Sensors
  * Search for OneWire addresses
  * value is read once a second and sent to CraftBeerPi
* Actors
  * Choose PIN
  * Used PINS are not shown
  * Inverted GPIO
  * Power Percentage: If a Value between 0 and 100% is sent, the ESP "pulses" with a duty cycle of 1000ms
* Induction
  * Control of a GGM Induction Cooker via serial communication

## Installation

[German Tutorial](https://hobbybrauer.de/forum/viewtopic.php?f=58&t=19036&p=309196#p309196)