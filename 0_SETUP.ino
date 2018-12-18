void setup() {
  Serial.begin(115200);

  // Sensoren Starten
  DS18B20.begin();        

  // Dateisystem laden
  ESP.wdtFeed();
  if (!SPIFFS.begin())  { Serial.println("SPIFFS Mount failed"); } 
  
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
  
  // MQTT starten
  client.setServer(mqtthost, 1883);
  client.setCallback(mqttcallback);

  // Webserver starten
  ESP.wdtFeed();
  setupServer();
}

void setupServer() {
  
  server.on("/",handleRoot);
  
  server.on("/setupActor",handleSetActor);        // Einstellen der Aktoren
  server.on("/setupSensor",handleSetSensor);      // Einstellen der Sensoren
  
  server.on("/reqSensors",handleRequestSensors);  // Liste der Sensoren ausgeben
  server.on("/reqActors",handleRequestActors);    // Liste der Aktoren ausgeben
  server.on("/reqInduction",handleRequestInduction);
  
  server.on("/reqSearchSensorAdresses",handleRequestSensorAddresses);
  server.on("/reqPins",handlereqPins);
  
  server.on("/reqSensor",handleRequestSensor);    // Infos der Sensoren für WebConfig
  server.on("/reqActor",handleRequestActor);      // Infos der Aktoren für WebConfig
  server.on("/reqIndu",handleRequestIndu);        // Infos der Indu für WebConfig

  server.on("/setSensor",handleSetSensor);        // Sensor Ändern
  server.on("/setActor",handleSetActor);          // Aktor ändern
  server.on("/setIndu",handleSetIndu);            // Indu ändenr

  server.on("/delSensor",handleDelSensor);        // Sensor löschen
  server.on("/delActor",handleDelActor);          // Aktor löschen
    
  server.onNotFound(handleWebRequests);           // Sonstiges
  
  server.begin();
}
