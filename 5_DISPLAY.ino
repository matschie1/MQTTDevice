#if (DISPLAY == 1)
class oled
{
    String t;

  public:
    bool dispEnabled = 0;
    int address;

    bool senOK = 0;
    bool actOK = 0;
    bool indOK = 0;

    oled() {
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

    void change(int dispAddress, bool is_enabled) {
      if (is_enabled == 1) {
        DBG_PRINTLN("OLED display configuration changed");
        address = dispAddress;
        display.begin(SSD1306_SWITCHCAPVCC, address, true);
        display.ssd1306_command(SSD1306_DISPLAYON);
        display.clearDisplay();
        display.display();
        dispEnabled = is_enabled;
        timeClient.begin();
        digClock();
      }
      else {
        dispEnabled = is_enabled;
      }
    }

    void digClock()
    {
      t = ""; // clear value for display

      // update the NTP client and get the UNIX UTC timestamp
      if (!timeClient.update())
      {
        //delay(100);           // if network is up and running just wait a second for NTP
        unsigned long pause = millis();
        while (millis() < pause + 100) {
          //wait approx. [period] ms
        }
        timeClient.update();
      }
      unsigned long epochTime =  timeClient.getEpochTime();

      // convert received time stamp to time_t object
      time_t local, utc;
      utc = epochTime;
      TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 0};     //Central European Summer Time
      TimeChangeRule CET = {"CET", Last, Sun, Oct, 3, 0};       //Central European Standard Time
      Timezone CE(CEST, CET );
      local = CE.toLocal(utc);
      setTime(local);

      t += hour(local);
      t += ":";
      if (minute(local) < 10) // add a zero if minute is under 10
        t += "0";
      t += minute(local);
    }
}
#endif

#if (DISPLAY == 1)
oledDisplay = oled();
#endif

void turnDisplayOff() {
#if (DISPLAY == 1)
  cbpiEventSystem(31);
#endif
}

void handleRequestDisplay() {
  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject& displayResponse = jsonBuffer.createObject();
#if (DISPLAY == 1)
  displayResponse["enabled"] = oledDisplay.dispEnabled;
  if (oledDisplay.dispEnabled == 1) {
    displayResponse["displayOn"] = 1;
  }
  else {
    displayResponse["displayOn"] = 0;
  }

#else
  displayResponse["enabled"] = 0;
  displayResponse["displayOn"] = 0;
#endif

  String response;
  displayResponse.printTo(response);
  server.send(200, "application/json", response);
}

void handleRequestDisp() {
  String request = server.arg(0);
  String message;
  if (request == "isEnabled") {
#if (DISPLAY == 1)
    if (oledDisplay.dispEnabled) {
      message = "1";
    }
    else {
      message = "0";
    }
#else
    message = "0";
#endif

    goto SendMessage;
  }
  if (request == "address") {
#if (DISPLAY == 1)
    message = String(decToHex(oledDisplay.address, 2));
#else
    message = "0";
#endif
    goto SendMessage;
  }

SendMessage:
  server.send(200, "text/plain", message);
}

void handleSetDisp() {

  bool isEnabled;

  String dispAddress;
  int address;
#if (DISPLAY == 1)
  address = oledDisplay.address;
  isEnabled = oledDisplay.dispEnabled;
#else
  address = 0;
  isEnabled = 0;
#endif

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
#if (DISPLAY == 1)
  oledDisplay.change(address, isEnabled);
#endif
  saveConfig();
}

void dispAPMode() {             // show screen in AP Mode
#if (DISPLAY == 1)
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile)
  {
    display.begin(SSD1306_SWITCHCAPVCC, DISP_DEF_ADDRESS, true);
    display.ssd1306_command(SSD1306_DISPLAYON);
    display.clearDisplay();
    display.display();
    showDispAP("AP Mode");
  }
#endif
}

void dispSTAMode() {          // Start screen in station mode
#if (DISPLAY == 1)
  display.begin(SSD1306_SWITCHCAPVCC, DISP_DEF_ADDRESS, true);
  display.ssd1306_command(SSD1306_DISPLAYON);
  display.clearDisplay();
  display.display();
  showDispSet("Setup");
  unsigned long last = 0;
  if (millis() > last + 1000)
  {
    // just wait a sec for WiFiManager otherwise 0.0.0.0 will be returend for localIP will be
  }
  showDispVal(WiFi.localIP().toString());
#endif
}

/* ######### Display functions ######### */

void showDispClear()              // Clear Display
{
#if (DISPLAY == 1)
  display.clearDisplay();
  display.display();
#endif
}

void showDispDisplay()            // Show
{
#if (DISPLAY == 1)
  display.display();
#endif
}

void showDispVal(String value)    // Display a String value
{
#if (DISPLAY == 1)
  display.print(value);
  display.display();
#endif
}

void showDispVal(int value)       // Display a Int value
{
#if (DISPLAY == 1)
  display.print(value);
  display.display();
#endif
}

void showDispWlan()               // Show WLAN icon
{
#if (DISPLAY == 1)
  if (WiFi.status() == WL_CONNECTED) {
    display.drawBitmap(77, 3, wlan_logo, 20, 20, WHITE);
  }
#endif
}
void showDispMqtt()               // SHow MQTT icon
{
#if (DISPLAY == 1)
  if (client.connected()) {
    display.drawBitmap(102, 3, mqtt_logo, 20, 20, WHITE);
  }
#endif
}
void showDispCbpi()               // SHow CBPI icon
{
#if (DISPLAY == 1)
  display.clearDisplay();
  //display.drawBitmap(39, 7, cbpi_logo, 50, 50, WHITE);
  display.drawBitmap(41, 0, cbpi_logo, 50, 50, WHITE);
  display.display();
#endif
}

void showDispLines()              // Draw lines in the bottom
{
#if (DISPLAY == 1)
  display.drawLine(0, 50, 128, 50, WHITE);
  display.drawLine(42, 50, 42, 64, WHITE);
  display.drawLine(84, 50, 84, 64, WHITE);
#endif
}

void showDispSen()                // Show Sensor status on the left
{
#if (DISPLAY == 1)
  display.setTextSize(1);
  display.setCursor(3, 55);
  display.setTextColor(WHITE);
  oledDisplay.senOK ? display.print("Sen:Er") : display.print("Sen:ok");
#endif
}
void showDispAct()                // Show actor status in the mid
{
#if (DISPLAY == 1)
  display.setCursor(45, 55);
  display.setTextColor(WHITE);
  oledDisplay.actOK ? display.print("Act:Er") : display.print("Act:ok");
#endif
}
void showDispInd()                // Show InductionCooker status on the right
{
#if (DISPLAY == 1)
  display.setTextSize(1);
  display.setCursor(87, 55);
  display.setTextColor(WHITE);
  oledDisplay.indOK ? display.print("Ind:Er") : display.print("Ind:ok");
#endif
}

void showDispTime(String value)   // Show time value in the upper left with fontsize 2
{
#if (DISPLAY == 1)
  display.setCursor(5, 5);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.print(value);
#endif
}

void showDispIP(String value)      // Show IP address under time value with fontsize 1
{
#if (DISPLAY == 1)
  display.setCursor(5, 30);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print(value);
#endif
}
void showDispSet(String value)    // Show current station mode
{
#if (DISPLAY == 1)
  showDispCbpi();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(8, 54);
  display.print(value);
  display.display();
#endif
}

void showDispAP(String value)       // Show AP mode
{
#if (DISPLAY == 1)
  //display.clearDisplay();
  showDispCbpi();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(8, 54);
  display.print(value);
  display.print(": 192.168.4.1");
  //display.print(WiFi.softAPIP().toString());
  display.display();
#endif
}
