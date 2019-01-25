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

void mqttreconnect() {
  // 10 Tries for reconnect
  // Wenn Client nicht verbunden, Verbindung herstellen

  // Delay prüfen - mqttreconnect hängt wenn es keine subscribes gibt
  if (!client.connected()) {
    if (millis() > mqttconnectlasttry + MQTT_DELAY) {
      //DBG_PRINT("MQTT Trying to connect. Device name: ");
      //DBG_PRINT(mqtt_clientid);
      for (int i = 0; i < MQTT_NUM_TRY; i++) {
        //DBG_PRINT(".. Try #");
        //DBG_PRINTLN(i + 1);
        if (client.connect(mqtt_clientid)) {
          DBG_PRINT("MQTT connect successful. Subscribing.");
          goto Subscribe;
        }
        unsigned long pause = millis();
        while (millis() < pause + 1000) {
          //wait approx. 1 sec
        }
      }

      // Event MQTT
      cbpiEventSystem(2);
      mqttconnectlasttry = millis();
      DBG_PRINTLN("");
      DBG_PRINT("MQTT connect failed. Try again in ");
      DBG_PRINT(MQTT_DELAY / 1000);
      DBG_PRINTLN(" seconds");
      return;
    }
  }

Subscribe:
  for (int i = 0; i < numberOfActors; i++) {
    actors[i].mqtt_subscribe();
    yield();
  }
  inductionCooker.mqtt_subscribe();
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
  server.send(200, "application/json", "");
}

void handleRequestMisc() {
  String request = server.arg(0);
  String message;
  if (request == "MQTTHOST") {
    message = mqtthost;
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
        unsigned long last = 0;
        if (millis() > last + 2000)
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
        unsigned long last = 0;
        if (millis() > last + 2000)
        {
          // just wait for approx 2sec
        }
        ESP.reset();
      }
    }
    if (server.argName(i) == "MQTTHOST")  {
      server.arg(i).toCharArray(mqtthost, 16);
    }
    yield();
  }
  saveConfig();
}
