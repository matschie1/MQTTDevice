void listenerSystem(int event, int parm) // System event listener
{
  switch (parm)
  {
    case EM_OK: // Normal mode
      break;
    // 1 - 9 Error events
    case EM_WLANER: // WLAN error -> handling
      /*
         Error Reihenfolge
         1. WLAN connected?
         2. MQTT connected
         Wenn WiFi.status() != WL_CONNECTED (wlan_state false nach maxRetries und Delay) ist, ist ein check mqtt überflüssig
      */

      oledDisplay.wlanOK = false;
      // try to reconnect
      if ((millis() > (wlanconnectlasttry + wait_on_error_wlan)) && ( retriesWLAN <= maxRetriesWLAN))
      {
        WiFi.mode(WIFI_STA);
        WiFi.begin();
        //wifiManager.setSaveConfigCallback(saveConfigCallback);
        //wifiManager.autoConnect(mqtt_clientid);
        if (WiFi.status() == WL_CONNECTED)
        {
          DBG_PRINTLN("EM WLAN: WLAN reconnect successful");
          retriesWLAN = 1;
          wlan_state = true;
          oledDisplay.wlanOK = true;
          break;
        }
        DBG_PRINT("EM WLAN: WLAN error. ");
        DBG_PRINT(retriesWLAN);
        DBG_PRINTLN(". try to reconnect");

        wlanconnectlasttry = millis();
        if ( retriesWLAN == maxRetriesWLAN )
        {
          // reconnect fails after maxtries and delay
          DBG_PRINT("EM WLAN: WLAN connection lost. Max retries ");
          DBG_PRINT(retriesWLAN);
          DBG_PRINTLN(" reached.");
          DBG_PRINT("EM WLANER: StopOnWLANError: ");
          DBG_PRINTLN(StopOnWLANError);
          if (StopOnWLANError && wlan_state) {
            cbpiEventActors(EM_ACTER);
            cbpiEventInduction(EM_INDER);
            wlan_state = false;
            mqtt_state = false;   // MQTT in error state - required to restore values
          }
        }
        retriesWLAN++;
      }
      break;
    case EM_MQTTER: // MQTT Error -> handling

      /*
         Error Reihenfolge
         1. WLAN connected?
         2. MQTT connected
         Nur wenn WiFi.status() == WL_CONNECTED ist, kann heck mqtt durchgeführt werden
      */

      oledDisplay.mqttOK = false;
      //if ( (millis() > (mqttconnectlasttry + wait_on_error_mqtt) ) && (retriesMQTT <= maxRetriesMQTT) && WiFi.status() == WL_CONNECTED )
      if ( (millis() > (mqttconnectlasttry + wait_on_error_mqtt) ) && WiFi.status() == WL_CONNECTED )
      {
        if (client.connect(mqtt_clientid))
        {
          DBG_PRINTLN("MQTT auto reconnect successful. Subscribing.");
          cbpiEventSystem(EM_MQTTSUB); // MQTT subscribe
          cbpiEventSystem(EM_MQTTRES);
          break;
        }
        if ( retriesMQTT <= maxRetriesMQTT )
        {
          DBG_PRINT("EM MQTT: MQTT error. ");
          DBG_PRINT(retriesMQTT);
          DBG_PRINTLN(". try to reconnect");
        }
        mqttconnectlasttry = millis();
        if ( retriesMQTT == maxRetriesMQTT )
        {
          DBG_PRINT("EM MQTTER: MQTT server ");
          DBG_PRINT(mqtthost);
          DBG_PRINT(" unavailable. Max retries ");
          DBG_PRINT(maxRetriesMQTT);
          DBG_PRINTLN(" reached.");
          DBG_PRINT("EM MQTTER: StopOnMQTTError: ");
          DBG_PRINTLN(StopOnMQTTError);
          DBG_PRINT("EM MQTTER: mqtt_state: ");
          DBG_PRINTLN(mqtt_state);
          if (StopOnMQTTError && mqtt_state)
          {
            cbpiEventActors(EM_ACTER);
            cbpiEventInduction(EM_INDER);
            mqtt_state = false;   // MQTT in error state
          }
        }
        retriesMQTT++;
      }
      break;
    case EM_SPIFFS: // SPIFFS error (6)
      DBG_PRINTLN("EM SPIFFS: SPIFFS Mount failed");
      break;
    case EM_WEBER: // Webserver error (7)
      DBG_PRINTLN("EM WEBER: Webserver failed");
      break;

    // 10-19 System triggered events
    case EM_MQTTRES: // restore saved values after reconnect MQTT (10)
      if (client.connected())
      {
        wlan_state = true;
        mqtt_state = true;
        retriesWLAN = 1;
        retriesMQTT = 1;

        for (int i = 0; i < numberOfActors; i++)
        {
          if (actors[i].switchable && !actors[i].actor_state)
          {
            DBG_PRINT("EM MQTTRES: ");
            DBG_PRINT(actors[i].name_actor);
            DBG_PRINT("EM MQTTRES: isOnBeforeError: ");
            DBG_PRINTLN(actors[i].isOnBeforeError);
            DBG_PRINT("EM MQTTRES: ");
            DBG_PRINT(actors[i].name_actor);
            DBG_PRINT(" power level: ");
            DBG_PRINTLN(actors[i].power_actor);
            actors[i].isOn = actors[i].isOnBeforeError;
            actors[i].actor_state = true;     // Sensor ok
            actors[i].Update();
          }
        }
        if (!inductionCooker.induction_state) {
          DBG_PRINT("EM MQTTRES: Induction power: ");
          DBG_PRINTLN(inductionCooker.power);
          DBG_PRINT("EM MQTTRES: Induction powerLevelOnError: ");
          DBG_PRINTLN(inductionCooker.powerLevelOnError);
          DBG_PRINT("EM MQTTRES: Induction powerLevelBeforeError: ");
          DBG_PRINTLN(inductionCooker.powerLevelBeforeError);
          inductionCooker.newPower = inductionCooker.powerLevelBeforeError;
          inductionCooker.isInduon = true;
          inductionCooker.induction_state = true;     // Induction ok
          inductionCooker.Update();
          DBG_PRINT("EM MQTTRES: Induction restore old value: ");
          DBG_PRINTLN(inductionCooker.newPower);
        }
      }
      break;
    case EM_REBOOT: // Reboot ESP (11) - manual task
      // Stop actors
      DBG_PRINTLN("EM reboot: Switch actors temp to off due to reboot - MQTT payload CBPi will restore old state");
      cbpiEventActors(EM_ACTOFF);
      // Stop induction
      if (inductionCooker.isInduon)
      {
        DBG_PRINTLN("EM reboot: Set induction temp to off due to reboot - MQTT payload CBPi will restore old state");
        cbpiEventInduction(EM_INDOFF);
      }
      server.send(200, "text/plain", "rebooting...");
      SPIFFS.end(); // unmount SPIFFS
      ESP.restart();
      break;
    case EM_OTASET: // Setup OTA Service (12)
      DBG_PRINTLN("EM OTASET: Enable Over The Air Updates - Start OTA handle");
      startOTA = true;
      setupOTA();
      break;

    // System run & set events
    case EM_WLAN: // check WLAN (20) and reconnect on error
      if (WiFi.status() == WL_CONNECTED)
      {
        oledDisplay.wlanOK = true;
        retriesWLAN = 1;
      }
      else
      {
        //if (testing > 0 && testing < 3) // Test event - ignore! sensor tests only
        //  break;
        cbpiEventSystem(EM_WLANER);
      }
      break;
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

    case EM_OTA: // check OTA (21)
      if (startOTA)
      {
        ArduinoOTA.handle();
      }
      break;
    case EM_MQTT: // check MQTT (22)
      if (client.connected()) {
        oledDisplay.mqttOK = true;
        retriesMQTT = 1;
        client.loop();
      }
      else
      {
        //if (testing > 0 && testing < 3) // Test event - ignore! sensor tests only
        //  break;
        cbpiEventSystem(EM_MQTTER);
      }
      break;
    case EM_MQTTCON: // MQTT connect (27)
      if (WiFi.status() == WL_CONNECTED) // kein wlan = kein mqtt
      {
        client.setServer(mqtthost, 1883);
        client.setCallback(mqttcallback);
        client.connect(mqtt_clientid);
      }
      break;
    case EM_MQTTSUB: // MQTT subscribe (28)
      if (client.connected())
      {
        DBG_PRINTLN("MQTT connect successful. Subscribing...");
        for (int i = 0; i < numberOfActors; i++)
        {
          actors[i].mqtt_subscribe();
        }
        inductionCooker.mqtt_subscribe();
        oledDisplay.mqttOK = true;                // Display MQTT
        mqtt_state = true;                        // MQTT state ok
        retriesMQTT = 1;                          // Reset retries
      }
      //      else
      //      {
      //        DBG_PRINTLN("MQTT error: connect not successful!");
      //        oledDisplay.mqttOK = false;                // Display MQTT
      //        mqtt_state = false;                        // MQTT state ok
      //      }
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
      break;
    case EM_MDNSET: // MDNS setup (26)
      //if (startMDNS && nameMDNS[0] != '\0' && WiFi.status() != WL_CONNECTED)
      if (startMDNS && nameMDNS[0] != '\0' && WiFi.status() == WL_CONNECTED)
      {
        if (!mdns.begin(nameMDNS, WiFi.localIP()))
        {
          DBG_PRINT("EM MDNSSET: MDNS failed");
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
    case EM_TELSET: // Telnet setup
      TelnetServer.begin();
      TelnetServer.setNoDelay(true);
      DBG_PRINTLN("Please connect Telnet Client, exit with ^] and 'quit'");
      break;
    case EM_TELNET: // Telnet
      if (TelnetServer.hasClient())
      {
        if (!Telnet || !Telnet.connected())
        {
          if (Telnet) {
            Telnet.stop();
          }
          Telnet = TelnetServer.available();
        }
        else
        {
          TelnetServer.available().stop();
        }
      }
      /* 
       *  von telnet lesen
       *  
        if (Telnet && Telnet.connected() && Telnet.available()){
        while(Telnet.available())
          Serial.write(Telnet.read());
        }*/
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

      for (int i = 0; i < numberOfSensors; i++)
      {
        if (sensors[i].sens_sw && !sensors[i].sens_state)
        {
          DBG_PRINTLN("EM OK sensor status false");
          break;
        }
      }
      
      lastSenInd = 0; // Delete induction timestamp after event
      lastSenAct = 0; // Delete actor timestamp after event

      //if ( (WiFi.status() == WL_CONNECTED || (testing > 0 && wlan_state)) && (client.connected() || (testing > 0 && mqtt_state) )) // wlan and mqtt connected?
      if (WiFi.status() == WL_CONNECTED && client.connected() && wlan_state && mqtt_state)
      {
        for (int i = 0; i < numberOfActors; i++)
        {
          if (actors[i].switchable && !actors[i].actor_state)   // Sensor in normal mode: check actor in error state
          {
            DBG_PRINT("EM SenOK: ");
            DBG_PRINTLN(actors[i].name_actor);
            DBG_PRINT("EM SenOK: isOnBeforeError: ");
            DBG_PRINTLN(actors[i].isOnBeforeError);
            DBG_PRINT("EM SenOK: ");
            DBG_PRINT(actors[i].name_actor);
            DBG_PRINT(" power level: ");
            DBG_PRINTLN(actors[i].power_actor);
            actors[i].isOn = actors[i].isOnBeforeError;
            actors[i].actor_state = true;
            actors[i].Update();
            lastSenAct = 0; // Delete actor timestamp after event
          }
        }

        if (!inductionCooker.induction_state) {
          DBG_PRINT("EM SenOK: Induction power: ");
          DBG_PRINTLN(inductionCooker.power);
          DBG_PRINT("EM SenOK: Induction powerLevelOnError: ");
          DBG_PRINTLN(inductionCooker.powerLevelOnError);
          DBG_PRINT("EM SenOK: Induction powerLevelBeforeError: ");
          DBG_PRINTLN(inductionCooker.powerLevelBeforeError);
          if (!inductionCooker.induction_state) {
            inductionCooker.newPower = inductionCooker.powerLevelBeforeError;
            inductionCooker.isInduon = true;
            inductionCooker.induction_state = true;
            inductionCooker.Update();
            DBG_PRINT("EM SenOK: Induction restore old value: ");
            DBG_PRINTLN(inductionCooker.newPower);
            lastSenInd = 0; // Delete induction timestamp after event
          }
        }
      }
      break;
    case EM_CRCER:
    // Sensor CRC ceck failed
    //DBG_PRINTLN("EM CRCER: sensor crc check failed");
    //cbpiEventSensors(EM_SENER);
    //break;  // sensor handling in EM_SENER
    case EM_DEVER:
    // -127°C device error
    //DBG_PRINTLN("EM DEVER: sensor data error (-127°C)");
    //cbpiEventSensors(EM_SENER);
    //break;  // sensor handling in EM_SENER
    case EM_UNPL:
    // sensor unpluged
    // DBG_PRINTLN("EM UNPL: sensor not connected");
    //cbpiEventSensors(EM_SENER);
    //break;  // sensor handling in EM_SENER
    case EM_SENER:
      // all other errors
      /*
         Error Reihenfolge
         1. WLAN connected?
         2. MQTT connected
         Wenn 1 oder 2 false ist, kann kein sensor publishmqtt
      */

      //if ( (WiFi.status() == WL_CONNECTED || (testing > 0 && wlan_state)) && (client.connected() || (testing > 0 && mqtt_state) )) // wlan and mqtt connected?
      if (WiFi.status() == WL_CONNECTED && client.connected() && wlan_state && mqtt_state)
      {
        for (int i = 0; i < numberOfSensors; i++)
        {
          if (sensors[i].sens_sw && !sensors[i].sens_state)
          {
            if (lastSenAct == 0)
            {
              lastSenAct = millis(); // Timestamp on error
              DBG_PRINT("EM SENER: Create actors timestamp on sensor error: ");
              DBG_PRINTLNTS(lastSenAct);
              DBG_PRINT("Wait on error actors: ");
              DBG_PRINTLN(wait_on_Sensor_error_actor / 1000);
            }
            if (lastSenInd == 0)
            {
              lastSenInd = millis(); // Timestamp on error
              DBG_PRINT("EM SENER: Create induction timestamp on sensor error: ");
              DBG_PRINTLNTS(lastSenInd);
              DBG_PRINT("Wait on error induction: ");
              DBG_PRINTLN(wait_on_Sensor_error_induction / 1000);
            }
            if (millis() > lastSenAct + wait_on_Sensor_error_actor) // Wait for approx WAIT_ON_ERROR/1000 seconds
            {
              //DBG_PRINTLN("EM SENER: error event actors off");
              cbpiEventActors(EM_ACTER);
            }
            if (millis() > lastSenInd + wait_on_Sensor_error_induction) // Wait for approx WAIT_ON_ERROR/1000 seconds
            {
              if (inductionCooker.isInduon && inductionCooker.powerLevelOnError < 100 && inductionCooker.induction_state)
              {
                //DBG_PRINTLN("EM SENER: error event induction off");
                cbpiEventInduction(EM_INDER);
              }
            }
          }   // Switchable
        }     // Iterate sensors
      }     // wlan und mqtt state
      break;
#ifdef TEST
    case EM_SENTEST1:
      sentest1();
      break;
    case EM_SENTEST2:
      // Test event - ignore!
      sentest2();
      break;
#endif
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
      for (int i = 0; i < numberOfActors; i++)
      {
        //if (actors[i].switchable && actors[i].actor_state)    // Check if actor is switchable and in normal mode
        if (actors[i].switchable && actors[i].actor_state && actors[i].isOn)    // Check if actor is switchable, isOn and in normal mode - 20190917: Prüfen!
        {
          actors[i].isOnBeforeError = actors[i].isOn;
          actors[i].isOn = false;
          actors[i].actor_state = false;
          actors[i].Update();
          DBG_PRINT("EM ACTER: Actor: ");
          DBG_PRINTLN(actors[i].name_actor);
          DBG_PRINT("EM ACTER: state: ");
          DBG_PRINTLN(actors[i].actor_state);
          DBG_PRINT("EM ACTER: isOnBeforeError: ");         // Actor on/off before error event
          DBG_PRINTLN(actors[i].isOnBeforeError);
        }
      }
      break;
    case EM_ACTOFF:
      for (int i = 0; i < numberOfActors; i++)
      {
        if (actors[i].isOn) {
          actors[i].isOn = false;
          actors[i].Update();
          DBG_PRINT("EM ACTER: Actor: ");
          DBG_PRINT(actors[i].name_actor);
          DBG_PRINTLN(" switched off");
        }
      }
      break;
    case EM_ACTTEST:        // Test event - ignore!
      DBG_PRINTLN("*** CAUTION *** EM ACTTEST: actors test event: switch on ALL actors");
      for (int i = 0; i < numberOfActors; i++)
      {
        actors[i].isOn = true;
        actors[i].power_actor = 100;
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
      //DBG_PRINTLN("EM IND1: received induction event - relay on"); // bislang keine Verwendung
      break;
    case 2:
      //DBG_PRINTLN("EM IND2: received induction event - error"); // bislang keine Verwendung
      break;
    case EM_INDER:
      if (inductionCooker.isInduon && inductionCooker.powerLevelOnError < 100 && inductionCooker.induction_state) // powerlevelonerror == 100 -> kein event handling
      {
        inductionCooker.powerLevelBeforeError = inductionCooker.power;
        DBG_PRINT("EM INDER: induction power: ");
        DBG_PRINTLN(inductionCooker.power);
        DBG_PRINT("EM INDER: induction newPower: ");
        DBG_PRINTLN(inductionCooker.newPower);

        DBG_PRINT("EM INDER: Set induction power level to ");
        if (inductionCooker.powerLevelOnError == 0) {
          DBG_PRINT(inductionCooker.powerLevelOnError);
          DBG_PRINTLN(" - induction switched off");
          inductionCooker.isInduon = false;
        }
        else {
          DBG_PRINTLN(inductionCooker.powerLevelOnError);
          inductionCooker.newPower = inductionCooker.powerLevelOnError;
        }
        inductionCooker.newPower = inductionCooker.powerLevelOnError;
        inductionCooker.induction_state = false;
        inductionCooker.Update();
      }
      // Sollte überflüssig sein. induction_state änderung beachten
      //      else if (inductionCooker.isInduon && inductionCooker.powerLevelOnError == 100 && !inductionCooker.induction_state)
      //      {
      //        DBG_PRINTLN("EM INDER: Induction power level not changed: 100%");
      //        inductionCooker.induction_state = true;
      //      }
      break;
    case EM_INDOFF:
      if (inductionCooker.isInduon)
      {
        DBG_PRINTLN("EM INDOFF: Induction switched off");
        inductionCooker.newPower = 0;
        inductionCooker.isInduon = false;
        inductionCooker.Update();
      }
      break;
    case EM_INDTEST:        // Test event - ignore!
      DBG_PRINTLN("*** CAUTION *** EM INDTEST: induction test event - switch on induction 100%");
      inductionCooker.newPower = 100;
      inductionCooker.isInduon = true;
      inductionCooker.isRelayon = true;
      inductionCooker.isPower = true;
      inductionCooker.isEnabled = true;
      inductionCooker.induction_state = true;
      inductionCooker.Update();
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
