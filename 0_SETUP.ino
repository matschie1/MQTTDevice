void setup() {
  Serial.begin(115200);

  // Sensoren Starten
  DS18B20.begin();

  // Dateisystem laden
  ESP.wdtFeed();
  if (!SPIFFS.begin())  {
    Serial.println("SPIFFS Mount failed");
  }

  // Einstellungen laden
  ESP.wdtFeed();
  loadConfig();

  // WiFi Manager
  ESP.wdtFeed();
  WiFiManagerParameter cstm_mqtthost("host", "cbpi ip", mqtthost, 16);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&cstm_mqtthost);
  wifiManager.autoConnect("MQTTDevice");
  strcpy(mqtthost, cstm_mqtthost.getValue());

  // Änderungen speichern
  ESP.wdtFeed();
  saveConfig();

  // ArduinoOTA aktivieren
  setupOTA();
  
  // MQTT starten
  client.setServer(mqtthost, 1883);
  client.setCallback(mqttcallback);

  // Webserver starten
  ESP.wdtFeed();
  setupServer();
}

void setupServer() {

  server.on("/", handleRoot);

  server.on("/setupActor", handleSetActor);       // Einstellen der Aktoren
  server.on("/setupSensor", handleSetSensor);     // Einstellen der Sensoren

  server.on("/reqSensors", handleRequestSensors); // Liste der Sensoren ausgeben
  server.on("/reqActors", handleRequestActors);   // Liste der Aktoren ausgeben
  server.on("/reqInduction", handleRequestInduction);

  server.on("/reqSearchSensorAdresses", handleRequestSensorAddresses);
  server.on("/reqPins", handlereqPins);

  server.on("/reqSensor", handleRequestSensor);   // Infos der Sensoren für WebConfig
  server.on("/reqActor", handleRequestActor);     // Infos der Aktoren für WebConfig
  server.on("/reqIndu", handleRequestIndu);       // Infos der Indu für WebConfig

  server.on("/setSensor", handleSetSensor);       // Sensor ändern
  server.on("/setActor", handleSetActor);         // Aktor ändern
  server.on("/setIndu", handleSetIndu);           // Indu ändern

  server.on("/delSensor", handleDelSensor);       // Sensor löschen
  server.on("/delActor", handleDelActor);         // Aktor löschen

  server.on("/reboot", rebootDevice);             // reboots the whole Device
  server.on("/mqttOff", turnMqttOff);             // Turns off MQTT completly until reboot
  server.onNotFound(handleWebRequests);           // Sonstiges

  server.begin();
}


void setupOTA() {
  Serial.print("Configuring OTA device...");
  TelnetServer.begin();   //Necesary to make Arduino Software autodetect OTA device
  ArduinoOTA.onStart([]() {
    Serial.println("OTA starting...");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("OTA update finished!");
    Serial.println("Rebooting...");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA in progress: %u%%\r\n", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA OK");
}
