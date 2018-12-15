bool loadConfig() {
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  } else { Serial.println("opened config file"); }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  std::unique_ptr<char[]> buf(new char[size]);

  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    return false;
  } 

  //Read Actors  
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
     actors[i].change(pin,script,aname,ainverted);     
   } 
   yield();
  }

  // Read Sensors
  JsonArray& jsonsensors = json["sensors"];
  numberOfSensors = jsonsensors.size();
  Serial.print("Number of Sensors loaded: ");
  Serial.println(numberOfSensors);

  if (numberOfSensors > 10) { 
    numberOfSensors = 10; 
  }

  for (int i = 0; i < 10; i++) {
    if (i < numberOfSensors) {
     JsonObject& jsonsensor = jsonsensors[i];    
     String aadress = jsonsensor["ADDRESS"];
     String ascript = jsonsensor["SCRIPT"];
     String aname = jsonsensor["NAME"];
     sensors[i].change(aadress,ascript,aname); 
    } else {
      sensors[i].change("","","");
    }   
  }

  // Read Induction
  JsonArray& jsinductions = json["induction"];
  JsonObject& jsinduction = jsinductions[0]; 
  String pin_white = jsinduction["PINWHITE"];
  String pin_yellow = jsinduction["PINYELLOW"];
  String pin_blue = jsinduction["PINBLUE"];
  String is_enabled_str = jsinduction["ENABLED"];
  bool is_enabled_bl = false;
    if (is_enabled_str == "1") { is_enabled_bl = true; }

  String mqtttopic = jsinduction["TOPIC"];
  long delayoff = atol(jsinduction["DELAY"]);
 
  inductionCooker.change(StringToPin(pin_white),StringToPin(pin_yellow),StringToPin(pin_blue),mqtttopic,delayoff,is_enabled_bl);  
  
  yield();
return true;
}

void saveConfigCallback () {
  Serial.println("Should save config");
}


bool saveConfig() {
  
  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
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
    yield();
  }
  
  // Write Sensors
  JsonArray& jssensors = json.createNestedArray("sensors");
  for (int i = 0; i < numberOfSensors; i++) {
    JsonObject& jssensor = jssensors.createNestedObject();
    jssensor["ADDRESS"] = sensors[i].getSens_adress_string();
    jssensor["NAME"] = sensors[i].sens_name;
    jssensor["SCRIPT"] = sensors[i].sens_mqtttopic;
    yield();
  }

  // Write Induction
  JsonArray& jsinductions = json.createNestedArray("induction");
  JsonObject&  jsinduction = jsinductions.createNestedObject();
    if (inductionCooker.isEnabled) { jsinduction["ENABLED"] = "1"; } else { jsinduction["ENABLED"] = "0"; }
    jsinduction["PINWHITE"] = PinToString(inductionCooker.PIN_WHITE); 
    jsinduction["PINYELLOW"] = PinToString(inductionCooker.PIN_YELLOW); 
    jsinduction["PINBLUE"] = PinToString(inductionCooker.PIN_INTERRUPT); 
    jsinduction["TOPIC"] = inductionCooker.mqtttopic; 
    jsinduction["DELAY"] = inductionCooker.delayAfteroff; 
    
  json.printTo(configFile);
  return true;
}
