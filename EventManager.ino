void listenerSystem( int event, int parm )                           // System event listener
{
  switch (parm)
  {
    case 1:       // WLAN error
      // Stop actors
      #ifdef StopActorsOnSensorError
      if (millis() > lastAct + WAIT_ON_ERROR)      // Wait for approx WAIT_ON_ERROR/1000 seconds before switch off all actors
      {
        
        for (int i = 0; i < numberOfActors; i++) {
          if (actors[i].isOn) {
            DBG_PRINT("Set actor ");
            DBG_PRINT(i);
            DBG_PRINTLN(" to off due to WLAN error");
            actors[i].isOn = false;
            actors[i].Update();
            //actors[i].publishmqtt(); // Not yet ready
          }
        }
        lastAct = millis();
      }
      #endif
      // Stop induction
      #ifdef StopInductionOnSensorError
      if (millis() > lastInd + WAIT_ON_ERROR)      // Wait for approx WAIT_ON_ERROR/1000 seconds before switch off all actors
      {
        if (inductionCooker.isInduon) {
          DBG_PRINTLN("Set induction off due to WLAN error");
          inductionCooker.isInduon = false;
          inductionCooker.Update();
          //inductionCooker.publishmqtt(); // Not yet ready
        }
        lastInd = millis();
      }
      #endif
      break;
    case 2:       // MQTT Error
      // Stop actors
      #ifdef StopActorsOnSensorError
      if (millis() > lastAct + WAIT_ON_ERROR)      // Wait for approx WAIT_ON_ERROR/1000 seconds before switch off all actors
      {
        for (int i = 0; i < numberOfActors; i++) {
          if (actors[i].isOn) {
            DBG_PRINT("Set actor ");
            DBG_PRINT(i);
            DBG_PRINTLN(" to off due to WLAN error");
            actors[i].isOn = false;
            actors[i].Update();
            //actors[i].publishmqtt(); // Not yet ready
          }
        }
        lastAct = millis();
      }
      #endif

      // Stop induction
      #ifdef StopInductionOnSensorError
      if (millis() > lastInd + WAIT_ON_ERROR)      // Wait for approx WAIT_ON_ERROR/1000 seconds before switch off all actors
      {
        if (inductionCooker.isInduon) {
          DBG_PRINTLN("Set induction off due to MQTT error");
          inductionCooker.isInduon = false;
          inductionCooker.Update();
          //inductionCooker.publishmqtt(); // Not yet ready
        }
        lastInd = millis();
      }
      #endif
      break;
      
    case 3:       // Sensor error

        // Up now there would be only duplicate code -> see event parm 1 and 2
        // Use event parm 1 until
         
      break;

    case 4:       // Actor error
      break;

    case 5:       // Induction error
      break;

    // case 5 - 9 should be used for error handling

    case 10:      // Disable MQTT - this event is called manuell. Do not use WAIT_ON_ERROR before switch off
      // Stop actors
      showDispSet("MQTT error");
      #ifdef StopActorsOnSensorError
      for (int i = 0; i < numberOfActors; i++) {
        if (actors[i].isOn) {
          DBG_PRINT("Set actor ");
          DBG_PRINT(i);
          DBG_PRINTLN(" to off due to WLAN error");
          actors[i].isOn = false;
          actors[i].Update();
          //actors[i].publishmqtt();
        }
      }
      #endif
      // Stop induction
      #ifdef StopInductionOnSensorError
      if (inductionCooker.isInduon) {
        DBG_PRINTLN("Set induction off due to MQTT disabled");
        inductionCooker.isInduon = false;
        inductionCooker.Update();
        //inductionCooker.publishmqtt();
      }
      #endif
      //mqttCommunication = false;
      server.send(200, "text/plain", "CAUTION! I don't work yet: turned off, please reboot to turn on again...");
      break;
    case 11:  // Reboot ESP - this event is called manuell. Do not use WAIT_ON_ERROR before switch off
      
      showDispSet("Reboot device");
      // Stop actors
      #ifdef StopActorsOnSensorError
      for (int i = 0; i < numberOfActors; i++) {
        if (actors[i].isOn) {
          DBG_PRINT("Set actor ");
          DBG_PRINT(i);
          DBG_PRINTLN(" to off due to WLAN error");
          actors[i].isOn = false;
          actors[i].Update();
          //actors[i].publishmqtt();
        }
      }
      #endif
      // Stop induction
      #ifdef StopInductionOnSensorError
      if (inductionCooker.isInduon) {
        DBG_PRINTLN("Set induction off due to reboot");
        inductionCooker.isInduon = false;
        inductionCooker.Update();
        //inductionCooker.publishmqtt();
      }
      #endif
      server.send(200, "text/plain", "rebooting...");
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
      if (WiFi.status() == WL_DISCONNECTED) cbpiEventSystem(1);  // ToDo: Check status disconnected!!!
      if (WiFi.status() == WL_CONNECTION_LOST) cbpiEventSystem(1);
      if (WiFi.status() == WL_CONNECT_FAILED) cbpiEventSystem(1);
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
    case 30:      // Display screen output update
      oledDisplay.digClock();
      if (lastToggledSys - millis() > SYS_UPDATE)
      {
        oledDisplay.dispUpdate();
      }
      break;
    case 31:      // Display on/off
      if (oledDisplay.dispEnabled) {
        display.ssd1306_command(SSD1306_DISPLAYOFF);
        oledDisplay.dispEnabled = 0;
      }
      else {
        display.ssd1306_command(SSD1306_DISPLAYON);
        oledDisplay.dispEnabled = 1;
      }
      break;
    case 32:      // While starting ESP device used in setup.ino
      if (WiFi.status() != WL_CONNECTED) {        // no WLAN settings (AP mode) or no config (STA mode)
        dispAPMode();
      }
      break;
    case 33:      // While starting ESP device used in setup.ino
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
      if (millis() > lastAct + WAIT_ON_ERROR)      // Wait for approx WAIT_ON_ERROR/1000 seconds before switch off all actors
      {
        for (int i = 0; i < numberOfActors; i++) {
          if (actors[i].isOn) {
            DBG_PRINT("Set actor ");
            DBG_PRINT(i);
            DBG_PRINTLN(" to off due to Sensor error");
            actors[i].isOn = false;
            actors[i].Update();
            //actors[i].publishmqtt();
          }
        }
        lastAct = millis();
      }
      #endif
      #ifdef StopInductionOnSensorError
      // Stop Induction
      if (millis() > lastInd + WAIT_ON_ERROR)      // Wait for approx WAIT_ON_ERROR/1000 seconds before switch off all actors
      {
        if (inductionCooker.isInduon) {
          DBG_PRINTLN("Set induction off due to sensor error");
          inductionCooker.isInduon = false;
          inductionCooker.Update();
          //inductionCooker.publishmqtt();
        }
        lastInd = millis();
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
      #ifdef StopActorsOnSensorError
      for (int i = 0; i < numberOfActors; i++) {
        DBG_PRINT("Set actor ");
        DBG_PRINT(i);
        DBG_PRINTLN(" to off due to Actor error");
        actors[i].isOn = false;
        actors[i].Update();
        //actors[i].publishmqtt();
      }
      #endif
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
      #ifdef StopInductionOnSensorError
      if (inductionCooker.isInduon) {
        DBG_PRINTLN("Set induction off due to induction error");
        inductionCooker.isInduon = false;
        inductionCooker.Update();
        //inductionCooker.publishmqtt();
      }
      #endif
      break;
    default:
      break;
  }
  handleInduction();
}

// Some helper functions WebIf
void rebootDevice() {
  cbpiEventSystem(11);
}

// TODO: Implement
void turnMqttOff() {
  cbpiEventSystem(10);
}

// EventManer Queues
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
