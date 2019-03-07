class oled
{
    String t;
    unsigned long lastNTPupdate = 0;

  public:
    bool dispEnabled = 0;
    int address;

    bool senOK = true;
    bool actOK = true;
    bool indOK = true;
    bool wlanOK = true;
    bool mqttOK = false;

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
      if (is_enabled == 1 && dispAddress != 0) {
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
      time_t local, utc;
      if (millis() > (lastNTPupdate + NTP_INTERVAL))
      {
        cbpiEventSystem(EM_NTP);
        lastNTPupdate = millis();
      }
      unsigned long epochTime =  timeClient.getEpochTime();
      // convert received time stamp to time_t object
      // time_t local, utc;
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

oledDisplay = oled();

void turnDisplayOff() {
  if (oledDisplay.dispEnabled) {
    DBG_PRINTLN("Switch OLED display off");
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    oledDisplay.dispEnabled = 0;
  }
  else {
    if (oledDisplay.address != 0) {
      DBG_PRINTLN("Switch OLED display on");
      display.ssd1306_command(SSD1306_DISPLAYON);
      oledDisplay.dispEnabled = 1;
    }
  }
}

void handleRequestDisplay() {
  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject& displayResponse = jsonBuffer.createObject();
  displayResponse["enabled"] = 0;
  displayResponse["displayOn"] = 0;
  displayResponse["enabled"] = oledDisplay.dispEnabled;
  displayResponse["updisp"] = DISP_UPDATE;
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
    message = "0";
    if (oledDisplay.dispEnabled) {
      message = "1";
    }
    goto SendMessage;
  }
  if (request == "address") {
    message = "0";
    message = String(decToHex(oledDisplay.address, 2));
    goto SendMessage;
  }
  if (request == "updisp") {
    message = DISP_UPDATE / 1000;
    goto SendMessage;
  }
SendMessage:
  server.send(200, "text/plain", message);
}

void handleSetDisp() {
  String dispAddress;
  int address;
  address = oledDisplay.address;
  for (int i = 0; i < server.args(); i++) {
    if (server.argName(i) == "enabled") {
      if (server.arg(i) == "1") {
        oledDisplay.dispEnabled = 1;
      } else {
        oledDisplay.dispEnabled = 0;
      }
    }
    if (server.argName(i) == "address")  {

      dispAddress = server.arg(i);
      dispAddress.remove(0, 2);
      char copy[4];
      dispAddress.toCharArray(copy, 4);
      address = strtol(copy, 0, 16);
    }
    if (server.argName(i) == "updisp")  {
      int newdup = server.arg(i).toInt();
      if (newdup > 0) {
        DISP_UPDATE = newdup * 1000;
      }
    }
    yield();
  }
  oledDisplay.change(address, oledDisplay.dispEnabled);
  saveConfig();
}

void dispStartScreen()               // Show Startscreen
{
  if (useDisplay) {
    if (oledDisplay.dispEnabled == 1 && oledDisplay.address != 0) {
      showDispCbpi();
      showDispSTA();
      showDispDisplay();
    }
  }
}

/* ######### Display functions ######### */
void showDispClear()              // Clear Display
{
  display.clearDisplay();
  display.display();
}

void showDispDisplay()            // Show
{
  display.display();
}

void showDispVal(String value)    // Display a String value
{
  display.print(value);
  display.display();
}

void showDispVal(int value)       // Display a Int value
{
  display.print(value);
  display.display();
}

void showDispWlan()               // Show WLAN icon
{
  if (oledDisplay.wlanOK) {
    display.drawBitmap(77, 3, wlan_logo, 20, 20, WHITE);
  }
}
void showDispMqtt()               // SHow MQTT icon
{
  if (oledDisplay.mqttOK) {
    display.drawBitmap(102, 3, mqtt_logo, 20, 20, WHITE);
  }
}

void showDispOTA(unsigned int progress, unsigned int total)               // Show OTA icon
{
  int otaStatus = progress / (total / 100);
  bool up = false;
  switch (otaStatus) {
    case 0:
    case 20:
    case 40:
    case 60:
    case 80:
    case 100:
      up = true;
      break;
    default:
      up = false;
      break;
  }
  if (up) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(5, 5);
    display.print("OTA Update: ");
    display.print(otaStatus);
    int xoffset = 5;
    display.drawRect( xoffset, 25, otaStatus + xoffset, 2, WHITE);
    display.fillRect( xoffset, 25, otaStatus + xoffset, 2, WHITE);
    if (otaStatus > 80) {
      display.setCursor(5, 40);
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.print("OTA Update finished");
      display.setCursor(50, 50);
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.print("... reboot");
    }
    display.display();
  }
}

void showDispOTAEr(String value)
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(5, 5);
  display.print("OTA Update Error: ");
  display.print(value);
  display.display();
}


void showDispCbpi()               // SHow CBPI icon
{
  display.clearDisplay();
  display.drawBitmap(41, 0, cbpi_logo, 50, 50, WHITE);
}

void showDispLines()              // Draw lines in the bottom
{
  display.drawLine(0, 50, 128, 50, WHITE);
  display.drawLine(42, 50, 42, 64, WHITE);
  display.drawLine(84, 50, 84, 64, WHITE);
}

void showDispSen()                // Show Sensor status on the left
{
  display.setTextSize(1);
  display.setCursor(3, 55);
  display.setTextColor(WHITE);
  //oledDisplay.senOK ? display.print("Sen:ok") : display.print("Sen:Er");
  if (sensorsStatus == 0) {
    display.print("Sen: ");
    display.print(numberOfSensors);
  }
  else display.print("Sen:Er");
}
void showDispAct()                // Show actor status in the mid
{
  display.setCursor(45, 55);
  display.setTextColor(WHITE);
  if (actorsStatus == 0) {
    display.print("Act: ");
    display.print(numberOfActors);
  }
  else display.print("Act:Er");
}
void showDispInd()                // Show InductionCooker status on the right
{
  display.setTextSize(1);
  display.setCursor(87, 55);
  display.setTextColor(WHITE);
  if (inductionStatus == 0) display.print("Ind:ok");
  else display.print("Ind:Er");
}

void showDispTime(String value)   // Show time value in the upper left with fontsize 2
{
  display.setCursor(5, 5);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.print(value);
}

void showDispIP(String value)      // Show IP address under time value with fontsize 1
{
  display.setCursor(5, 30);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print(value);
}

void showDispSet(String value)    // Show current station mode
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(5, 30);
  display.print(value);
  display.display();
}

void showDispSet()                // Show current station mode
{
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(1, 54);
  display.print("SET ");
  display.print(WiFi.localIP().toString());
  display.display();
}

void showDispSTA()               // Show AP mode
{
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(8, 54);
  display.print("STA ");
  display.print(WiFi.localIP().toString());
  display.display();
}
