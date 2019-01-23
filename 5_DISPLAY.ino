#ifdef DISPLAY
class oled
{
    String date;
    String t;

  public:
    //byte PIN_SDA = 5;
    //byte PIN_SDL = 4;
    byte PIN_SDA = 9;
    byte PIN_SDL = 9;
    bool dispEnabled = 0;
    int address;

    bool senOK = 0;
    bool actOK = 0;
    bool sysOK = 0;

    oled()
    {
      change(address, dispEnabled);
    }

    void dispInit() {
      if (dispEnabled == 1) {

        // update the NTP client and get the UNIX UTC timestamp
//        while (!timeClient.update())
//        {
//          showDispSet("NTP Update ...");
//          delay(100);           // if network is up and running just wait a second for NTP
//          timeClient.update();
//          DBG_PRINTLN("Timeclient ... ");
//        }

        // update the NTP client and get the UNIX UTC timestamp
        if (!timeClient.update())
        {
          delay(100);           // if network is up and running just wait a second for NTP
          timeClient.update();
        }
        unsigned long epochTime =  timeClient.getEpochTime();
        // convert received time stamp to time_t object
        time_t local, utc;
        utc = epochTime;

        // Then convert the UTC UNIX timestamp to local time
        //  TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
        //  TimeChangeRule CET = {"CET", Last, Sun, Oct, 3, 60};       //Central European Standard Time

        TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 0};     //Central European Summer Time
        TimeChangeRule CET = {"CET", Last, Sun, Oct, 3, 0};       //Central European Standard Time
        Timezone CE(CEST, CET );

        local = CE.toLocal(utc);
        setTime(local);
        local = CE.toLocal(utc);
        digClock();
      }
    }

    void dispUpdate() {
      if (dispEnabled == 1) {
        showDispClear();
        showDispTime(t);
        showDispIP(WiFi.localIP().toString());
        showDispWlan();
        showDispMqtt();
        showDispLines();
        showDispSen();
        showDispAct();
        showDispInd();
        showDispDisplay();
      }
    }

    void digClock() {

      date = "";  // clear the variables
      t = "";

      // update the NTP client and get the UNIX UTC timestamp
      if (!timeClient.update())
      {
        delay(100);           // if network is up and running just wait a second for NTP
        timeClient.update();
      }
      unsigned long epochTime =  timeClient.getEpochTime();

      // convert received time stamp to time_t object
      time_t local, utc;
      utc = epochTime;

      // Then convert the UTC UNIX timestamp to local time
      //  TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
      //  TimeChangeRule CET = {"CET", Last, Sun, Oct, 3, 60};       //Central European Standard Time

      TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 0};     //Central European Summer Time
      TimeChangeRule CET = {"CET", Last, Sun, Oct, 3, 0};       //Central European Standard Time
      Timezone CE(CEST, CET );

      local = CE.toLocal(utc);
      setTime(local);
      //      local = CE.toLocal(utc);

      /*  Anzeige Datum: const char nach MQTTDevice.ino verschiebe

            const char * days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"} ;
            const char * months[] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"} ;
            const char * ampm[] = {"AM", "PM"} ;

            // now format the Time variables into strings with proper names for month, day etc
            date += days[weekday(local) - 1];
            date += ", ";
            date += months[month(local) - 1];
            date += " ";
            date += day(local);
            date += ", ";
            date += year(local);
      */
      // format the time to 12-hour format with AM/PM and no seconds
      //t += hourFormat12(local);
      t += hour(local);
      t += ":";
      if (minute(local) < 10) // add a zero if minute is under 10
        t += "0";
      t += minute(local);
      //t += " ";
      //t += ampm[isPM(local)];
    }

    void change(int dispAddress, bool is_enabled) {
      if (is_enabled == 1) {
        address = dispAddress;
        display.begin(SSD1306_SWITCHCAPVCC, address);
        display.ssd1306_command(SSD1306_DISPLAYON);
        showDispClear();
        showDispSet("OLED changed");
        showDispDisplay();
        dispEnabled = is_enabled;
        timeClient.begin();
        digClock();
        dispUpdate();
      }
      else {
        DBG_PRINTLN("Display ist aus");
        dispEnabled = is_enabled;
      }
    }
}

oledDisplay = oled();

void turnDisplayOff() {
  cbpiEventSystem(31);
}

void handleRequestDisplay() {
  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject& displayResponse = jsonBuffer.createObject();
  displayResponse["enabled"] = oledDisplay.dispEnabled;
  if (oledDisplay.dispEnabled == 1) {
    displayResponse["displayOn"] = 1;
  }
  else {
    displayResponse["displayOn"] = 0;
  }
  String response;
  displayResponse.printTo(response);
  server.send(200, "application/json", response);
}

void handleRequestDisp() {
  String request = server.arg(0);
  String message;
  if (request == "isEnabled") {
    if (oledDisplay.dispEnabled) {
      message = "1";
    }
    else {
      message = "0";
    }
    goto SendMessage;
  }
  if (request == "address") {
    message = String(decToHex(oledDisplay.address, 2));
    goto SendMessage;
  }

SendMessage:
  server.send(200, "text/plain", message);
}

void handleSetDisp() {

  bool isEnabled = oledDisplay.dispEnabled;
  String dispAddress;
  int address = oledDisplay.address;

  for (int i = 0; i < server.args(); i++) {
    if (server.argName(i) == "enabled") {
      if (server.arg(i) == "1") {
        isEnabled = 1;
      } else {
        isEnabled = 0;
      }
    }

    if (server.argName(i) == "address")  {

      dispAddress = server.arg(i);
      dispAddress.remove(0, 2);
      char copy[4];
      dispAddress.toCharArray(copy, 4);
      address = strtol(copy, 0, 16);
    }
    yield();
  }

  oledDisplay.change(address, isEnabled);

  saveConfig();
}
#endif

void showDispVal(String value)
{
#ifdef DISPLAY
  display.print(value);
  display.display();
#endif
}

void showDispVal(int value)
{
#ifdef DISPLAY
  display.print(value);
  display.display();
#endif
}

void showDispClear()
{
#ifdef DISPLAY
  display.clearDisplay();
  display.display();
#endif
}

void showDispDisplay()
{
#ifdef DISPLAY
  display.display();
#endif
}

void showDispWlan()
{
#ifdef DISPLAY
  if (WiFi.status() == WL_CONNECTED) {
    display.drawBitmap(75, 5, wlan_logo, 20, 20, WHITE);
  }
#endif
}
void showDispMqtt()
{
#ifdef DISPLAY
  if (client.connected()) {
    display.drawBitmap(100, 5, mqtt_logo, 20, 20, WHITE);
  }
#endif
}
void showDispCbpi()
{
#ifdef DISPLAY
  display.clearDisplay();
  display.drawBitmap(39, 7, cbpi_logo, 50, 50, WHITE);
  display.display();
#endif
}

void showDispLines()
{
#ifdef DISPLAY
  display.drawLine(0, 50, 128, 50, WHITE);
  display.drawLine(42, 50, 42, 64, WHITE);
  display.drawLine(84, 50, 84, 64, WHITE);
#endif
}

void showDispSen()
{
#ifdef DISPLAY
  display.setTextSize(1);
  display.setCursor(3, 55);
  display.setTextColor(WHITE);
  // Not yet ready: check sensor status
  //  senOK ? display.print("Sen: Er") : display.print("Sen: ");
  display.print("Sen: ");
  display.print(numberOfSensors);
#endif
}
void showDispAct()
{
#ifdef DISPLAY
  display.setCursor(44, 55);
  display.setTextColor(WHITE);
  // Not yet ready: check actor status
  //  actOK ? display.print("Act: Er") : display.print("Act: ");
  display.print("Act: ");
  display.print(numberOfActors);
#endif
}
void showDispInd()
{
#ifdef DISPLAY
  display.setTextSize(1);
  display.setCursor(86, 55);
  display.setTextColor(WHITE);
  // Not yet ready: check sensor status
  //  indOK ? display.print("Ind: Er") : display.print("Ind: ");
  display.print("Ind: ");
  display.print(inductionCooker.isEnabled);
#endif
}

void showDispTime(String value)
{
#ifdef DISPLAY
  display.setCursor(5, 5);
  display.setTextSize(2);
  //  display.setTextColor(BLACK);
  //  display.print("     ");
  //  display.setCursor(5, 5);
  display.setTextColor(WHITE);
  display.print(value);
#endif
}

void showDispIP(String value)
{
#ifdef DISPLAY
  display.setCursor(5, 30);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print(value);
#endif
}
void showDispSet(String value)
{
#ifdef DISPLAY
  display.clearDisplay();
  display.display();
  display.drawPixel(0, 0, WHITE);
  display.drawPixel(127, 0, WHITE);
  display.drawPixel(0, 63, WHITE);
  display.drawPixel(127, 63, WHITE);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(27, 30);
  display.print(value);
  display.display();
#endif
}
