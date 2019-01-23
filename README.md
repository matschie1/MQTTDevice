# MQTTDevice

![Overview Image](/img/Overview.png)
## General Introduction

### What is it?

The MQTTDevice is an Arduino Sketch based on the ESP8266 to enable wireless stable communication between [CraftBeerPi V3](https://github.com/Manuel83/craftbeerpi3) actors and sensors.

### When do I need it?

If you have a centralized CBPi installation to collect and show all information at a central point, but you have sensors and actors all over the place. E.g. the fridge in the basement, the fermenter somewhere in the house and the brewery again someplace else.
This Device offers a wireless conntection via the MQTT protocol from CBPi to your sensors and actors.

### What does it offer?

* Web interface for configuration
* Sensors
  * Search for OneWire addresses
  * Value is read each second and sent to CraftBeerPi
* Actors
  * Choose PIN
  * Used PINS are not shown
  * Inverted GPIO
  * Power Percentage: If a value between 0 and 100% is sent, the ESP "pulses" with a duty cycle of 1000ms
* Induction
  * Control of a GGM Induction Cooker via serial communication

## Installation

[German Tutorial](https://hobbybrauer.de/forum/viewtopic.php?f=58&t=19036&p=309196#p309196)

EventManager:
Download EventManager (Copyright (c) 2016 Igor Mikolic-Torreira) from (https://github.com/InnuendoPi/arduino-EventManager) and unpack into ..\libraries\EventManager.
Change the value onErrorInterval as you prefer.

Activate or deactive the following lines to manage the behavior of Eventmanager
#define StopActorsOnSensorError
#define StopInductionOnSensorError

If you activate one or both lines corresponding actors will automatically set to off if sensor errors occour. The following sensor attributes are checked periodically: CRC valid, is connected (0 or 1), valid addresses, valid temperature (sensors are init with -127.0 degrees which means no valid data from sensor)

#define Debug 
for testing purpose and check output in serial monitor to find out working settings for your equipment.