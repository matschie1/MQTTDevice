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
    if (sim_mode == SIM_NONE)
    {
      WiFi.mode(WIFI_STA);
      WiFi.begin();
      if (WiFi.status() == WL_CONNECTED)
      {
        DBG_PRINTLN("EM WLAN: WLAN reconnect successful");
        retriesWLAN = 1;
        wlan_state = true;
        oledDisplay.wlanOK = true;
        break;
      }
    }
    else
    {
      DBG_PRINT("SIM: WLAN loop ");
      DBG_PRINTLN(sim_counter - 20);
      sim_counter++;
    }

    DBG_PRINT("EM WLAN: WLAN error. ");
    DBG_PRINT(retriesWLAN);
    DBG_PRINTLN(". try to reconnect");

    if (retriesWLAN == maxRetriesWLAN)
    {
      // reconnect fails after maxtries and delay
      DBG_PRINT("EM WLAN: WLAN connection lost. Max retries ");
      DBG_PRINT(retriesWLAN);
      DBG_PRINTLN(" reached.");
      DBG_PRINT("EM WLANER: StopOnWLANError: ");
      DBG_PRINTLN(StopOnWLANError);
      if ((StopOnWLANError && wlan_state && sim_mode == SIM_NONE) || (sim_mode == SIM_WLAN && !wlan_state))
      {
        cbpiEventActors(EM_ACTER);
        cbpiEventInduction(EM_INDER);
        wlan_state = false;
        mqtt_state = false; // MQTT in error state - required to restore values
      }
    }
    retriesWLAN++;
    break;
  case EM_MQTTER: // MQTT Error -> handling
    oledDisplay.mqttOK = false;
    if (sim_mode == SIM_NONE)
    {
      DBG_PRINTLN("MQTT try to auto reconnect ..");
      if (client.connect(mqtt_clientid))
      {
        DBG_PRINTLN("MQTT auto reconnect successful. Subscribing..");
        cbpiEventSystem(EM_MQTTSUB); // MQTT subscribe
        cbpiEventSystem(EM_MQTTRES);
        break;
      }
    }
    else
    {
      sim_counter++;
      DBG_PRINT("SIM: MQTT loop ");
      DBG_PRINTLN(sim_counter - 10);
    }

    if (retriesMQTT <= maxRetriesMQTT)
    {
      DBG_PRINT("EM MQTT: MQTT error. ");
      DBG_PRINT(retriesMQTT);
      DBG_PRINTLN(". try to reconnect");
    }
    if (retriesMQTT == maxRetriesMQTT)
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
      if ((StopOnMQTTError && mqtt_state && sim_mode == SIM_NONE) || (sim_mode == SIM_MQTT && !mqtt_state))
      {
        cbpiEventActors(EM_ACTER);
        cbpiEventInduction(EM_INDER);
        mqtt_state = false; // MQTT in error state
      }
    }
    retriesMQTT++;
    break;
  case EM_SPIFFS: // SPIFFS error (6)
    DBG_PRINTLN("EM SPIFFS: SPIFFS Mount failed");
    break;
  case EM_WEBER: // Webserver error (7)
    DBG_PRINTLN("EM WEBER: Webserver failed");
    break;

  // 10-19 System triggered events
  case EM_MQTTRES: // restore saved values after reconnect MQTT (10)
    if (client.connected() || sim_mode != SIM_NONE)
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
          actors[i].actor_state = true; // Sensor ok
          actors[i].Update();
        }
      }
      if (!inductionCooker.induction_state)
      {
        DBG_PRINT("EM MQTTRES: Induction power: ");
        DBG_PRINTLN(inductionCooker.power);
        DBG_PRINT("EM MQTTRES: Induction powerLevelOnError: ");
        DBG_PRINTLN(inductionCooker.powerLevelOnError);
        DBG_PRINT("EM MQTTRES: Induction powerLevelBeforeError: ");
        DBG_PRINTLN(inductionCooker.powerLevelBeforeError);
        inductionCooker.newPower = inductionCooker.powerLevelBeforeError;
        inductionCooker.isInduon = true;
        inductionCooker.induction_state = true; // Induction ok
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
    if ((WiFi.status() == WL_CONNECTED) && sim_mode == SIM_NONE)
    {
      oledDisplay.wlanOK = true;
      retriesWLAN = 1;
    }
    else if (sim_mode == SIM_WLAN)
      simWLAN();
    else if ((!WiFi.status() == WL_CONNECTED) && sim_mode == SIM_NONE)
      if (millis() > (wlanconnectlasttry + wait_on_error_wlan))
      {
        DBG_PRINT("EM: WLAN error rc=");
        DBG_PRINT(WiFi.status());
        switch (WiFi.status())
        {
        case 0: // WL_IDLE_STATUS
          DBG_PRINTLN(" WLAN idle status");
          break;
        case 1: // WL_NO_SSID_AVAIL
          DBG_PRINTLN(" WLAN no SSID availible");
          break;
        case 2: // WL_SCAN_COMPLETED
          DBG_PRINTLN(" WLAN scan completed");
          break;
        case 3: // WL_CONNECTED
          DBG_PRINTLN(" WLAN connected");
          break;
        case 4: // WL_CONNECT_FAILED
          DBG_PRINTLN(" WLAN connect failed");
          cbpiEventSystem(EM_WLANER);
          break;
        case 5: // WL_CONNECTION_LOST
          DBG_PRINTLN(" WLAN connection lost");
          cbpiEventSystem(EM_WLANER);
          break;
        case 6: // WL_DISCONNECTED
          DBG_PRINTLN(" WLAN disconnected");
          cbpiEventSystem(EM_WLANER);
          break;
        case 255: // WL_NO_SHIELD
          DBG_PRINTLN(" WLAN no shield");
          break;
        default:
          break;
        }
        wlanconnectlasttry = millis();
      }
    break;
  case EM_OTA: // check OTA (21)
    if (startOTA)
    {
      ArduinoOTA.handle();
    }
    break;
  case EM_MQTT:                                                   // check MQTT (22)
    if ((!WiFi.status() == WL_CONNECTED) && sim_mode == SIM_NONE) // Kein WLAN und keine Simulation
      break;
    if (client.connected() && sim_mode == SIM_NONE)
    {
      oledDisplay.mqttOK = true;
      retriesMQTT = 1;
      mqttconnectlasttry = 0;
      client.loop();
    }
    else if (!client.connected() && sim_mode == SIM_NONE)
    {
      /*
        MQTT typischer Fehlerverlauf
        RC -3 Connection lost
        mosquitto meldet zeitgleich: Socket error on client ESP8266-xxxxxxx, disconnecting.

        next loop MQTTDevice
        RC -2 Connect Failed

        ESP8266 device IP Address: 192.168.100.203
        ---------------------------------
        EM: Device IP changed to (IP unset)
        EM: MQTT error rc=-3 MQTT_CONNECTION_LOST
        MQTT try to auto reconnect ..
        EM MQTT: MQTT error. 1. try to reconnect
        EM: Device IP changed to (IP unset)
        EM: Device IP changed to (IP unset)
        EM: Device IP changed to 192.168.100.34
        ESP8266 device IP Address: 192.168.100.203
      */

      // Workaround Fritz.box IP Wechsel

      if (aktIP != WiFi.localIP())
      {
        DBG_PRINT("EM: Device IP changed to ");
        DBG_PRINTLN(WiFi.localIP().toString());
        aktIP = WiFi.localIP();
        if (WiFi.localIP().toString() == "(IP unset)")
          break;
        else //(WiFi.localIP().toString() != "(IP unset)")
          cbpiEventSystem(EM_MQTTER);
        break;
      }

      if (millis() > (mqttconnectlasttry + wait_on_error_mqtt))
      {
        /* Ignore
        // Debug output queued MQTT events
        unsigned long allSeconds=millis()/1000;
        int runHours= allSeconds/3600;
        int secsRemaining=allSeconds%3600;
        int runMinutes=secsRemaining/60;
        int runSeconds=secsRemaining%60;
        char buf[21];
        sprintf(buf,"EM_MQTT: MQTT delay %02d:%02d:%02d",runHours,runMinutes,runSeconds);
        DBG_PRINTLN(buf);
        */

        DBG_PRINT("EM: MQTT error rc=");
        DBG_PRINT(client.state());
        switch (client.state())
        {
        case -4: // MQTT_CONNECTION_TIMEOUT - the server didn't respond within the keepalive time
          DBG_PRINTLN(" MQTT_CONNECTION_TIMEOUT");
          cbpiEventSystem(EM_MQTTER);
          break;
        case -3: // MQTT_CONNECTION_LOST - the network connection was broken
          DBG_PRINTLN(" MQTT_CONNECTION_LOST");
          cbpiEventSystem(EM_MQTTER);
          break;
        case -2: // MQTT_CONNECT_FAILED - the network connection failed
          DBG_PRINTLN(" MQTT_CONNECT_FAILED");
          cbpiEventSystem(EM_MQTTER);
          break;
        case -1: // MQTT_DISCONNECTED - the client is disconnected cleanly
          DBG_PRINTLN(" MQTT_DISCONNECTED");
          cbpiEventSystem(EM_MQTTER);
          break;
        case 0: // MQTT_CONNECTED - the client is connected
          // kann hier nicht vorkommen: MQTT connected
          client.loop();
          break;
        case 1: // MQTT_CONNECT_BAD_PROTOCOL - the server doesn't support the requested version of MQTT
          DBG_PRINTLN(" MQTT_CONNECT_BAD_PROTOCOL");
          break;
        case 2: // MQTT_CONNECT_BAD_CLIENT_ID - the server rejected the client identifier
          DBG_PRINTLN(" MQTT_CONNECT_BAD_CLIENT_ID");
          break;
        case 3: // MQTT_CONNECT_UNAVAILABLE - the server was unable to accept the connection
          DBG_PRINTLN(" MQTT_CONNECT_UNAVAILABLE");
          break;
        case 4: // MQTT_CONNECT_BAD_CREDENTIALS - the username/password were rejected
          DBG_PRINTLN(" MQTT_CONNECT_BAD_CREDENTIALS");
          break;
        case 5: // MQTT_CONNECT_UNAUTHORIZED - the client was not authorized to connect
          DBG_PRINTLN(" MQTT_CONNECT_UNAUTHORIZED");
          break;
        default:
          break;
        }
        mqttconnectlasttry = millis();
      }
      else if (sim_mode == SIM_MQTT)
        simMQTT();
    }
    else if (!client.connected() && sim_mode != SIM_MQTT)
    {
      oledDisplay.mqttOK = true;
      retriesMQTT = 1;
      mqtt_state = true;
    }
    break;
  case EM_MQTTCON:                     // MQTT connect (27)
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
      if (inductionCooker.isEnabled)
        inductionCooker.mqtt_subscribe();

      if (startTCP)
      {
        for (int i = 1; i < 10; i++)
        {
          if (tcpServer[i].kettle_id != "0")
          {
            tcpServer[i].mqtt_subscribe();
          }
        }
      }

      oledDisplay.mqttOK = true; // Display MQTT
      mqtt_state = true;         // MQTT state ok
      retriesMQTT = 1;           // Reset retries
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
        if (Telnet)
        {
          Telnet.stop();
        }
        Telnet = TelnetServer.available();
      }
      else
      {
        TelnetServer.available().stop();
      }
    }
    /* ToDo: von telnet lesen
    if (Telnet && Telnet.connected() && Telnet.available())
    {
    while(Telnet.available())
      Serial.write(Telnet.read());
    }
    */
    break;
  case EM_DISPUP: // Display screen output update (30)
    if (oledDisplay.dispEnabled)
    {
      oledDisplay.digClock();
      oledDisplay.dispUpdate();
    }
    break;
  case EM_TCP: // Telnet setup
    publishTCP();
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

    lastSenInd = 0; // Delete induction timestamp after event
    lastSenAct = 0; // Delete actor timestamp after event

    if ((WiFi.status() == WL_CONNECTED && client.connected() && wlan_state && mqtt_state) || (sim_mode != SIM_NONE && wlan_state && mqtt_state)) // (sim_mode == SIM_SEN_ERR || sim_mode == SIM_SEN_FALSE || sim_mode == SIM_SEN_OK )) )
    {
      for (int i = 0; i < numberOfActors; i++)
      {
        if (actors[i].switchable && !actors[i].actor_state) // Sensor in normal mode: check actor in error state
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

      if (!inductionCooker.induction_state)
      {
        DBG_PRINT("EM SenOK: Induction power: ");
        DBG_PRINTLN(inductionCooker.power);
        DBG_PRINT("EM SenOK: Induction powerLevelOnError: ");
        DBG_PRINTLN(inductionCooker.powerLevelOnError);
        DBG_PRINT("EM SenOK: Induction powerLevelBeforeError: ");
        DBG_PRINTLN(inductionCooker.powerLevelBeforeError);
        if (!inductionCooker.induction_state)
        {
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
    if ((WiFi.status() == WL_CONNECTED && client.connected() && wlan_state && mqtt_state) || (WiFi.status() == WL_CONNECTED && sim_mode != SIM_NONE))
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
            cbpiEventActors(EM_ACTER);
          if (millis() > lastSenInd + wait_on_Sensor_error_induction) // Wait for approx WAIT_ON_ERROR/1000 seconds
          {
            if (inductionCooker.isInduon && inductionCooker.powerLevelOnError < 100 && inductionCooker.induction_state)
              cbpiEventInduction(EM_INDER);
          }
        } // Switchable
      }   // Iterate sensors
    }     // wlan und mqtt state
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
    for (int i = 0; i < numberOfActors; i++)
    {
      if (actors[i].switchable && actors[i].actor_state && actors[i].isOn) // Check if actor is switchable, isOn and in normal mode - 20190917: Prüfen!
      {
        actors[i].isOnBeforeError = actors[i].isOn;
        actors[i].isOn = false;
        //actors[i].power_actor = 0;
        actors[i].actor_state = false;
        actors[i].Update();
        DBG_PRINT("EM ACTER: Actor: ");
        DBG_PRINTLN(actors[i].name_actor);
        DBG_PRINT("EM ACTER: state: ");
        DBG_PRINTLN(actors[i].actor_state);
        DBG_PRINT("EM ACTER: isOnBeforeError: "); // Actor on/off before error event
        DBG_PRINTLN(actors[i].isOnBeforeError);
      }
    }
    break;
  case EM_ACTOFF:
    for (int i = 0; i < numberOfActors; i++)
    {
      if (actors[i].isOn)
      {
        actors[i].isOn = false;
        actors[i].Update();
        DBG_PRINT("EM ACTER: Actor: ");
        DBG_PRINT(actors[i].name_actor);
        DBG_PRINTLN(" switched off");
      }
    }
    break;
  case SIM_ACT: // Test event - ignore!
    DBG_PRINTLN("SIM ACTTEST: actors simulation event: switch on ALL actors");
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
  //setTCPPowerAct();
}
void listenerInduction(int event, int parm) // Induction event listener
{
  switch (parm)
  {
  case EM_OK: // Induction off
    break;
  case 1: // Induction on
    break;
  case 2:
    //DBG_PRINTLN("EM IND2: received induction event"); // bislang keine Verwendung
    break;
  case EM_INDER:
    if (inductionCooker.isInduon && inductionCooker.powerLevelOnError < 100 && inductionCooker.induction_state) // powerlevelonerror == 100 -> kein event handling
    {
      inductionCooker.powerLevelBeforeError = inductionCooker.power;
      DBG_PRINT("EM INDER: induction power: ");
      DBG_PRINTLN(inductionCooker.power);

      DBG_PRINT("EM INDER: Set induction power level to ");
      if (inductionCooker.powerLevelOnError == 0)
      {
        DBG_PRINT(inductionCooker.powerLevelOnError);
        DBG_PRINTLN(" - induction switched off");
        inductionCooker.isInduon = false;
      }
      else
      {
        DBG_PRINT(inductionCooker.powerLevelOnError);
        DBG_PRINTLN("%");
        inductionCooker.newPower = inductionCooker.powerLevelOnError;
      }
      inductionCooker.newPower = inductionCooker.powerLevelOnError;
      inductionCooker.induction_state = false;
      inductionCooker.Update();
    }
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
  case SIM_IND: // Test event - ignore!
    if (inductionCooker.isEnabled)
    {
      DBG_PRINTLN("SIM INDTEST: induction simulation event - switch on induction 100%");
      inductionCooker.newPower = 100;
      inductionCooker.isInduon = true;
      inductionCooker.isRelayon = true;
      inductionCooker.isPower = true;
      inductionCooker.isEnabled = true;
      inductionCooker.induction_state = true;
      inductionCooker.Update();
    }
    break;
  default:
    break;
  }
  handleInduction();
  //setTCPPowerInd();
}

// EventManer Queues
// starting with firmware 1.048 original unmodified EventManager library required
// Remove modified lib from older versions an download from https://github.com/igormiktor/arduino-EventManager
void cbpiEventSystem(int parm) // System events
{
  gEM.queueEvent(EventManager::kEventUser0, parm);
}

void cbpiEventSensors(int parm) // Sensor events
{
  gEM.queueEvent(EventManager::kEventUser1, parm);
}
void cbpiEventActors(int parm) // Actor events
{
  gEM.queueEvent(EventManager::kEventUser2, parm);
}
void cbpiEventInduction(int parm) // Induction events
{
  gEM.queueEvent(EventManager::kEventUser3, parm);
}
