class Actor
{
    unsigned long powerLast; // Zeitmessung für High oder Low
    bool isInverted = false;
    int dutycycle_actor = 5000;
    unsigned char OFF;
    unsigned char ON;

  public:
    unsigned char pin_actor = 9; // the number of the LED pin
    String argument_actor;
    String name_actor;
    unsigned char power_actor;
    bool isOn;
    bool switchable;                // actors switchable on error events?
    bool isOnBeforeError = false;   // isOn status before error event
    bool actor_state = true;        // Error state actor

    // MQTT Publish
    char actor_mqtttopic[50]; // Für MQTT Kommunikation

    Actor(String pin, String argument, String aname, String ainverted, String aswitchable)
    {
      change(pin, argument, aname, ainverted, aswitchable);
    }

    void Update()
    {
      if (isPin(pin_actor))
      {
        if (isOn && power_actor > 0)
        {
          if (millis() > powerLast + dutycycle_actor)
          {
            powerLast = millis();
          }
          if (millis() > powerLast + (dutycycle_actor * power_actor / 100L))
          {
            digitalWrite(pin_actor, OFF);
          }
          else
          {
            digitalWrite(pin_actor, ON);
          }
        }
        else
        {
          digitalWrite(pin_actor, OFF);
        }
      }
    }

    void change(String pin, String argument, String aname, String ainverted, String aswitchable)
    {
      // Set PIN
      if (isPin(pin_actor))
      {
        digitalWrite(pin_actor, HIGH);
        pins_used[pin_actor] = false;
        millis2wait(10);
      }

      pin_actor = StringToPin(pin);
      if (isPin(pin_actor))
      {
        pinMode(pin_actor, OUTPUT);
        digitalWrite(pin_actor, HIGH);
        pins_used[pin_actor] = true;
      }

      isOn = false;

      name_actor = aname;

      if (argument_actor != argument)
      {
        mqtt_unsubscribe();
        argument_actor = argument;
        mqtt_subscribe();

        // MQTT Publish - not yet ready
        // argument.toCharArray(actor_mqtttopic, argument.length() + 1);
      }

      if (ainverted == "1")
      {
        isInverted = true;
        ON = HIGH;
        OFF = LOW;
      }
      if (ainverted == "0")
      {
        isInverted = false;
        ON = LOW;
        OFF = HIGH;
      }
      if (aswitchable == "1")
      {
        switchable = true;
      }
      else
      {
        switchable = false;
      }
      actor_state = true;
      isOnBeforeError = false;

    }

    /*    //    Not yet ready
          void publishmqtt() {
            if (client.connected()) {
              StaticJsonBuffer<256> jsonBuffer;
              JsonObject& json = jsonBuffer.createObject();
              if (isOn) {
                json["State"] = "on";
                json["power"] = String(power_actor);
              }
              else
                json["State"] = "off";

              char jsonMessage[100];
              json.printTo(jsonMessage);
              client.publish(actor_mqtttopic, jsonMessage);
            }
          }
    */
    void mqtt_subscribe()
    {
      if (client.connected())
      {
        char subscribemsg[50];
        argument_actor.toCharArray(subscribemsg, 50);
        DBG_PRINT("Act: ");
        DBG_PRINT("Subscribing to ");
        DBG_PRINTLN(subscribemsg);
        client.subscribe(subscribemsg);
      }
    }

    void mqtt_unsubscribe()
    {
      if (client.connected())
      {
        char subscribemsg[50];
        argument_actor.toCharArray(subscribemsg, 50);
        DBG_PRINT("Act: ");
        DBG_PRINT("Unsubscribing from ");
        DBG_PRINTLN(subscribemsg);
        client.unsubscribe(subscribemsg);
      }
    }

    void handlemqtt(char *payload)
    {
      StaticJsonBuffer<128> jsonBuffer;
      JsonObject &json = jsonBuffer.parseObject(payload);

      if (!json.success())
      {
        return;
      }

      String state = json["state"];

      if (state == "off")
      {
        isOn = false;
        power_actor = 0;
        return;
      }

      if (state == "on")
      {
        int newpower = atoi(json["power"]);
        isOn = true;
        power_actor = min(100, newpower);
        power_actor = max(0, newpower);
        return;
      }
    }

    String getInverted()
    {
      if (isInverted)
      {
        return "1";
      }
      else
      {
        return "0";
      }
    };
    String getSwitchable()
    {
      if (switchable)
      {
        return "1";
      }
      else
      {
        return "0";
      }
    }
};

/* Initialisierung des Arrays */
Actor actors[6] = {
  Actor("", "", "", "", ""),
  Actor("", "", "", "", ""),
  Actor("", "", "", "", ""),
  Actor("", "", "", "", ""),
  Actor("", "", "", "", ""),
  Actor("", "", "", "", "")
};

/* Funktionen für Loop */
void handleActors()
{
  for (int i = 0; i < numberOfActors; i++)
  {
    actors[i].Update();
    yield();
  }
}

/* Funktionen für Web */
void handleRequestActors()
{
  StaticJsonBuffer<1024> jsonBuffer;
  JsonArray &actorsResponse = jsonBuffer.createArray();

  for (int i = 0; i < numberOfActors; i++)
  {
    JsonObject &actorResponse = jsonBuffer.createObject();
    ;
    actorResponse["name"] = actors[i].name_actor;
    actorResponse["status"] = actors[i].isOn;
    actorResponse["power"] = actors[i].power_actor;
    actorResponse["mqtt"] = actors[i].argument_actor;
    actorResponse["pin"] = PinToString(actors[i].pin_actor);
    actorResponse["sw"] = actors[i].switchable;
    actorResponse["state"] = actors[i].actor_state;
    actorsResponse.add(actorResponse);
    yield();
  }

  String response;
  actorsResponse.printTo(response);
  server.send(200, "application/json", response);
}

void handleRequestActor()
{
  int id = server.arg(0).toInt();
  String request = server.arg(1);
  String message;

  if (id == -1)
  {
    message = "";
    goto SendMessage;
  }
  else
  {
    if (request == "name")
    {
      message = actors[id].name_actor;
      goto SendMessage;
    }
    if (request == "script")
    {
      message = actors[id].argument_actor;
      goto SendMessage;
    }
    if (request == "pin")
    {
      message = PinToString(actors[id].pin_actor);
      goto SendMessage;
    }
    if (request == "inv")
    {
      message = actors[id].getInverted();
      goto SendMessage;
    }
    if (request == "sw")
    {
      message = actors[id].getSwitchable();
      goto SendMessage;
    }
    message = "not found";
  }
  saveConfig();
SendMessage:
  server.send(200, "text/plain", message);
}

void handleSetActor()
{
  int id = server.arg(0).toInt();

  if (id == -1)
  {
    id = numberOfActors;
    numberOfActors += 1;
  }

  String ac_pin = PinToString(actors[id].pin_actor);
  String ac_argument = actors[id].argument_actor;
  String ac_name = actors[id].name_actor;
  String ac_isinverted = actors[id].getInverted();
  String ac_switchable = actors[id].getSwitchable();

  for (int i = 0; i < server.args(); i++)
  {
    if (server.argName(i) == "name")
    {
      ac_name = server.arg(i);
    }
    if (server.argName(i) == "pin")
    {
      ac_pin = server.arg(i);
    }
    if (server.argName(i) == "script")
    {
      ac_argument = server.arg(i);
    }
    if (server.argName(i) == "inv")
    {
      ac_isinverted = server.arg(i);
    }
    if (server.argName(i) == "sw")
    {
      ac_switchable = server.arg(i);
    }
    yield();
  }

  actors[id].change(ac_pin, ac_argument, ac_name, ac_isinverted, ac_switchable);

  saveConfig();
  server.send(201, "text/plain", "created");
}

void handleDelActor()
{
  int id = server.arg(0).toInt();

  for (int i = id; i < numberOfActors; i++)
  {
    if (i == 5)
    {
      actors[i].change("", "", "", "", "");
    }
    else
    {
      actors[i].change(PinToString(actors[i + 1].pin_actor), actors[i + 1].argument_actor, actors[i + 1].name_actor, actors[i + 1].getInverted(), actors[i + 1].getSwitchable());
    }
  }

  numberOfActors -= 1;
  saveConfig();
  server.send(200, "text/plain", "deleted");
}

void handlereqPins()
{
  int id = server.arg(0).toInt();
  String message;

  if (id != -1)
  {
    message += F("<option>");
    message += PinToString(actors[id].pin_actor);
    message += F("</option><option disabled>──────────</option>");
  }
  for (int i = 0; i < numberOfPins; i++)
  {
    if (pins_used[pins[i]] == false)
    {
      message += F("<option>");
      message += pin_names[i];
      message += F("</option>");
    }
    yield();
  }
  server.send(200, "text/plain", message);
}

unsigned char StringToPin(String pinstring)
{
  for (int i = 0; i < numberOfPins; i++)
  {
    if (pin_names[i] == pinstring)
    {
      return pins[i];
    }
  }
  return 9;
}

String PinToString(unsigned char pinbyte)
{
  for (int i = 0; i < numberOfPins; i++)
  {
    if (pins[i] == pinbyte)
    {
      return pin_names[i];
    }
  }
  return "NaN";
}

bool isPin(unsigned char pinbyte)
{
  bool returnValue = false;
  for (int i = 0; i < numberOfPins; i++)
  {
    if (pins[i] == pinbyte)
    {
      returnValue = true;
      goto Ende;
    }
  }
Ende:
  return returnValue;
}
