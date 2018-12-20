# MQTTDevice

## General Introduction
### What is is?

The MQTTDevice is an Arduino Sketch based on the ESP8266 to enable stable communication between the CraftBeerPi and wireless actors and sensors.

### Why do i need it?

I can only speak for myself. I wanted a centralized CBPi installation that shows me all the information in one place, but i need sensors and actors all over the place. E.g. the fridge in the basement, the fermenter somewhere in the house and the brewery again someplace else.

This is why i wanted WiFi-Connected Devices. MQTT offers a stable communication which is why i chose it.

### What does it offer?

* Web Interface for configuration

(https://github.com/matschie1/MQTTDevice/blob/master/img/Overview.png)

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
