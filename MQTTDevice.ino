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

// DEFAULT PINS
// Change according to your wiring (see also 99_PINMAP_WEMOS_D1Mini)

#define ONE_WIRE_BUS D5
/*
  common pins across all PT100/1000 sensors
  DI, DO, CLK (currently hardwired in code, change here accordingly)
  When using multiple sensors, reuse these pins and only (re)define
  the CS PIN, meaning you need one additional pin per sensor
*/
const byte PT_PINS[3] = {D4, D3, D2};
// default pin for the CS of a PT sensor (for initial initialization, can be overwritten)
const byte DEFAULT_CS_PIN = D0;

// ranges from 9 to 12, higher is better (and slower!)
#define ONE_WIRE_RESOLUTION 10
// 430.0 for PT100 and 4300.0 for PT1000
#define RREF 430.0
// 100.0 for PT100, 1000.0 for PT1000
#define RNOMINAL 100.0

// WiFi und MQTT
#define WEB_SERVER_PORT 80
#define TELNET_SERVER_PORT 8266
#define MQTT_SERVER_PORT 1883

// Differentiate between the two currently supported sensor types
const String SENSOR_TYPE_ONE_WIRE = "OneWire";
const String SENSOR_TYPE_PT = "PTSensor";
const long MQTT_CONNECT_DELAY = 30000;
const byte MQTT_NUMBER_OF_TRIES = 3;

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
const String ERROR_MESSAGES[10] = {"E0", "E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8", "EC"};

const byte NUMBER_OF_PINS = 9;
const byte PINS[NUMBER_OF_PINS] = {D0, D1, D2, D3, D4, D5, D6, D7, D8};
const String PIN_NAMES[NUMBER_OF_PINS] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8"};

const byte NUMBER_OF_SENSORS_MAX = 6;            // max number of sensors per sensor type
const byte NUMBER_OF_ACTORS_MAX = 6;             // max number of actors
const int DEFAULT_SENSOR_UPDATE_INTERVAL = 5000; // how often should sensors update

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

// careful here, these are not the wemos-numbered GIPO (D0-D8) but all of them!
bool pins_used[17]; // determines which pins currently are in use

byte numberOfPTSensors = 0; // current number of PT100 sensors

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
byte oneWireAddressesFound[NUMBER_OF_SENSORS_MAX][8];
byte numberOfOneWireSensors = 0;      // current number of OneWire sensors
byte numberOfOneWireSensorsFound = 0; // OneWire sensors found on the bus

byte numberOfActors = 0; // current number of actors

char mqtthost[16] = "192.168.178.234"; // default value of MQTT Server
long mqttconnectlasttry;
// if false, no mqtt communcation is send or received
// (security feature to turn on when leaving the brewery)
bool mqttCommunication = true; // TODO: Not yet implemented!
