void setup()
{
  Serial.begin(115200);

  // declare OneWire and PT Pins pin as used
  pins_used[ONE_WIRE_BUS] = true;
  pins_used[PT_PINS[0]] = true;
  pins_used[PT_PINS[1]] = true;
  pins_used[PT_PINS[2]] = true;
  Serial.println(PT_PINS[1]);
  Serial.println(pins_used[PT_PINS[1]]);

  // Sensoren Starten
  DS18B20.begin();

  // Dateisystem laden
  ESP.wdtFeed();
  if (!SPIFFS.begin())
  {
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
  client.setServer(mqtthost, MQTT_SERVER_PORT);
  client.setCallback(mqttcallback);

  // Webserver starten
  ESP.wdtFeed();
  setupServer();
}

void setupServer()
{
  server.on("/", handleRoot);

  // provides current sensor/actor/induction cooker readings
  server.on("/reqSensors", handleRequestSensors);
  server.on("/reqActors", handleRequestActors);
  server.on("/reqInduction", handleRequestInduction);

  // provides information about sensor/actor/induction cooker configuration
  server.on("/reqSensorConfig", handleRequestSensorConfig);
  server.on("/reqActorConfig", handleRequestActorConfig);
  server.on("/reqInductionConfig", handleRequestInductionConfig);

  // search for OneWire sensors on the bus
  server.on("/reqSearchSensorAdresses", handleRequestOneWireSensorAddresses);

  // returns the list of (named) pins on this chip
  server.on("/reqPins", handleRequestPins);

  // create or update sensor/actor/induction cooker
  server.on("/setSensor", handleSetSensor);
  server.on("/setActor", handleSetActor);
  server.on("/setIndu", handleSetIndu);

  // delete sensor/actor
  server.on("/delSensor", handleDelSensor);
  server.on("/delActor", handleDelActor);

  server.on("/reboot", rebootDevice); // reboots the device
  server.on("/mqttOff", turnMqttOff); // turns off MQTT completly until reboot

  server.onNotFound(handleWebRequests); // fallback

  server.begin();
}

void setupOTA()
{
  Serial.print("Configuring OTA device...");
  TelnetServer.begin(); // necesary to autodetect OTA device
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
    if (error == OTA_AUTH_ERROR)
      Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)
      Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR)
      Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR)
      Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR)
      Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA OK");
}
