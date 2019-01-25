void listenerSystem( int event, int parm )                           // System event listener
{
  switch (parm)
  {
    case 1: // WLAN error
      // Stop actors
      for (int i = 0; i < numberOfActors; i++) {
        if (actors[i].isOn) {
          Serial.printf("Set actor %i to off due to WLAN error%\r\n", i);
          actors[i].isOn = false;
          actors[i].Update();
          actors[i].publishmqtt();
        }
      }
      // Stop induction
      if (inductionCooker.isInduon) {
        DBG_PRINTLN("Set induction off due to WLAN error");
        inductionCooker.isInduon = false;
        inductionCooker.Update();
        inductionCooker.publishmqtt();
      }
      break;
    case 2: // MQTT Error
      // Stop actors
      for (int i = 0; i < numberOfActors; i++) {
        if (actors[i].isOn) {
          Serial.printf("Set actor %i off due to MQTT error%\r\n", i);
          actors[i].isOn = false;
          actors[i].Update();
          actors[i].publishmqtt();
        }
      }
      // Stop induction
      if (inductionCooker.isInduon) {
        DBG_PRINTLN("Set induction off due to MQTT error");
        inductionCooker.isInduon = false;
        inductionCooker.Update();
        inductionCooker.publishmqtt();
      }
      break;
    case 10: // Disable MQTT
      // Stop actors
      showDispSet("MQTT error");
      for (int i = 0; i < numberOfActors; i++) {
        if (actors[i].isOn) {
          Serial.printf("Set actor %i off due to MQTT disabled%\r\n", i);
          actors[i].isOn = false;
          actors[i].Update();
          actors[i].publishmqtt();
        }
      }
      // Stop induction
      if (inductionCooker.isInduon) {
        DBG_PRINTLN("Set induction off due to MQTT disabled");
        inductionCooker.isInduon = false;
        inductionCooker.Update();
        inductionCooker.publishmqtt();
      }
      //mqttCommunication = false;
      server.send(200, "text/plain", "CAUTION! I don't work yet: turned off, please reboot to turn on again...");
      break;
    case 11:          // Reboot ESP
      // Stop actors
      showDispSet("Reboot device");
      for (int i = 0; i < numberOfActors; i++) {
        if (actors[i].isOn) {
          Serial.printf("Set actor %i off due to reboot%\r\n", i);
          actors[i].isOn = false;
          actors[i].Update();
          actors[i].publishmqtt();
        }
      }
      // Stop induction
      if (inductionCooker.isInduon) {
        DBG_PRINTLN("Set induction off due to reboot");
        inductionCooker.isInduon = false;
        inductionCooker.Update();
        inductionCooker.publishmqtt();
      }
      server.send(200, "text/plain", "rebooting...");
      //delay(1000);
      showDispClear();
      ESP.restart();
      break;

    // Loop events comes here
    case EM_WLAN: // check WLAN (20)
      /* WiFi.status response code: WL_DISCONNECTED appears not to be precise, notified connection result 6 when connected - no clue about NO_SHIELD
        if      (WiFi.status() == WL_NO_SHIELD)       DBG_PRINTLN("Wifi Status: WL_NO_SHIELD");       // connection result 255
        else if (WiFi.status() == WL_IDLE_STATUS)     DBG_PRINTLN("Wifi Status: WL_IDLE_STATUS");     // connection result 0
        else if (WiFi.status() == WL_NO_SSID_AVAIL)   DBG_PRINTLN("Wifi Status: WL_NO_SSID_AVAIL");   // connection result 1
        else if (WiFi.status() == WL_SCAN_COMPLETED)  DBG_PRINTLN("Wifi Status: WL_SCAN_COMPLETED");  // connection result 2
        else if (WiFi.status() == WL_CONNECTED)       DBG_PRINTLN("Wifi Status: WL_CONNECTED");       // connection result 3
        else if (WiFi.status() == WL_CONNECT_FAILED)  DBG_PRINTLN("Wifi Status: WL_CONNECT_FAILED");  // connection result 4
        else if (WiFi.status() == WL_CONNECTION_LOST) DBG_PRINTLN("Wifi Status: WL_CONNECTION_LOST"); // connection result 5
        else if (WiFi.status() == WL_DISCONNECTED)    DBG_PRINTLN("Wifi Status: WL_DISCONNECTED");    // connection result 6
      */

      if (WiFi.status() != WL_CONNECTED) {        // no WLAN settings (AP mode) or no config (STA mode)
        dispAPMode();
        wifiManager.autoConnect(mqtt_clientid);
      }
      //if (WiFi.status() != WL_DISCONNECTED) DBG_PRINTLN("WiFi status disconnected");  // ToDo: Check status disconnected!!!
      if (WiFi.status() != WL_CONNECTION_LOST) cbpiEventSystem(1);
      if (WiFi.status() != WL_CONNECT_FAILED) cbpiEventSystem(1);
      break;
    case EM_OTA: // check OTA (21)
      ArduinoOTA.handle();
      break;
    case EM_MQTT: // check MQTT (22)
      if ((numberOfActors + numberOfSensors) || inductionCooker.isEnabled) // anything to subscribe?
      {
        if (!client.connected()) {
          mqttreconnect();
        }
        client.loop();
      }
      break;
    case EM_WEB:  // Webserver (23)
      server.handleClient();
      break;
    case EM_MDNS: // check MDSN (24)
      MDNS.update();
      break;
    case 25: // SPIFFS mount error
      DBG_PRINT("SPIFFS Mount failed");
      showDispSet("Error: SPIFFS mount");
      break;
    case 26: // MDNS failed
      DBG_PRINT("MDNS failed");
      showDispSet("Error: MDNS failed");
      break;

#ifdef DISPLAY
    case 30:
      oledDisplay.digClock();
      if (lastToggledSys - millis() > SYS_UPDATE)
      {
        oledDisplay.dispUpdate();
      }
      break;
    case 31:
      if (oledDisplay.dispEnabled) {
        display.ssd1306_command(SSD1306_DISPLAYOFF);
        oledDisplay.dispEnabled = 0;
      }
      else {
        display.ssd1306_command(SSD1306_DISPLAYON);
        oledDisplay.dispEnabled = 1;
      }
      break;
      case 32:
      if (WiFi.status() != WL_CONNECTED) {        // no WLAN settings (AP mode) or no config (STA mode)
        dispAPMode();
      }
      break;
      case 33:
      if (WiFi.status() == 6 && oledDisplay.address == 0) {  // no WLAN connected but no config (STA mode)
          dispSTAMode();
      }
      break;
#endif
    default:
      break;
  }
}

void listenerSensors( int event, int parm )                           // Sensor event listener
{
  // 1:= Sensor on Err
  switch (parm) {
    case 1:
#ifdef StopActorsOnSensorError
      // Stop actors
      for (int i = 0; i < numberOfActors; i++) {
        if (actors[i].isOn) {
          Serial.printf("Set actor %i off due to Sensor error%\r\n", i);
          actors[i].isOn = false;
          actors[i].Update();
          actors[i].publishmqtt();
        }
      }
#endif
#ifdef StopInductionOnSensorError
      // Stop Induction
      if (inductionCooker.isInduon) {
        DBG_PRINTLN("Set induction off due to sensor error");
        inductionCooker.isInduon = false;
        inductionCooker.Update();
        inductionCooker.publishmqtt();
      }
#endif
      break;
    default:
      break;
  }
  handleSensors();
}
void listenerActors( int event, int parm )                           // Actor event listener
{
  switch (parm) {
    case 1:
      for (int i = 0; i < numberOfActors; i++) {
#ifdef DEBUG
        Serial.printf("Set actor %i off due to actor error%\r\n", i);
#endif
        actors[i].isOn = false;
        actors[i].Update();
        actors[i].publishmqtt();
      }
      break;
    default:
      break;
  }
  handleActors();
}
void listenerInduction( int event, int parm )                           // Induction event listener
{
  switch (parm) {
    case 1:
      if (inductionCooker.isInduon) {
        DBG_PRINTLN("Set induction off due to induction error");
        inductionCooker.isInduon = false;
        inductionCooker.Update();
        inductionCooker.publishmqtt();
      }
      break;
    default:
      break;
  }
  handleInduction();
}

void rebootDevice() {
  cbpiEventSystem(11);
}

// TODO: Implement
void turnMqttOff() {
  cbpiEventSystem(10);
}

void cbpiEventSystem(int parm)                                   // System events
{
  gEM.queueEvent( EventManager::cbpiEventSystem, parm );
}

void cbpiEventSensors(int parm)                                   // Sensor events
{
  gEM.queueEvent( EventManager::cbpiEventSensors, parm );
}
void cbpiEventActors(int parm)                                   // Actor events
{
  gEM.queueEvent( EventManager::cbpiEventActors, parm );
}
void cbpiEventInduction(int parm)                                // Induction events
{
  gEM.queueEvent( EventManager::cbpiEventInduction, parm );
}
