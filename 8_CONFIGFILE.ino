bool loadConfig() {
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    DBG_PRINTLN("Failed to open config file");
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

  JsonArray& jsodisplay = json["display"];
  JsonObject& jsdisplay = jsodisplay[0];
  String dispAddress = jsdisplay["ADDRESS"];
  dispAddress.remove(0, 2);
  char copy[4];
  dispAddress.toCharArray(copy, 4);
  int address = strtol(copy, 0, 16);
  String dispStatus = jsdisplay["ENABLED"];
  if (dispStatus == "1") {
    oledDisplay.dispEnabled = 1;
  }
  else {
    oledDisplay.dispEnabled = 0;
  }
  DBG_PRINT("Display address: ");
  DBG_PRINTLN(dispAddress);
  oledDisplay.change(address, oledDisplay.dispEnabled);
  DBG_PRINTLN("--------------------");

  // Misc Settings
  JsonArray& jsomisc = json["misc"];
  JsonObject& jsmisc = jsomisc[0];
  String str_act = jsmisc["enable_actors"];
  String str_act_del = jsmisc["delay_actors"];
  DBG_PRINT("Actoren: ");
  DBG_PRINT(str_act);
  DBG_PRINT("nach: ");
  DBG_PRINT(str_act_del);
  DBG_PRINTLN("sec");
  wait_on_error_actors = atol(jsmisc["delay_actors"]);
  DBG_PRINT("Switch off all actors on error: ");
  if (str_act == "1") {
    StopActorsOnError = true;
    DBG_PRINT(" ON after ");
    DBG_PRINT(wait_on_error_induction);
    DBG_PRINTLN("ms");
  }
  else {
    StopActorsOnError = false;
    DBG_PRINTLN(" OFF ");
  }

  String str_ind = jsmisc["enable_induction"];
  wait_on_error_induction = atol(jsmisc["delay_induction"]);
  DBG_PRINT("Switch off induction on error: ");
  if (str_ind == "1") {
    StopInductionOnError = true;
    DBG_PRINT(" ON after ");
    DBG_PRINT(wait_on_error_induction);
    DBG_PRINTLN("ms");
  }
  else {
    StopInductionOnError = false;
    DBG_PRINTLN(" OFF ");
  }
  String str_debug = jsmisc["debug"];
  if (str_debug == "1") {
    setDEBUG = true;
    DBG_PRINTLN("1 DEBUG ein");
  }
  else {
    setDEBUG = false;
    DBG_PRINTLN("1 DEBUG aus");
  }

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

  // Write Display
  JsonArray& jsodisplay = json.createNestedArray("display");
  JsonObject&  jsdisplay = jsodisplay.createNestedObject();
  jsdisplay["ENABLED"] = "0";
  jsdisplay["ADDRESS"] = "0";
  if (oledDisplay.dispEnabled) {
    jsdisplay["ENABLED"] = "1";
  } else {
    jsdisplay["ENABLED"] = "0";
  }

  DBG_PRINT("Display address saved as String: ");
  DBG_PRINTLN(String(decToHex(oledDisplay.address, 2)));
  jsdisplay["ADDRESS"] = String(decToHex(oledDisplay.address, 2));
  DBG_PRINTLN("--------------------");

  // Write Misc Stuff
  JsonArray& jsomisc = json.createNestedArray("misc");
  JsonObject&  jsmisc = jsomisc.createNestedObject();

  if (StopActorsOnError) {
    jsmisc["enable_actors"] = "1";
  } else {
    jsmisc["enable_actors"] = "0";
  }
  jsmisc["delay_actors"] = wait_on_error_actors;
  DBG_PRINT("Swtich off actors on error: ");
  DBG_PRINT(StopActorsOnError);
  DBG_PRINT(" after: ");
  DBG_PRINT(wait_on_error_actors);
  DBG_PRINTLN("ms");
  if (StopInductionOnError) {
    jsmisc["enable_induction"] = "1";
  } else {
    jsmisc["enable_induction"] = "0";
  }
  jsmisc["delay_induction"] = wait_on_error_induction;
  DBG_PRINT("Swtich off induction on error: ");
  DBG_PRINT(StopActorsOnError);
  DBG_PRINT(" after: ");
  DBG_PRINT(wait_on_error_induction);
  DBG_PRINTLN("ms");
  if (setDEBUG) {
    jsmisc["debug"] = "1";
  } else {
    jsmisc["debug"] = "0";
  }

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
