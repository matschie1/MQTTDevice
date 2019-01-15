class induction
{

    unsigned long timeTurnedoff;

    long timeOutCommand = 5000;      // TimeOut für Seriellen Befehl
    long timeOutReaction = 2000;    // TimeOut für Induktionskochfeld
    unsigned long lastInterrupt;
    unsigned long lastCommand;
    bool inputStarted = false;
    byte inputCurrent = 0;
    byte inputBuffer[33];
    bool isError = false;
    byte error = 0;

    //int storePower = 0;
    long powerSampletime = 20000;
    unsigned long powerLast;
    long powerHigh = powerSampletime; // Dauer des "HIGH"-Anteils im Schaltzyklus
    long powerLow = 0;

  public:
    byte PIN_WHITE = 9;       // RELAIS
    byte PIN_YELLOW = 9;      // AUSGABE AN PLATTE
    byte PIN_INTERRUPT = 9;   // EINGABE VON PLATTE
    int power = 0;
    int newPower = 0;
    byte CMD_CUR = 0;                 // Aktueller Befehl
    boolean isRelayon = false;        // Systemstatus: ist das Relais in der Platte an?
    boolean isInduon = false;         // Systemstatus: ist Power > 0?
    boolean isPower = false;
    String mqtttopic = "";
    boolean isEnabled = false;
    long delayAfteroff = 120000;

    induction() {
      setupCommands();
    }

    void change(byte pinwhite, byte pinyellow, byte pinblue, String topic, long delayoff, bool is_enabled) {
      if (isEnabled) {
        // aktuelle PINS deaktivieren
        if (isPin(PIN_WHITE)) {
          digitalWrite(PIN_WHITE, HIGH);
          pins_used[PIN_WHITE] = false;
        }

        if (isPin(PIN_YELLOW)) {
          digitalWrite(PIN_YELLOW, HIGH);
          pins_used[PIN_YELLOW] = false;
        }

        if (isPin(PIN_INTERRUPT)) {
          //detachInterrupt(PIN_INTERRUPT);
          //pinMode(PIN_INTERRUPT, OUTPUT);
          digitalWrite(PIN_INTERRUPT, HIGH);
          pins_used[PIN_INTERRUPT] = false;
        }

        mqtt_unsubscribe();
      }

      // Neue Variablen Speichern

      PIN_WHITE = pinwhite;
      PIN_YELLOW = pinyellow;
      PIN_INTERRUPT = pinblue;

      mqtttopic = topic;
      delayAfteroff = delayoff;

      isEnabled = is_enabled;

      if (isEnabled) {
        // neue PINS aktiveren
        if (isPin(PIN_WHITE)) {
          pinMode(PIN_WHITE, OUTPUT);
          digitalWrite(PIN_WHITE, LOW);
          pins_used[PIN_WHITE] = true;
        }

        if (isPin(PIN_YELLOW)) {
          pinMode(PIN_YELLOW, OUTPUT);
          digitalWrite(PIN_YELLOW, LOW);
          pins_used[PIN_YELLOW] = true;
        }

        if (isPin(PIN_INTERRUPT)) {
          pinMode(PIN_INTERRUPT, INPUT_PULLUP);
          //attachInterrupt(digitalPinToInterrupt(PIN_INTERRUPT), readInputWrap, CHANGE);
          pins_used[PIN_INTERRUPT] = true;
        }
        if (client.connected()) {
          mqtt_subscribe();
        }
      }
    }

    void mqtt_subscribe() {
      if (isEnabled) {
        if (client.connected()) {
          char subscribemsg[50];
          mqtttopic.toCharArray(subscribemsg, 50);
          Serial.print("Subscribing to ");
          Serial.println(subscribemsg);
          client.subscribe(subscribemsg);
        }
      }
    }

    void mqtt_unsubscribe() {
      if (client.connected()) {
        char subscribemsg[50];
        mqtttopic.toCharArray(subscribemsg, 50);
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
        newPower = 0;
        return;
      } else {
        newPower = atoi(json["power"]);
      }
    }

    void setupCommands() {
      for (int i = 0; i < 33; i++) {
        for (int j = 0; j < 6; j++) {
          if    ( CMD[j][i] == 1)  {
            CMD[j][i] = SIGNAL_HIGH;
          }
          else                     {
            CMD[j][i] = SIGNAL_LOW;
          }
        }
      }
    }

    bool updateRelay() {
      if (isInduon == true && isRelayon == false) {         /* Relais einschalten */
        Serial.println("Turning Relay on");
        digitalWrite(PIN_WHITE, HIGH);
        return true;
      }

      if (isInduon == false && isRelayon == true) {         /* Relais ausschalten */
        if (millis() > timeTurnedoff + delayAfteroff) {
          Serial.println("Turning Relay off");
          digitalWrite(PIN_WHITE, LOW);
          return false;
        }
      }

      if (isInduon == false && isRelayon == false) {        /* Ist aus, bleibt aus. */
        return false;
      }

      return true;                                          /* Ist an, bleibt an. */
    }

    void Update() {
      updatePower();

      isRelayon = updateRelay();

      if (isInduon && power > 0) {
        if (millis() > powerLast + powerSampletime) {
          powerLast = millis();
        }
        if (millis() > powerLast + powerHigh) {
          sendCommand(CMD[CMD_CUR - 1]);
          isPower = false;
        }
        else                                  {
          sendCommand(CMD[CMD_CUR]);
          isPower = true;
        }
      } else if (isRelayon) {
        sendCommand(CMD[0]);
      }
    }

    void updatePower() {
      lastCommand = millis();
      if (power != newPower) {                              /* Neuer Befehl empfangen */

        if (newPower > 100) {
          newPower = 100;  /* Nicht > 100 */
        }
        if (newPower < 0)   {
          newPower = 0;    /* Nicht < 0 */
        }
        Serial.print("Setting Power to ");
        Serial.println(newPower);

        power = newPower;

        timeTurnedoff = 0;
        isInduon = true;
        long difference = 0;

        if (power == 0) {
          CMD_CUR = 0;
          timeTurnedoff = millis();
          isInduon = false;
          difference = 0;
          goto setPowerLevel;
        }

        for (int i = 1; i < 7; i++) {
          if (power <= PWR_STEPS[i]) {
            CMD_CUR = i;
            difference = PWR_STEPS[i] - power;
            goto setPowerLevel;
          }
        }

setPowerLevel:                                      /* Wie lange "HIGH" oder "LOW" */
        if (difference != 0) {
          powerLow = powerSampletime * difference / 20L;
          powerHigh = powerSampletime - powerLow;
        } else {
          powerHigh = powerSampletime;
          powerLow = 0;
        };
      }
    }

    void sendCommand(int command[33]) {
      digitalWrite(PIN_YELLOW, HIGH);
      delay(SIGNAL_START);
      digitalWrite(PIN_YELLOW, LOW);
      delay(SIGNAL_WAIT);


      for (int i = 0; i < 33; i++) {
        digitalWrite(PIN_YELLOW, HIGH);
        delayMicroseconds(command[i]);
        digitalWrite(PIN_YELLOW, LOW);
        delayMicroseconds(SIGNAL_LOW);
      }
    }

    void readInput() {
      //  // Variablen sichern
      bool ishigh = digitalRead(PIN_INTERRUPT);
      unsigned long newInterrupt = micros();
      long signalTime = newInterrupt - lastInterrupt;

      // Glitch rausfiltern
      if (signalTime > 10) {

        if (ishigh) {
          lastInterrupt = newInterrupt;         // PIN ist auf Rising, Bit senden hat gestartet :)
        }  else {                               // Bit ist auf Falling, Bit Übertragung fertig. Auswerten.

          if (!inputStarted) {                  // suche noch nach StartBit.
            if (signalTime < 35000L && signalTime > 15000L) {
              inputStarted = true;
              inputCurrent = 0;
            }
          } else {                              // Hat Begonnen. Nehme auf.
            if (inputCurrent < 34) {            // nur bis 33 aufnehmen.
              if (signalTime < (SIGNAL_HIGH + SIGNAL_HIGH_TOL) && signalTime > (SIGNAL_HIGH - SIGNAL_HIGH_TOL) ) {
                // HIGH BIT erkannt
                inputBuffer[inputCurrent] = 1;
                inputCurrent += 1;
              }
              if (signalTime < (SIGNAL_LOW + SIGNAL_LOW_TOL) && signalTime > (SIGNAL_LOW - SIGNAL_LOW_TOL) ) {
                // LOW BIT erkannt
                inputBuffer[inputCurrent] = 0;
                inputCurrent += 1;
              }
            } else {                            // Aufnahme vorbei.

              /* Auswerten */
              //newError = BtoI(13,4);          // Fehlercode auslesen.

              /* von Vorne */
              //timeLastReaction = millis();
              inputCurrent = 0;
              inputStarted = false;
            }
          }
        }
      }
    }

    unsigned long BtoI(int start, int numofbits) {   //binary array to integer conversion
      unsigned long integer = 0;
      //   unsigned long mask=1;
      //   for (int i = numofbits+start-1; i >= start; i--) {
      //     if (inputBuffer[i]) integer |= mask;
      //     mask = mask << 1;
      //   }
      return integer;
    }
}

inductionCooker = induction();

void readInputWrap() {
  inductionCooker.readInput();
}

/* Funktion für Loop */
void handleInduction() {
  inductionCooker.Update();
}

void handleRequestInduction() {
  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject& inductionResponse = jsonBuffer.createObject();

  inductionResponse["enabled"] = inductionCooker.isEnabled;
  if (inductionCooker.isEnabled) {
    inductionResponse["relayOn"] = inductionCooker.isRelayon;
    inductionResponse["power"] = inductionCooker.power;
    inductionResponse["relayOn"] = inductionCooker.isRelayon;
    if (inductionCooker.isPower) {
      inductionResponse["powerLevel"] = inductionCooker.CMD_CUR;
    } else {
      inductionResponse["powerLevel"] = max(0, inductionCooker.CMD_CUR - 1);
    }
  }
  String response;
  inductionResponse.printTo(response);
  server.send(200, "application/json", response);
}

void handleRequestIndu() {
  String request = server.arg(0);
  String message;

  if (request == "isEnabled") {
    if (inductionCooker.isEnabled) {
      message = "1";
    }
    else {
      message = "0";
    }
    goto SendMessage;
  }
  if (request == "topic") {
    message = inductionCooker.mqtttopic;
    goto SendMessage;
  }
  if (request == "delay") {
    message = inductionCooker.delayAfteroff;
    goto SendMessage;
  }
  if (request == "pins")  {
    int id = server.arg(1).toInt();
    byte pinswitched;
    switch (id) {
      case 0:
        pinswitched = inductionCooker.PIN_WHITE;
        break;
      case 1:
        pinswitched = inductionCooker.PIN_YELLOW;
        break;
      case 2:
        pinswitched = inductionCooker.PIN_INTERRUPT;
        break;
    }
    if (isPin(pinswitched)) {
      message += F("<option>");
      message += PinToString(pinswitched);
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
    goto SendMessage;
  }

SendMessage:
  server.send(200, "text/plain", message);
}

void handleSetIndu() {

  byte pin_white = inductionCooker.PIN_WHITE;
  byte pin_blue = inductionCooker.PIN_INTERRUPT;
  byte pin_yellow = inductionCooker.PIN_YELLOW;
  long delayoff = inductionCooker.delayAfteroff;
  bool is_enabled = inductionCooker.isEnabled;
  String topic = inductionCooker.mqtttopic;


  for (int i = 0; i < server.args(); i++) {
    if (server.argName(i) == "enabled") {
      if (server.arg(i) == "1") {
        is_enabled = true;
      } else {
        is_enabled = false;
      }
    }
    if (server.argName(i) == "topic")  {
      topic = server.arg(i);
    }
    if (server.argName(i) == "pinwhite")  {
      pin_white = StringToPin(server.arg(i));
    }
    if (server.argName(i) == "pinyellow")  {
      pin_yellow = StringToPin(server.arg(i));
    }
    if (server.argName(i) == "pinblue")  {
      pin_blue = StringToPin(server.arg(i));
    }
    if (server.argName(i) == "delay")  {
      delayoff = server.arg(i).toInt();
    }
    yield();
  }

  inductionCooker.change(pin_white, pin_yellow, pin_blue, topic, delayoff, is_enabled);

  saveConfig();
  server.send(201, "text/plain", "created");
}
