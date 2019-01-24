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
      delay(1000);
      showDispClear();
      ESP.restart();
      break;

    // Loop events comes here
    case 20: // check WLAN
      if (WiFi.status() != WL_CONNECTED) {
        cbpiEventSystem(1);
        wifiManager.autoConnect("MQTTDevice");
      }
      break;
    case 21: // check OTA
      ArduinoOTA.handle();
      break;
    case 22: // check MQTT
      if ((numberOfActors + numberOfSensors) || inductionCooker.isEnabled) // subs available?
      {
        if (!client.connected()) {
          mqttreconnect();
        }
      }
      break;
    case 23:  // Webserver
      server.handleClient();
      break;
    case 24: // check MDSN
      MDNS.update();
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
