/*
   Sketch für ESP8266
   Kommunikation via MQTT mit CraftBeerPi v3

   Unterstützung für DS18B20 Sensoren
   Unterstützung für GPIO Aktoren
   Unterstützung für GGM Induktionskochfeld
   Unterstützung für "PWM" Steuerung mit GPIO (Heizstab)

   Unterstützung for OverTheAir Firmware Changes
*/

/*########## INCLUDES ##########*/
//#include <Wire.h>               // i2C Kommunikation, derzeit ungenutzt
#include <OneWire.h>            // OneWire Bus Kommunikation
#include <DallasTemperature.h>  // Vereinfachte Benutzung der DS18B20 Sensoren

#include <ESP8266WiFi.h>        // Generelle WiFi Funktionalität
#include <ESP8266WebServer.h>   // Unterstützung Webserver
#include <WiFiManager.h>        // WiFiManager zur Einrichtung
#include <DNSServer.h>          // Benötigt für WiFiManager
#include <PubSubClient.h>       // MQTT Kommunikation

#include <FS.h>                 // SPIFFS Zugriff
#include <ArduinoJson.h>        // Lesen und schreiben von JSON Dateien

#include <ESP8266mDNS.h>        // OTA
#include <WiFiUdp.h>            // OTA
#include <ArduinoOTA.h>         // OTA
#include <EventManager.h>       // Eventmanager
/*########## KONSTANTEN #########*/

//#define DEBUG
//#define StopActorsOnSensorError
#define StopInductionOnSensorError

// OneWire
#define ONE_WIRE_BUS D3
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

// WiFi und MQTT
ESP8266WebServer server(80);
WiFiManager wifiManager;
WiFiClient espClient;
PubSubClient client(espClient);

// Induktion
/*  Signallaufzeiten */
const int SIGNAL_HIGH = 5120;
const int SIGNAL_HIGH_TOL = 1500;
const int SIGNAL_LOW = 1280;
const int SIGNAL_LOW_TOL = 500;
const int SIGNAL_START = 25;
const int SIGNAL_START_TOL = 10;
const int SIGNAL_WAIT = 10;
const int SIGNAL_WAIT_TOL = 5;

/*  Binäre Signale für Induktionsplatte */
int CMD[6][33] = {
  {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},  // Aus
  {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0},  // P1
  {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},  // P2
  {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0},  // P3
  {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},  // P4
  {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0}
};// P5
byte PWR_STEPS[] = {0, 20, 40, 60, 80, 100}; // Prozentuale Abstufung zwischen den Stufen

String errorMessages[10] = {
  "E0",
  "E1",
  "E2",
  "E3",
  "E4",
  "E5",
  "E6",
  "E7",
  "E8",
  "EC"
};

bool pins_used[17];
const byte pins[9] = {D0, D1, D2, D3, D4, D5, D6, D7, D8};
const byte numberOfPins = 9;
const String pin_names[9] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8"};

/*########## VARIABLEN #########*/

byte numberOfSensors = 0;              // Gesamtzahl der Sensoren
const byte numberOfSensorsMax = 10;    // Maximale Gesamtzahl Sensoren
byte addressesFound[numberOfSensorsMax][8];
byte numberOfSensorsFound = 0;

byte numberOfActors = 0;                // Gesamtzahl der Aktoren

char mqtthost[16] = "192.168.178.234";  // Default Value für MQTT Server
long mqttconnectlasttry;
long mqttconnectdelay = 5000;
byte mqttnumberoftrys = 3;
// if false, no mqtt communcation is send or received (security feature to turn on when leaving the brewery
// TODO: Not yet implemented!
bool mqttCommunication = true;

int mqtt_chip_key = ESP.getChipId();
char mqtt_clientid[25];

File fsUploadFile;                      // a File object to temporarily store the received file

EventManager gEM;                       // Eventmanager
unsigned long onErrorInterval = 10000; // Event interval 10sec waiting before
unsigned long lastToggledSys;           // System event delta
unsigned long lastToggledSen;           // Sensor event delta
unsigned long lastToggledAct;           // Actor event delta
unsigned long lastToggledInd;           // Induction event delta
