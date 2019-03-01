/*
  Sensor classes and related functions.
  Currently supports OneWire and PT100/1000 sensors.
*/
class OneWireSensor
{
  private:
    unsigned long lastCalled; // Wann wurde der Sensor zuletzt aktualisiert

  public:
    char sens_mqtttopic[50]; // Für MQTT Kommunikation
    byte sens_address[8];    // 1-Wire Adresse
    String sens_name;        // Name für Anzeige auf Website
    float sens_value;        // Aktueller Wert

    String getSens_address_string()
    {
      return OneWireAddressToString(sens_address);
    }

    OneWireSensor(String address, String mqtttopic, String name)
    {
      change(address, mqtttopic, name);
    }

    void update()
    {
      if (millis() > (lastCalled + DEFAULT_SENSOR_UPDATE_INTERVAL))
      {

        DS18B20.requestTemperatures();
        sens_value = DS18B20.getTempC(sens_address);

        if (sens_value == -127.0)
        {
          Serial.print("OneWire Sensor ");
          Serial.print(sens_name);
          Serial.println(" not found!");
        }
        else
        {
          if (sens_value == 85.0)
          {
            Serial.print("One Wire Sensor ");
            Serial.print(sens_name);
            Serial.println(" Error!");
          }
          else
          {
            publishmqtt();
          }
        }

        lastCalled = millis();
      }
    }

    void change(String new_address, String new_mqtttopic, String new_name)
    {
      new_mqtttopic.toCharArray(sens_mqtttopic, new_mqtttopic.length() + 1);
      sens_name = new_name;
      // if this check fails, this could also mean a call from array init
      // (no actual sensor defined for this array entry, so skip init here)
      if (new_address.length() == 16)
      {
        char address_char[16];

        new_address.toCharArray(address_char, 17);

        char hexbyte[2];
        int octets[8];

        for (int d = 0; d < 16; d += 2)
        {
          // Assemble a digit pair into the hexbyte string
          hexbyte[0] = address_char[d];
          hexbyte[1] = address_char[d + 1];

          // Convert the hex pair to an integer
          sscanf(hexbyte, "%x", &octets[d / 2]);
          yield();
        }
        Serial.print("Starting OneWire sensor: ");
        for (int i = 0; i < 8; i++)
        {
          sens_address[i] = octets[i];
          Serial.print(sens_address[i], HEX);
        }
        Serial.println("");
        DS18B20.setResolution(sens_address, ONE_WIRE_RESOLUTION);
      }
    }

    void publishmqtt()
    {
      if (client.connected())
      {
        client.publish(sens_mqtttopic, getValueString());
      }
    }

    char *getValueString()
    {
      char buf[8];
      dtostrf(sens_value, 3, 2, buf);
      return buf;
    }
};

class PTSensor
{
  private:
    unsigned long lastCalled; // timestamp
    // reference to amplifier board, with initial values (will be overwritten in "constructor")
    Adafruit_MAX31865 maxChip = Adafruit_MAX31865(DEFAULT_CS_PIN, PT_PINS[0], PT_PINS[1], PT_PINS[2]);

  public:
    byte csPin = DEFAULT_CS_PIN; // set to default, otherwise would be "D3" (-> Arduino 0)
    byte numberOfWires;
    char mqttTopic[50]; // topic for mqtt sending
    String name;        // frontend name
    float value;        // current value

    PTSensor(String csPin, byte numberOfWires, String mqtttopic, String name)
    {
      change(csPin, numberOfWires, mqtttopic, name);
    }

    void update()
    {
      if (millis() > (lastCalled + DEFAULT_SENSOR_UPDATE_INTERVAL))
      {
        value = maxChip.temperature(RNOMINAL, RREF);

        // sensor reads very low temps if disconnected
        if (maxChip.readFault() || value < -100)
        {
          value = -127.0;
          maxChip.clearFault();
        }
        else
        {
          publishmqtt();
        }
        lastCalled = millis();
      }
    }

    void change(String newCSPin, byte newNumberOfWires, String newMqttTopic, String newName)
    {
      // check for initial empty array entry initialization
      // (no actual sensor defined in this call)
      if (newCSPin != "")
      {
        byte byteNewCSPin = StringToPin(newCSPin);
        // if this pin fits the bill, go for it..
        if (isPin(byteNewCSPin))
        {
          pins_used[csPin] = false;
          csPin = byteNewCSPin;
          pins_used[csPin] = true;
          newMqttTopic.toCharArray(mqttTopic, newMqttTopic.length() + 1);
          numberOfWires = newNumberOfWires;
          name = newName;
          maxChip = Adafruit_MAX31865(csPin, PT_PINS[0], PT_PINS[1], PT_PINS[2]);
          Serial.print("Starting PT sensor with ");
          Serial.print(newNumberOfWires);
          Serial.print(" wires. CS Pin is ");
          Serial.println(PinToString(csPin));
          if (newNumberOfWires == 4)
          {
            maxChip.begin(MAX31865_4WIRE);
          }
          else if (newNumberOfWires == 3)
          {
            maxChip.begin(MAX31865_3WIRE);
          }
          else
          {
            maxChip.begin(MAX31865_2WIRE); // set to 2 wires as default
          }
        }
      }
    }

    void publishmqtt()
    {
      if (client.connected())
      {
        client.publish(mqttTopic, getValueString());
      }
    }

    char *getValueString()
    {
      char buf[8];
      dtostrf(value, 3, 2, buf);
      return buf;
    }
};

/*
  Initializing Sensor Arrays
  please mind: max sensor capacity is interpreted as max for each type
*/
OneWireSensor oneWireSensors[NUMBER_OF_SENSORS_MAX] = {
  OneWireSensor("", "", ""),
  OneWireSensor("", "", ""),
  OneWireSensor("", "", ""),
  OneWireSensor("", "", ""),
  OneWireSensor("", "", ""),
  OneWireSensor("", "", "")
};

PTSensor ptSensors[NUMBER_OF_SENSORS_MAX] = {
  PTSensor("", 0, "", ""),
  PTSensor("", 0, "", ""),
  PTSensor("", 0, "", ""),
  PTSensor("", 0, "", ""),
  PTSensor("", 0, "", ""),
  PTSensor("", 0, "", ""),
};

/* Called in loop() */
void handleSensors()
{
  for (int i = 0; i < numberOfOneWireSensors; i++)
  {
    oneWireSensors[i].update();
    yield();
  }
  for (int i = 0; i < numberOfPTSensors; i++)
  {
    ptSensors[i].update();
    yield();
  }
}
/* Search the OneWire bus for available sensor addresses */
byte searchOneWireSensors()
{
  byte n = 0;
  byte addr[8];
  while (oneWire.search(addr))
  {
    if (OneWire::crc8(addr, 7) == addr[7])
    {
      Serial.print("Sensor found: ");
      for (int i = 0; i < 8; i++)
      {
        oneWireAddressesFound[n][i] = addr[i];
        Serial.print(addr[i], HEX);
      }
      Serial.println("");
      n += 1;
    }
    yield();
  }
  oneWire.reset_search();
  return n;
}

/* Convert the OneWire sensor address to a (printable) string */
String OneWireAddressToString(byte addr[8])
{
  char charbuffer[50];
  sprintf(charbuffer, "%02x%02x%02x%02x%02x%02x%02x%02x", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
  return charbuffer;
}

/* All following fuctions called from frontend (mapping in 0_Setup.ino) */

/* Update sensor attributes or create new sensor */
void handleSetSensor()
{
  int id = server.arg(0).toInt();
  String type = server.arg(1);

  if (type == SENSOR_TYPE_ONE_WIRE)
  {
    // means: create new sensor request
    if (id == -1)
    {
      id = numberOfOneWireSensors;
      numberOfOneWireSensors += 1;
    }
    String newName = server.arg(2);
    String newTopic = server.arg(3);
    String newAddress = server.arg(4);
    oneWireSensors[id].change(newAddress, newTopic, newName);
  }
  else if (type == SENSOR_TYPE_PT)
  {
    // means: create new sensor request
    if (id == -1)
    {
      id = numberOfPTSensors;
      numberOfPTSensors += 1;
    }
    String newName = server.arg(2);
    String newTopic = server.arg(3);
    String newCsPin = server.arg(4);
    byte newNumberOfWires = server.arg(5).toInt();
    ptSensors[id].change(newCsPin, newNumberOfWires, newTopic, newName);
  }
  // unknown type
  else
  {
    server.send(400, "text/plain", "Unknown Sensor Type");
    return;
  }
  // all done, save and exit
  saveConfig();
  server.send(201, "text/plain", "created");
}

/* Delete a sensor. TODO: Check, if sensor existed */
void handleDelSensor()
{
  int id = server.arg(0).toInt();
  String type = server.arg(1);
  // OneWire
  if (type == SENSOR_TYPE_ONE_WIRE)
  {
    // move all sensors following the given id one to the front of array,
    // effectively overwriting the sensor to be deleted..
    for (int i = id; i < numberOfOneWireSensors; i++)
    {
      oneWireSensors[i].change(oneWireSensors[i + 1].getSens_address_string(), oneWireSensors[i + 1].sens_mqtttopic, oneWireSensors[i + 1].sens_name);
      yield();
    }
    // ..and declare the array's content to one sensor less
    numberOfOneWireSensors -= 1;
  }
  else if (type == SENSOR_TYPE_PT)
  {
    // first declare the pin unused
    pins_used[ptSensors[id].csPin] = false;
    // move all sensors following the given id one to the front of array,
    // effectively overwriting the sensor to be deleted..
    for (int i = id; i < numberOfPTSensors; i++)
    {
      String csPinString = String(ptSensors[i + 1].csPin); // yeah, not very nice or efficient..
      ptSensors[i].change(csPinString, ptSensors[i + 1].numberOfWires, ptSensors[i + 1].mqttTopic, ptSensors[i + 1].name);
      yield();
    }
    // ..and declare the array's content to one sensor less
    numberOfPTSensors -= 1;
  }
  // unknown type
  else
  {
    server.send(400, "text/plain", "Unknown Sensor Type");
    return;
  }
  // all done, save and exit
  saveConfig();
  server.send(200, "text/plain", "deleted");
}

/* Provides search results of OneWire bus search */
void handleRequestOneWireSensorAddresses()
{
  numberOfOneWireSensorsFound = searchOneWireSensors();
  int id = server.arg(0).toInt();
  String message = "";
  // if id given, render this sensor's address first
  // and check if id is valid (client could send nonsense for id..)
  if (id != -1 && id < numberOfOneWireSensors)
  {
    message += F("<option>");
    message += oneWireSensors[id].getSens_address_string();
    message += F("</option><option disabled>──────────</option>");
  }
  // Now render all found addresses, except the one already assigned to the sensor
  for (int i = 0; i < numberOfOneWireSensorsFound; i++)
  {
    String foundAddress = OneWireAddressToString(oneWireAddressesFound[i]);
    if (id == -1 || !(oneWireSensors[id].getSens_address_string() == foundAddress))
    {
      message += F("<option>");
      message += foundAddress;
      message += F("</option>");
    }
    yield();
  }
  server.send(200, "text/html", message);
}

/* Similar as the actor pin request function, but this time for PT sensors */
void handleRequestPtSensorPins()
{
  int id = server.arg(0).toInt();
  String message;

  if (id != -1)
  {
    message += F("<option>");
    message += PinToString(ptSensors[id].csPin);
    message += F("</option><option disabled>──────────</option>");
  }
  for (int i = 0; i < NUMBER_OF_PINS; i++)
  {
    if (pins_used[PINS[i]] == false)
    {
      message += F("<option>");
      message += PIN_NAMES[i];
      message += F("</option>");
    }
    yield();
  }
  server.send(200, "text/plain", message);
}

/*
  Returns a JSON Array with the current sensor data of all sensors.
*/
void handleRequestSensors()
{
  StaticJsonBuffer<1024> jsonBuffer;
  JsonArray &sensorsResponse = jsonBuffer.createArray();

  for (int i = 0; i < numberOfOneWireSensors; i++)
  {
    JsonObject &sensorResponse = jsonBuffer.createObject();
    ;
    sensorResponse["name"] = oneWireSensors[i].sens_name;
    if ((oneWireSensors[i].sens_value != -127.0) && (oneWireSensors[i].sens_value != 85.0))
    {
      sensorResponse["value"] = oneWireSensors[i].getValueString();
    }
    else
    {
      sensorResponse["value"] = "ERR";
    }
    sensorResponse["mqtt"] = oneWireSensors[i].sens_mqtttopic;
    sensorResponse["type"] = SENSOR_TYPE_ONE_WIRE;
    sensorResponse["id"] = i;
    sensorsResponse.add(sensorResponse);
    yield();
  }
  for (int i = 0; i < numberOfPTSensors; i++)
  {
    JsonObject &sensorResponse = jsonBuffer.createObject();
    sensorResponse["name"] = ptSensors[i].name;
    // While it is technically possible to read this low value with a PT sensor
    // We reuse "OneWire Error Codes" here for simplicity
    if (ptSensors[i].value != -127.0)
    {
      sensorResponse["value"] = ptSensors[i].getValueString();
    }
    else
    {
      sensorResponse["value"] = "ERR";
    }
    sensorResponse["mqtt"] = ptSensors[i].mqttTopic;
    sensorResponse["type"] = SENSOR_TYPE_PT;
    sensorResponse["id"] = i;
    sensorsResponse.add(sensorResponse);
    yield();
  }
  String response;
  sensorsResponse.printTo(response);
  server.send(200, "application/json", response);
}

/* Returns information for one specific sensor (for update / delete menu in frontend) */
void handleRequestSensorConfig()
{
  int id = server.arg(0).toInt();
  String type = server.arg(1);
  String request = server.arg(2);
  String response;

  if (type == SENSOR_TYPE_ONE_WIRE)
  {
    if (id == -1 || id > numberOfOneWireSensors)
    {
      response = "not found";
      server.send(404, "text/plain", response);
      return;
    }
    else
    {
      StaticJsonBuffer<256> jsonBuffer;
      JsonObject &sensorJson = jsonBuffer.createObject();
      sensorJson["name"] = oneWireSensors[id].sens_name;
      sensorJson["topic"] = oneWireSensors[id].sens_mqtttopic;
      sensorJson.printTo(response);
      server.send(200, "application/json", response);
      return;
    }
  }
  else if (type == SENSOR_TYPE_PT)
  {
    if (id == -1 || id > numberOfPTSensors)
    {
      response = "not found";
      server.send(404, "text/plain", response);
      return;
    }
    else
    {
      StaticJsonBuffer<256> jsonBuffer;
      JsonObject &sensorJson = jsonBuffer.createObject();
      sensorJson["name"] = ptSensors[id].name;
      sensorJson["topic"] = ptSensors[id].mqttTopic;
      sensorJson["csPin"] = PinToString(ptSensors[id].csPin);
      sensorJson["numberOfWires"] = ptSensors[id].numberOfWires;
      sensorJson.printTo(response);
      server.send(200, "application/json", response);
      return;
    }
  }
  response = "unknown type: ";
  response += type;
  server.send(406, "text/plain", response);
  return;
}

byte convertCharToHex(char ch)
{
  byte returnType;
  switch (ch)
  {
    case '0':
      returnType = 0;
      break;
    case '1':
      returnType = 1;
      break;
    case '2':
      returnType = 2;
      break;
    case '3':
      returnType = 3;
      break;
    case '4':
      returnType = 4;
      break;
    case '5':
      returnType = 5;
      break;
    case '6':
      returnType = 6;
      break;
    case '7':
      returnType = 7;
      break;
    case '8':
      returnType = 8;
      break;
    case '9':
      returnType = 9;
      break;
    case 'A':
      returnType = 10;
      break;
    case 'B':
      returnType = 11;
      break;
    case 'C':
      returnType = 12;
      break;
    case 'D':
      returnType = 13;
      break;
    case 'E':
      returnType = 14;
      break;
    case 'F':
      returnType = 15;
      break;
    default:
      returnType = 0;
      break;
  }
  return returnType;
}
