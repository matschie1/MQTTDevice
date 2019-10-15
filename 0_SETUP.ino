void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    yield(); // wait for serial port to connect. Needed for native USB port only
  }

  gEM.addListener(EventManager::kEventUser0, listenerSystem);
  gEM.addListener(EventManager::kEventUser1, listenerSensors);
  gEM.addListener(EventManager::kEventUser2, listenerActors);
  gEM.addListener(EventManager::kEventUser3, listenerInduction);

  // Set device name
  snprintf(mqtt_clientid, 25, "ESP8266-%08X", mqtt_chip_key);
  WiFi.hostname(mqtt_clientid);

  // Start sensors
  DS18B20.begin();

  // Load filesystem
  ESP.wdtFeed();
  if (!SPIFFS.begin())
  {
    DBG_PRINT("SPIFFS Mount failed");
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

  // Load configuration
  if (SPIFFS.exists("/config.json"))
  {
    DBG_PRINT("Firmware version: ");
    DBG_PRINTLN(Version);
    loadConfig();
  }
  else
    DBG_PRINTLN("Config file not found. Use defaults ...");

  // set pins as used
  pins_used[ONE_WIRE_BUS] = true;
  if (useDisplay)
  {
    pins_used[SDA] = true;
    pins_used[SDL] = true;
  }

  // WiFi Manager
  ESP.wdtFeed();
  WiFiManagerParameter cstm_mqtthost("host", "MQTT broker IP", mqtthost, 16);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&cstm_mqtthost);
  wifiManager.autoConnect(mqtt_clientid);
  strcpy(mqtthost, cstm_mqtthost.getValue());

  // Telnet
  if (startTEL)
    cbpiEventSystem(EM_TELSET); // Telnet

  // Save configuration
  ESP.wdtFeed();
  saveConfig();

  // Start MQTT
  ESP.wdtFeed();
  cbpiEventSystem(EM_MQTTCON); // MQTT connect
  cbpiEventSystem(EM_MQTTSUB); // MQTT subscribe

  // Display Start Screen
  dispStartScreen();

  // Load mDNS
  if (startMDNS)
  {
    cbpiEventSystem(EM_MDNSET);
  }

  // Init Arduino Over The Air
  if (startOTA)
  {
    setupOTA();
  }

  // Start Webserver
  setupServer();

  ESP.wdtFeed();
  cbpiEventSystem(EM_WLAN); // Check WLAN
  cbpiEventSystem(EM_MQTT); // Check MQTT
  cbpiEventSystem(EM_NTP);  // NTP handle
  cbpiEventSystem(EM_MDNS); // MDNS handle

  while (gEM.getNumEventsInQueue()) // Eventmanager process all queued events
  {
    gEM.processEvent();
  }
}

void setupServer()
{
  server.on("/", handleRoot);
  server.on("/setupActor", handleSetActor);       // Einstellen der Aktoren
  server.on("/setupSensor", handleSetSensor);     // Einstellen der Sensoren
  server.on("/reqSensors", handleRequestSensors); // Liste der Sensoren ausgeben
  server.on("/reqActors", handleRequestActors);   // Liste der Aktoren ausgeben
  server.on("/reqInduction", handleRequestInduction);
  server.on("/reqSearchSensorAdresses", handleRequestSensorAddresses);
  server.on("/reqPins", handlereqPins);
  server.on("/reqSensor", handleRequestSensor); // Infos der Sensoren für WebConfig
  server.on("/reqActor", handleRequestActor);   // Infos der Aktoren für WebConfig
  server.on("/reqIndu", handleRequestIndu);     // Infos der Indu für WebConfig
  server.on("/setSensor", handleSetSensor);     // Sensor ändern
  server.on("/setActor", handleSetActor);       // Aktor ändern
  server.on("/setIndu", handleSetIndu);         // Indu ändern
  server.on("/delSensor", handleDelSensor);     // Sensor löschen
  server.on("/delActor", handleDelActor);       // Aktor löschen
  server.on("/reboot", rebootDevice);           // reboots the whole Device
  server.on("/OTA", OTA);
  //server.on("/reconmqtt", reconMQTT);           // Reconnect MQTT
  server.on("/simulation", startSIM); // Simulation
  server.on("/reqDisplay", handleRequestDisplay);
  server.on("/reqDisp", handleRequestDisp); // Infos Display für WebConfig
  server.on("/setDisp", handleSetDisp);     // Display ändern
  server.on("/displayOff", turnDisplayOff); // Display on / off
  server.on("/reqMiscSet", handleRequestMiscSet);
  server.on("/reqMisc", handleRequestMisc); // Misc Infos für WebConfig
  server.on("/setMisc", handleSetMisc);     // Misc ändern

  // FSBrowser initialisieren
  server.on("/list", HTTP_GET, handleFileList); // list directory
  server.on("/edit", HTTP_GET, []() {           // load editor
    if (!handleFileRead("/edit.htm"))
    {
      server.send(404, "text/plain", "FileNotFound");
    }
  });
  server.on("/edit", HTTP_PUT, handleFileCreate);    // create file
  server.on("/edit", HTTP_DELETE, handleFileDelete); // delete file
  server.on("/edit", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  },
            handleFileUpload);

  server.onNotFound(handleWebRequests); // Sonstiges

  httpUpdate.setup(&server); // ESP8266HTTPUpdateServer.cpp https://github.com/esp8266/Arduino/pull/3732/files
  server.begin();
}

void setupOTA()
{
  ArduinoOTA.setHostname(mqtt_clientid);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
      type = "sketch";
      DBG_PRINTLN("OTA starting - updateing sketch");
    }
    else
    { // U_SPIFFS
      type = "filesystem";
      DBG_PRINTLN("OTA starting - updateing SPIFFS");
      //SPIFFS.end();
    }
    if (useDisplay)
      showDispOTA(0, 100);
  });
  ArduinoOTA.onEnd([]() {
    DBG_PRINTLN("OTA update finished!");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    if (useDisplay)
      showDispOTA(progress, total);
    DBG_PRINT("OTA in progress: ");
    DBG_PRINTLN((progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    if (useDisplay)
      showDispOTAEr(String(error));
    DBG_PRINT("Error: ");
    DBG_PRINTLN(error);
    if (error == OTA_AUTH_ERROR)
      DBG_PRINTLN("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)
      DBG_PRINTLN("Begin Failed");
    else if (error == OTA_CONNECT_ERROR)
      DBG_PRINTLN("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR)
      DBG_PRINTLN("Receive Failed");
    else if (error == OTA_END_ERROR)
      DBG_PRINTLN("End Failed");
  });
  ArduinoOTA.begin();
}
