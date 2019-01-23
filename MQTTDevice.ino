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
#include <WiFiManager.h>        // WiFiManager zur Einrichtung
#include <DNSServer.h>          // Benötigt für WiFiManager
#include <PubSubClient.h>       // MQTT Kommunikation

#include <FS.h>                 // SPIFFS Zugriff
#include <ArduinoJson.h>        // Lesen und schreiben von JSON Dateien

#include <ESP8266mDNS.h>        // OTA
#include <WiFiUdp.h>            // OTA
#include <ArduinoOTA.h>         // OTA
#include <EventManager.h>       // Eventmanager

#include <NTPClient.h>          // NTP
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>

/*############ DEBUG ############*/
#define DEBUG                 // Uncomment this line for debug output on serial monitor

#ifdef DEBUG
#define DBG_PRINT(x)     Serial.print (x)
#define DBG_PRINTLN(x)  Serial.println (x)
#else
#define DBG_PRINT(x)
#define DBG_PRINTLN(x)
#endif
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
long mqttconnectlasttry;
long mqttconnectdelay = 5000;
byte mqttnumberoftrys = 3;

int mqtt_chip_key = ESP.getChipId();
char mqtt_clientid[25];
bool dispEnabled = 0;                   // Display init default off

/* ## Set up NTP ## */
//unsigned long NTP_PRE_MILLIS = 0;
//const unsigned long NTP_INTERVALL = 3600000;

/* ## Set up the NTP UDP client ## */
/* ## Define NTP properties ## */
#define NTP_OFFSET   60 * 60            // OTA: in seconds
#define NTP_INTERVAL 60 * 1000          // OTA: in miliseconds
#define NTP_ADDRESS  "pool.ntp.org"     // change this to whatever pool is closest (see ntp.org)
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);
/*########## VARIABLEN #########*/

/*######### FileBrowser #########*/
File fsUploadFile;                      // a File object to temporarily store the received file
/*######### FileBrowser #########*/

/*######### EventManager ########*/
EventManager gEM;                       // Eventmanager
#define ON_ERROR_SEN 100000  // wait this time in ms before an event is raised up - change this value as you need
#define ON_ERROR_ACT 100000
#define ON_ERROR_IND 100000
#define SYS_UPDATE 300000


//#define StopActorsOnSensorError       // Uncomment this line, if you want to stop all actors on error after onErrorInterval ms
#define StopInductionOnSensorError      // Uncomment this line, if you want to stop InductionCooker on error after onErrorInterval ms

unsigned long lastToggledSys = 0;           // System event delta
unsigned long lastToggledSen = 0;           // Sensor event delta
unsigned long lastToggledAct = 0;           // Actor event delta
unsigned long lastToggledInd = 0;           // Induction event delta
/*######### EventManager ########*/

/*########### DISPLAY ###########*/

#define DISPLAY                         // Uncomment this line if you have an OLED display connected
//#ifdef DISPLAY
//#define DISP_PRINTVAL(x)    showDispVal (x)
//#define DISP_PRINTLINE(x)   showDispLine (x)
//#define DISP_CLEAR()       showDispClear ()
//#else
//#define DISP_PRINTVAL
//#define DISP_PRINTLINE
//#define DISP_CLEAR
//#endif

/*########### DISPLAY ###########*/

#ifdef DISPLAY
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <Fonts/FreeMono9pt7b.h>
//#include <Fonts/FreeMono24pt7b.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(-1);
#include "wlan_logo.h"
#include "mqtt_logo.h"
#include "cbpi_logo.h"
#endif
