bool loadConfig()
{
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile)
  {
    Serial.println("Failed to open config file");
    return false;
  }
  else
  {
    Serial.println("opened config file");
  }

  size_t size = configFile.size();
  if (size > 1024)
  {
    Serial.println("Config file size is too large");
    return false;
  }

  std::unique_ptr<char[]> buf(new char[size]);

  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject &json = jsonBuffer.parseObject(buf.get());

  if (!json.success())
  {
    return false;
  }

  //Read Actors
  JsonArray &jsonactors = json["actors"];
  numberOfActors = jsonactors.size();
  Serial.print("Number of actors loaded: ");
  Serial.println(numberOfActors);

  if (numberOfActors > numberOfActorsMax)
  {
    numberOfActors = numberOfActorsMax;
  }

  for (int i = 0; i < numberOfActors; i++)
  {
    if (i < numberOfActors)
    {
      JsonObject &jsonactor = jsonactors[i];
      String pin = jsonactor["PIN"];
      String topic = jsonactor["TOPIC"];
      String name = jsonactor["NAME"];
      String inverted = jsonactor["INVERTED"];
      actors[i].change(pin, topic, name, inverted);
    }
  }

  // Read OneWire Sensors
  JsonArray &jsonOneWireSensors = json["OneWireSensors"];
  numberOfOneWireSensors = jsonOneWireSensors.size();
  if (numberOfOneWireSensors > numberOfSensorsMax)
  {
    numberOfOneWireSensors = numberOfSensorsMax;
  }
  Serial.print("Number of OneWire sensors loaded: ");
  Serial.println(numberOfOneWireSensors);

  for (int i = 0; i < numberOfSensorsMax; i++)
  {
    if (i < numberOfOneWireSensors)
    {
      JsonObject &jsonsensor = jsonOneWireSensors[i];
      String address = jsonsensor["ADDRESS"];
      String topic = jsonsensor["TOPIC"];
      String name = jsonsensor["NAME"];
      oneWireSensors[i].change(address, topic, name);
    }
    else
    {
      oneWireSensors[i].change("", "", "");
    }
  }

  // Read PT100/1000 sensors
  JsonArray &jsonPTSensors = json["PTSensors"];
  numberOfPTSensors = jsonPTSensors.size();
  if (numberOfPTSensors > numberOfSensorsMax)
  {
    numberOfPTSensors = numberOfSensorsMax;
  }
  Serial.print("Number of PT100/1000 sensors loaded: ");
  Serial.println(numberOfPTSensors);

  for (int i = 0; i < numberOfSensorsMax; i++)
  {
    if (i < numberOfPTSensors)
    {
      JsonObject &jsonPTSensor = jsonPTSensors[i];
      byte csPin = jsonPTSensor["CSPIN"];
      byte numberOfWires = jsonPTSensor["WIRES"];
      String topic = jsonPTSensor["TOPIC"];
      String name = jsonPTSensor["NAME"];
      ptSensors[i].change(csPin, numberOfWires, topic, name);
    }
    else
    {
      ptSensors[i].change(NO_PT_SENSOR, NO_PT_SENSOR, "", "");
    }
  }

  // Read Induction
  JsonArray &jsinductions = json["induction"];
  JsonObject &jsinduction = jsinductions[0];
  String pin_white = jsinduction["PINWHITE"];
  String pin_yellow = jsinduction["PINYELLOW"];
  String pin_blue = jsinduction["PINBLUE"];
  String is_enabled_str = jsinduction["ENABLED"];
  bool is_enabled_bl = false;
  if (is_enabled_str == "1")
  {
    is_enabled_bl = true;
  }

  String js_mqtttopic = jsinduction["TOPIC"];
  long delayoff = atol(jsinduction["DELAY"]);

  inductionCooker.change(StringToPin(pin_white), StringToPin(pin_yellow), StringToPin(pin_blue), js_mqtttopic, delayoff, is_enabled_bl);

  // General Settings
  String json_mqtthost = json["MQTTHOST"];
  json_mqtthost.toCharArray(mqtthost, 16);

  return true;
}

bool saveConfig()
{

  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject &json = jsonBuffer.createObject();

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile)
  {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  // Write Actors
  JsonArray &jsactors = json.createNestedArray("actors");
  for (int i = 0; i < numberOfActors; i++)
  {
    JsonObject &jsactor = jsactors.createNestedObject();
    jsactor["PIN"] = PinToString(actors[i].pin_actor);
    jsactor["NAME"] = actors[i].name_actor;
    jsactor["TOPIC"] = actors[i].argument_actor;
    jsactor["INVERTED"] = actors[i].getInverted();
  }

  // Write OneWire sensors
  JsonArray &jsonOneWireSensors = json.createNestedArray("OneWireSensors");
  for (int i = 0; i < numberOfOneWireSensors; i++)
  {
    JsonObject &jsonOneWireSensor = jsonOneWireSensors.createNestedObject();
    jsonOneWireSensor["ADDRESS"] = oneWireSensors[i].getSens_address_string();
    jsonOneWireSensor["NAME"] = oneWireSensors[i].sens_name;
    jsonOneWireSensor["TOPIC"] = oneWireSensors[i].sens_mqtttopic;
  }

  // Write PT100/1000 sensors
  JsonArray &jsonPTSensors = json.createNestedArray("PTSensors");
  for (int i = 0; i < numberOfOneWireSensors; i++)
  {
    JsonObject &jsonPTSensor = jsonPTSensors.createNestedObject();
    jsonPTSensor["CSPIN"] = ptSensors[i].csPin;
    jsonPTSensor["WIRES"] = ptSensors[i].numberOfWires;
    jsonPTSensor["NAME"] = ptSensors[i].name;
    jsonPTSensor["TOPIC"] = ptSensors[i].mqttTopic;
  }

  // Write Induction
  JsonArray &jsinductions = json.createNestedArray("induction");
  JsonObject &jsinduction = jsinductions.createNestedObject();
  if (inductionCooker.isEnabled)
  {
    jsinduction["ENABLED"] = "1";
  }
  else
  {
    jsinduction["ENABLED"] = "0";
  }
  jsinduction["PINWHITE"] = PinToString(inductionCooker.PIN_WHITE);
  jsinduction["PINYELLOW"] = PinToString(inductionCooker.PIN_YELLOW);
  jsinduction["PINBLUE"] = PinToString(inductionCooker.PIN_INTERRUPT);
  jsinduction["TOPIC"] = inductionCooker.mqtttopic;
  jsinduction["DELAY"] = inductionCooker.delayAfteroff;

  // Write General Stuff
  json["MQTTHOST"] = mqtthost;
  json.printTo(configFile);
  return true;
}

/* Needed for the WifiManager */
void saveConfigCallback()
{
  Serial.println("Should save config");
}
