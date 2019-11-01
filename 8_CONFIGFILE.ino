bool loadConfig()
{
  DBG_PRINTLN("------ loadConfig started ------");
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile)
  {
    DBG_PRINTLN("Failed to open config file");
    DBG_PRINTLN("------ loadConfig aborted ------");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024)
  {
    DBG_PRINT("Config file size is too large");
    DBG_PRINTLN("------ loadConfig aborted ------");
    return false;
  }
  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);

  StaticJsonDocument<1400> doc;
  DeserializationError error = deserializeJson(doc, buf.get());
  if (error)
  {
    DBG_PRINT("Conf: Error Json ");
    DBG_PRINTLN(error.c_str());
    return false;
  }

  int tempInt = 0; // temp int var

  JsonArray actorsArray = doc["actors"];
  numberOfActors = actorsArray.size();
  if (numberOfActors > numberOfActorsMax)
  {
    numberOfActors = numberOfActorsMax;
  }

  for (int i = 0; i < numberOfActors; i++)
  {
    if (i < numberOfActors)
    {
      JsonObject actorObj = actorsArray[i];
      String actorPin = actorObj["PIN"];
      String actorScript = actorObj["SCRIPT"];
      String actorName = actorObj["NAME"];
      String actorInv = actorObj["INV"];
      String actorSwitch = actorObj["SW"];
      String actorKettle_id = "0";
      if (actorObj.containsKey("kettle_id"))
        actorKettle_id = actorObj["kettle_id"].as<String>();

      actors[i].change(actorPin, actorScript, actorName, actorInv, actorSwitch, actorKettle_id);
      DBG_PRINT("Actor no. ");
      DBG_PRINT(i + 1);
      DBG_PRINTLN(" read settings");
      DBG_PRINT("Name: ");
      DBG_PRINTLN(actorName);
      DBG_PRINT("MQTT topic: ");
      DBG_PRINTLN(actorScript);
      DBG_PRINT("PIN: ");
      DBG_PRINTLN(actorPin);
      DBG_PRINT("Inverted: ");
      DBG_PRINTLN(actorInv);
      DBG_PRINT("Switchable: ");
      DBG_PRINTLN(actorSwitch);
      DBG_PRINT("Kettle ID: ");
      DBG_PRINTLN(actorKettle_id);
    }
  }
  if (numberOfActors > 0)
  {
    DBG_PRINT("Total number of actors loaded: ");
    DBG_PRINTLN(numberOfActors);
  }
  else
    DBG_PRINTLN("No actors loaded from config file");
  DBG_PRINTLN("--------------------");

  JsonArray sensorsArray = doc["sensors"];
  numberOfSensors = sensorsArray.size();

  if (numberOfSensors > numberOfSensorsMax)
  {
    numberOfSensors = numberOfSensorsMax;
  }
  for (int i = 0; i < numberOfSensorsMax; i++)
  {
    if (i < numberOfSensors)
    {
      JsonObject sensorsObj = sensorsArray[i];
      String sensorsAddress = sensorsObj["ADDRESS"];
      String sensorsScript = sensorsObj["SCRIPT"];
      String sensorsName = sensorsObj["NAME"];
      String sensorsSwitch = sensorsObj["SW"];
      String sensorsKettle_id = "0";
      if (sensorsObj.containsKey("kettle_id"))
        sensorsKettle_id = sensorsObj["kettle_id"].as<String>();
      float sensorsOffset = 0.0;
      if (sensorsObj.containsKey("OFFSET"))
        sensorsOffset = sensorsObj["OFFSET"];

      sensors[i].change(sensorsAddress, sensorsScript, sensorsName, sensorsOffset, sensorsSwitch, sensorsKettle_id);
      DBG_PRINT("Sensor no. ");
      DBG_PRINT(i + 1);
      DBG_PRINTLN(" read settings");
      DBG_PRINT("Name: ");
      DBG_PRINTLN(sensorsName);
      DBG_PRINT("Address: ");
      DBG_PRINTLN(sensorsAddress);
      DBG_PRINT("MQTT topic: ");
      DBG_PRINTLN(sensorsScript);
      DBG_PRINT("Offset: ");
      DBG_PRINTLN(sensorsOffset);
      DBG_PRINT("Switchable: ");
      DBG_PRINTLN(sensorsSwitch);
      DBG_PRINT("Kettle ID: ");
      DBG_PRINTLN(sensorsKettle_id);
    }
    else
      sensors[i].change("", "", "", 0.0, false, "0");
  }

  if (numberOfSensors > 0)
  {
    DBG_PRINT("Total number of sensors loaded: ");
    DBG_PRINTLN(numberOfSensors);
  }
  else
    DBG_PRINTLN("No sensors loaded from config file");

  DBG_PRINTLN("--------------------");

  DBG_PRINT("Induction status: ");
  JsonArray indArray = doc["induction"];
  JsonObject indObj = indArray[0];
  if (indObj.containsKey("ENABLED"))
  {
    inductionStatus = 1;
    String indEnabled = indObj["ENABLED"];
    String indPinWhite = indObj["PINWHITE"];
    String indPinYellow = indObj["PINYELLOW"];
    String indPinBlue = indObj["PINBLUE"];
    String indScript = indObj["TOPIC"];
    String indKettleid = "0";
    if (indObj.containsKey("kettle_id"))
      indKettleid = indObj["kettle_id"].as<String>();

    long indDelayOff = DEF_DELAY_IND; //default delay
    int indPowerLevel = 100;
    if (indObj.containsKey("PL"))
      indPowerLevel = indObj["PL"];

    if (indObj.containsKey("DELAY"))
      indDelayOff = indObj["DELAY"];

    inductionCooker.change(StringToPin(indPinWhite), StringToPin(indPinYellow), StringToPin(indPinBlue), indScript, indDelayOff, indEnabled, indPowerLevel, indKettleid);

    DBG_PRINTLN(inductionStatus);
    DBG_PRINTLN("Read induction settings");
    DBG_PRINT("MQTT topic: ");
    DBG_PRINTLN(indScript);
    DBG_PRINT("Relais (WHITE): ");
    DBG_PRINTLN(indPinWhite);
    DBG_PRINT("Command channel (YELLOW): ");
    DBG_PRINTLN(indPinYellow);
    DBG_PRINT("Backchannel (BLUE): ");
    DBG_PRINTLN(indPinBlue);
    DBG_PRINT("Delay after power off: ");
    DBG_PRINT(indDelayOff / 1000);
    DBG_PRINTLN("sec");
    DBG_PRINT("Power level on error: ");
    DBG_PRINTLN(indPowerLevel);
    DBG_PRINT("Kettle ID: ");
    DBG_PRINTLN(indKettleid);
  }
  else
  {
    inductionStatus = 0;
    DBG_PRINTLN(inductionStatus);
  }
  DBG_PRINTLN("--------------------");
  DBG_PRINT("OLED display status: ");
  JsonArray displayArray = doc["display"];
  JsonObject displayObj = displayArray[0];
  if (displayObj["ENABLED"] == "1")
    useDisplay = true;
  else
    useDisplay = false;

  if (useDisplay)
  {
    String dispAddress = displayObj["ADDRESS"];
    dispAddress.remove(0, 2);
    char copy[4];
    dispAddress.toCharArray(copy, 4);
    int address = strtol(copy, 0, 16);
    if (displayObj.containsKey("updisp"))
      DISP_UPDATE = displayObj["updisp"];

    oledDisplay.dispEnabled = true;
    oledDisplay.change(address, oledDisplay.dispEnabled);

    DBG_PRINTLN(oledDisplay.dispEnabled);
    DBG_PRINTLN("Read display settings");
    DBG_PRINT("Address: ");
    DBG_PRINT(dispAddress);
    DBG_PRINT(" Update interval: ");
    DBG_PRINT(DISP_UPDATE / 1000);
    DBG_PRINTLN("sec");
  }
  else
  {
    useDisplay = false;
    oledDisplay.dispEnabled = false;
    DBG_PRINTLN(oledDisplay.dispEnabled);
  }

  DBG_PRINTLN("--------------------");

  // Misc Settings
  DBG_PRINTLN("Misc read settings");
  JsonArray miscArray = doc["misc"];
  JsonObject miscObj = miscArray[0];

  if (miscObj.containsKey("del_sen_act"))
    wait_on_Sensor_error_actor = miscObj["del_sen_act"];

  if (miscObj.containsKey("del_sen_ind"))
    wait_on_Sensor_error_induction = miscObj["del_sen_ind"];

  if (miscObj.containsKey("delay_mqtt"))
    wait_on_error_mqtt = miscObj["delay_mqtt"];

  DBG_PRINT("Wait on sensor error actors: ");
  DBG_PRINTLN(wait_on_Sensor_error_actor);
  DBG_PRINT("Wait on sensor error induction: ");
  DBG_PRINTLN(wait_on_Sensor_error_induction);

  DBG_PRINT("Switch off actors on error ");
  if (miscObj["enable_mqtt"] == "1")
  {
    StopOnMQTTError = true;
    DBG_PRINT("enabled after ");
    DBG_PRINT(wait_on_error_mqtt / 1000);
    DBG_PRINTLN("sec");
  }
  else
  {
    StopOnMQTTError = false;
    DBG_PRINTLN("disabled");
  }
  DBG_PRINT("Switch off induction on error ");
  if (miscObj.containsKey("delay_wlan"))
    wait_on_error_wlan = miscObj["delay_wlan"];

  if (miscObj["enable_wlan"] == "1")
  {
    StopOnWLANError = true;
    DBG_PRINT("enabled after ");
    DBG_PRINT(wait_on_error_wlan / 1000);
    DBG_PRINTLN("sec");
  }
  else
  {
    StopOnWLANError = false;
    DBG_PRINTLN("disabled");
  }
  strlcpy(nameMDNS, miscObj["mdns_name"], sizeof(nameMDNS));
  if (miscObj["mdns"] == "1")
  {
    startMDNS = true;
    DBG_PRINT("mDNS activated: ");
    DBG_PRINTLN(nameMDNS);
  }
  else
  {
    startMDNS = false;
    DBG_PRINTLN("mDNS disabled");
  }

  if (miscObj.containsKey("upsen"))
    SEN_UPDATE = miscObj["upsen"];
  if (miscObj.containsKey("upact"))
    ACT_UPDATE = miscObj["upact"];
  if (miscObj.containsKey("upind"))
    IND_UPDATE = miscObj["upind"];
  if (miscObj.containsKey("upsys"))
    SYS_UPDATE = miscObj["upsys"];

  DBG_PRINT("Sensors update intervall: ");
  DBG_PRINT(SEN_UPDATE / 1000);
  DBG_PRINTLN("sec");
  DBG_PRINT("Actors update intervall: ");
  DBG_PRINT(ACT_UPDATE / 1000);
  DBG_PRINTLN("sec");
  DBG_PRINT("Induction update intervall: ");
  DBG_PRINT(IND_UPDATE / 1000);
  DBG_PRINTLN("sec");
  if (miscObj["tcp"] == "1")
    startTCP = true;
  else
    startTCP = false;
  if (miscObj.containsKey("uptcp"))
    TCP_UPDATE = miscObj["uptcp"];

  if (miscObj.containsKey("TCPHOST"))
  {
    strlcpy(tcpHost, miscObj["TCPHOST"], sizeof(tcpHost));
    DBG_PRINT("TCP server IP: ");
    DBG_PRINTLN(tcpHost);
  }
  else
  {
    if (startTCP)
      DBG_PRINTLN("TCP Server disabled (no host)");
    startTCP = false;
  }
  if (miscObj.containsKey("TCPPORT"))
  {
    tcpPort = miscObj["TCPPORT"];
    DBG_PRINT("TCP server Port ");
    DBG_PRINTLN(tcpPort);
  }
  else
  {
    if (startTCP)
      DBG_PRINTLN("TCP Server disabled (no port)");
    startTCP = false;
  }
  if (miscObj.containsKey("MQTTHOST"))
  {
    strlcpy(mqtthost, miscObj["TCPHOST"], sizeof(mqtthost));
    DBG_PRINT("MQTT server IP: ");
    DBG_PRINTLN(mqtthost);
  }
  else
  {
    DBG_PRINT("MQTT server not found in config file. Using default server address: ");
    DBG_PRINTLN(mqtthost);
  }
  if (miscObj["telnet"] == "1")
  {
    startTEL = true;
    DBG_PRINTLN("Telnet activated");
  }
  else
  {
    startTEL = false;
    DBG_PRINTLN("Telnet disabled");
  }
  if (miscObj["debug"] == "1")
  {
    setDEBUG = true;
    DBG_PRINTLN("Debug output on serial monitor enabled");
  }
  else
    setDEBUG = false;

  DBG_PRINTLN("------ loadConfig finished ------");
  configFile.close();

  if (startTCP)
    setTCPConfig();
  return true;
}

void saveConfigCallback()
{
  DBG_PRINTLN("Should save config");
  // shouldSaveConfig = true;
}

bool saveConfig()
{
  DBG_PRINTLN("------ saveConfig started ------");
  StaticJsonDocument<1400> doc;
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile)
  {
    DBG_PRINTLN("Failed to open config file for writing");
    DBG_PRINTLN("------ saveConfig aborted ------");
    return false;
  }

  // Write Actors
  JsonArray actorsArray = doc.createNestedArray("actors");
  for (int i = 0; i < numberOfActors; i++)
  {
    JsonObject actorsObj = actorsArray.createNestedObject();
    actorsObj["PIN"] = PinToString(actors[i].pin_actor);
    actorsObj["NAME"] = actors[i].name_actor;
    actorsObj["SCRIPT"] = actors[i].argument_actor;
    actorsObj["INV"] = actors[i].getInverted();
    actorsObj["SW"] = actors[i].getSwitchable();
    actorsObj["kettle_id"] = actors[i].kettle_id;

    DBG_PRINT("Actor no. ");
    DBG_PRINT(i + 1);
    DBG_PRINTLN(" write settings");
    DBG_PRINT("Name: ");
    DBG_PRINTLN(actors[i].name_actor);
    DBG_PRINT("PIN: ");
    DBG_PRINTLN(actors[i].pin_actor);
    DBG_PRINT("MQTT topic: ");
    DBG_PRINTLN(actors[i].argument_actor);
    DBG_PRINT("Inverted: ");
    DBG_PRINTLN(actors[i].getInverted());
    DBG_PRINT("Switchable: ");
    DBG_PRINTLN(actors[i].getSwitchable());
    DBG_PRINT("Kettle ID: ");
    DBG_PRINTLN(actors[i].kettle_id);
  }
  if (numberOfActors > 0)
  {
    DBG_PRINT("Total number of actors saved: ");
    DBG_PRINTLN(numberOfActors);
  }
  else
    DBG_PRINTLN("No actors saved to config file");

  DBG_PRINTLN("--------------------");

  // Write Sensors
  JsonArray sensorsArray = doc.createNestedArray("sensors");
  for (int i = 0; i < numberOfSensors; i++)
  {
    JsonObject sensorsObj = sensorsArray.createNestedObject();
    sensorsObj["ADDRESS"] = sensors[i].getSens_adress_string();
    sensorsObj["NAME"] = sensors[i].sens_name;
    sensorsObj["OFFSET"] = sensors[i].sens_offset;
    sensorsObj["SCRIPT"] = sensors[i].sens_mqtttopic;
    sensorsObj["SW"] = sensors[i].sens_sw;
    sensorsObj["kettle_id"] = sensors[i].kettle_id;
    DBG_PRINT("Sensor no. ");
    DBG_PRINT(i + 1);
    DBG_PRINTLN(" write settings");
    DBG_PRINT("Name: ");
    DBG_PRINTLN(sensors[i].sens_name);
    DBG_PRINT("Address: ");
    DBG_PRINTLN(sensors[i].getSens_adress_string());
    DBG_PRINT("MQTT topic: ");
    DBG_PRINTLN(sensors[i].sens_mqtttopic);
    DBG_PRINT("Offset: ");
    DBG_PRINTLN(sensors[i].sens_offset);
    DBG_PRINT("Switchable: ");
    DBG_PRINTLN(sensors[i].sens_sw);
    DBG_PRINT("Kettle ID: ");
    DBG_PRINTLN(sensors[i].kettle_id);
  }
  if (numberOfSensors > 0)
  {
    DBG_PRINT("Total number of sensors saved: ");
    DBG_PRINTLN(numberOfSensors);
  }
  else
    DBG_PRINTLN("No sensors saved to config file");

  DBG_PRINTLN("--------------------");

  // Write Induction
  JsonArray indArray = doc.createNestedArray("induction");
  DBG_PRINTLN("Induction write settings");
  DBG_PRINT("Status: ");
  if (inductionCooker.isEnabled)
  {
    JsonObject indObj = indArray.createNestedObject();
    indObj["PINWHITE"] = PinToString(inductionCooker.PIN_WHITE);
    indObj["PINYELLOW"] = PinToString(inductionCooker.PIN_YELLOW);
    indObj["PINBLUE"] = PinToString(inductionCooker.PIN_INTERRUPT);
    indObj["TOPIC"] = inductionCooker.mqtttopic;
    indObj["DELAY"] = inductionCooker.delayAfteroff;
    indObj["ENABLED"] = "1";
    indObj["PL"] = inductionCooker.powerLevelOnError;
    indObj["kettle_id"] = inductionCooker.kettle_id;

    DBG_PRINTLN(inductionCooker.isEnabled);

    DBG_PRINT("MQTT topic: ");
    DBG_PRINTLN(inductionCooker.mqtttopic);
    DBG_PRINT("Relais (WHITE): ");
    DBG_PRINTLN(PinToString(inductionCooker.PIN_WHITE));
    DBG_PRINT("Command channel (YELLOW): ");
    DBG_PRINTLN(PinToString(inductionCooker.PIN_YELLOW));
    DBG_PRINT("Backchannel (BLUE): ");
    DBG_PRINTLN(PinToString(inductionCooker.PIN_INTERRUPT));
    DBG_PRINT("Delay after power off: ");
    DBG_PRINTLN(inductionCooker.delayAfteroff / 1000);
    DBG_PRINT("Power level on error: ");
    DBG_PRINTLN(inductionCooker.powerLevelOnError);
    DBG_PRINT("Kettle ID: ");
    DBG_PRINTLN(inductionCooker.kettle_id);
  }
  else
    DBG_PRINTLN(inductionCooker.isEnabled);

  DBG_PRINTLN("--------------------");

  // Write Display
  JsonArray displayArray = doc.createNestedArray("display");

  DBG_PRINTLN("OLED display write settings");
  DBG_PRINT("Status: ");
  DBG_PRINTLN(oledDisplay.dispEnabled);
  if (oledDisplay.dispEnabled)
  {
    JsonObject displayObj = displayArray.createNestedObject();
    displayObj["ENABLED"] = "1";
    displayObj["ADDRESS"] = String(decToHex(oledDisplay.address, 2));
    displayObj["updisp"] = DISP_UPDATE;

    DBG_PRINT("Address ");
    DBG_PRINTLN(String(decToHex(oledDisplay.address, 2)));
    DBG_PRINT("Update interval ");
    DBG_PRINT(DISP_UPDATE / 1000);
    DBG_PRINTLN("sec");
    if (oledDisplay.address == 0x3C || oledDisplay.address == 0x3D)
    {
      display.ssd1306_command(SSD1306_DISPLAYON);
      cbpiEventSystem(EM_DISPUP);
    }
    else
    {
      displayObj["ENABLED"] = "0";
      oledDisplay.dispEnabled = false;
      useDisplay = false;
    }
  }
  else
    display.ssd1306_command(SSD1306_DISPLAYOFF);

  DBG_PRINTLN("--------------------");

  // Write Misc Stuff
  DBG_PRINTLN("Misc write settings");
  JsonArray miscArray = doc.createNestedArray("misc");
  JsonObject miscObj = miscArray.createNestedObject();

  miscObj["del_sen_act"] = wait_on_Sensor_error_actor;
  miscObj["del_sen_ind"] = wait_on_Sensor_error_induction;
  DBG_PRINT("Wait on sensor error actors: ");
  DBG_PRINTLN(wait_on_Sensor_error_actor);
  DBG_PRINT("Wait on sensor error induction: ");
  DBG_PRINTLN(wait_on_Sensor_error_induction);
  miscObj["delay_mqtt"] = wait_on_error_mqtt;

  DBG_PRINT("Switch off actors on error ");
  if (StopOnMQTTError)
  {
    miscObj["enable_mqtt"] = "1";
    DBG_PRINT("enabled after ");
    DBG_PRINT(wait_on_error_mqtt / 1000);
    DBG_PRINTLN("sec");
  }
  else
  {
    miscObj["enable_mqtt"] = "0";
    DBG_PRINTLN("disabled");
  }

  miscObj["delay_wlan"] = wait_on_error_wlan;

  DBG_PRINT("Switch off induction on error ");
  if (StopOnWLANError)
  {
    miscObj["enable_wlan"] = "1";
    DBG_PRINT("enabled after ");
    DBG_PRINT(wait_on_error_wlan / 1000);
    DBG_PRINTLN("sec");
  }
  else
  {
    miscObj["enable_wlan"] = "0";
    DBG_PRINTLN("disabled");
  }
  DBG_PRINT("Telnet Server ");
  if (startTEL)
  {
    miscObj["telnet"] = "1";
    DBG_PRINTLN("enabled");
  }
  else
  {
    miscObj["telnet"] = "0";
    DBG_PRINTLN("disabled");
  }
  if (setDEBUG)
    miscObj["debug"] = "1";
  else
    miscObj["debug"] = "0";

  miscObj["mdns_name"] = nameMDNS;
  if (startMDNS)
    miscObj["mdns"] = "1";
  else
    miscObj["mdns"] = "0";

  miscObj["TCPHOST"] = tcpHost;
  miscObj["TCPPORT"] = tcpPort;
  miscObj["uptcp"] = TCP_UPDATE;
  DBG_PRINT("TCP Server ");
  if (startTCP)
  {
    miscObj["tcp"] = "1";
    DBG_PRINT("enabled ");
    DBG_PRINT(tcpHost);
    DBG_PRINT(" ");
    DBG_PRINT(tcpPort);
    DBG_PRINT(" ");
    DBG_PRINT(TCP_UPDATE / 1000);
    DBG_PRINTLN("sec");
  }
  else
  {
    miscObj["tcp"] = "0";
    DBG_PRINTLN("disabled");
  }

  miscObj["MQTTHOST"] = mqtthost;
  miscObj["upsen"] = SEN_UPDATE;
  miscObj["upact"] = ACT_UPDATE;
  miscObj["upind"] = IND_UPDATE;
  miscObj["upsys"] = SYS_UPDATE;

  DBG_PRINT("Sensor update interval ");
  DBG_PRINT(SEN_UPDATE / 1000);
  DBG_PRINTLN("sec");
  DBG_PRINT("Actors update interval ");
  DBG_PRINT(ACT_UPDATE / 1000);
  DBG_PRINTLN("sec");
  DBG_PRINT("Induction update interval ");
  DBG_PRINT(IND_UPDATE / 1000);
  DBG_PRINTLN("sec");
  DBG_PRINT("Sys update interval ");
  DBG_PRINT(SYS_UPDATE / 1000);
  DBG_PRINTLN("sec");
  DBG_PRINT("MQTT broker IP: ");
  DBG_PRINTLN(mqtthost);

  serializeJson(doc, configFile);
  configFile.close();
  DBG_PRINTLN("------ saveConfig finished ------");
  size_t len = measureJson(doc);
  DBG_PRINT("JSON config length: ");
  DBG_PRINTLN(len);
  if (len > 1400)
    DBG_PRINTLN("Error: JSON config too big!");
  aktIP = WiFi.localIP();
  String Network = WiFi.SSID();
  DBG_PRINT("ESP8266 device IP Address: ");
  DBG_PRINTLN(aktIP.toString());
  DBG_PRINT("Configured WLAN SSID: ");
  DBG_PRINTLN(Network);
  DBG_PRINTLN("---------------------------------");
  if (startTCP)
    setTCPConfig();
  return true;
}
