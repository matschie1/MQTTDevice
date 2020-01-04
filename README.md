![ov1](/img/fw105x.jpg)

# MQTTDevice

## Changelog

Version 1.060
- Reverted: ESP8266 V2.5.2
            Die Bibliothek ESP8266HTTPUpdateServer muss in den Ordner <arduino sketchfolder>\libraries kopiert werden
- Add:      WiFiManager configModeCallback (AP Mode)
- Fixed:    Ein lange bestehender Fehler im WLAN/mDNS wurde behoben
            Der Fehler konnte zu WLAN Abbrüchen oder Zuweisungung neuer IP im laufenden Betrieb führen
- Fixed:    WiFiManager saveconfig behoben (speichern MQTT broker IP)
- Reworked: Überprüfung der Zahleneinngaben im WeIf
- Reworked: Start MQTTDevice

Version 1.059
- Update:   Bibliotheken aktualisiert
            ESP8266 2.6.3: die Lib im Ordner lib/ESP8266HTTPUpdateServer muss gelöscht werden!
			-- Update Filesystem (SPIFFS) ist nun im Master branch ESP8266 enthalten
			Update aller zusätzlichen Bibliotheken 
- Fixed:    Typo index.html (sensorsw)

Version 1.058
- Fixed:    Sensoren EM

Version 1.057
- Fixed:    Winterzeit, falscher Wert für CET
- Reworked: MQTT reconnect bei IP Wechsel (Fritz.box)
- Improved: Speicherverbrauch


Version 1.056
- Added:    Überprüfung der Eingaben aus dem WebIf Sensor Offset und Kettle-IDs
            - Bei falscher Eingabe bspw. Offset  1,50 statt 1.50 wird der Wert durch 0 ersetzt
            Überprüfung der Eingabe aus dem WebIf Induktion PowerLevelOnError
            - Bei falscher Eingabe wird der Wert auf 100 gesetzt (event handling aus, keine Änderung im Fehlerfall)
- Reworked: Debug Ausgaben Read/Write config.json überarbeitet
- Added:    Deutsche Anleitung
- Added:    Anzeige Zieltemperatur im WebIf, wenn TCP Server aktiv ist und eine Kettle ID konfiguriert ist
- Cleanup


Version 1.055
- Fix:      DeserializationError Json6
- Added:    TCP Server Support


Version 1.050
- Update: 	Update ArduinoJson Version 6
- Update:	bootstrap
- Update:	jquery
- Reworked:	Read/Write configFile

-> vor dem Update die config.json sichern!

-> nach dem Update alle Einstellungen kontrollieren! 

-> Empfehlung: die Konfiguration neu aufbauen (ohne Backup einspielen) !!!

Version 1.048
- Fixed: 	Sensor Suche
- Reworked:	EventManager 
- Reworked: Sensor handling
- Fixed:	WebIf 
- Cleanup code und kleine Ändeurngen

## Generelle Informationen
### Was ist MQTTDevice?

MQTTDevice ist ein Arduino Sketch für die Module ESP8266. Damit ist es möglich eine Kommunikation zwischen CraftbeerPi3 und WLAN Sensoren und Aktoren herzustellen.

### Was bietet diese Firmware?

* Ein Web Interface (WebIf) für die Konfiguration

* Sensoren
  * Suche nach angeschlossenen Sensoren basierend auf OneWire Adressen
  * Das Leseintervall der Sensordaten und das Offset sind konfigurierbar (in Sek) 
* Aktoren
  * PIN Auswahl
  * PINs in Verwendung werden ausgeblendet
  * Invertierte GPIO
  * Power Percentage: Es werden Werte zwischen 0 und 100% gesendet. Das ESP8266 "pulses" mit einem Zyklus von 1000ms
* Induktion
  * das Induktionskochfeld GGM ID2 kann über serielle Kommunikation gesteuert werden
* OLED display Integration
* TCP Server Support
![tcp](/img/TCPServer.jpg)


Installation: https://hobbybrauer.de/forum/viewtopic.php?f=58&t=19036&p=309196#p309196 (german)


### Voraussetzungen: (2019.10)

* Arduino IDE 1.8.10
* Optional Microsoft VSCode + Arduino + ESP8266FS
* ESP8266 by ESP8266 Community version 2.6.3
* Folgende Bibliotheken müssen über die Arduino IDE hinzugefügt werden:
  * Standard Bibliotheken (buildin) von der Arduino IDE
    * ESP8266WiFi
    * ESP8266WebServer
    * DNSServer
    * ESP8266mDNS
    * SPI
    * Wire
  * Zusätzliche Bibliotheken
    * NTPClient by Fabrice Weinberg Version 3.2.0
    * Adafruit GFX Library by Adafruit Version 1.7.3
    * Adafruit SSD1306 by Adafruit Version 2.0.4
    * ArduinoJSON by Benoit Blanchon Version 6.13.0 
    * DallasTemperature by Miles Burton Version 3.8.0
    * OneWire By Jim Studt Version 2.3.5
    * PubSubClient by Nick O'Leary Version 2.7.0
    * Time by Michael Margolis Version 1.6.0
    * Timezone by Jack Christensen Version 1.2.4
    * WiFiManager by tzapu Version 0.15.0
    * TimeZone lib: die Datei library.properties muss in einem Editor bearbeitet werden Zeile: architectures=avr into architectures=*

    -> siehe Textdatei im Ordner ./lib/Timezone_library.properties.txt
    * EventManager Download von https://github.com/igormiktor/arduino-EventManager

    -> Die Bibliothek muss in den Sketchordner in den Unterordner libraries

    Wichtiger Hinweis:

	-> Ab Firmware Version 1.050 ist ArduinoJson 6 erforderlich

    -> Ab Firmware Version 1.048 ist die original EventManager Bibliothek erforderlich


### Wie kann die Firmware geflashed werden ohne den Quellcode zu komplilieren

* Mit Hilfe von esptool.exe (see https://github.com/igrr/esptool-ck/releases ) aus dem Ordner tools kann die Firmware auf das ESP Modul geladen werden

Beispiel für ein ESP8266 Modul vom Typ D1 mini mit 4MB Flash verbunden mit COM3

	* Eingabeaufforderung öffnen

	* in den Order ./MQTTDevice/build wechseln

		* Firmware aufspielen: ../tools/esptool.exe -ca 0x000000 -cd nodemcu -cp COM3 -cb 921600 -cf MQTTDevice.ino.bin

	* Das ESP8266 Modul resetten

	* Das ESP8266 Modul startet anschließend im Access Point Modus mit der IP Adresse 192.168.4.1

	* Das ESP8266 Modul über einen Webbrowser mit dem WLAN verbinden

    * Das SPIFFS kann nun über das WebIf ausgespielt werden <ip-address>/update

    * alternativ mit esptool: ../tools/esptool.exe -ca 0x100000 -cd nodemcu -cp COM3 -cb 921600 -cf MQTTDevice.spiffs.bin


* Updates
	Updates (firmware und SPIFFS) können über das WebIf geladen werden: <IP Adresse ESP Modul>/update

* Backup and restore der Konfiguration
    Der FileBrowser ist erreichbar über <IP Adresse ESP Modul>/edit download oder upload config.json 

### Die Hauptfunktionen

* Hinzufügen, editieren und löschen von Sensoren
* Auto reconnect MQTT
* Auto reconnect WLAN
* OLED display konfigurieren
* Misc Einstellungen bearbeiten
* Firmware und SPIFFS Updates über Dateiupload
* Firmware update OTA (Over The Air)
* Filebrowser für einefaches Datei-Management (zB backup und restore config.json)
* DS18B20 Temperatur Offset - einfaches kalibrieren der Sensoren
* Serielle Ausgabe über Telnet (Putty)
* Simulation

### Das Misc Menü:

* Konfiguration der Update Intervalle für Sensoren, Aktoren, Induktion, Display und Systemdienste
* Reset WiFi Einstellungen	-> ESP Modul startet im AP mode!
* Lösche alle Einstellungen	-> ESP Modul startet im AP mode!
* MQTT broker IP Adresse
* Konfiguration Event handling (Aktoren und Induktion im Fehlerfall ein/aus mit Verzögerung)
* Konfiguration Debug Ausgaben über den seriellen Monitor
* Konfiguration Telnet
* Konfiguration mDNS        -> MDNS Namen eingeben und über Webbrowser http://<mDNS Name> das ESP Modul aufrufen

### EventManager:
Es gibt 4 Warteschlangen für Ereignisse (Events), die automatisiert behandelt werden können: Sensoren, Aktoren, Induktion und System
Alle Ereignisse in einer Warteschlange werden nach dem FIFO Prinzip (First in First out) abgearbeitet.

* SYS_UPDATE  0		-> Systemdienste in der Warteschage sollten ohne Verzögerung abgeabreitet werden. Empfohlene Werte 0-1000 (0-1 Sek)
* SEN_UPDATE  5000	-> Sensordaten werden ca. alle 5 Sekunden abgefragt. Empfohlene Werte: 2000-5000 (2-5 Sek)
* ACT_UPDATE  5000	-> Aktordaten werden ca. alle 5 Sekunden abgeabreitet. Empfohlene Werte 2000-10000 (2-10 Sek)
* IND_UPDATE  5000	-> Daten für das Induktionsfeld werden ca. alle 5 Sekunden abgearbeitet. Empfohlene Werte 2000-10000 (2-10 Sek)
* DISP_UPDATE 5000	-> Das Display wird ca. alle 5 Sekunden aktualisiert. Empfohlene Werte 1000-5000 (2-10 Sek)
* TCP_UPDATE  60000 -> Daten an den TCP Server werden ca. alle 60 Sekunden gesendet.

Zusätzlich zu diesen Ereinissen werden Fehlerereignisse in die Warteschlangen geschrieben. Zum Beispiel ein Temperatursensor meldet einen Fehler oder der 
MQTT Broker auf dem RaspberryPi ist nicht erreichbar. Diese Ereignisse können automatisiert behandlet werden. Um eine autmoatisierte Fehlerbehandlung zu aktivieren
muss die Fehlerbehandlung (event handling) für Sensoren, Aktoren, Induktion, WLAN und/oder MQTT am Objekt aktiviert werden. Zusätzlich kann eine Wartezeit
konfiguriert werden, bevor die Fehlerbehandlung aktiv wird.
Beispiel:
Wenn der MQTT Broker auf dem RaspberryPi unerwartet die Verbindung beendet, dann
- wird automatisch versucht die Verbindung wieder aufzubauen
- die konfigurierte Wartezeit eine Verzögerung, bevor ein Gerät automatisch ausgeschaltet 
nach 5 erfolglosen Verbindungsversuchen mit dem MQTT Broker und 5 Wartezeiten zwischen den Versuchen kann bspw.
- das Induktionsfeld auf einen niedrigen Powerlevel gesetzt werden (von 100% auf 20% -> Temperatur halten)
Wenn ein Temeratursensor beim Brauen -127°C meldet, dann
- kann ein Aktor Pumpe am Nachgusskessel ausgeschaltet werden
- das Rührwerk im Sudkessel soll weiterlaufen


### FileBrowser:
Mit dem FileBrowser kann man sehr einfach im SPIFFS Dateisystem arbeiten. So ist es sehr einfach, die Konfiguration zu sichern und wiederherszustellen.


### Debug information:
Mithilfe der Debug Ausgaben über die serielle Konsole oder über Telnet an Putty kann die fehlerfreie Arbeit des ESP Moduls kontrolliert werden. 
Bei einem Fehler in der Firmware hilft die Ausgabe, den Fehelrfall zu reproduzieren.

### Simulation:
Die Simulation soll dabei helfen, die Einstellungen zu testen, insbesondere die Ereignisbehanldung und die Wartezeiten.

Wichtiger Hinweis 1: die Simulation schaltet alle konfiurierten Aktoren und Induktion beim Start ein und bei Abschluss wieder aus!

Die Simulation findet in 3 Schritten statt:
Durchgang 0 bis 9 simuliert einen Sensorfehler
Durchgang 10 bis 19 simuliert einen MQTT Fehlerfall
Durchgang 20 bis 29 simuliert einen WLAN Fehlerfall

Während einer Simulation sollte die Debug Ausgabe eingeschaltet werden.

### OLED Display:
Diese Firmware unterstützt OLED Display monochrom OLED 128x64 I2C 0.96".
Das Display kann über das WebIf konfiguriert werden. Wenn das Display aktiviert wird, sind die PINS D1 (SDL) und D2 (SDA) belegt. 
Sensoren, Aktoren und Induktion werden mit ihren aktuellen Werten dargestellt. Dabei bedeutet "S1 78 | A2 100 | I off" 
Sensor 1 meldet eine Temperatur von 78°C
Aktor 2 hat einen Powerlevel von 100%
Induktion ist ausgeschaltet (oder nicht konfiguriert)
Mit jeder Aktualisierung Display (siehe DISP_UPDATE) wandert die Anziege auf den nächsten Sensor bzw. Aktor. Im Beispiel wäre das S2 und A3

![oled1](/img/display4.jpg)

Anschluss ESP8266 D1 Mini an AZ-Delivery 0.96 i2c 128x64 OLED Display (Verwendung aller Information auf eigene Gefahr!)

 * VCC -> 3.3V
 * GND -> GND
 * SCL -> D1
 * SDA -> D2

Basis für diesen Sketch ist https://github.com/matschie1/MQTTDevice
