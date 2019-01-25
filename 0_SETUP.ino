void setup()
{
  Serial.begin(115200);
  
  gEM.addListener(EventManager::cbpiEventSystem, listenerSystem);
  gEM.addListener(EventManager::cbpiEventSensors, listenerSensors);
  gEM.addListener(EventManager::cbpiEventActors, listenerActors);
  gEM.addListener(EventManager::cbpiEventInduction, listenerInduction);

  // Start sensors
  DS18B20.begin();

  // Load filesystem
  ESP.wdtFeed();
  if (!SPIFFS.begin())
  {
    cbpiEventSystem(26);
  }
  else
  {
    DBG_PRINTLN("");
    Dir dir = SPIFFS.openDir("/");
    while (dir.next())
    {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      DBG_PRINT("FS File: ");
      DBG_PRINT(fileName.c_str());
      DBG_PRINT(" size: ");
      DBG_PRINTLN(formatBytes(fileSize).c_str());
    }
  }

  // Set device name
  ESP.wdtFeed();
  snprintf(mqtt_clientid, 25, "ESP8266-%08X", mqtt_chip_key);
  WiFi.hostname(mqtt_clientid);

  // Load configuration
  ESP.wdtFeed();
  loadConfig();
 
  // Check AP / STA Display Mode
  //cbpiEventSystem(32);  // Check AP
  dispAPMode();
  
//  while (gEM.getNumEventsInQueue())     // Eventmanager process all queued events
//  {
//    gEM.processEvent();
//  }

  // WiFi Manager
  ESP.wdtFeed();
  WiFiManagerParameter cstm_mqtthost("host", "MQTT broker IP", mqtthost, 16);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&cstm_mqtthost);
  wifiManager.autoConnect(mqtt_clientid);
  strcpy(mqtthost, cstm_mqtthost.getValue());

  // Check AP / STA Display Mode
  //cbpiEventSystem(33);  // Check STA
  dispSTAMode();

  // Save configuration
  ESP.wdtFeed();
  saveConfig();

  // Load mDNS
  ESP.wdtFeed();
  if (!MDNS.begin(mqtt_clientid))
  {
    cbpiEventSystem(26);
  }

  // Init Arduino Over The Air
  setupOTA();

  // Start MQTT
  client.setServer(mqtthost, 1883);
  client.setCallback(mqttcallback);

  /*
    // FSBrowser initialisieren
    // list directory
    server.on("/list", HTTP_GET, handleFileList);
    // load editor
    server.on("/edit", HTTP_GET, []() {
      if (!handleFileRead("/edit.htm")) {
        server.send(404, "text/plain", "FileNotFound");
      }
    });
    // create file
    server.on("/edit", HTTP_PUT, handleFileCreate);
    // delete file
    server.on("/edit", HTTP_DELETE, handleFileDelete);
    // first callback is called after the request has ended with all parsed arguments
    // second callback handles file uploads at that location
    server.on("/edit", HTTP_POST, []() {
      server.send(200, "text/plain", "");
    }, handleFileUpload);
    server.onNotFound([]() {
      if (!handleFileRead(server.uri())) {
        server.send(404, "text/plain", "FileNotFound");
      }
    });
  */
  // Start Webserver
  ESP.wdtFeed();
  setupServer();

  cbpiEventSystem(EM_MDNS);           // MDNS handle
  cbpiEventSystem(EM_WLAN);           // Check WLAN
  cbpiEventSystem(EM_MQTT);           // Check MQTT
  cbpiEventSystem(EM_DISPUP);         // Display Update
  while (gEM.getNumEventsInQueue())     // Eventmanager process all queued events
  {
    gEM.processEvent();
  }
}

void setupServer()
{

  server.on("/", handleRoot);

  server.on("/setupActor", handleSetActor);   // Einstellen der Aktoren
  server.on("/setupSensor", handleSetSensor); // Einstellen der Sensoren

  server.on("/reqSensors", handleRequestSensors); // Liste der Sensoren ausgeben
  server.on("/reqActors", handleRequestActors);   // Liste der Aktoren ausgeben
  server.on("/reqInduction", handleRequestInduction);

  server.on("/reqSearchSensorAdresses", handleRequestSensorAddresses);
  server.on("/reqPins", handlereqPins);

  server.on("/reqSensor", handleRequestSensor); // Infos der Sensoren für WebConfig
  server.on("/reqActor", handleRequestActor);   // Infos der Aktoren für WebConfig
  server.on("/reqIndu", handleRequestIndu);     // Infos der Indu für WebConfig

  server.on("/setSensor", handleSetSensor); // Sensor ändern
  server.on("/setActor", handleSetActor);   // Aktor ändern
  server.on("/setIndu", handleSetIndu);     // Indu ändern

  server.on("/delSensor", handleDelSensor); // Sensor löschen
  server.on("/delActor", handleDelActor);   // Aktor löschen

  server.on("/reboot", rebootDevice); // reboots the whole Device
  server.on("/mqttOff", turnMqttOff); // Turns off MQTT completly until reboot

#ifdef DISPLAY
  server.on("/reqDisplay", handleRequestDisplay);
  server.on("/reqDisp", handleRequestDisp); // Infos Display für WebConfig
  server.on("/setDisp", handleSetDisp);     // Display ändern
  server.on("/displayOff", turnDisplayOff); // Turns off display completly until reboot
#endif

  server.on("/reqMiscSet", handleRequestMiscSet);
  server.on("/reqMisc", handleRequestMisc); // Misc Infos für WebConfig
  server.on("/setMisc", handleSetMisc);     // Misc ändern

  // FSBrowser initialisieren
  server.on("/list", HTTP_GET, handleFileList);   // list directory
  server.on("/edit", HTTP_GET, []() {             // load editor
    if (!handleFileRead("/edit.htm")) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });
  server.on("/edit", HTTP_PUT, handleFileCreate); // create file
  server.on("/edit", HTTP_DELETE, handleFileDelete);  // delete file
  // first callback is called after the request has ended with all parsed arguments
  // second callback handles file uploads at that location
  server.on("/edit", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  }, handleFileUpload);
  server.onNotFound([]() {
    if (!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });

  // server.onNotFound(handleWebRequests);           // Sonstiges

  server.begin();
}

void setupOTA() {
  ArduinoOTA.onStart([]() {
    DBG_PRINTLN("OTA starting...");
  });
  ArduinoOTA.onEnd([]() {
    DBG_PRINTLN("OTA update finished!");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    DBG_PRINT("OTA in progress: ");
    DBG_PRINTLN((progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) DBG_PRINTLN("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) DBG_PRINTLN("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) DBG_PRINTLN("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) DBG_PRINTLN("Receive Failed");
    else if (error == OTA_END_ERROR) DBG_PRINTLN("End Failed");
  });
  ArduinoOTA.begin();
}
