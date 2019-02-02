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
#include <OneWire.h>            // OneWire Bus Kommunikation
#include <DallasTemperature.h>  // Vereinfachte Benutzung der DS18B20 Sensoren

#include <ESP8266WiFi.h>        // Generelle WiFi Funktionalität
#include <ESP8266WebServer.h>   // Unterstützung Webserver
#include <ESP8266HTTPUpdateServer.h>

#include <WiFiManager.h>        // WiFiManager zur Einrichtung
#include <DNSServer.h>          // Benötigt für WiFiManager
#include <PubSubClient.h>       // MQTT Kommunikation

#include <FS.h>                 // SPIFFS Zugriff
#include <ArduinoJson.h>        // Lesen und schreiben von JSON Dateien

#include <ESP8266mDNS.h>        // mDNS
#include <WiFiUdp.h>            // WiFi
#include <ArduinoOTA.h>         // OTA
#include <EventManager.h>       // Eventmanager

#include <NTPClient.h>          // NTP
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>

/*############ DEBUG ############*/
bool setDEBUG = true;
/*############ DEBUG ############*/

/*########## KONSTANTEN #########*/
// OneWire
#define ONE_WIRE_BUS D3
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

// WiFi und MQTT
ESP8266WebServer server(80);
WiFiManager wifiManager;
WiFiClient espClient;
PubSubClient client(espClient);
MDNSResponder mdns;
ESP8266HTTPUpdateServer httpUpdate;
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
char mqtthost[16] = "192.168.100.30";  // Default Value für MQTT Server

// Set device name
int mqtt_chip_key = ESP.getChipId();
char mqtt_clientid[25];

/* ## Define NTP properties ## */
#define NTP_OFFSET   60 * 60            // NTP in seconds
#define NTP_INTERVAL 60 * 1000          // NTP in miliseconds
#define NTP_ADDRESS  "pool.ntp.org"     // NTP change this to whatever pool is closest (see ntp.org)
//#define NTP_ADDRESS  "172.22.10.11"
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);
/*########## VARIABLEN #########*/

/*######### FileBrowser #########*/
File fsUploadFile;                      // a File object to temporarily store the received file
/*######### FileBrowser #########*/

/*######### EventManager ########*/
EventManager gEM;                   // Eventmanager
long SEN_UPDATE = 5000;            //  wait this time in ms before a sensor event is raised up - change this value as you need
long ACT_UPDATE = 10000;          //  actor event
long IND_UPDATE = 10000;           //  induction cooker event
long DISP_UPDATE = 10000;           //  NTP and display update

#define SYS_UPDATE  100
#define MQTT_DELAY  30000           // MQTT reconnect
// System error events
#define EM_WLANER 1
#define EM_MQTTER 2
#define EM_SPIFFS 6
#define EM_WEBER  7

// System triggered events 
#define EM_MQTTDIS 10
#define EM_REBOOT 11
#define EM_OTASET 12

// System run & set events
#define EM_WLAN   20
#define EM_OTA    21
#define EM_MQTT   22
#define EM_WEB    23
#define EM_MDNS   24
#define EM_NTP    25
#define EM_MDNSET 26

#define EM_DISPUP 30

// Sensor, actor and induction 
#define EM_OK     0   // Normal mode
// Sensor, actor and induction error
#define EM_CRCER  1   // Sensor CRC failed
#define EM_DEVER  2   // Sensor device error
#define EM_UNPL   3   // Sensor unplugged
#define EM_SENER  4   // Sensor all errors
#define EM_ACTER  1   // Actor error
#define EM_INDER  1   // Induction error

#define PAUSE1SEC 1000
#define PAUSE2SEC 2000

bool startOTA = false;
bool startMDNS = true;
char nameMDNS[16];
bool StopActorsOnError = false;             // Use webif to configure: switch on/off if you want to stop all actors on error after WAIT_ON_ERROR ms
bool StopInductionOnError = false;          // Use webif to configure: switch on/off if you want to stop InductionCooker on error after WAIT_ON_ERROR ms
long wait_on_error_actors = 60000;          // approx in ms - use webif to configure
long wait_on_error_induction = 30000;       // approx in ms - use webif to configure

unsigned long lastToggledSys = 0;           // System event delta
unsigned long lastToggledSen = 0;           // Sensor event delta
unsigned long lastToggledAct = 0;           // Actor event delta
unsigned long lastToggledInd = 0;           // Induction event delta
unsigned long lastToggledDisp = 0;
unsigned long mqttconnectlasttry = 0;
unsigned long lastSenAct;
unsigned long lastSenInd;
unsigned long lastSysAct;
unsigned long lastSysInd;

byte sensorsStatus = 0;
byte actorsStatus = 0;
byte inductionStatus = 0;
/*######### EventManager ########*/

/*########### DISPLAY ###########*/
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128                // OLED display width, in pixels
#define SCREEN_HEIGHT 64                // OLED display height, in pixels
#define DISP_DEF_ADDRESS 0x3c           // Only used on init setup!
Adafruit_SSD1306 display(-1);
#include "icons.h"

//const char* serverIndex = "<form method='POST' action='/firmware' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
//#include <ESP8266HTTPClient.h>
//#include <ESP8266httpUpdate.h>
