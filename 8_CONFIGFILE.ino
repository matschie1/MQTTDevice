bool loadConfig() {
  DBG_PRINTLN("------ loadConfig started ------");
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    DBG_PRINTLN("Failed to open config file");
    DBG_PRINTLN("------ loadConfig aborted ------");

    return false;
  }
  else {
    DBG_PRINTLN("opened config file");
  }
  size_t size = configFile.size();
  if (size > 1024) {
    DBG_PRINT("Config file size is too large");
    DBG_PRINTLN("------ loadConfig aborted ------");
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
      DBG_PRINT("Actor ");
      DBG_PRINT(aname);
      DBG_PRINT(" SCRIPT ");
      DBG_PRINT(script);
      DBG_PRINT(" PIN ");
      DBG_PRINT(pin);
      DBG_PRINT(" inverted ");
      DBG_PRINTLN(ainverted);
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
      DBG_PRINT("Sensor ");
      DBG_PRINT(aname);
      DBG_PRINT(" SCRIPT ");
      DBG_PRINT(ascript);
      DBG_PRINT(" ADDRESS ");
      DBG_PRINTLN(aadress);
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


  String js_mqtttopic = jsinduction["TOPIC"];
  long delayoff = atol(jsinduction["DELAY"]);
  if (is_enabled_str == "1") {
    is_enabled_bl = true;
    DBG_PRINT("Induction ");
    DBG_PRINT(js_mqtttopic);
    DBG_PRINT(" WHITE ");
    DBG_PRINT(pin_white);
    DBG_PRINT(" YELLOW ");
    DBG_PRINT(pin_yellow);
    DBG_PRINT(" BLUE ");
    DBG_PRINT(pin_blue);
    DBG_PRINT(" Enabled ");
    DBG_PRINTLN(is_enabled_bl);
    DBG_PRINTLN("--------------------");
  }
  inductionCooker.change(StringToPin(pin_white), StringToPin(pin_yellow), StringToPin(pin_blue), js_mqtttopic, delayoff, is_enabled_bl);

  JsonArray& jsodisplay = json["display"];
  JsonObject& jsdisplay = jsodisplay[0];
  String dispAddress = jsdisplay["ADDRESS"];
  dispAddress.remove(0, 2);
  char copy[4];
  dispAddress.toCharArray(copy, 4);
  int address = strtol(copy, 0, 16);
  int newdup = atol(jsdisplay["updisp"]);
  if (newdup > 0) {
    DISP_UPDATE = newdup;
  }
  String dispStatus = jsdisplay["ENABLED"];
  if (dispStatus == "1") {
    oledDisplay.dispEnabled = 1;
    DBG_PRINT("Display address: ");
    DBG_PRINTLN(dispAddress);
    DBG_PRINT("Display update interval: ");
    DBG_PRINTLN(dispAddress);
  }
  else {
    oledDisplay.dispEnabled = 0;
    DBG_PRINTLN("Display disabled");
  }
  oledDisplay.change(address, oledDisplay.dispEnabled);
  DBG_PRINTLN("--------------------");

  // Misc Settings
  JsonArray& jsomisc = json["misc"];
  JsonObject& jsmisc = jsomisc[0];
  String str_act = jsmisc["enable_actors"];
  String str_act_del = jsmisc["delay_actors"];
  wait_on_error_actors = atol(jsmisc["delay_actors"]);
  DBG_PRINT("Switch off all actors on error: ");
  if (str_act == "1") {
    StopActorsOnError = true;
    DBG_PRINT(" enabled after ");
    DBG_PRINT(wait_on_error_induction);
    DBG_PRINTLN("ms");
  }
  else {
    StopActorsOnError = false;
    DBG_PRINTLN(" disabled");
  }

  String str_ind = jsmisc["enable_induction"];
  wait_on_error_induction = atol(jsmisc["delay_induction"]);
  DBG_PRINT("Switch off induction on error: ");
  if (str_ind == "1") {
    StopInductionOnError = true;
    DBG_PRINT(" enabled after ");
    DBG_PRINT(wait_on_error_induction);
    DBG_PRINTLN("ms");
  }
  else {
    StopInductionOnError = false;
    DBG_PRINTLN(" disabled ");
  }
  String str_debug = jsmisc["debug"];
  if (str_debug == "1") {
    setDEBUG = true;
  }
  //  else {   // moved to end of loadconfig
  //    DBG_PRINTLN("Debug output on serial monitor now disabled");
  //    setDEBUG = false;
  //  }
  String str_mdns = jsmisc["mdns"];
  String jsmisc_nameMDNS = jsmisc["mdns_name"];
  jsmisc_nameMDNS.toCharArray(nameMDNS, 16);
  if (str_mdns == "1") {
    startMDNS = true;
    DBG_PRINT("mDNS activated: ");
    DBG_PRINTLN(nameMDNS);
  }
  else {
    startMDNS = false;
    DBG_PRINTLN("mDNS disabled");
  }

  int newsup = atol(jsmisc["upsen"]);
  int newaup = atol(jsmisc["upact"]);
  int newiup = atol(jsmisc["upind"]);
  if (newsup > 0) {
    SEN_UPDATE = newsup;
  }
  if (newaup > 0) {
    ACT_UPDATE = newaup;
  }
  if (newiup > 0) {
    IND_UPDATE = newiup;
  }
  DBG_PRINT("Sensors update intervall: ");
  DBG_PRINTLN(SEN_UPDATE);
  DBG_PRINT("Actors update intervall: ");
  DBG_PRINTLN(ACT_UPDATE);
  DBG_PRINT("Induction update intervall: ");
  DBG_PRINTLN(IND_UPDATE);

  // General Settings
  String json_mqtthost = json["MQTTHOST"];
  json_mqtthost.toCharArray(mqtthost, 16);

  DBG_PRINT("MQTTHost: ");
  DBG_PRINTLN(mqtthost);

  if (str_debug == "0") {
    DBG_PRINTLN("Debug output on serial monitor now disabled");
    DBG_PRINTLN("------ loadConfig finished ------");
    setDEBUG = false;
  }

  DBG_PRINTLN("------ loadConfig finished ------");
  return true;
}

void saveConfigCallback () {
  DBG_PRINTLN("Should save config"); // was soll callback tun? Aufruf aus setup wifiManager.setSaveConfigCallback(saveConfigCallback)
}

bool saveConfig()
{
  DBG_PRINTLN("------ saveConfig started ------");
  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    DBG_PRINTLN("Failed to open config file for writing");
    DBG_PRINTLN("------ saveConfig aborted ------");
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
  DBG_PRINT("Number of Actors saved: ");
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
  DBG_PRINT("Number of Sensors saved: ");
  DBG_PRINTLN(numberOfSensors);
  DBG_PRINTLN("--------------------");

  // Write Induction
  JsonArray& jsinductions = json.createNestedArray("induction");
  JsonObject&  jsinduction = jsinductions.createNestedObject();
  jsinduction["PINWHITE"] = PinToString(inductionCooker.PIN_WHITE);
  jsinduction["PINYELLOW"] = PinToString(inductionCooker.PIN_YELLOW);
  jsinduction["PINBLUE"] = PinToString(inductionCooker.PIN_INTERRUPT);
  jsinduction["TOPIC"] = inductionCooker.mqtttopic;
  jsinduction["DELAY"] = inductionCooker.delayAfteroff;
  if (inductionCooker.isEnabled) {
    jsinduction["ENABLED"] = "1";
    DBG_PRINT("Induction ");
    DBG_PRINT(inductionCooker.mqtttopic);
    DBG_PRINT(" WHITE ");
    DBG_PRINT(PinToString(inductionCooker.PIN_WHITE));
    DBG_PRINT(" YELLOW ");
    DBG_PRINT(PinToString(inductionCooker.PIN_YELLOW));
    DBG_PRINT(" BLUE ");
    DBG_PRINT(PinToString(inductionCooker.PIN_INTERRUPT));
    DBG_PRINT(" Status ");
    DBG_PRINTLN(inductionCooker.isEnabled);
  } else {
    jsinduction["ENABLED"] = "0";
    DBG_PRINTLN("Induction status disabled");
  }
  DBG_PRINTLN("--------------------");

  // Write Display
  JsonArray& jsodisplay = json.createNestedArray("display");
  JsonObject&  jsdisplay = jsodisplay.createNestedObject();
  jsdisplay["ENABLED"] = "0";
  jsdisplay["ADDRESS"] = "0";
  jsdisplay["ADDRESS"] = String(decToHex(oledDisplay.address, 2));
  jsdisplay["updisp"] = DISP_UPDATE;

  if (oledDisplay.dispEnabled) {
    jsdisplay["ENABLED"] = "1";
    DBG_PRINT("Display address ");
    DBG_PRINTLN(String(decToHex(oledDisplay.address, 2)));
    DBG_PRINT("Display update interval ");
    DBG_PRINTLN(DISP_UPDATE);
  } else {
    jsdisplay["ENABLED"] = "0";
    DBG_PRINTLN("Display disabled");
  }
  DBG_PRINTLN("--------------------");

  // Write Misc Stuff
  JsonArray& jsomisc = json.createNestedArray("misc");
  JsonObject&  jsmisc = jsomisc.createNestedObject();
  jsmisc["delay_actors"] = wait_on_error_actors;
  if (StopActorsOnError) {
    jsmisc["enable_actors"] = "1";
    DBG_PRINT("Switch off actors on error: ");
    DBG_PRINT(StopActorsOnError);
    DBG_PRINT(" after: ");
    DBG_PRINT(wait_on_error_actors);
    DBG_PRINTLN("ms");
  } else {
    jsmisc["enable_actors"] = "0";
    DBG_PRINTLN("Switch off actors on error disabled");
  }

  jsmisc["delay_induction"] = wait_on_error_induction;
  if (StopInductionOnError) {
    jsmisc["enable_induction"] = "1";
    DBG_PRINT("Switch off induction on error: ");
    DBG_PRINT(StopInductionOnError);
    DBG_PRINT(" after: ");
    DBG_PRINT(wait_on_error_induction);
    DBG_PRINTLN("ms");
  } else {
    jsmisc["enable_induction"] = "0";
    DBG_PRINTLN("Switch off induction on error disabled");
  }

  if (setDEBUG) {
    jsmisc["debug"] = "1";
  } else {
    jsmisc["debug"] = "0";
  }
  jsmisc["mdns_name"] = nameMDNS;
  if (startMDNS) {
    jsmisc["mdns"] = "1";
    DBG_PRINT("mDNS enabled: ");
    DBG_PRINTLN(nameMDNS);
  } else {
    jsmisc["mdns"] = "0";
    DBG_PRINTLN("mDNS disabled");
  }

  jsmisc["upsen"] = SEN_UPDATE;
  jsmisc["upact"] = ACT_UPDATE;
  jsmisc["upind"] = IND_UPDATE;

  DBG_PRINT("Sensor update interval ");
  DBG_PRINTLN(SEN_UPDATE);
  DBG_PRINT("Actors update interval ");
  DBG_PRINTLN(ACT_UPDATE);
  DBG_PRINT("Induction update interval ");
  DBG_PRINTLN(IND_UPDATE);

  // Write General Stuff
  json["MQTTHOST"] = mqtthost;
  DBG_PRINT("MQTT broker IP: ");
  DBG_PRINTLN(mqtthost);
  json.printTo(configFile);
  DBG_PRINTLN("------ saveConfig finished ------");
  return true;
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

void DBG_PRINTLNTS(unsigned long value) // Timestamp
{
  value = value/1000;
  if (setDEBUG) {
    Serial.print((value/3600)%24); // Stunden
    Serial.print(":");
    Serial.print((value/60)%60); // Minuten
    Serial.print(":");
    Serial.println(value%60); // Sekunden
  }
}

String decToHex(byte decValue, byte desiredStringLength)
{
  String hexString = String(decValue, HEX);
  while (hexString.length() < desiredStringLength) hexString = "0" + hexString;

  return "0x" + hexString;
}
