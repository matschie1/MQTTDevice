void listenerSystem( int event, int parm )                           // System event listener
{
  switch (parm)
  {
    case 0:       //
      break;
    case 1:       // WLAN error
      oledDisplay.wlanOK = false;
      // Stop actors
      if (StopActorsOnError && lastSysAct == 0) {
        lastSysAct = millis();       // Timestamp on error
        DBG_PRINT("Create Timestamp on WLAN error: ");
        DBG_PRINT((lastSysAct / 3600) % 24);
        DBG_PRINT(":");
        DBG_PRINT((lastSysAct / 3600) % 24);
        DBG_PRINT(":");
        DBG_PRINT((lastSysAct / 60) % 60);
        DBG_PRINT(":");
        DBG_PRINTLN(lastSysAct % 60);
        DBG_PRINT("Wait on error actors: ");
        DBG_PRINTLN(wait_on_error_actors);
      }
      if (StopInductionOnError && lastSysInd == 0) {
        lastSysInd = millis();       // Timestamp on error
        DBG_PRINT("Create Timestamp on WLAN error: ");
        DBG_PRINT((lastSysInd / 3600) % 24);
        DBG_PRINT(":");
        DBG_PRINT((lastSysInd / 3600) % 24);
        DBG_PRINT(":");
        DBG_PRINT((lastSysInd / 60) % 60);
        DBG_PRINT(":");
        DBG_PRINTLN(lastSysInd % 60);
        DBG_PRINT("Wait on error induction: ");
        DBG_PRINTLN(wait_on_error_induction);
      }

      if (millis() > lastSysAct + wait_on_error_actors)      // Wait for approx WAIT_ON_ERROR/1000 seconds before switch off all actors
      {
        if (StopActorsOnError) {
          for (int i = 0; i < numberOfActors; i++)
          {
            if (actors[i].isOn) {
              DBG_PRINT("Set actor ");
              DBG_PRINT(i);
              DBG_PRINTLN(" to off due to WLAN error");
              actors[i].isOn = false;
              actors[i].Update();
              //actors[i].publishmqtt(); // Not yet ready

            }
          }
          lastSysAct = 0;
        }
      }
      if (millis() > lastSysInd + wait_on_error_induction)      // Wait for approx WAIT_ON_ERROR/1000 seconds before switch off all actors
      {
        // Stop induction
        if (inductionCooker.isInduon && StopInductionOnError)
        {
          DBG_PRINTLN("Set induction off due to WLAN error");
          inductionCooker.isInduon = false;
          inductionCooker.Update();
          //inductionCooker.publishmqtt(); // Not yet ready
          lastSysInd = 0;                              // Delete Timestamp after event
        }
      }
      break;

    case 2:       // MQTT Error
      oledDisplay.mqttOK = false;
      // Stop actors
      if (StopActorsOnError && lastSysAct == 0) {
        lastSysAct = millis();       // Timestamp on error
        DBG_PRINT("Create Timestamp on MQTT error: ");
        DBG_PRINT((lastSysAct / 3600) % 24);
        DBG_PRINT(":");
        DBG_PRINT((lastSysAct / 3600) % 24);
        DBG_PRINT(":");
        DBG_PRINT((lastSysAct / 60) % 60);
        DBG_PRINT(":");
        DBG_PRINTLN(lastSysAct % 60);
        DBG_PRINT("Wait on error actors: ");
        DBG_PRINTLN(wait_on_error_actors);
      }
      if (StopInductionOnError && lastSysInd == 0) {
        lastSysInd = millis();       // Timestamp on error
        DBG_PRINT("Create Timestamp on MQTT error: ");
        DBG_PRINT((lastSysInd / 3600) % 24);
        DBG_PRINT(":");
        DBG_PRINT((lastSysInd / 3600) % 24);
        DBG_PRINT(":");
        DBG_PRINT((lastSysInd / 60) % 60);
        DBG_PRINT(":");
        DBG_PRINTLN(lastSysInd % 60);
        DBG_PRINT("Wait on error induction: ");
        DBG_PRINTLN(wait_on_error_induction);
      }
      if (millis() > lastSysAct + wait_on_error_actors)      // Wait for approx WAIT_ON_ERROR/1000 seconds before switch off all actors
      {
        if (StopActorsOnError) {
          for (int i = 0; i < numberOfActors; i++)
          {
            if (actors[i].isOn) {
              DBG_PRINT("Set actor ");
              DBG_PRINT(i);
              DBG_PRINTLN(" to off due to MQTT error");
              actors[i].isOn = false;
              actors[i].Update();
              //actors[i].publishmqtt(); // Not yet ready
            }
          }
          lastSysAct = 0;
        }
      }
      if (millis() > lastSysInd + wait_on_error_induction)     // Wait for approx WAIT_ON_ERROR/1000 seconds before switch off all actors
      {
        // Stop induction
        if (inductionCooker.isInduon && StopInductionOnError)
        {
          DBG_PRINTLN("Set induction off due to MQTT error");
          inductionCooker.isInduon = false;
          inductionCooker.Update();
          //inductionCooker.publishmqtt(); // Not yet ready
          lastSysInd = 0;                              // Delete Timestamp after event
        }
      }
      break;

    case 6:       // SPIFFS error
      DBG_PRINT("SPIFFS Mount failed");
      break;
    case 7:       // MDNS error
      DBG_PRINT("MDNS failed");
      break;
    case 8:       // Webserver error
      DBG_PRINT("Webserver failed");
      break;
    case 10:      // Disable MQTT - this event is called manuell. Do not use WAIT_ON_ERROR before switch off
      // Stop actors
      showDispSet("MQTT error");
      if (StopActorsOnError) {
        for (int i = 0; i < numberOfActors; i++)
        {
          if (actors[i].isOn) {
            DBG_PRINT("Set actor ");
            DBG_PRINT(i);
            DBG_PRINTLN(" to off due to WLAN error");
            actors[i].isOn = false;
            actors[i].Update();
            //actors[i].publishmqtt();
          }
        }
      }
      // Stop induction
      if (inductionCooker.isInduon && StopInductionOnError)
      {
        DBG_PRINTLN("Set induction off due to MQTT disabled");
        inductionCooker.isInduon = false;
        inductionCooker.Update();
        //inductionCooker.publishmqtt();
      }
      //mqttCommunication = false;
      server.send(200, "text/plain", "CAUTION! I don't work yet: turned off, please reboot to turn on again...");
      break;

    case 11:  // Reboot ESP - this event is called manuell. Do not use WAIT_ON_ERROR before switch off
      //showDispSet("Reboot device");
      // Stop actors
      for (int i = 0; i < numberOfActors; i++)
      {
        if (actors[i].isOn) {
          DBG_PRINT("Set actor ");
          DBG_PRINT(i);
          DBG_PRINTLN(" to off due to WLAN error");
          actors[i].isOn = false;
          actors[i].Update();
          //actors[i].publishmqtt();
        }
      }
      // Stop induction
      if (inductionCooker.isInduon) {
        DBG_PRINTLN("Set induction off due to reboot");
        inductionCooker.isInduon = false;
        inductionCooker.Update();
        //inductionCooker.publishmqtt();
      }
      server.send(200, "text/plain", "rebooting...");
      //showDispClear();
      ESP.restart();
      break;

    case EM_WLAN:       // check WLAN (20)
      // ToDo: Check WLAN status
      if (WiFi.status() != WL_CONNECTED) cbpiEventSystem(1);
      else oledDisplay.wlanOK = true;

      //         WiFi.status response code: WL_DISCONNECTED appears not to be precise, notified connection result 6 when connected - no clue about NO_SHIELD
      //        if      (WiFi.status() == WL_NO_SHIELD)       DBG_PRINTLN("Wifi Status: WL_NO_SHIELD");       // connection result 255
      //        else if (WiFi.status() == WL_IDLE_STATUS)     DBG_PRINTLN("Wifi Status: WL_IDLE_STATUS");     // connection result 0
      //        else if (WiFi.status() == WL_NO_SSID_AVAIL)   DBG_PRINTLN("Wifi Status: WL_NO_SSID_AVAIL");   // connection result 1
      //        else if (WiFi.status() == WL_SCAN_COMPLETED)  DBG_PRINTLN("Wifi Status: WL_SCAN_COMPLETED");  // connection result 2
      //        else if (WiFi.status() == WL_CONNECTED)       DBG_PRINTLN("Wifi Status: WL_CONNECTED");       // connection result 3
      //        else if (WiFi.status() == WL_CONNECT_FAILED)  DBG_PRINTLN("Wifi Status: WL_CONNECT_FAILED");  // connection result 4
      //        else if (WiFi.status() == WL_CONNECTION_LOST) DBG_PRINTLN("Wifi Status: WL_CONNECTION_LOST"); // connection result 5
      //        else if (WiFi.status() == WL_DISCONNECTED)    DBG_PRINTLN("Wifi Status: WL_DISCONNECTED");    // connection result 6
      //        if (WiFi.status() == WL_DISCONNECTED) cbpiEventSystem(1);
      //        if (WiFi.status() == WL_CONNECTION_LOST) cbpiEventSystem(1);
      //        if (WiFi.status() == WL_CONNECT_FAILED) cbpiEventSystem(1);

      break;
    case EM_OTA:        // check OTA (21)
      oledDisplay.otaOK = true;
      ArduinoOTA.handle();
      break;
    case EM_MQTT:       // check MQTT (22)
      if ((numberOfActors + numberOfSensors > 0) || inductionCooker.isEnabled) // anything to subscribe?
      {
        yield();
        if (!client.connected())
        {
          oledDisplay.mqttOK = false;
          if (millis() > (mqttconnectlasttry + MQTT_DELAY)) {
            DBG_PRINT("MQTT Trying to connect. Device name: ");
            DBG_PRINTLN(mqtt_clientid);
            oledDisplay.mqttOK = false;
            if (client.connect(mqtt_clientid)) {
              DBG_PRINTLN("MQTT connect successful. Subscribing.");
              for (int i = 0; i < numberOfActors; i++) {
                actors[i].mqtt_subscribe();
              }
              inductionCooker.mqtt_subscribe();
            }
            mqttconnectlasttry = millis();
          }
        }
        else {
          oledDisplay.mqttOK = true;
          client.loop();
        }
      }
      break;
    case EM_WEB:  // Webserver (23)
      server.handleClient();
      break;
    case EM_MDNS: // check MDSN (24)
      MDNS.update();
      break;
    case EM_DISPUP:      // Display screen output update (30)
      if (oledDisplay.dispEnabled) {
        oledDisplay.digClock();
        oledDisplay.dispUpdate();
      }
      break;
    default:
      break;
  }
}

void listenerSensors( int event, int parm )                           // Sensor event listener
{
  // 1:= Sensor on Err
  switch (parm) {
    case 0:
      //DBG_PRINTLN("EM: sensors event ok");
      // all sensors ok
      break;
    case 1:
      // Sensor CRC ceck failed
      DBG_PRINTLN("EM: received event sensor crc check failed");
    case 2:
      // -127°C device error
      DBG_PRINTLN("EM: received event sensor data error (-127°C)");
    case 3:
      // sensor unpluged
      DBG_PRINTLN("EM: received event sensor not connected");
    //break;  // uncomment this line, if you don't want to stop actors when sensor is unpluged (share device)
    case 4:
      // all other errors
      // Stop actors
      if (StopActorsOnError && lastSenAct == 0) {
        lastSenAct = millis();       // Timestamp on error
        DBG_PRINT("Create Timestamp on sensor error: ");
        DBG_PRINT((lastSenAct / 3600) % 24);
        DBG_PRINT(":");
        DBG_PRINT((lastSenAct / 3600) % 24);
        DBG_PRINT(":");
        DBG_PRINT((lastSenAct / 60) % 60);
        DBG_PRINT(":");
        DBG_PRINTLN(lastSenAct % 60);
        DBG_PRINT("Wait on error actors: ");
        DBG_PRINTLN(wait_on_error_actors);
      }
      if (StopInductionOnError && lastSenInd == 0) {
        lastSenInd = millis();       // Timestamp on error
        DBG_PRINT("Create Timestamp on sensor error: ");
        DBG_PRINT((lastSenInd / 3600) % 24);
        DBG_PRINT(":");
        DBG_PRINT((lastSenInd / 3600) % 24);
        DBG_PRINT(":");
        DBG_PRINT((lastSenInd / 60) % 60);
        DBG_PRINT(":");
        DBG_PRINTLN(lastSenInd % 60);
        DBG_PRINT("Wait on error induction: ");
        DBG_PRINTLN(wait_on_error_induction);
      }
      if (millis() > lastSenAct + wait_on_error_actors)      // Wait for approx WAIT_ON_ERROR/1000 seconds before switch off all actors
      {
        if (StopActorsOnError) {
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
          lastSenAct = 0;                              // Delete Timestamp after event
        }
      }
      if (millis() > lastSenInd + wait_on_error_induction)     // Wait for approx WAIT_ON_ERROR/1000 seconds before switch off all actors
      {
        // Stop Induction
        if (inductionCooker.isInduon && StopInductionOnError) {
          DBG_PRINTLN("Set induction off due to sensor error");
          inductionCooker.isInduon = false;
          inductionCooker.Update();
          //inductionCooker.publishmqtt();
          lastSenInd = 0;                              // Delete Timestamp after event
        }
      }
      break;
    default:
      break;
  }
  handleSensors();
}

void listenerActors( int event, int parm )                           // Actor event listener
{
  switch (parm) {
    case 0:
      //DBG_PRINTLN("EM: actors event ok");
      break;
    case 1:
      if (StopActorsOnError) {
        for (int i = 0; i < numberOfActors; i++) {
          DBG_PRINT("Set actor ");
          DBG_PRINT(i);
          DBG_PRINTLN(" to off due to Actor error");
          actors[i].isOn = false;
          actors[i].Update();
          //actors[i].publishmqtt();
        }
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
    case 0:
      //DBG_PRINTLN("EM: induction event ok");
      break;
    case 1:
      if (inductionCooker.isInduon && StopInductionOnError) {
        DBG_PRINTLN("Set induction off due to induction error");
        inductionCooker.isInduon = false;
        inductionCooker.Update();
        //inductionCooker.publishmqtt();
      }
      break;
    default:
      break;
  }
  handleInduction();
}

// Some helper functions WebIf
void rebootDevice()
{
  cbpiEventSystem(11);
}

// TODO: Implement
void turnMqttOff()
{
  cbpiEventSystem(10);
}

void DBG_PRINT(String value)
{
  if (setDEBUG) Serial.print(value);
}
void DBG_PRINT(int value)
{
  if (setDEBUG) Serial.print(value);
}
void DBG_PRINTHEX(int value)
{
  if (setDEBUG) Serial.print(value, HEX);
}
void DBG_PRINTLN(String value)
{
  if (setDEBUG) Serial.println(value);
}
void DBG_PRINTLN(int value)
{
  if (setDEBUG) Serial.println(value);
}
void DBG_PRINTLNHEX(int value)
{
  if (setDEBUG) Serial.println(value, HEX);
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
