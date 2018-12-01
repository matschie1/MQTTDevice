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

  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    return false;
  } 

  JsonArray& jsonactors = json["actors"];

  numberOfActors = jsonactors.size();
  
  if (numberOfActors > 6) { 
    numberOfActors = 6; 
  }
  
  // Read Actors  
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
    yield();
return true;
}

void saveConfigCallback () {
  Serial.println("Should save config");
}


bool saveConfig() {
  
  StaticJsonBuffer<512> jsonBuffer;
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
    jsactor["PIN"] = actors[i].getPinStr();
    jsactor["NAME"] = actors[i].name_actor;
    jsactor["SCRIPT"] = actors[i].argument_actor;
    jsactor["INVERTED"] = actors[i].getInverted();
    yield();
  }

  JsonArray& jssensors = json.createNestedArray("sensors");
  for (int i = 0; i < numberOfSensors; i++) {
    JsonObject& jssensor = jssensors.createNestedObject();
    jssensor["ADDRESS"] = sensors[i].getSens_adress_string();
    jssensor["NAME"] = sensors[i].sens_name;
    jssensor["SCRIPT"] = sensors[i].sens_mqtttopic;
    yield();
  }
   
  json.printTo(configFile);
  return true;
}
