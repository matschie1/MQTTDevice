bool loadConfig() {

  DBG_PRINTLN("------ loadConfig started ------");
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    DBG_PRINTLN("Failed to open config file");
    DBG_PRINTLN("------ loadConfig aborted ------");
    return false;
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
      DBG_PRINT("Actor name: ");
      DBG_PRINTLN(aname);
      DBG_PRINT("Actor MQTT topic: ");
      DBG_PRINTLN(script);
      DBG_PRINT("Actor PIN: ");
      DBG_PRINTLN(pin);
      DBG_PRINT("Actor inverted: ");
      DBG_PRINTLN(ainverted);
      DBG_PRINT("Actor switchable: ");
      DBG_PRINTLN(aswitchable);
      DBG_PRINT("Actor no. ");
      DBG_PRINT(i + 1);
      DBG_PRINTLN(" loaded from config file");
    }
  }
  if (numberOfActors > 0) {
    DBG_PRINT("Total number of actors loaded: ");
    DBG_PRINTLN(numberOfActors);
  }
  else
    DBG_PRINTLN("No actors loaded from config file");
  DBG_PRINTLN("--------------------");

  JsonArray& jsonsensors = json["sensors"];
  numberOfSensors = jsonsensors.size();

  if (numberOfSensors > 10) {
    numberOfSensors = 10;
  }
  for (int i = 0; i < 10; i++) {
    if (i < numberOfSensors) {
      JsonObject& jsonsensor = jsonsensors[i];
      String aadress = jsonsensor["ADDRESS"];
      String ascript = jsonsensor["SCRIPT"];
      String aname = jsonsensor["NAME"];
      float aoffset = 0.0;
      bool asw = false;
      if ( (jsonsensor.containsKey("OFFSET")) && (jsonsensor["OFFSET"].is<float>()) )
        aoffset = jsonsensor["OFFSET"];
      if ( jsonsensor.containsKey("SW") )
        asw = jsonsensor["SW"];

      sensors[i].change(aadress, ascript, aname, aoffset, asw);
      DBG_PRINT("Sensor name: ");
      DBG_PRINTLN(aname);
      DBG_PRINT("Sensor address: ");
      DBG_PRINTLN(aadress);
      DBG_PRINT("Sensor MQTT topic: ");
      DBG_PRINTLN(ascript);
      DBG_PRINT("Sensor offset: ");
      DBG_PRINTLN(aoffset);
      DBG_PRINT("Sensor switchable: ");
      DBG_PRINTLN(asw);
      DBG_PRINT("Sensor no. ");
      DBG_PRINT(i + 1);
      DBG_PRINTLN(" loaded from config file");
    }
    else
      sensors[i].change("", "", "", 0.0, false);
  }

  if (numberOfSensors > 0) {
    DBG_PRINT("Total number of sensors loaded: ");
    DBG_PRINTLN(numberOfSensors);
  }
  else
    DBG_PRINTLN("No sensors loaded from config file");

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
  long delayoff = DEF_DELAY_IND; //default delay
  int pl = 100;

  if ( (jsinduction.containsKey("PL")) && (jsinduction["PL"].is<int>()) )
    pl = jsinduction["PL"];

  if ( (jsinduction.containsKey("DELAY")) && (jsinduction["DELAY"].is<long>()) )
    delayoff = jsinduction["DELAY"];

  DBG_PRINT("Induction status: ");
  if (is_enabled_str == "1") {
    is_enabled_bl = true;
    inductionStatus = 1;
    DBG_PRINTLN(inductionStatus);
    //DBG_PRINTLN(is_enabled_bl);
    DBG_PRINT("Induction MQTT topic: ");
    DBG_PRINTLN(js_mqtttopic);
    DBG_PRINT("Induction relais (WHITE): ");
    DBG_PRINTLN(pin_white);
    DBG_PRINT("Induction command channel (YELLOW): ");
    DBG_PRINTLN(pin_yellow);
    DBG_PRINT("Induction backchannel (BLUE): ");
    DBG_PRINTLN(pin_blue);
    DBG_PRINT("Induction delay after power off: ");
    DBG_PRINT(delayoff / 1000);
    DBG_PRINTLN("sec");
    DBG_PRINT("Induction power level on error: ");
    DBG_PRINTLN(pl);
  }
  else
    DBG_PRINTLN(inductionStatus);

  DBG_PRINTLN("--------------------");
  inductionCooker.change(StringToPin(pin_white), StringToPin(pin_yellow), StringToPin(pin_blue), js_mqtttopic, delayoff, is_enabled_bl, pl);

  JsonArray& jsodisplay = json["display"];
  JsonObject& jsdisplay = jsodisplay[0];
  String dispAddress = jsdisplay["ADDRESS"];
  dispAddress.remove(0, 2);
  char copy[4];
  dispAddress.toCharArray(copy, 4);
  int address = strtol(copy, 0, 16);
  if ( (jsdisplay.containsKey("updisp")) && (jsdisplay["updisp"].is<int>()) ) // else use default
    DISP_UPDATE = jsdisplay["updisp"];

  if (jsdisplay.containsKey("ENABLED"))
    useDisplay = jsdisplay["ENABLED"];

  if (useDisplay) {
    oledDisplay.dispEnabled = 1;
    DBG_PRINT("OLED display status: ");
    DBG_PRINTLN(oledDisplay.dispEnabled);
    DBG_PRINT("Display address: ");
    DBG_PRINTLN(dispAddress);
    DBG_PRINT("Display update interval: ");
    DBG_PRINT(DISP_UPDATE / 1000);
    DBG_PRINTLN("sec");
  }
  else
  {
    useDisplay = false;
    oledDisplay.dispEnabled = 0;
    DBG_PRINTLN(oledDisplay.dispEnabled);
  }
  oledDisplay.change(address, oledDisplay.dispEnabled);
  DBG_PRINTLN("--------------------");

  // Misc Settings
  JsonArray& jsomisc = json["misc"];
  JsonObject& jsmisc = jsomisc[0];

  if ( (jsmisc.containsKey("del_sen_act"))  && (jsmisc["del_sen_act"].is<int>()) )
    wait_on_Sensor_error_actor = jsmisc["del_sen_act"];

  if ( (jsmisc.containsKey("del_sen_ind"))  && (jsmisc["del_sen_ind"].is<int>()) )
    wait_on_Sensor_error_induction = jsmisc["del_sen_ind"];


  if ( (jsmisc.containsKey("delay_mqtt"))  && (jsmisc["delay_mqtt"].is<int>()) )
    wait_on_error_mqtt = jsmisc["delay_mqtt"];

  DBG_PRINT("Wait on sensor error actors: ");
  DBG_PRINTLN(wait_on_Sensor_error_actor);
  DBG_PRINT("Wait on sensor error induction: ");
  DBG_PRINTLN(wait_on_Sensor_error_induction);

  DBG_PRINT("Switch off actors on error ");
  if (jsmisc.containsKey("enable_mqtt"))
    StopOnMQTTError = jsmisc["enable_mqtt"];

  if (StopOnMQTTError == 1) {
    DBG_PRINT("enabled after ");
    DBG_PRINT(wait_on_error_mqtt / 1000);
    DBG_PRINTLN("sec");
  }
  else
    DBG_PRINTLN("disabled");

  DBG_PRINT("Switch off induction on error ");
  if (jsmisc.containsKey("enable_wlan"))
    StopOnWLANError = jsmisc["enable_wlan"];

  if ( (jsmisc.containsKey("delay_wlan")) && (jsmisc["delay_wlan"].is<int>()) )
    wait_on_error_wlan = jsmisc["delay_wlan"];

  if (StopOnWLANError == 1) {
    DBG_PRINT("enabled after ");
    DBG_PRINT(wait_on_error_wlan / 1000);
    DBG_PRINTLN("sec");
  }
  else
    DBG_PRINTLN("disabled");


  if (jsmisc.containsKey("mdns")) {
    startMDNS = jsmisc["mdns"];
  }
  String jsmisc_nameMDNS = jsmisc["mdns_name"];
  jsmisc_nameMDNS.toCharArray(nameMDNS, 16);
  if (startMDNS) {
    DBG_PRINT("mDNS activated: ");
    DBG_PRINTLN(nameMDNS);
  }
  else
    DBG_PRINTLN("mDNS disabled");

  if ( (jsmisc.containsKey("upsen")) && (jsmisc["upsen"].is<int>()) )
    SEN_UPDATE = jsmisc["upsen"];
  if ( (jsmisc.containsKey("upact")) && (jsmisc["upact"].is<int>()) )
    ACT_UPDATE = jsmisc["upact"];
  if ( (jsmisc.containsKey("upind")) && (jsmisc["upind"].is<int>()) )
    IND_UPDATE = jsmisc["upind"];

  DBG_PRINT("Sensors update intervall: ");
  DBG_PRINT(SEN_UPDATE / 1000);
  DBG_PRINTLN("sec");
  DBG_PRINT("Actors update intervall: ");
  DBG_PRINT(ACT_UPDATE / 1000);
  DBG_PRINTLN("sec");
  DBG_PRINT("Induction update intervall: ");
  DBG_PRINT(IND_UPDATE / 1000);
  DBG_PRINTLN("sec");

  if (jsmisc.containsKey("MQTTHOST")) {
    String json_mqtthost = jsmisc["MQTTHOST"];
    json_mqtthost.toCharArray(mqtthost, 16);
    DBG_PRINT("MQTT server IP: ");
    DBG_PRINTLN(mqtthost);
  }
  else {
    DBG_PRINT("MQTT server not found in config file. Using default server address: ");
    DBG_PRINTLN(mqtthost);
  }

  if (jsmisc.containsKey("telnet"))
    startTEL = jsmisc["telnet"];
  if (jsmisc.containsKey("debug"))
    setDEBUG = jsmisc["debug"];

  DBG_PRINT("Debug output on serial monitor: ");
  DBG_PRINTLN(setDEBUG);
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
    DBG_PRINT("Actor name: ");
    DBG_PRINTLN(actors[i].name_actor);
    DBG_PRINT("Actor PIN: ");
    DBG_PRINTLN(actors[i].pin_actor);
    DBG_PRINT("Actor MQTT topic: ");
    DBG_PRINTLN(actors[i].argument_actor);
    DBG_PRINT("Actor inverted: ");
    DBG_PRINTLN(actors[i].getInverted());
    DBG_PRINT("Actor switchable: ");
    DBG_PRINTLN(actors[i].getSwitchable());
    DBG_PRINT("Actor no. ");
    DBG_PRINT(i);
    DBG_PRINTLN(" saved to config file");
  }
  if (numberOfActors > 0) {
    DBG_PRINT("Total number of actors saved: ");
    DBG_PRINTLN(numberOfActors);
  }
  else
    DBG_PRINTLN("No actors saved to config file");

  DBG_PRINTLN("--------------------");

  // Write Sensors
  JsonArray& jssensors = json.createNestedArray("sensors");
  for (int i = 0; i < numberOfSensors; i++) {
    JsonObject& jssensor = jssensors.createNestedObject();
    jssensor["ADDRESS"] = sensors[i].getSens_adress_string();
    jssensor["NAME"] = sensors[i].sens_name;
    jssensor["OFFSET"] = sensors[i].sens_offset;
    jssensor["SCRIPT"] = sensors[i].sens_mqtttopic;
    jssensor["SW"] = sensors[i].sens_sw;
    DBG_PRINT("Sensor Name: ");
    DBG_PRINTLN(sensors[i].sens_name);
    DBG_PRINT("Sensor address: ");
    DBG_PRINTLN(sensors[i].getSens_adress_string());
    DBG_PRINT("Sensor MQTT topic: ");
    DBG_PRINTLN(sensors[i].sens_mqtttopic);
    DBG_PRINT("Sensor offset: ");
    DBG_PRINTLN(sensors[i].sens_offset);
    DBG_PRINT("Sensor switchable: ");
    DBG_PRINTLN(sensors[i].sens_sw);
    DBG_PRINT("Sensor no. ");
    DBG_PRINT(i + 1);
    DBG_PRINTLN(" saved to config file");
  }
  if (numberOfSensors > 0) {
    DBG_PRINT("Total number of sensors saved: ");
    DBG_PRINTLN(numberOfSensors);
  }
  else
    DBG_PRINTLN("No sensors saved to config file");

  DBG_PRINTLN("--------------------");

  // Write Induction
  JsonArray& jsinductions = json.createNestedArray("induction");
  JsonObject&  jsinduction = jsinductions.createNestedObject();
  jsinduction["PINWHITE"] = PinToString(inductionCooker.PIN_WHITE);
  jsinduction["PINYELLOW"] = PinToString(inductionCooker.PIN_YELLOW);
  jsinduction["PINBLUE"] = PinToString(inductionCooker.PIN_INTERRUPT);
  jsinduction["TOPIC"] = inductionCooker.mqtttopic;
  jsinduction["DELAY"] = inductionCooker.delayAfteroff;
  jsinduction["ENABLED"] = "0";
  jsinduction["PL"] = inductionCooker.powerLevelOnError;
  DBG_PRINT("Induction status: ");
  if (inductionCooker.isEnabled) {
    jsinduction["ENABLED"] = "1";
    DBG_PRINTLN(inductionCooker.isEnabled);
    DBG_PRINT("Induction MQTT topic: ");
    DBG_PRINTLN(inductionCooker.mqtttopic);
    DBG_PRINT("Induction relais (WHITE): ");
    DBG_PRINTLN(PinToString(inductionCooker.PIN_WHITE));
    DBG_PRINT("Induction command channel (YELLOW): ");
    DBG_PRINTLN(PinToString(inductionCooker.PIN_YELLOW));
    DBG_PRINT("Induction backchannel (BLUE): ");
    DBG_PRINTLN(PinToString(inductionCooker.PIN_INTERRUPT));
    DBG_PRINT("Induction delay after power off: ");
    DBG_PRINTLN(inductionCooker.delayAfteroff / 1000);
    DBG_PRINT("Induction power level on error: ");
    DBG_PRINTLN(inductionCooker.powerLevelOnError);
  }
  else
    DBG_PRINTLN(inductionCooker.isEnabled);

  DBG_PRINTLN("--------------------");

  // Write Display
  JsonArray& jsodisplay = json.createNestedArray("display");
  JsonObject&  jsdisplay = jsodisplay.createNestedObject();
  jsdisplay["ENABLED"] = "0";
  jsdisplay["ADDRESS"] = "0";
  jsdisplay["ADDRESS"] = String(decToHex(oledDisplay.address, 2));
  jsdisplay["updisp"] = DISP_UPDATE;
  jsdisplay["ENABLED"] = "0";
  DBG_PRINT("OLED display status: ");
  DBG_PRINTLN(oledDisplay.dispEnabled);
  if (oledDisplay.dispEnabled) {
    jsdisplay["ENABLED"] = "1";
    DBG_PRINT("Display address ");
    DBG_PRINTLN(String(decToHex(oledDisplay.address, 2)));
    DBG_PRINT("Display update interval ");
    DBG_PRINT(DISP_UPDATE / 1000);
    DBG_PRINTLN("sec");
  }

  DBG_PRINTLN("--------------------");

  // Write Misc Stuff
  JsonArray& jsomisc = json.createNestedArray("misc");
  JsonObject&  jsmisc = jsomisc.createNestedObject();

  jsmisc["del_sen_act"] = wait_on_Sensor_error_actor;
  jsmisc["del_sen_ind"] = wait_on_Sensor_error_induction;
  DBG_PRINT("Wait on sensor error actors: ");
  DBG_PRINTLN(wait_on_Sensor_error_actor);
  DBG_PRINT("Wait on sensor error induction: ");
  DBG_PRINTLN(wait_on_Sensor_error_induction);
  jsmisc["delay_mqtt"] = wait_on_error_mqtt;
  jsmisc["enable_mqtt"] = StopOnMQTTError;
  DBG_PRINT("Switch off actors on error ");
  if (StopOnMQTTError) {
    jsmisc["enable_mqtt"] = "1";
    DBG_PRINT("enabled after ");
    DBG_PRINT(wait_on_error_mqtt / 1000);
    DBG_PRINTLN("sec");
  }
  else
    DBG_PRINTLN("disabled");

  jsmisc["delay_wlan"] = wait_on_error_wlan;
  jsmisc["enable_wlan"] = StopOnWLANError;
  DBG_PRINT("Switch off induction on error ");

  if (StopOnWLANError) {
    jsmisc["enable_wlan"] = "1";
    DBG_PRINT("enabled after ");
    DBG_PRINT(wait_on_error_wlan / 1000);
    DBG_PRINTLN("sec");
  }
  else
    DBG_PRINTLN("disabled");

  if (startTEL)
    jsmisc["telnet"] = "1";
  else
    jsmisc["telnet"] = "0";

  if (setDEBUG)
    jsmisc["debug"] = "1";
  else
    jsmisc["debug"] = "0";

  jsmisc["mdns_name"] = nameMDNS;
  jsmisc["mdns"] = "0";
  if (startMDNS) {
    jsmisc["mdns"] = "1";
    DBG_PRINT("mDNS enabled: ");
    DBG_PRINTLN(nameMDNS);
  }
  else
    DBG_PRINTLN("mDNS disabled");

  jsmisc["MQTTHOST"] = mqtthost;
  jsmisc["upsen"] = SEN_UPDATE;
  jsmisc["upact"] = ACT_UPDATE;
  jsmisc["upind"] = IND_UPDATE;

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

  json.printTo(configFile);
  configFile.close();

  size_t len = json.measureLength();
  DBG_PRINT("JSON config length: ");
  DBG_PRINTLN(len);
  if (len > 1500)
    DBG_PRINTLN("Error: JSON config too big!");

  DBG_PRINTLN("------ saveConfig finished ------");
  IPAddress ip = WiFi.localIP();
  String Network = WiFi.SSID();
  DBG_PRINT("ESP8266 device IP Address: ");
  DBG_PRINTLN(ip.toString());
  DBG_PRINT("Configured WLAN SSID: ");
  DBG_PRINTLN(Network);
  DBG_PRINTLN("---------------------------------");
  return true;
}
