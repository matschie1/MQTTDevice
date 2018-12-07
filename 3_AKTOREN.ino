class Actor
{
    unsigned long powerLast;          // Zeitmessung für High oder Low
    bool isInverted = false;
    int dutycycle_actor = 5000;
    
  public:
    byte pin_actor = 99;      // the number of the LED pin
    String argument_actor;
    String name_actor;
    byte power_actor;    
    bool isOn;
     
  
  Actor(String pin, String argument, String aname, String ainverted) 
    { 
      change(pin,argument,aname,ainverted);     
    }

  void Update() {
    if (isOn && power_actor > 0) {
      if (millis() > powerLast + dutycycle_actor) { powerLast = millis(); }
      if (millis() > powerLast + (dutycycle_actor * power_actor / 100L)) {
        if (isInverted) { digitalWrite(pin_actor, LOW); }
        else { digitalWrite(pin_actor, HIGH); }
      } else {
        if (isInverted) { digitalWrite(pin_actor, HIGH); }
        else { digitalWrite(pin_actor, LOW); }
      }         
    } else {
      if (isInverted) { digitalWrite(pin_actor, LOW); }
      else { digitalWrite(pin_actor, HIGH); }
    }
  }
  
  void change(String pin, String argument, String aname, String ainverted) {
    // Set PIN    
    if (pin_actor != 99) {
      digitalWrite(pin_actor,HIGH);
      pins_used[pin_actor] = false;
      delay(5);
    }
    if (getPIN(pin) != 99) {
      pin_actor = getPIN(pin);    
      pinMode(pin_actor,OUTPUT);
      digitalWrite(pin_actor,HIGH);
      pins_used[pin_actor] = true;
    }
    
    isOn = false;
    
    name_actor = aname;    
    argument_actor = argument;

    if (ainverted == "1") { isInverted = true; }
    if (ainverted == "0") { isInverted = false; }
    
    mqtt_subscribe();
  }

  void mqtt_subscribe() {
    char subscribemsg[50];
    argument_actor.toCharArray(subscribemsg,50);
    Serial.print("Subscribing to ");
    Serial.println(subscribemsg);
    client.subscribe(subscribemsg);
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
      power_actor = min(100,newpower);
      power_actor = max(0,newpower);
      return;    
    }  
  }

  String getInverted() {
    if (isInverted) { return "1"; }
    else {return "0"; }
  };

  int getPIN(String pin) {
      if (pin == "D0") { return D0; }
      if (pin == "D1") { return D1; }
      if (pin == "D2") { return D2; }
      if (pin == "D3") { return D3; }
      if (pin == "D4") { return D4; }
      if (pin == "D5") { return D5; }
      if (pin == "D6") { return D6; }
      if (pin == "D7") { return D7; }
      if (pin == "D8") { return D8; }
      return 99;                 
  }

  String getPinStr() {
      if (pin_actor == D0) { return "D0"; }
      if (pin_actor == D1) { return "D1"; }
      if (pin_actor == D2) { return "D2"; }
      if (pin_actor == D3) { return "D3"; }
      if (pin_actor == D4) { return "D4"; }
      if (pin_actor == D5) { return "D5"; }
      if (pin_actor == D6) { return "D6"; }
      if (pin_actor == D7) { return "D7"; }
      if (pin_actor == D8) { return "D8"; }
      return "NaN";                 
  }  
};

/* Initialisierung des Arrays */
Actor actors[6] = {
  Actor("","","",""),
  Actor("","","",""),
  Actor("","","",""),
  Actor("","","",""),
  Actor("","","",""),
  Actor("","","","")
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
    message += actors[i].getPinStr();
    message += F("</span> <a href=\"\" class=\"badge badge-warning\" data-toggle=\"modal\" data-target=\"#actor_modal\" data-value=\"");
    message += i;
    message += F("\">Edit</a></li>");
    yield();
  }
  server.send(200,"text/html", message);
}

void handleRequestActor() {
  int id = server.arg(0).toInt();
  String request = server.arg(1);
  String message;

  if (id == -1) {
    message = "";
    goto SendMessage;  
  } else {
    if (request == "name") { message = actors[id].name_actor; goto SendMessage; }
    if (request == "script") { message = actors[id].argument_actor; goto SendMessage; }
    if (request == "pin") { message = actors[id].getPinStr(); goto SendMessage; }
    if (request == "inverted") { message = actors[id].getInverted(); goto SendMessage; }
    message = "not found";    
  }
  saveConfig();
  SendMessage:
    server.send(200,"text/plain",message);    
}

void handleSetActor() {
  int id = server.arg(0).toInt();
  
  if (id == -1) {
    id = numberOfActors;
    numberOfActors += 1;            
  }

  String ac_pin = actors[id].getPinStr();
  String ac_argument = actors[id].argument_actor;
  String ac_name = actors[id].name_actor;
  String ac_isinverted = actors[id].getInverted();
  
  for (int i = 0; i < server.args(); i++) {
    if (server.argName(i) == "name") { ac_name = server.arg(i); }  
    if (server.argName(i) == "pin")  { ac_pin = server.arg(i); }  
    if (server.argName(i) == "script")  { ac_argument = server.arg(i); }
    if (server.argName(i) == "inverted")  { ac_isinverted = server.arg(i); }
    yield();       
  }

  actors[id].change(ac_pin,ac_argument,ac_name,ac_isinverted);
  
  saveConfig();
}

void handleDelActor() {
  int id = server.arg(0).toInt();

   for (int i = id; i < numberOfActors; i++) {
    if (i == 5) { actors[i].change("","","",""); } else {
      actors[i].change(actors[i+1].getPinStr(),actors[i+1].argument_actor,actors[i+1].name_actor,actors[i+1].getInverted());     
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
    message += actors[id].getPinStr();
    message += F("</option><option disabled>──────────</option>");   
  }
  for (int i = 0; i < numberOfPins; i++) {
    if (pins_used[pins[i]]== false) {
      message += F("<option>");
      message += pin_names[i];
      message += F("</option>");      
    }
    yield();
  }
  server.send(200,"text/plain",message);    
}
