void handleRoot()
{
  server.sendHeader("Location", "/index.html", true); //Redirect to our html web page
  server.send(302, "text/plain", "");
}

void handleWebRequests()
{
  if (loadFromSpiffs(server.uri()))
  {
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
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

bool loadFromSpiffs(String path)
{
  String dataType = "text/plain";
  if (path.endsWith("/"))
    path += "index.html";

  if (path.endsWith(".src"))
    path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".html"))
    dataType = "text/html";
  else if (path.endsWith(".htm"))
    dataType = "text/html";
  else if (path.endsWith(".css"))
    dataType = "text/css";
  else if (path.endsWith(".js"))
    dataType = "application/javascript";
  else if (path.endsWith(".png"))
    dataType = "image/png";
  else if (path.endsWith(".gif"))
    dataType = "image/gif";
  else if (path.endsWith(".jpg"))
    dataType = "image/jpeg";
  else if (path.endsWith(".ico"))
    dataType = "image/x-icon";
  else if (path.endsWith(".xml"))
    dataType = "text/xml";
  else if (path.endsWith(".pdf"))
    dataType = "application/pdf";
  else if (path.endsWith(".zip"))
    dataType = "application/zip";

  if (!SPIFFS.exists(path.c_str()))
  {
    return false;
  }
  File dataFile = SPIFFS.open(path.c_str(), "r");
  if (server.hasArg("download"))
    dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size())
  {
  }
  dataFile.close();
  return true;
}

void mqttcallback(char *topic, unsigned char *payload, unsigned int length)
{
  DBG_PRINT("Web: Received MQTT");
  DBG_PRINT(" Topic: ");
  DBG_PRINTLN(topic);
  DBG_PRINT("Web: Payload: ");
  for (int i = 0; i < length; i++)
  {
    DBG_PRINT((char)payload[i]);
  }
  DBG_PRINTLN(" ");
  char payload_msg[length];
  for (int i = 0; i < length; i++)
  {
    payload_msg[i] = payload[i];
  }

  if (inductionCooker.mqtttopic == topic)
  {
    if (inductionCooker.induction_state)
    {
      DBG_PRINTLN("Web: passing mqtt to induction");
      inductionCooker.handlemqtt(payload_msg);
    }
    else
      DBG_PRINTLN("Web: bypass mqtt due to induction state");
  }
  for (int i = 0; i < numberOfActors; i++)
  {
    if (actors[i].argument_actor == topic)
    {
      if (actors[i].actor_state)
      {
        DBG_PRINT("Web: passing mqtt to actor ");
        DBG_PRINTLN(actors[i].name_actor);
        actors[i].handlemqtt(payload_msg);
      }
      else
        DBG_PRINTLN("Web: bypass mqtt due to actor state");
    }
    yield();
  }
}

void handleRequestMiscSet()
{
  StaticJsonDocument<512> doc;

  doc["MQTTHOST"] = mqtthost;
  doc["del_sen_act"] = wait_on_Sensor_error_actor / 1000;
  doc["del_sen_ind"] = wait_on_Sensor_error_induction / 1000;
  doc["enable_mqtt"] = StopOnMQTTError;
  doc["enable_wlan"] = StopOnWLANError;
  doc["mqtt_state"] = mqtt_state;
  doc["wlan_state"] = wlan_state;
  doc["delay_mqtt"] = wait_on_error_mqtt / 1000;
  doc["delay_wlan"] = wait_on_error_wlan / 1000;
  doc["debug"] = setDEBUG;
  doc["telnet"] = startTEL;

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleRequestMisc()
{
  String request = server.arg(0);
  String message;

  if (request == "MQTTHOST")
  {
    message = mqtthost;
    goto SendMessage;
  }
  if (request == "mdns_name")
  {
    message = nameMDNS;
    goto SendMessage;
  }
  if (request == "mdns")
  {
    if (startMDNS)
    {
      message = "1";
    }
    else
    {
      message = "0";
    }
    goto SendMessage;
  }
  if (request == "enable_mqtt")
  {
    if (StopOnMQTTError)
    {
      message = "1";
    }
    else
    {
      message = "0";
    }
    goto SendMessage;
  }
  if (request == "enable_wlan")
  {
    if (StopOnWLANError)
    {
      message = "1";
    }
    else
    {
      message = "0";
    }
    goto SendMessage;
  }
  if (request == "delay_mqtt")
  {
    message = wait_on_error_mqtt / 1000;
    goto SendMessage;
  }
  if (request == "delay_wlan")
  {
    message = wait_on_error_wlan / 1000;
    goto SendMessage;
  }
  if (request == "wlan_state")
  {
    if (wlan_state)
    {
      message = "1";
    }
    else
    {
      message = "0";
    }
    goto SendMessage;
  }
  if (request == "mqtt_state")
  {
    if (mqtt_state)
    {
      message = "1";
    }
    else
    {
      message = "0";
    }
    goto SendMessage;
  }
  if (request == "del_sen_act")
  {
    message = wait_on_Sensor_error_actor / 1000;
    goto SendMessage;
  }
  if (request == "del_sen_ind")
  {
    message = wait_on_Sensor_error_induction / 1000;
    goto SendMessage;
  }
  if (request == "debug")
  {
    if (setDEBUG)
    {
      message = "1";
    }
    else
    {
      message = "0";
    }
    goto SendMessage;
  }
  if (request == "telnet")
  {
    if (startTEL)
    {
      message = "1";
    }
    else
    {
      message = "0";
    }
    goto SendMessage;
  }
  if (request == "upsen")
  {
    message = SEN_UPDATE / 1000;
    goto SendMessage;
  }
  if (request == "upact")
  {
    message = ACT_UPDATE / 1000;
    goto SendMessage;
  }
  if (request == "upind")
  {
    message = IND_UPDATE / 1000;
    goto SendMessage;
  }

SendMessage:
  server.send(200, "text/plain", message);
}

void handleSetMisc()
{
  for (int i = 0; i < server.args(); i++)
  {
    if (server.argName(i) == "reset")
    {
      if (server.arg(i) == "1")
      {
        WiFi.disconnect();
        wifiManager.resetSettings();

        unsigned long last = millis();
        while (millis() < last + PAUSE2SEC)
        {
          // just wait for approx 2sec
          yield();
        }

        ESP.reset();
      }
    }
    if (server.argName(i) == "clear")
    {
      if (server.arg(i) == "1")
      {
        SPIFFS.remove("/config.json");
        WiFi.disconnect();
        wifiManager.resetSettings();

        unsigned long last = millis();
        while (millis() < last + PAUSE2SEC)
        {
          // just wait for approx 2sec
          yield();
        }
        ESP.reset();
      }
    }
    if (server.argName(i) == "MQTTHOST")
    {
      server.arg(i).toCharArray(mqtthost, 16);
    }
    if (server.argName(i) == "mdns_name")
    {
      server.arg(i).toCharArray(nameMDNS, 16);
    }
    if (server.argName(i) == "mdns")
    {
      if (server.arg(i) == "1")
      {
        startMDNS = true;
      }
      else
      {
        startMDNS = false;
      }
    }
    if (server.argName(i) == "enable_mqtt")
    {
      if (server.arg(i) == "1")
      {
        StopOnMQTTError = true;
      }
      else
      {
        StopOnMQTTError = false;
      }
    }
    if (server.argName(i) == "delay_mqtt")
    {
      wait_on_error_mqtt = server.arg(i).toInt() * 1000;
    }
    if (server.argName(i) == "enable_wlan")
    {
      if (server.arg(i) == "1")
      {
        StopOnWLANError = true;
      }
      else
      {
        StopOnWLANError = false;
      }
    }
    if (server.argName(i) == "delay_wlan")
    {
      wait_on_error_wlan = server.arg(i).toInt() * 1000;
    }
    if (server.argName(i) == "del_sen_act")
    {
      wait_on_Sensor_error_actor = server.arg(i).toInt() * 1000;
    }
    if (server.argName(i) == "del_sen_ind")
    {
      wait_on_Sensor_error_induction = server.arg(i).toInt() * 1000;
    }
    if (server.argName(i) == "debug")
    {
      if (server.arg(i) == "1")
      {
        setDEBUG = true;
      }
      else
      {
        setDEBUG = false;
      }
    }
    if (server.argName(i) == "telnet")
    {
      if (server.arg(i) == "1")
      {
        startTEL = true;
      }
      else
      {
        if (Telnet)
          Telnet.stop();
        startTEL = false;
      }
    }
    if (server.argName(i) == "upsen")
    {
      int newsup = server.arg(i).toInt();
      if (newsup > 0)
      {
        SEN_UPDATE = newsup * 1000;
      }
    }
    if (server.argName(i) == "upact")
    {
      int newaup = server.arg(i).toInt();
      if (newaup > 0)
      {
        ACT_UPDATE = newaup * 1000;
      }
    }
    if (server.argName(i) == "upind")
    {
      int newiup = server.arg(i).toInt();
      if (newiup > 0)
      {
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
  //cbpiEventSystem(11);
  cbpiEventSystem(EM_REBOOT);
}

void reconMQTT()
{
  //cbpiEventSystem(10);
  retriesMQTT = 1;
  mqttconnectlasttry = 0;
  cbpiEventSystem(EM_MQTTER);
}

void OTA()
{
  //cbpiEventSystem(9);
  cbpiEventSystem(EM_OTASET);
}
