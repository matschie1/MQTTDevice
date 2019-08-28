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
  
  StaticJsonBuffer<1400> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());
  if (!json.success()) {
    DBG_PRINTLN("JSON buffer unsuccessful: aborted");
    return false;
  }

  // JSON 6
  //StaticJsonDocument<1400> jsonDoc;
  //auto error = deserializeJson(jsonDoc, buf.get());
  
  
  JsonArray& jsonactors = json["actors"];
  
  // JSON 6
  // JsonArray jsonactors = jsonDoc["actors"];
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
      String ainverted = jsonactor["INV"];
      String aswitchable = jsonactor["SW"];
      actors[i].change(pin, script, aname, ainverted, aswitchable);
      DBG_PRINT("Actor ");
      DBG_PRINT(aname);
      DBG_PRINT(" SCRIPT ");
      DBG_PRINT(script);
      DBG_PRINT(" PIN ");
      DBG_PRINT(pin);
      DBG_PRINT(" inverted ");
      DBG_PRINT(ainverted);
      DBG_PRINT(" switchable ");
      DBG_PRINTLN(aswitchable);
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
      float aoffset = jsonsensor["OFFSET"];
      sensors[i].change(aadress, ascript, aname, aoffset);
      DBG_PRINT("Sensor ");
      DBG_PRINT(aname);
      DBG_PRINT(" Offset ");
      DBG_PRINT(aoffset);
      DBG_PRINT(" SCRIPT ");
      DBG_PRINT(ascript);
      DBG_PRINT(" ADDRESS ");
      DBG_PRINTLN(aadress);
    } else {
      sensors[i].change("", "", "", 0.0);
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
  inductionStatus = 0;

  String js_mqtttopic = jsinduction["TOPIC"];
  long delayoff = 0;
  if (is_enabled_str == "1") {
    delayoff = atol(jsinduction["DELAY"]);
    is_enabled_bl = true;
    inductionStatus = 1;
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

  //
  // Neu 20190426
  // Exception on atol function when no induction is configured
  //
  //if (useDisplay == true) {
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
      useDisplay = true;
      DBG_PRINT("OLED display enabled: ");
      DBG_PRINTLN(oledDisplay.dispEnabled);
      DBG_PRINT(" useDisplay Status: ");
      DBG_PRINTLN(useDisplay);
      DBG_PRINT("Display address: ");
      DBG_PRINTLN(dispAddress);
      DBG_PRINT("Display update interval: ");
      DBG_PRINTLN(dispAddress);
    }
    else {
      useDisplay = false;
      oledDisplay.dispEnabled = 0;
      DBG_PRINTLN("Display disabled");
      DBG_PRINT("OLED display enabled: ");
      DBG_PRINTLN(oledDisplay.dispEnabled);
      DBG_PRINT(" useDisplay Status: ");
      DBG_PRINTLN(useDisplay);
    }
    oledDisplay.change(address, oledDisplay.dispEnabled);

//  }
//  else
//  {
//    oledDisplay.dispEnabled = 0;
//    DBG_PRINTLN("Display disabled");
//    oledDisplay.change(0, oledDisplay.dispEnabled);
//  }
  
  DBG_PRINTLN("--------------------");


  // Misc Settings
     
  JsonArray& jsomisc = json["misc"];
  JsonObject& jsmisc = jsomisc[0];
  String str_act = jsmisc["enable_act"];
  String str_act_del = jsmisc["delay_act"];
  if (str_act_del.length() > 0)
  {
    wait_on_error_actors = str_act_del.toInt();
  }
  DBG_PRINT("Switch off actors on error: ");
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

  String str_ind = jsmisc["enable_ind"];
  String str_ind_del = jsmisc["delay_ind"];

  if (str_ind_del.length() > 0)
  {
    wait_on_error_induction = str_ind_del.toInt();
  }

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
    String str_upsen = jsmisc["upsen"];
    String str_upact = jsmisc["upact"];
    String str_upind = jsmisc["upind"];
    
    if (str_upsen.length() > 0) {
      SEN_UPDATE = str_upsen.toInt();
    }
    if (str_upact.length() > 0) {
      ACT_UPDATE = str_upact.toInt();
    }
    if (str_upind.length() > 0) {
      IND_UPDATE = str_upind.toInt();
    }
  DBG_PRINT("Sensors update intervall: ");
  DBG_PRINTLN(SEN_UPDATE);
  DBG_PRINT("Actors update intervall: ");
  DBG_PRINTLN(ACT_UPDATE);
  DBG_PRINT("Induction update intervall: ");
  DBG_PRINTLN(IND_UPDATE);

  String json_mqtthost = jsmisc["MQTTHOST"];

  if (json_mqtthost.length() > 6) {  //1.1.1.1
    json_mqtthost.toCharArray(mqtthost, 16);
    DBG_PRINT("MQTT server IP: ");
    DBG_PRINTLN(mqtthost);
  }
  else {
    DBG_PRINT("mqtthost not found in config file. Using default server address: ");
    DBG_PRINTLN(mqtthost);
  }
  
  if (str_debug == "0") {
    DBG_PRINTLN("Debug output on serial monitor now disabled");
    DBG_PRINTLN("------ loadConfig finished ------");
    setDEBUG = false;
  }

  DBG_PRINTLN("------ loadConfig finished ------");
  configFile.close();
  return true;
}

void saveConfigCallback () {
  DBG_PRINTLN("Should save config");
  // shouldSaveConfig = true;
}

bool saveConfig()
{
  DBG_PRINTLN("------ saveConfig started ------");
  StaticJsonBuffer<1400> jsonBuffer;
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
    jsactor["INV"] = actors[i].getInverted();
    jsactor["SW"] = actors[i].getSwitchable();
    DBG_PRINT("Actor ");
    DBG_PRINT(actors[i].name_actor);
    DBG_PRINT(" PIN ");
    DBG_PRINT(actors[i].pin_actor);
    DBG_PRINT(" Script ");
    DBG_PRINT(actors[i].argument_actor);
    DBG_PRINT(" inverted ");
    DBG_PRINT(actors[i].getInverted());
    DBG_PRINT(" switchable ");
    DBG_PRINT(actors[i].getSwitchable());
    DBG_PRINTLN(" saved");
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
    jssensor["OFFSET"] = sensors[i].sens_offset;
    jssensor["SCRIPT"] = sensors[i].sens_mqtttopic;
    DBG_PRINT("Sensor ");
    DBG_PRINT(sensors[i].sens_name);
    DBG_PRINT(" Offset ");
    DBG_PRINT(sensors[i].sens_offset);
    DBG_PRINT(" address ");
    DBG_PRINT(sensors[i].getSens_adress_string());
    DBG_PRINTLN(" saved");
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
    DBG_PRINT("OLED display enabled: ");
    DBG_PRINTLN(oledDisplay.dispEnabled);
    DBG_PRINT(" useDisplay Status: ");
    DBG_PRINTLN(useDisplay);
    DBG_PRINT("Display address ");
    DBG_PRINTLN(String(decToHex(oledDisplay.address, 2)));
    DBG_PRINT("Display update interval ");
    DBG_PRINTLN(DISP_UPDATE);
  } else {
    jsdisplay["ENABLED"] = "0";
    DBG_PRINTLN("Display disabled");
    DBG_PRINT("OLED display enabled: ");
    DBG_PRINTLN(oledDisplay.dispEnabled);
    DBG_PRINT(" useDisplay Status: ");
    DBG_PRINTLN(useDisplay);
  }
  DBG_PRINTLN("--------------------");

  // Write Misc Stuff
  JsonArray& jsomisc = json.createNestedArray("misc");
  JsonObject&  jsmisc = jsomisc.createNestedObject();
  jsmisc["delay_act"] = wait_on_error_actors;
  if (StopActorsOnError) {
    jsmisc["enable_act"] = "1";
    DBG_PRINT("Switch off actors on error: ");
    DBG_PRINT(StopActorsOnError);
    DBG_PRINT(" after: ");
    DBG_PRINT(wait_on_error_actors);
    DBG_PRINTLN("ms");
  } else {
    jsmisc["enable_act"] = "0";
    DBG_PRINTLN("Switch off actors on error disabled");
  }

  jsmisc["delay_ind"] = wait_on_error_induction;
  if (StopInductionOnError) {
    jsmisc["enable_ind"] = "1";
    DBG_PRINT("Switch off induction on error: ");
    DBG_PRINT(StopInductionOnError);
    DBG_PRINT(" after: ");
    DBG_PRINT(wait_on_error_induction);
    DBG_PRINTLN("ms");
  } else {
    jsmisc["enable_ind"] = "0";
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

  jsmisc["MQTTHOST"] = mqtthost;
  jsmisc["upsen"] = SEN_UPDATE;
  jsmisc["upact"] = ACT_UPDATE;
  jsmisc["upind"] = IND_UPDATE;
  

  DBG_PRINT("Sensor update interval ");
  DBG_PRINTLN(SEN_UPDATE);
  DBG_PRINT("Actors update interval ");
  DBG_PRINTLN(ACT_UPDATE);
  DBG_PRINT("Induction update interval ");
  DBG_PRINTLN(IND_UPDATE);
  DBG_PRINT("MQTT broker IP: ");
  DBG_PRINTLN(mqtthost);

  json.printTo(configFile);
  configFile.close();
  
  size_t len = json.measureLength();
  DBG_PRINT("JSON config length: ");
  DBG_PRINTLN(len);
  if (len > 1500) {
    DBG_PRINTLN("Error: JSON config too big!");
    DBG_PRINTLN("Error: JSON config coud not be saved completely to SPIFFS!");
    DBG_PRINTLN("Try short names for all sensors and actors, eg. s1, a1 etc.");
  }
  
  DBG_PRINTLN("------ saveConfig finished ------");
  return true;
}
