void handleRoot() {
  server.sendHeader("Location", "/index.html", true);  //Redirect to our html web page
  server.send(302, "text/plain", "");
}

void handleWebRequests() {
  if (loadFromSpiffs(server.uri())) {
    return;
  }
  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}


bool loadFromSpiffs(String path) {
  String dataType = "text/plain";
  if (path.endsWith("/")) path += "index.html";

  if (path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".html")) dataType = "text/html";
  else if (path.endsWith(".htm")) dataType = "text/html";
  else if (path.endsWith(".css")) dataType = "text/css";
  else if (path.endsWith(".js")) dataType = "application/javascript";
  else if (path.endsWith(".png")) dataType = "image/png";
  else if (path.endsWith(".gif")) dataType = "image/gif";
  else if (path.endsWith(".jpg")) dataType = "image/jpeg";
  else if (path.endsWith(".ico")) dataType = "image/x-icon";
  else if (path.endsWith(".xml")) dataType = "text/xml";
  else if (path.endsWith(".pdf")) dataType = "application/pdf";
  else if (path.endsWith(".zip")) dataType = "application/zip";

  if (!SPIFFS.exists(path.c_str())) {
    return false;
  }
  File dataFile = SPIFFS.open(path.c_str(), "r");
  if (server.hasArg("download")) dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size()) {}
  dataFile.close();
  return true;
}

void mqttcallback(char* topic, byte* payload, unsigned int length) {

  DBG_PRINTLN("Received MQTT");
  DBG_PRINT("Topic: ");
  DBG_PRINTLN(topic);
  DBG_PRINT("Payload: ");
  for (int i = 0; i < length; i++) {
    DBG_PRINT((char)payload[i]);
  } DBG_PRINTLN(" ");
  char payload_msg[length];
  for (int i = 0; i < length; i++) {
    payload_msg[i] = payload[i];
  }

  if (inductionCooker.mqtttopic == topic) {
    DBG_PRINTLN("passing mqtt to induction");
    inductionCooker.handlemqtt(payload_msg);
  }
  for (int i = 0; i < numberOfActors; i++) {
    if (actors[i].argument_actor == topic) {
      DBG_PRINT("passing mqtt to actor ");
      DBG_PRINTLN(actors[i].name_actor);
      actors[i].handlemqtt(payload_msg);
    }
    yield();
  }
}

void handleRequestMiscSet() {
  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject& miscResponse = jsonBuffer.createObject();
  miscResponse["MQTTHOST"] = mqtthost;
  miscResponse["enable_actors"] = StopActorsOnError;
  miscResponse["enable_induction"] = StopInductionOnError;
  miscResponse["delay_actors"] = wait_on_error_actors / 1000;
  miscResponse["delay_induction"] = wait_on_error_induction / 1000;
  miscResponse["debug"] = setDEBUG;
  
  String response;
  miscResponse.printTo(response);
  server.send(200, "application/json", response);
}

void handleRequestMisc() {
  String request = server.arg(0);
  String message;

  if (request == "MQTTHOST") {
    message = mqtthost;
    goto SendMessage;
  }
  if (request == "mdns_name") {
    message = nameMDNS;
    goto SendMessage;
  }
  if (request == "mdns") {
    if (startMDNS) {
      message = "1";
    }
    else {
      message = "0";
    }
    goto SendMessage;
  }
  if (request == "enable_actors") {
    if (StopActorsOnError) {
      message = "1";
    }
    else {
      message = "0";
    }
    goto SendMessage;
  }
  if (request == "enable_induction") {
    if (StopInductionOnError) {
      message = "1";
    }
    else {
      message = "0";
    }
    goto SendMessage;
  }
  if (request == "delay_actors") {
    message = wait_on_error_actors / 1000;
    goto SendMessage;
  }
  if (request == "delay_induction") {
    message = wait_on_error_induction / 1000;
    goto SendMessage;
  }
  if (request == "debug") {
    if (setDEBUG) {
      message = "1";
    }
    else {
      message = "0";
    }
    goto SendMessage;
  }
  if (request == "upsen") {
    message = SEN_UPDATE / 1000;
    goto SendMessage;
  }
  if (request == "upact") {
    message = ACT_UPDATE / 1000;
    goto SendMessage;
  }
  if (request == "upind") {
    message = IND_UPDATE / 1000;
    goto SendMessage;
  }

SendMessage:
  server.send(200, "text/plain", message);
}

void handleSetMisc() {
  for (int i = 0; i < server.args(); i++) {
    if (server.argName(i) == "reset") {
      if (server.arg(i) == "1") {
        WiFi.disconnect();
        wifiManager.resetSettings();
        
        unsigned long last = millis();
        while (millis() < last + PAUSE2SEC)
        {
          // just wait for approx 2sec
        }
        ESP.reset();
      }
    }
    if (server.argName(i) == "clear") {
      if (server.arg(i) == "1") {
        SPIFFS.remove("/config.json");
        WiFi.disconnect();
        wifiManager.resetSettings();
        
        unsigned long last = millis();
        while (millis() < last + PAUSE2SEC)
        {
          // just wait for approx 2sec
        }
        ESP.reset();
      }
    }
    if (server.argName(i) == "MQTTHOST")  {
      server.arg(i).toCharArray(mqtthost, 16);
    }
    if (server.argName(i) == "mdns_name")  {
      server.arg(i).toCharArray(nameMDNS, 16);
    }
    if (server.argName(i) == "mdns")  {
      if (server.arg(i) == "1") {
        startMDNS = true;
      } else {
        startMDNS = false;
      }
    }
    if (server.argName(i) == "enable_actors")  {
      if (server.arg(i) == "1") {
        StopActorsOnError = true;
      } else {
        StopActorsOnError = false;
      }
    }
    if (server.argName(i) == "delay_actors")  {
      wait_on_error_actors = server.arg(i).toInt() * 1000;
    }
    if (server.argName(i) == "enable_induction")  {
      if (server.arg(i) == "1") {
        StopInductionOnError = true;
      } else {
        StopInductionOnError = false;
      }
    }
    if (server.argName(i) == "delay_induction")  {
      wait_on_error_induction = server.arg(i).toInt() * 1000;
    }
    if (server.argName(i) == "debug")  {
      if (server.arg(i) == "1") {
        setDEBUG = true;
      } else {
        setDEBUG = false;
      }
    }
    if (server.argName(i) == "upsen")  {
      int newsup = server.arg(i).toInt();
      if (newsup > 0 ) {
        SEN_UPDATE = newsup * 1000;
      }
    }
    if (server.argName(i) == "upact")  {
      int newaup = server.arg(i).toInt();
      if (newaup > 0 ) {
        ACT_UPDATE = newaup * 1000;
      }
    }
    if (server.argName(i) == "upind")  {
      int newiup = server.arg(i).toInt();
      if (newiup > 0 ) {
        IND_UPDATE = newiup * 1000;
      }
    }
    yield();
  }
  saveConfig();
}

// Some helper functions WebIf
void rebootDevice()
{
  cbpiEventSystem(11);
}

void turnMqttOff()
{
  cbpiEventSystem(10);
}

void OTA()
{
  cbpiEventSystem(9);
}
