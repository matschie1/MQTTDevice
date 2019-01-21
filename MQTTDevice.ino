/*
   Sketch für ESP8266
   Kommunikation via MQTT mit CraftBeerPi v3

   Unterstützung für DS18B20 Sensoren
   Unterstützung für GPIO Aktoren
   Unterstützung für GGM Induktionskochfeld
   Unterstützung für "PWM" Steuerung mit GPIO (Heizstab)

   Unterstützung for OverTheAir Firmware Changes

   Supports PT100/1000 Sensors (using the Adafruit max31865 amplifier and library)
*/

/*########## INCLUDES ##########*/
//#include <Wire.h>             // i2C Kommunikation, derzeit ungenutzt

#include <OneWire.h>           // OneWire Bus Kommunikation
#include <DallasTemperature.h> // Vereinfachte Benutzung der DS18B20 Sensoren

#include <Adafruit_MAX31865.h> // PT100/1000

#include <ESP8266WiFi.h>      // Generelle WiFi Funktionalität
#include <ESP8266WebServer.h> // Unterstützung Webserver
#include <WiFiManager.h>      // WiFiManager zur Einrichtung
#include <DNSServer.h>        // Benötigt für WiFiManager
#include <PubSubClient.h>     // MQTT Kommunikation

#include <FS.h>          // SPIFFS Zugriff
#include <ArduinoJson.h> // Lesen und Schreiben von JSON Dateien

#include <ESP8266mDNS.h> // OTA
#include <WiFiUdp.h>     // OTA
#include <ArduinoOTA.h>  // OTA

/*########## CONSTANTS #########*/

// Differentiate between the two currently supported sensor types
#define SENSOR_TYPE_ONE_WIRE 0
#define SENSOR_TYPE_PT 1
// OneWire
// Change this according to your wiring
#define ONE_WIRE_BUS D6
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
// PT100/1000
// 430.0 for PT100 and 4300.0 for PT1000
#define RREF 430.0
// 100.0 for PT100, 1000.0 for PT1000
#define RNOMINAL 100.0
// default pin for the CS of a PT100 (for initial initialization, will be overwritten)
const byte defaultCSPin = 16;

// WiFi und MQTT
#define WEB_SERVER_PORT 80
#define TELNET_SERVER_PORT 8266
#define MQTT_SERVER_PORT 1883

const long mqttconnectdelay = 30000;
const byte mqttnumberoftrys = 3;

// Induktion Signallaufzeiten
const int SIGNAL_HIGH = 5120;
const int SIGNAL_HIGH_TOL = 1500;
const int SIGNAL_LOW = 1280;
const int SIGNAL_LOW_TOL = 500;
const int SIGNAL_START = 25;
const int SIGNAL_START_TOL = 10;
const int SIGNAL_WAIT = 10;
const int SIGNAL_WAIT_TOL = 5;

// Prozentuale Abstufung zwischen den Stufen
const byte PWR_STEPS[] = {0, 20, 40, 60, 80, 100};

// Error Messages der Induktionsplatte
const String errorMessages[10] = {"E0", "E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8", "EC"};

const byte numberOfPins = 9;
const byte pins[numberOfPins] = {D0, D1, D2, D3, D4, D5, D6, D7, D8};
const String pin_names[numberOfPins] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8"};

const byte numberOfSensorsMax = 10;           // max number of sensors per sensor type
const byte numberOfActorsMax = 6;             // max number of actors
const int defaultSensorUpdateInterval = 5000; // how often should sensors update

/*########## VARIABLES #########*/

ESP8266WebServer server(WEB_SERVER_PORT);
WiFiManager wifiManager;
WiFiClient espClient;
PubSubClient client(espClient);
WiFiServer TelnetServer(TELNET_SERVER_PORT); // OTA

//  Binäre Signale für Induktionsplatte
int CMD[6][33] = {
    {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0}, // Aus
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0}, // P1
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0}, // P2
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0}, // P3
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0}, // P4
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0}  // P5
};

bool pins_used[17];

/*
Common pins across all PT100/1000 sensors: DI, DO, CLK
When using multiple sensors, you can reuse these pins
and only define the CS PIN. (example: 2 sensors -> 5 pins needed)
*/
// current initial values:  D1, D2, D3 of NODEMCU Dev Board
byte PTPins[3] = {5, 4, 0};

byte numberOfSensors = 0; // current number of sensors
byte oneWireAddressesFound[numberOfSensorsMax][8];
byte numberOfOneWireSensorsFound = 0;

byte numberOfActors = 0; // current number of actors

char mqtthost[16] = "192.168.178.234"; // default value of MQTT Server
long mqttconnectlasttry;
// if false, no mqtt communcation is send or received
// (security feature to turn on when leaving the brewery)
bool mqttCommunication = true; // TODO: Not yet implemented!
