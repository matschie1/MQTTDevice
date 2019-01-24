bool loadConfig() {
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }
  else {
    DBG_PRINTLN("------ loadConfig started ------");
    DBG_PRINTLN("opened config file");
  }
  size_t size = configFile.size();
  if (size > 1024) {
    DBG_PRINT("Config file size is too large");
    return false;
  }
  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);
  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    return false;
  }

  JsonArray& jsonactors = json["actors"];
  numberOfActors = jsonactors.size();
  if (numberOfActors > 6) {
    numberOfActors = 6;
  }

  for (int i = 0; i < numberOfActors; i++) {
    if (i < numberOfActors) {
      JsonObject& jsonactor = jsonactors[i];
      String pin = jsonactor["PIN"];
      String script = jsonactor["SCRIPT"];
      String aname = jsonactor["NAME"];
      String ainverted = jsonactor["INVERTED"];
      actors[i].change(pin, script, aname, ainverted);
    }
  }

  DBG_PRINT("Number of Actors loaded: ");
  DBG_PRINTLN(numberOfActors);
  DBG_PRINTLN("--------------------");

  JsonArray& jsonsensors = json["sensors"];
  numberOfSensors = jsonsensors.size();
  DBG_PRINT("Number of Sensors loaded: ");
  DBG_PRINTLN(numberOfSensors);

  if (numberOfSensors > 10) {
    numberOfSensors = 10;
  }
  for (int i = 0; i < 10; i++) {
    if (i < numberOfSensors) {
      JsonObject& jsonsensor = jsonsensors[i];
      String aadress = jsonsensor["ADDRESS"];
      String ascript = jsonsensor["SCRIPT"];
      String aname = jsonsensor["NAME"];
      sensors[i].change(aadress, ascript, aname);
    } else {
      sensors[i].change("", "", "");
    }
  }
  DBG_PRINTLN("--------------------");
  
  JsonArray& jsinductions = json["induction"];
  JsonObject& jsinduction = jsinductions[0];
  String pin_white = jsinduction["PINWHITE"];
  String pin_yellow = jsinduction["PINYELLOW"];
  String pin_blue = jsinduction["PINBLUE"];
  String is_enabled_str = jsinduction["ENABLED"];
  bool is_enabled_bl = false;
  if (is_enabled_str == "1") {
    is_enabled_bl = true;
  }

  String js_mqtttopic = jsinduction["TOPIC"];
  long delayoff = atol(jsinduction["DELAY"]);

  inductionCooker.change(StringToPin(pin_white), StringToPin(pin_yellow), StringToPin(pin_blue), js_mqtttopic, delayoff, is_enabled_bl);
  DBG_PRINT("Induction enabled: ");
  DBG_PRINTLN(is_enabled_bl);
  DBG_PRINTLN("--------------------");
  
#ifdef DISPLAY
  JsonArray& jsodisplay = json["display"];
  JsonObject& jsdisplay = jsodisplay[0];
  String dispAddress = jsdisplay["ADDRESS"];
  DBG_PRINT("Display address: ");
  DBG_PRINTLN(dispAddress);
  dispAddress.remove(0,2);
  char copy[4];
  dispAddress.toCharArray(copy, 4);     // hier werden mal so richtig schön ressourcen verschwendet 
  int address = strtol(copy, 0, 16);
  String is_enabled_disp = jsdisplay["ENABLED"];
  if (is_enabled_disp == "1") {
    dispEnabled = 1;
  }
  else
  {
    dispEnabled = 0;
  }
  DBG_PRINTLN("Config: Display change");
  oledDisplay.change(address, dispEnabled);
  DBG_PRINTLN("--------------------");
#endif

  // General Settings
  String json_mqtthost = json["MQTTHOST"];
  json_mqtthost.toCharArray(mqtthost, 16);

  DBG_PRINT("MQTTHost: ");
  DBG_PRINTLN(mqtthost);
  DBG_PRINTLN("------ loadConfig finished ------");
  return true;
}

void saveConfigCallback () {
  DBG_PRINTLN("Should save config"); // was soll callback mal tun?
}


bool saveConfig() 
{
  DBG_PRINTLN("------ saveConfig started ------");
  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    DBG_PRINTLN("Failed to open config file for writing");
    return false;
  }

  // Write Actors
  JsonArray& jsactors = json.createNestedArray("actors");
  for (int i = 0; i < numberOfActors; i++) {
    JsonObject& jsactor = jsactors.createNestedObject();
    jsactor["PIN"] = PinToString(actors[i].pin_actor);
    jsactor["NAME"] = actors[i].name_actor;
    jsactor["SCRIPT"] = actors[i].argument_actor;
    jsactor["INVERTED"] = actors[i].getInverted();
  }
  DBG_PRINT("Number of Actors saveded: ");
  DBG_PRINTLN(numberOfActors);
  DBG_PRINTLN("--------------------");

  // Write Sensors
  JsonArray& jssensors = json.createNestedArray("sensors");
  for (int i = 0; i < numberOfSensors; i++) {
    JsonObject& jssensor = jssensors.createNestedObject();
    jssensor["ADDRESS"] = sensors[i].getSens_adress_string();
    jssensor["NAME"] = sensors[i].sens_name;
    jssensor["SCRIPT"] = sensors[i].sens_mqtttopic;
  }
  DBG_PRINT("Number of Sensors saveded: ");
  DBG_PRINTLN(numberOfSensors);
  DBG_PRINTLN("--------------------");

  // Write Induction
  JsonArray& jsinductions = json.createNestedArray("induction");
  JsonObject&  jsinduction = jsinductions.createNestedObject();
  if (inductionCooker.isEnabled) {
    jsinduction["ENABLED"] = "1";
  } else {
    jsinduction["ENABLED"] = "0";
  }
  jsinduction["PINWHITE"] = PinToString(inductionCooker.PIN_WHITE);
  jsinduction["PINYELLOW"] = PinToString(inductionCooker.PIN_YELLOW);
  jsinduction["PINBLUE"] = PinToString(inductionCooker.PIN_INTERRUPT);
  jsinduction["TOPIC"] = inductionCooker.mqtttopic;
  jsinduction["DELAY"] = inductionCooker.delayAfteroff;
  DBG_PRINT("Induction enabled saveded: ");
  DBG_PRINTLN(inductionCooker.isEnabled);
  DBG_PRINTLN("--------------------");

#ifdef DISPLAY
  // Write Display
  JsonArray& jsodisplay = json.createNestedArray("display");
  JsonObject&  jsdisplay = jsodisplay.createNestedObject();
  if (oledDisplay.dispEnabled) {
    jsdisplay["ENABLED"] = "1";
  } else {
    jsdisplay["ENABLED"] = "0";
  }

  DBG_PRINT("Display address saved as String: ");
  DBG_PRINTLN(String(decToHex(oledDisplay.address, 2)));
  jsdisplay["ADDRESS"] = String(decToHex(oledDisplay.address, 2));
  DBG_PRINTLN("--------------------");
#endif
  // Write General Stuff
  json["MQTTHOST"] = mqtthost;
  json.printTo(configFile);
  DBG_PRINTLN("------ saveConfig finished ------");
  return true;
}

String decToHex(byte decValue, byte desiredStringLength)
{
  String hexString = String(decValue, HEX);
  while (hexString.length() < desiredStringLength) hexString = "0" + hexString;

  return "0x" + hexString;
}
