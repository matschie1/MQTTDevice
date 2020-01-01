/*
   Sketch für ESP8266
   Kommunikation via MQTT mit CraftBeerPi v3

   Unterstützung für DS18B20 Sensoren
   Unterstützung für GPIO Aktoren
   Unterstützung für GGM Induktionskochfeld
   Unterstützung für "PWM" Steuerung mit GPIO (Heizstab)

   Unterstützung für Web Update
   Unterstützung für OLED Display 126x64 I2C (D1+D2)
*/

/*########## INCLUDES ##########*/
#include <OneWire.h>           // OneWire Bus Kommunikation
#include <DallasTemperature.h> // Vereinfachte Benutzung der DS18B20 Sensoren
#include <Math.h>

#include <ESP8266WiFi.h>      // Generelle WiFi Funktionalität
#include <ESP8266WebServer.h> // Unterstützung Webserver
#include <ESP8266HTTPUpdateServer.h>

#include <WiFiManager.h>  // WiFiManager zur Einrichtung
#include <DNSServer.h>    // Benötigt für WiFiManager
#include <PubSubClient.h> // MQTT Kommunikation

#include <FS.h>          // SPIFFS Zugriff
#include <ArduinoJson.h> // Lesen und schreiben von JSON Dateien

#include <ESP8266mDNS.h>  // mDNS
#include <WiFiUdp.h>      // WiFi
#include <ArduinoOTA.h>   // OTA
#include <EventManager.h> // Eventmanager

#include <NTPClient.h> // NTP
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>
// Warnung: Bibliothek Timezone behauptet auf (avr) Architekturen ausgeführt werden zu können...
// in der Datei library.properties
// architectures=avr
// ändern in
// architectures=*
// Ordner lib Timezone_library.properties.txt

/*############ Version ############*/
const char Version[7] = "1.059";
/*############ Version ############*/

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
WiFiClient tcpClient;
PubSubClient client(espClient);
MDNSResponder mdns;
ESP8266HTTPUpdateServer httpUpdate;
WiFiServer TelnetServer(23); // declare telnet server
WiFiClient Telnet;
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
#define DEF_DELAY_IND 120000 //default delay after power off induction

/*  Binäre Signale für Induktionsplatte */
int CMD[6][33] = {
    {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},  // Aus
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0},  // P1
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},  // P2
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0},  // P3
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},  // P4
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0}}; // P5
unsigned char PWR_STEPS[] = {0, 20, 40, 60, 80, 100};                                                     // Prozentuale Abstufung zwischen den Stufen
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
    "EC"};

bool pins_used[17];
const unsigned char numberOfPins = 9;
const unsigned char pins[numberOfPins] = {D0, D1, D2, D3, D4, D5, D6, D7, D8};
const String pin_names[numberOfPins] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8"};

/*########## VARIABLEN #########*/
unsigned char numberOfSensors = 0;           // Gesamtzahl der Sensoren
const unsigned char numberOfSensorsMax = 10; // Maximale Gesamtzahl Sensoren
unsigned char addressesFound[numberOfSensorsMax][8];
unsigned char numberOfSensorsFound = 0;
unsigned char numberOfActors = 0;          // Gesamtzahl der Aktoren
const unsigned char numberOfActorsMax = 6; // Maximale Gesamtzahl Aktoren
char mqtthost[16] = "192.168.100.30";      // Default Value für MQTT Server

// Set device name
int mqtt_chip_key = ESP.getChipId();
char mqtt_clientid[25];

/* ## Define NTP properties ## */
#define NTP_OFFSET 60 * 60         // NTP in seconds
#define NTP_INTERVAL 60 * 1000     // NTP in miliseconds
#define NTP_ADDRESS "pool.ntp.org" // NTP change this to whatever pool is closest (see ntp.org)
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);
/*########## VARIABLEN #########*/

/*######### FileBrowser #########*/
File fsUploadFile; // a File object to temporarily store the received file
/*######### FileBrowser #########*/

/*######### EventManager ########*/
EventManager gEM; //  Eventmanager
IPAddress aktIP;  //  Workaround IP changed

int SEN_UPDATE = 5000;  //  sensors update delay loop
int ACT_UPDATE = 5000;  //  actors update delay loop
int IND_UPDATE = 5000;  //  induction update delay loop
int DISP_UPDATE = 5000; //  NTP and display update
int SYS_UPDATE = 0;     //  sys update delay - 0 means no delay (handle every loop)
int TCP_UPDATE = 60000; //  TCP server Update interval

// System error events
#define EM_WLANER 1
#define EM_MQTTER 2
#define EM_SPIFFS 6
#define EM_WEBER 7

// System triggered events
#define EM_MQTTRES 10
#define EM_REBOOT 11
#define EM_OTASET 12

// System run & set events
#define EM_WLAN 20
#define EM_OTA 21
#define EM_MQTT 22
#define EM_WEB 23
#define EM_MDNS 24
#define EM_NTP 25
#define EM_MDNSET 26
#define EM_MQTTCON 27
#define EM_MQTTSUB 28
#define EM_DISPUP 30
#define EM_TELSET 31
#define EM_TELNET 32
#define EM_TCP 33

// Sensor, actor and induction
#define EM_OK 0 // Normal mode
// Sensor, actor and induction error
#define EM_CRCER 1   // Sensor CRC failed
#define EM_DEVER 2   // Sensor device error
#define EM_UNPL 3    // Sensor unplugged
#define EM_SENER 4   // Sensor all errors
#define EM_ACTER 10  // Actor error
#define EM_INDER 10  // Induction error
#define EM_ACTOFF 11 // Actor error
#define EM_INDOFF 11 // Induction error

#define PAUSE1SEC 1000
#define PAUSE2SEC 2000
#define PAUSEDS18 750

// WLAN and MQTT reconnect parameters
bool StopOnWLANError = false;         // Use webif to configure: switch on/off event handling actors and induvtion on WLAN error
bool StopOnMQTTError = false;         // Use webif to configure: switch on/off event handling actors and induvtion on MQTT error
int retriesWLAN = 1;                  // Counter WLAN reconnects
int retriesMQTT = 1;                  // Counter MQTT reconnects
unsigned long mqttconnectlasttry = 0; // Timestamp MQTT
unsigned long wlanconnectlasttry = 0; // Timestamp WLAN
bool mqtt_state = true;               // Error state MQTT
bool wlan_state = true;               // Error state WLAN

#define maxRetriesWLAN 5        // Max retries before errer event
#define maxRetriesMQTT 5        // Max retries before error event
int wait_on_error_mqtt = 25000; // How long should device wait between tries to reconnect WLAN      - approx in ms
int wait_on_error_wlan = 25000; // How long should device wait between tries to reconnect WLAN      - approx in ms
// Sensor reconnect parameters
int wait_on_Sensor_error_actor = 120000;     // How long should actors wait between tries to reconnect sensor    - approx in ms
int wait_on_Sensor_error_induction = 120000; // How long should induction wait between tries to reconnect sensor - approx in ms

bool StopActorsOnError = false;    // Use webif to configure: switch on/off event handling actors on sensor error after wait_on_Sensor_error_actor ms
bool StopInductionOnError = false; // Use webif to configure: switch on/off event handling Induction on sensor error after wait_on_Sensor_error_induction ms

bool startOTA = false;
bool startMDNS = false;
bool startTEL = false;
bool startTCP = false;
char nameMDNS[16];

unsigned long lastToggledSys = 0;  // Timestamp system event
unsigned long lastToggledSen = 0;  // Timestamp sensor event
unsigned long lastToggledAct = 0;  // Timestamp actors event
unsigned long lastToggledInd = 0;  // Timestamp induction event
unsigned long lastToggledDisp = 0; // Timestamp display event
unsigned long lastToggledEvent = 0;
unsigned long lastToggledTCP = 0; // Timestamp system event
unsigned long lastSenAct = 0;     // Timestap actors on sensor error
unsigned long lastSenInd = 0;     // Timestamp induction on sensor error

int sensorsStatus = 0;
int actorsStatus = 0;
int inductionStatus = 0;
/*######### EventManager ########*/

/*########## TCP Server #########*/
int tcpPort = 9501;                  // TCP server Port
char tcpHost[16] = "192.168.100.30"; // TCP server IP
/*########## TCP Server #########*/

/*########### DISPLAY ###########*/
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128       // OLED display width, in pixels
#define SCREEN_HEIGHT 64       // OLED display height, in pixels
#define DISP_DEF_ADDRESS 0x3C  // Only used on init setup!
#define OLED_RESET LED_BUILTIN //4
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#include "icons.h"
bool useDisplay = false;
#define SDL D1
#define SDA D2
const unsigned char numberOfAddress = 2;
const int address[numberOfAddress] = {0x3C, 0x3D};

// Simulation
#define SIM_NONE 0
#define SIM_SEN_ERR 1
#define SIM_WLAN 2
#define SIM_MQTT 3
#define SIM_ACT 20 // SIM event start actors
#define SIM_IND 20 // SIM event start induction
int sim_mode = SIM_NONE;
int sim_counter = 0;
