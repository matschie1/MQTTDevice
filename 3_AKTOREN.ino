class Actor
{
    unsigned long powerLast;          // Zeitmessung für High oder Low
    bool isInverted = false;
    int dutycycle_actor = 5000;
    byte OFF;
    byte ON;

  public:
    byte pin_actor = 9;      // the number of the LED pin
    String argument_actor;
    String name_actor;
    byte power_actor;
    bool isOn;


    Actor(String pin, String argument, String aname, String ainverted)
    {
      change(pin, argument, aname, ainverted);
    }

    void Update() {
      if (isPin(pin_actor)) {
        if (isOn && power_actor > 0) {
          if (millis() > powerLast + dutycycle_actor) {
            powerLast = millis();
          }
          if (millis() > powerLast + (dutycycle_actor * power_actor / 100L)) {
            digitalWrite(pin_actor, OFF);
          } else {
            digitalWrite(pin_actor, ON);
          }
        } else {
          digitalWrite(pin_actor, OFF);
        }
      }
    }

    void change(String pin, String argument, String aname, String ainverted) {
      // Set PIN
      if (isPin(pin_actor)) {
        digitalWrite(pin_actor, HIGH);
        pins_used[pin_actor] = false;
        delay(5);
      }

      pin_actor = StringToPin(pin);
      if (isPin(pin_actor)) {
        pinMode(pin_actor, OUTPUT);
        digitalWrite(pin_actor, HIGH);
        pins_used[pin_actor] = true;
      }

      isOn = false;

      name_actor = aname;

      if (argument_actor != argument) {
        mqtt_unsubscribe();
        argument_actor = argument;
        mqtt_subscribe();
      }

      if (ainverted == "1") {
        isInverted = true;
        ON = HIGH;
        OFF = LOW;
      }
      if (ainverted == "0") {
        isInverted = false;
        ON = LOW;
        OFF = HIGH;
      }

    }

    void mqtt_subscribe() {
      if (client.connected()) {
        char subscribemsg[50];
        argument_actor.toCharArray(subscribemsg, 50);
        Serial.print("Subscribing to ");
        Serial.println(subscribemsg);
        client.subscribe(subscribemsg);
      }

    }

    void mqtt_unsubscribe() {
      if (client.connected()) {
        char subscribemsg[50];
        argument_actor.toCharArray(subscribemsg, 50);
        Serial.print("Unsubscribing from ");
        Serial.println(subscribemsg);
        client.unsubscribe(subscribemsg);
      }
    }

    void handlemqtt(char* payload) {
      StaticJsonBuffer<128> jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(payload);

      if (!json.success()) {
        return;
      }

      String state = json["state"];


      if (state == "off") {
        isOn = false;
        power_actor = 0;
        return;
      }

      if (state == "on") {
        int newpower = atoi(json["power"]);
        isOn = true;
        power_actor = min(100, newpower);
        power_actor = max(0, newpower);
        return;
      }
    }

    String getInverted() {
      if (isInverted) {
        return "1";
      }
      else {
        return "0";
      }
    };

};

/* Initialisierung des Arrays */
Actor actors[6] = {
  Actor("", "", "", ""),
  Actor("", "", "", ""),
  Actor("", "", "", ""),
  Actor("", "", "", ""),
  Actor("", "", "", ""),
  Actor("", "", "", "")
};

/* Funktionen für Loop */
void handleActors() {
  for (int i = 0; i < numberOfActors; i++) {
    actors[i].Update();
    yield();
  }
}

/* Funktionen für Web */
void handleRequestActors() {
  String message;
  for (int i = 0; i < numberOfActors; i++) {
    message += F("<li class=\"list-group-item d-flex justify-content-between align-items-center\">");
    message += actors[i].name_actor;
    message += F("</span> <span class=\"badge ");
    if (actors[i].isOn) {
      message += F("badge-success\">ON: ");
      message += actors[i].power_actor;
      message += F("%");
    } else {
      message += F("badge-dark\">OFF");
    }
    message += F("</span><span class=\"badge badge-light\">");
    message += actors[i].argument_actor;
    message += F("</span> <span class=\"badge badge-light\">PIN ");
    message += PinToString(actors[i].pin_actor);
    message += F("</span> <a href=\"\" class=\"badge badge-warning\" data-toggle=\"modal\" data-target=\"#actor_modal\" data-value=\"");
    message += i;
    message += F("\">Edit</a></li>");
    yield();
  }
  server.send(200, "text/html", message);
}

void handleRequestActor() {
  int id = server.arg(0).toInt();
  String request = server.arg(1);
  String message;

  if (id == -1) {
    message = "";
    goto SendMessage;
  } else {
    if (request == "name") {
      message = actors[id].name_actor;
      goto SendMessage;
    }
    if (request == "script") {
      message = actors[id].argument_actor;
      goto SendMessage;
    }
    if (request == "pin") {
      message = PinToString(actors[id].pin_actor);
      goto SendMessage;
    }
    if (request == "inverted") {
      message = actors[id].getInverted();
      goto SendMessage;
    }
    message = "not found";
  }
  saveConfig();
SendMessage:
  server.send(200, "text/plain", message);
}

void handleSetActor() {
  int id = server.arg(0).toInt();

  if (id == -1) {
    id = numberOfActors;
    numberOfActors += 1;
  }

  String ac_pin = PinToString(actors[id].pin_actor);
  String ac_argument = actors[id].argument_actor;
  String ac_name = actors[id].name_actor;
  String ac_isinverted = actors[id].getInverted();

  for (int i = 0; i < server.args(); i++) {
    if (server.argName(i) == "name") {
      ac_name = server.arg(i);
    }
    if (server.argName(i) == "pin")  {
      ac_pin = server.arg(i);
    }
    if (server.argName(i) == "script")  {
      ac_argument = server.arg(i);
    }
    if (server.argName(i) == "inverted")  {
      ac_isinverted = server.arg(i);
    }
    yield();
  }

  actors[id].change(ac_pin, ac_argument, ac_name, ac_isinverted);

  saveConfig();
}

void handleDelActor() {
  int id = server.arg(0).toInt();

  for (int i = id; i < numberOfActors; i++) {
    if (i == 5) {
      actors[i].change("", "", "", "");
    } else {
      actors[i].change(PinToString(actors[i + 1].pin_actor), actors[i + 1].argument_actor, actors[i + 1].name_actor, actors[i + 1].getInverted());
    }
  }

  numberOfActors -= 1;
  saveConfig();
}

void handlereqPins() {
  int id = server.arg(0).toInt();
  String message;

  if (id != -1) {
    message += F("<option>");
    message += PinToString(actors[id].pin_actor);
    message += F("</option><option disabled>──────────</option>");
  }
  for (int i = 0; i < numberOfPins; i++) {
    if (pins_used[pins[i]] == false) {
      message += F("<option>");
      message += pin_names[i];
      message += F("</option>");
    }
    yield();
  }
  server.send(200, "text/plain", message);
}

byte StringToPin(String pinstring) {
  for (int i = 0; i < numberOfPins; i++) {
    if (pin_names[i] == pinstring) {
      return pins[i];
    }
  }
  return 9;
}

String PinToString(byte pinbyte) {
  for (int i = 0; i < numberOfPins; i++) {
    if (pins[i] == pinbyte) {
      return pin_names[i];
    }
  }
  return "NaN";
}

bool isPin(byte pinbyte) {
  bool returnValue = false;
  for (int i = 0; i < numberOfPins; i++) {
    if (pins[i] == pinbyte) {
      returnValue = true;
      goto Ende;
    }
  }
Ende:
  return returnValue;
}
