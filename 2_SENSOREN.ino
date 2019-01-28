class TemperatureSensor
{
  public:
    char sens_mqtttopic[50];      // Für MQTT Kommunikation
    byte sens_address[8];         // 1-Wire Adresse
    String sens_name;             // Name für Anzeige auf Website
    float sens_value = -127.0;    // Aktueller Wert
    bool sens_isConnected;        // check if Sensor is connected


    String getSens_adress_string() {
      return SensorAddressToString(sens_address);
    }

    TemperatureSensor(String new_address, String new_mqtttopic, String new_name) {
      change(new_address, new_mqtttopic, new_name);
    }

    void Update() {
        DS18B20.requestTemperatures(); // new conversion to get recent temperatures
        if (sens_value == 85.0) { // can be real 85 degrees or reset default temp or an error value eg cable too long
          unsigned long pause = millis();
          while (millis() < pause + 750) {
            //wait approx. 750ms
          }
          DS18B20.requestTemperatures();
        }
        sens_isConnected = DS18B20.isConnected(sens_address); // attempt to determine if the device at the given address is connected to the bus
        sens_isConnected ? sens_value = DS18B20.getTempC(sens_address) : sens_value = -127.0;

        DBG_PRINT(sens_name);
        DBG_PRINT(" is connected: ");
        DBG_PRINT(sens_isConnected);
        DBG_PRINT(" sensor address: ");
        for (int i = 0; i < 8; i++) {
          DBG_PRINTHEX(sens_address[i]);
          DBG_PRINT(" ");
        }
        DBG_PRINT(" sensor value: ");
        DBG_PRINT(sens_value);
        if ( OneWire::crc8( sens_address, 7) != sens_address[7]) {
          DBG_PRINTLN(" CRC check failed");
          sensorsStatus = 1;
        }
        //else DBG_PRINTLN(" CRC check ok");
        //if (sens_value == -127.0 || sens_value == 85.0) {
        else if (sens_value == -127.0 || sens_value == 85.0) {
          if (sens_isConnected && sens_address[0] != 0xFF) { // Sensor connected AND sensor address exists (not default FF)
            DBG_PRINT(sens_name);
            DBG_PRINTLN(" is connected and has a valid ID, but temperature is -127 -  error, device not found");
            sensorsStatus = 2;
          }
          else if (!sens_isConnected && sens_address[0] != 0xFF) { // Sensor with valid address not connected
            DBG_PRINT(sens_name);
            DBG_PRINTLN(" is not connected, has no sensor value and device ID is not valid - unplugged?");
            sensorsStatus = 3;
          }
          else {// not connected and unvalid address
            sensorsStatus = 4;
          } // sens_isConnected
        } // sens_value -127 || +85
        else sensorsStatus = 0;
        publishmqtt();
    } // void Update

    void change(String new_address, String new_mqtttopic, String new_name) {
      new_mqtttopic.toCharArray(sens_mqtttopic, new_mqtttopic.length() + 1);
      sens_name = new_name;
      if (new_address.length() == 16) {
        char address_char[16];

        new_address.toCharArray(address_char, 17);

        char hexbyte[2];
        int octets[8] ;

        for ( int d = 0; d < 16; d += 2 )
        {
          // Assemble a digit pair into the hexbyte string
          hexbyte[0] = address_char[d] ;
          hexbyte[1] = address_char[d + 1] ;

          // Convert the hex pair to an integer
          sscanf( hexbyte, "%x", &octets[d / 2] ) ;
          yield();

        }
        for (int i = 0; i < 8; i++) {
          sens_address[i] = octets[i];
          DBG_PRINTHEX(sens_address[i]);
        }
        DBG_PRINTLN("");
      }
      DS18B20.setResolution(sens_address, 10);
    }

    void publishmqtt() {
      if (client.connected()) {
        StaticJsonBuffer<256> jsonBuffer;
        JsonObject& json = jsonBuffer.createObject();

        json["Name"] = sens_name;
        JsonObject& Sensor = json.createNestedObject("Sensor");
        Sensor["Value"] = sens_value;
        Sensor["Type"] = "1-wire";

        char jsonMessage[100];
        json.printTo(jsonMessage);
        client.publish(sens_mqtttopic, jsonMessage);
      }
    }

    char* getValueString() {
      char buf[5];
      dtostrf(sens_value, 2, 1, buf);
      return buf;
    }
};

/* Initialisierung des Arrays */
TemperatureSensor sensors[numberOfSensorsMax] = {
  TemperatureSensor("", "", ""),
  TemperatureSensor("", "", ""),
  TemperatureSensor("", "", ""),
  TemperatureSensor("", "", ""),
  TemperatureSensor("", "", ""),
  TemperatureSensor("", "", ""),
  TemperatureSensor("", "", ""),
  TemperatureSensor("", "", ""),
  TemperatureSensor("", "", ""),
  TemperatureSensor("", "", "")
};

/* Funktion für Loop */
void handleSensors() {
  for (int i = 0; i < numberOfSensors; i++) {
    sensors[i].Update();
    yield();
  }
}

byte searchSensors() {
  byte i;
  byte n = 0;
  byte addr[8];

  while (oneWire.search(addr)) {

    if ( OneWire::crc8( addr, 7) == addr[7]) {
      DBG_PRINT("Sensor found:");
      for ( i = 0; i < 8; i++) {
        addressesFound[n][i] = addr[i];
        DBG_PRINTHEX(addr[i]);
        DBG_PRINT(" ");
      }
      DBG_PRINTLN("");
      n += 1;
    }
    yield();
  }
  return n;
  oneWire.reset_search();
}

String SensorAddressToString(byte addr[8]) {
  char charbuffer[50];
  String AddressString;

  sprintf( charbuffer, "%02x%02x%02x%02x%02x%02x%02x%02x", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7] );
  //  for (int i = 0; i < 8; i++) {
  //    AddressString += addr[i];
  //  }
  return charbuffer;
}

/* Funktionen für Web */

// Sensor wird geändert
void handleSetSensor() {
  int id = server.arg(0).toInt();

  if (id == -1) {
    id = numberOfSensors;
    numberOfSensors += 1;
  }

  String new_mqtttopic = sensors[id].sens_mqtttopic;
  String new_name = sensors[id].sens_name;
  String new_address = sensors[id].getSens_adress_string();

  for (int i = 0; i < server.args(); i++) {
    if (server.argName(i) == "name") {
      new_name = server.arg(i);
    }
    if (server.argName(i) == "topic")  {
      new_mqtttopic = server.arg(i);
    }
    if (server.argName(i) == "address")  {
      new_address = server.arg(i);
    }
    yield();
  }

  sensors[id].change(new_address, new_mqtttopic, new_name);

  saveConfig();
  server.send(201, "text/plain", "created");
}

void handleDelSensor() {
  int id = server.arg(0).toInt();

  //  Alle einen nach vorne schieben
  for (int i = id; i < numberOfSensors; i++) {
    sensors[i].change(sensors[i + 1].getSens_adress_string(), sensors[i + 1].sens_mqtttopic, sensors[i + 1].sens_name);
  }

  // den letzten löschen
  numberOfSensors -= 1;
  saveConfig();
  server.send(200, "text/plain", "deleted");
}

void handleRequestSensorAddresses() {
  numberOfSensorsFound = searchSensors();
  int id = server.arg(0).toInt();
  String message;
  if ( id != -1 ) {
    message += F("<option>");
    message += SensorAddressToString(sensors[id].sens_address);
    message += F("</option><option disabled>──────────</option>");
  }
  for (int i = 0; i < numberOfSensorsFound; i++) {
    message += F("<option>");
    message += SensorAddressToString(addressesFound[i]);
    message += F("</option>");
    yield();
  }
  server.send(200, "text/html", message);
}

void handleRequestSensors() {
  StaticJsonBuffer<1024> jsonBuffer;
  JsonArray& sensorsResponse = jsonBuffer.createArray();

  for (int i = 0; i < numberOfSensors; i++) {
    JsonObject& sensorResponse = jsonBuffer.createObject();;
    sensorResponse["name"] = sensors[i].sens_name;
    //if ((sensors[i].sens_value != -127.0) && (sensors[i].sens_value != 85.0)) {
    if (sensors[i].sens_value != -127.0) {
      sensorResponse["value"] = sensors[i].getValueString();
    } else {
      sensorResponse["value"] = "ERR";
    }
    sensorResponse["mqtt"] = sensors[i].sens_mqtttopic;
    sensorsResponse.add(sensorResponse);
    yield();
  }

  String response;
  sensorsResponse.printTo(response);
  server.send(200, "application/json", response);
}

void handleRequestSensor() {
  int id = server.arg(0).toInt();
  String request = server.arg(1);
  String message;

  if (id == -1) {
    message = "";
    goto SendMessage;
  } else {
    if (request == "name") {
      message = sensors[id].sens_name;
      goto SendMessage;
    }
    if (request == "script") {
      message = sensors[id].sens_mqtttopic;
      goto SendMessage;
    }
    message = "not found";
  }
  saveConfig();
SendMessage:
  server.send(200, "text/plain", message);
}

byte convertCharToHex(char ch)
{
  byte returnType;
  switch (ch)
  {
    case '0':
      returnType = 0;
      break;
    case  '1' :
      returnType = 1;
      break;
    case  '2':
      returnType = 2;
      break;
    case  '3':
      returnType = 3;
      break;
    case  '4' :
      returnType = 4;
      break;
    case  '5':
      returnType = 5;
      break;
    case  '6':
      returnType = 6;
      break;
    case  '7':
      returnType = 7;
      break;
    case  '8':
      returnType = 8;
      break;
    case  '9':
      returnType = 9;
      break;
    case  'A':
      returnType = 10;
      break;
    case  'B':
      returnType = 11;
      break;
    case  'C':
      returnType = 12;
      break;
    case  'D':
      returnType = 13;
      break;
    case  'E':
      returnType = 14;
      break;
    case  'F' :
      returnType = 15;
      break;
    default:
      returnType = 0;
      break;
  }
  return returnType;
}
