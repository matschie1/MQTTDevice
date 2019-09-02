void listenerSystem(int event, int parm) // System event listener
{
  switch (parm)
  {
    case EM_OK: // Normal mode
      break;
    // 1 - 9 Error events
    case EM_WLANER: // WLAN error -> handling

      if ( StopWLANOnError)  // WLAN in error state. configured retries and delay unsuccessful
      {
        // Stop actors on WLAN error
        if (StopActorsOnError)
        {
          cbpiEventActors(EM_ACTER);
        }
        // Stop induction on WLAN error
        if (StopInductionOnError)
        {
          cbpiEventInduction(EM_INDER);
        }
        break;
      }


      // Stop actors
      if (StopActorsOnError && lastSysAct == 0)
      {
        lastSysAct = millis(); // Timestamp on error
        DBG_PRINT("Create Timestamp on WLAN error: ");
        DBG_PRINTLNTS(lastSysAct);
        DBG_PRINT("Wait on error actors: ");
        DBG_PRINTLN(wait_on_error_actors);
      }
      if (StopInductionOnError && lastSysInd == 0)
      {
        lastSysInd = millis(); // Timestamp on error
        DBG_PRINT("Create Timestamp on WLAN error: ");
        DBG_PRINTLNTS(lastSysInd);
        DBG_PRINT("Wait on error induction: ");
        DBG_PRINTLN(wait_on_error_induction);
      }

      if (millis() > lastSysAct + wait_on_error_actors) // Wait for approx WAIT_ON_ERROR/1000 seconds before switch off all actors
      {
        if (StopActorsOnError)
        {
          DBG_PRINTLN("Switch actors off due to WLAN error");
          cbpiEventActors(EM_ACTER);
          lastSysAct = 0; // Delete Timestamp after event
        }
      }
      if (millis() > lastSysInd + wait_on_error_induction) // Wait for approx WAIT_ON_ERROR/1000 seconds before switch off all actors
      {
        // Stop induction
        if (inductionCooker.isInduon && StopInductionOnError)
        {
          DBG_PRINTLN("Set induction off due to WLAN error");
          cbpiEventInduction(EM_INDER);
          lastSysInd = 0; // Delete Timestamp after event
        }
      }
      break;
    case EM_MQTTER: // MQTT Error -> handling
      // Stop actors

      if ( (StopMQTTOnError) && (StopWLANOnError) ) // WLAN und MQTT ausgefallen
      {
        if ( retriesWLAN == (maxRetriesWLAN + 1) )
        {
          DBG_PRINTLN("EM: MQTT and WLAN in error state");
        }
      }
      if ( (StopMQTTOnError) && !(StopWLANOnError) ) // MQTT Server nicht erreichbar, WLAN ok
      {
        if ( retriesMQTT == (maxRetriesMQTT + 1) )
        {
          DBG_PRINTLN("EM: MQTT in error state, but WLAN up'n'running");
          retriesMQTT++;
        }
      }
      if ( StopMQTTOnError)  // MQTT in error state. configured retries and delay unsuccessful
      {
        // Stop actors on WLAN error
        if (StopActorsOnError)
        {
          cbpiEventActors(EM_ACTER);
        }
        // Stop induction on WLAN error
        if (StopInductionOnError)
        {
          cbpiEventInduction(EM_INDER);
        }
        break;
      }

      if (StopActorsOnError && lastSysAct == 0)
      {
        lastSysAct = millis(); // Timestamp on error
        DBG_PRINT("Create Timestamp on MQTT error: ");
        DBG_PRINTLNTS(lastSysAct);
        DBG_PRINT("Wait on error actors: ");
        DBG_PRINTLN(wait_on_error_actors);
      }
      if (StopInductionOnError && lastSysInd == 0)
      {
        lastSysInd = millis(); // Timestamp on error
        DBG_PRINT("Create Timestamp on MQTT error: ");
        DBG_PRINTLNTS(lastSysInd);
        DBG_PRINT("Wait on error induction: ");
        DBG_PRINTLN(wait_on_error_induction);
      }
      if (millis() > lastSysAct + wait_on_error_actors) // Wait for approx WAIT_ON_ERROR/1000 seconds before switch off all actors
      {
        if (StopActorsOnError)
        {
          DBG_PRINTLN("Switch actors off due to MQTT error");
          cbpiEventActors(EM_ACTER);
          lastSysAct = 0; // Delete Timestamp after event
        }
      }
      if (millis() > lastSysInd + wait_on_error_induction) // Wait for approx WAIT_ON_ERROR/1000 seconds before switch off all actors
      {
        // Stop induction
        if (inductionCooker.isInduon && StopInductionOnError)
        {
          DBG_PRINTLN("Set induction off due to MQTT error");
          cbpiEventInduction(EM_INDER);
          lastSysInd = 0; // Delete Timestamp after event
        }
      }
      break;
    case EM_SPIFFS: // SPIFFS error (6)
      DBG_PRINTLN("EM: SPIFFS Mount failed");
      cbpiEventActors(EM_ACTER);
      cbpiEventInduction(EM_INDER);
      break;

    case EM_WEBER: // Webserver error (7)
      DBG_PRINTLN("EM: Webserver failed");
      cbpiEventActors(EM_ACTER);
      cbpiEventInduction(EM_INDER);
      break;

    // 10-19 System triggered events
    case EM_MQTTDIS: // Disable MQTT (10) - manual task WebIf
      // Stop actors
      DBG_PRINTLN("Switch actors off due to MQTT disabled");
      cbpiEventActors(EM_ACTER);

      // Stop induction
      if (inductionCooker.isInduon)
      {
        DBG_PRINTLN("Set induction off due to MQTT disabled");
        cbpiEventInduction(EM_INDER);
      }
      server.send(200, "text/plain", "CAUTION! I don't work yet: turned off, please reboot to turn on again...");
      break;
    case EM_REBOOT: // Reboot ESP (11) - manual task
      // Stop actors
      DBG_PRINTLN("Switch actors off due to reboot");
      cbpiEventActors(EM_ACTER);
      // Stop induction
      if (inductionCooker.isInduon)
      {
        DBG_PRINTLN("Set induction off due to reboot");
        cbpiEventInduction(EM_INDER);
      }
      server.send(200, "text/plain", "rebooting...");
      SPIFFS.end(); // unmount SPIFFS
      ESP.restart();
      break;
    case EM_OTASET: // Setup OTA Service (12)
      DBG_PRINTLN("Enable Over The Air Updates - Start OTA handle");
      startOTA = true;
      setupOTA();
      break;

    // System run & set events
    case EM_WLAN: // check WLAN (20) and reconnect on error
      // ToDo: Check WLAN status
      if (WiFi.status() != WL_CONNECTED)
      {
        oledDisplay.wlanOK = false;
        cbpiEventSystem(EM_WLANER);
        if ((millis() > (wlanconnectlasttry + WLAN_DELAY)) && ( retriesWLAN <= maxRetriesWLAN))
        {
          DBG_PRINT("EM: WLAN error. ");
          DBG_PRINT(retriesWLAN);
          DBG_PRINT(". try to reconnect");
          DBG_PRINT(retriesWLAN);
          wifiManager.setSaveConfigCallback(saveConfigCallback);
          wifiManager.autoConnect(mqtt_clientid);

          if (WiFi.status() == WL_CONNECTED)
          {
            DBG_PRINTLN("EM: WLAN reconnect successful");
            retriesWLAN = 1;
            oledDisplay.wlanOK = true;
            StopWLANOnError = false;
          }
          wlanconnectlasttry = millis();
          if ( retriesWLAN == maxRetriesWLAN )
          {
            DBG_PRINT("EM: WLAN connection lost. Max retries ");
            DBG_PRINT(retriesWLAN);
            DBG_PRINTLN(" reached");
            StopWLANOnError = true;
            cbpiEventSystem(EM_WLANER);
          }
          retriesWLAN++;
        }
      }
      else
      {
        oledDisplay.wlanOK = true;
        StopWLANOnError = false;
        retriesWLAN = 1;
      }

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
    case EM_OTA: // check OTA (21)
      if (startOTA)
      {
        ArduinoOTA.handle();
      }
      break;
    case EM_MQTT: // check MQTT (22) and reconnect on error

      if ((numberOfActors + numberOfSensors > 0) || inductionCooker.isEnabled) // anything to subscribe?
      {
        if (!client.connected())
        {
          oledDisplay.mqttOK = false;
          cbpiEventSystem(EM_MQTTER);
          if ( (millis() > (mqttconnectlasttry + MQTT_DELAY) ) && (retriesMQTT <= maxRetriesMQTT) )
          {
            DBG_PRINT("EM: MQTT error. ");
            DBG_PRINT(retriesMQTT);
            DBG_PRINTLN(". try to reconnect");

            if (client.connect(mqtt_clientid))
            {
              DBG_PRINTLN("MQTT connect successful. Subscribing.");
              retriesMQTT = 1;
              oledDisplay.mqttOK = true;
              StopMQTTOnError = false;
              for (int i = 0; i < numberOfActors; i++)
              {
                actors[i].mqtt_subscribe();
              }
              inductionCooker.mqtt_subscribe();
            }
            mqttconnectlasttry = millis();
            if ( retriesMQTT == maxRetriesMQTT )
            {
              DBG_PRINT("EM: MQTT server ");
              DBG_PRINT(mqtthost);
              DBG_PRINT(" unavailable. Max retries ");
              DBG_PRINT(maxRetriesMQTT);
              DBG_PRINTLN(" reached.");
              StopMQTTOnError = true;
              cbpiEventSystem(EM_MQTTER);
            }
            retriesMQTT++;
          }
        }
        else
        {
          oledDisplay.mqttOK = true;
          retriesMQTT = 1;
          StopMQTTOnError = false;
          client.loop();
        }
      }
      else
      {
        oledDisplay.mqttOK = false;  // MQTT not required, no sensors, actors or induction active
        retriesMQTT = 1;
      }
      break;
    case EM_WEB: // Webserver (23)
      server.handleClient();
      break;
    case EM_MDNS: // check MDSN (24)
      if (startMDNS)
        mdns.update();
      break;
    case EM_NTP: // NTP Update (25)
      timeClient.update();
    case EM_MDNSET: // MDNS setup (26)
      if (startMDNS && nameMDNS[0] != '\0' && WiFi.status() != WL_CONNECTED)
      {
        if (!mdns.begin(nameMDNS, WiFi.localIP()))
        {
          DBG_PRINT("EM: MDNS failed");
          startMDNS = false;
        }
        else
        {
          mdns.begin(nameMDNS);
          DBG_PRINT("EM MDNS started: ");
          DBG_PRINT(nameMDNS);
          DBG_PRINT(" to: ");
          IPAddress ip = WiFi.localIP();
          DBG_PRINTLN(ip.toString());
        }
      }
      break;
    case EM_DISPUP: // Display screen output update (30)
      if (oledDisplay.dispEnabled)
      {
        oledDisplay.digClock();
        oledDisplay.dispUpdate();
      }
      break;
    default:
      break;
  }
}

void listenerSensors(int event, int parm) // Sensor event listener
{
  // 1:= Sensor on Err
  switch (parm)
  {
    case EM_OK:
      // all sensors ok
      //DBG_PRINTLN("EM: sensor event - ok ");
      break;
    case EM_CRCER:
      // Sensor CRC ceck failed
      DBG_PRINTLN("EM: received sensor event - crc check failed");
    case EM_DEVER:
      // -127°C device error
      DBG_PRINTLN("EM: received sensor event - data error (-127°C)");
    case EM_UNPL:
      // sensor unpluged
      DBG_PRINTLN("EM: received sensor event - not connected");
    //break;  // uncomment this line, if you don't want to stop actors when sensor is unpluged (shared device)
    case EM_SENER:
      // all other errors
      // Stop actors
      if (StopActorsOnError && lastSenAct == 0)
      {
        lastSenAct = millis(); // Timestamp on error
        DBG_PRINT("Create Timestamp on sensor error: ");
        DBG_PRINTLNTS(lastSenAct);
        DBG_PRINT("Wait on error actors: ");
        DBG_PRINTLN(wait_on_error_actors);
      }
      if (StopInductionOnError && lastSenInd == 0)
      {
        lastSenInd = millis(); // Timestamp on error
        DBG_PRINT("Create Timestamp on sensor error: ");
        DBG_PRINTLNTS(lastSenInd);
        DBG_PRINT("Wait on error induction: ");
        DBG_PRINTLN(wait_on_error_induction);
      }
      if (millis() > lastSenAct + wait_on_error_actors) // Wait for approx WAIT_ON_ERROR/1000 seconds before switch off all actors
      {
        if (StopActorsOnError)
        {
          DBG_PRINTLN("Switch actors off due to Sensor error");
          cbpiEventActors(EM_ACTER);
          lastSenAct = 0; // Delete Timestamp after event
        }
      }
      if (millis() > lastSenInd + wait_on_error_induction) // Wait for approx WAIT_ON_ERROR/1000 seconds before switch off all actors
      {
        // Stop Induction
        if (inductionCooker.isInduon && StopInductionOnError)
        {
          DBG_PRINTLN("Set induction off due to sensor error");
          cbpiEventInduction(EM_INDER);
          lastSenInd = 0; // Delete Timestamp after event
        }
      }
      break;
    default:
      break;
  }
  handleSensors();
}

void listenerActors(int event, int parm) // Actor event listener
{
  switch (parm)
  {
    case EM_OK:
      break;
    case 1:
      break;
    case 2:
      break;
    case EM_ACTER:
      //DBG_PRINTLN("EM: received actor event - actor state not null (ok)");
      for (int i = 0; i < numberOfActors; i++)
      {
        if (actors[i].switchable)
        {
          actors[i].isOn = false;
          actors[i].Update();
          //actors[i].publishmqtt(); // not yet ready
        }
      }
      break;
    case EM_ACTTEST:
      for (int i = 0; i < numberOfActors; i++)
      {
        actors[i].isOn = true;
        actors[i].Update();
      }
      break;
    default:
      break;
  }
  handleActors();
}
void listenerInduction(int event, int parm) // Induction event listener
{
  switch (parm)
  {
    case EM_OK:
      break;
    case 1:
      //DBG_PRINTLN("EM: received induction event - relay on");
      break;
    case 2:
      DBG_PRINTLN("EM: received induction event - error");
      break;
    case EM_INDER:
      //DBG_PRINTLN("EM: received induction event - induction state not null (ok)");
      if (inductionCooker.isInduon)
      {
        inductionCooker.isInduon = false;
        inductionCooker.Update();
        DBG_PRINTLN("EM: received induction event - INDER");
      }
      break;
    case EM_INDTEST:
      // delayAfteroff muss einen Wert haben (120000)
      DBG_PRINTLN("EM: received induction event - INDTEST");
      inductionCooker.newPower = 100;
      inductionCooker.isInduon = true;
      inductionCooker.isRelayon = true;
      inductionCooker.isPower = true;
      inductionCooker.isEnabled = true;
      inductionCooker.Update();
      DBG_PRINTLN("EM: Induction ist switch on due to event INDTEST");
      break;
    default:
      break;
  }
  handleInduction();
}

// EventManer Queues
void cbpiEventSystem(int parm) // System events
{
  gEM.queueEvent(EventManager::cbpiEventSystem, parm);
}

void cbpiEventSensors(int parm) // Sensor events
{
  gEM.queueEvent(EventManager::cbpiEventSensors, parm);
}
void cbpiEventActors(int parm) // Actor events
{
  gEM.queueEvent(EventManager::cbpiEventActors, parm);
}
void cbpiEventInduction(int parm) // Induction events
{
  gEM.queueEvent(EventManager::cbpiEventInduction, parm);
}
