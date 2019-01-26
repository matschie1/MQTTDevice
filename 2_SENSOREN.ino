/*
  Sensor classes and related functions.
  Currently supports OneWire and PT100/1000 sensors.
*/
class OneWireSensor
{
    unsigned long lastCalled;                  // Wann wurde der Sensor zuletzt aktualisiert
    long period = defaultSensorUpdateInterval; // Aktualisierungshäufigkeit

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
      if (millis() > (lastCalled + period))
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
        StaticJsonBuffer<256> jsonBuffer;
        JsonObject &json = jsonBuffer.createObject();

        json["Name"] = sens_name;
        JsonObject &Sensor = json.createNestedObject("Sensor");
        Sensor["Value"] = sens_value;
        Sensor["Type"] = "1-wire";

        char jsonMessage[100];
        json.printTo(jsonMessage);
        client.publish(sens_mqtttopic, jsonMessage);
      }
    }

    char *getValueString()
    {
      char buf[5];
      dtostrf(sens_value, 2, 1, buf);
      return buf;
    }
};

class PTSensor
{
    unsigned long lastCalled;                  // timestamp
    long period = defaultSensorUpdateInterval; // update interval

    // reference to amplifier board, with initial values (will be overwritten in "constructor")
    Adafruit_MAX31865 maxChip = Adafruit_MAX31865(DEFAULT_CS_PIN, PTPins[0], PTPins[1], PTPins[2]);

  public:
    byte csPin;
    byte numberOfWires;
    char mqttTopic[50]; // topic for mqtt sending
    String name;        // frontend name
    float value;        // current value

    PTSensor(byte csPin, byte numberOfWires, String mqtttopic, String name)
    {
      change(csPin, numberOfWires, mqtttopic, name);
    }

    void update()
    {
      if (millis() > (lastCalled + period))
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

    void change(byte newCSPin, byte newNumberOfWires, String newMqttTopic, String newName)
    {
      // check for initial empty array entry initialization
      // (no actual sensor defined in this call)
      if (newCSPin != NO_PT_SENSOR)
      {
        newMqttTopic.toCharArray(mqttTopic, newMqttTopic.length() + 1);
        csPin = newCSPin;
        numberOfWires = newNumberOfWires;
        name = newName;
        // currently, CS = D0 = 16 (NodeMCU Dev Board) TODO!!
        csPin = 16;
        maxChip = Adafruit_MAX31865(newCSPin, PTPins[0], PTPins[1], PTPins[2]);
        Serial.print("Starting PT100 with ");
        Serial.print(newNumberOfWires);
        Serial.print(" wires. CS Pin is ");
        Serial.println(newCSPin);

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

    void publishmqtt()
    {
      if (client.connected())
      {
        StaticJsonBuffer<256> jsonBuffer;
        JsonObject &json = jsonBuffer.createObject();

        json["Name"] = name;
        JsonObject &Sensor = json.createNestedObject("Sensor");
        Sensor["Value"] = value;
        Sensor["Type"] = "PTSensor";

        char jsonMessage[100];
        json.printTo(jsonMessage);
        client.publish(mqttTopic, jsonMessage);
      }
    }

    char *getValueString()
    {
      char buf[5];
      dtostrf(value, 2, 1, buf);
      return buf;
    }
};

/*
  Initializing Arrays
  Please mind: max sensor capacity currently is interpreted as max of each type
*/
OneWireSensor oneWireSensors[numberOfSensorsMax] = {
  OneWireSensor("", "", ""),
  OneWireSensor("", "", ""),
  OneWireSensor("", "", ""),
  OneWireSensor("", "", ""),
  OneWireSensor("", "", ""),
  OneWireSensor("", "", "")
};

PTSensor ptSensors[numberOfSensorsMax] = {
  PTSensor(NO_PT_SENSOR, NO_PT_SENSOR, "", ""),
  PTSensor(NO_PT_SENSOR, NO_PT_SENSOR, "", ""),
  PTSensor(NO_PT_SENSOR, NO_PT_SENSOR, "", ""),
  PTSensor(NO_PT_SENSOR, NO_PT_SENSOR, "", ""),
  PTSensor(NO_PT_SENSOR, NO_PT_SENSOR, "", ""),
  PTSensor(NO_PT_SENSOR, NO_PT_SENSOR, "", ""),
};

/* Called in loop() */
void handleSensors()
{
  for (int i = 0; i < numberOfOneWireSensors; i++)
  {
    oneWireSensors[i].update();
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
  int type = server.arg(1).toInt();

  if (type == SENSOR_TYPE_ONE_WIRE)
  {
    if (id == -1)
    {
      // means: create new sensor request
      id = numberOfOneWireSensors;
      numberOfOneWireSensors += 1;
    }
    // Copy old values (TODO: needed?)
    String new_mqtttopic = oneWireSensors[id].sens_mqtttopic;
    String new_name = oneWireSensors[id].sens_name;
    String new_address = oneWireSensors[id].getSens_address_string();
    // TODO: server arguments should be ordered, no need to loop
    for (int i = 0; i < server.args(); i++)
    {
      if (server.argName(i) == "name")
      {
        new_name = server.arg(i);
      }
      if (server.argName(i) == "topic")
      {
        new_mqtttopic = server.arg(i);
      }
      if (server.argName(i) == "address")
      {
        new_address = server.arg(i);
      }
      yield();
    }
    oneWireSensors[id].change(new_address, new_mqtttopic, new_name);
  }
  else if (type == SENSOR_TYPE_PT)
  {
    server.send(400, "text/plain", "Not Yet Implemented :-(");
  }
  // unknown type
  else
  {
    server.send(400, "text/plain", "Unknown Sensor Type");
  }
  // all done, save and exit
  saveConfig();
  server.send(201, "text/plain", "created");
}

/* Delete a sensor. TODO: Check, if sensor existed */
void handleDelSensor()
{
  int id = server.arg(0).toInt();
  int type = server.arg(1).toInt();
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
    server.send(400, "text/plain", "Not Yet Implemented :-(");
  }
  // unknown type
  else
  {
    server.send(400, "text/plain", "Unknown Sensor Type");
  }
  // all done, save and exit
  saveConfig();
  server.send(200, "text/plain", "deleted");
}

/* Provides search results of OneWire bus search */
// TODO: Refactor, own id rendering etc should be done in frontend
void handleRequestOneWireSensorAddresses()
{
  numberOfOneWireSensorsFound = searchOneWireSensors();
  int id = server.arg(0).toInt();
  String message = "";
  // If id given, render this sensor's address first
  if (id != -1)
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

/*
  Returns a JSON Array with the current sensor data
  (name, value, mqttTopic, type) of all sensors.
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
    sensorResponse["type"] = "OneWire";
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
    sensorResponse["type"] = "PTSensor";
    sensorsResponse.add(sensorResponse);
    yield();
  }
  String response;
  sensorsResponse.printTo(response);
  server.send(200, "application/json", response);
}

/* Returns information for one specific sensor (for update / delete menu in frontend) */
void handleRequestSensor()
{
  int id = server.arg(0).toInt();
  String type = server.arg(2);
  String request = server.arg(1);

  String message;

  if (id == -1)
  {
    message = "not found";
    server.send(200, "text/plain", message);
  }
  else
  {
    if (request == "name")
    {
      message = oneWireSensors[id].sens_name;
      server.send(200, "text/plain", message);
    }
    if (request == "script")
    {
      message = oneWireSensors[id].sens_mqtttopic;
      server.send(200, "text/plain", message);
    }
    message = "not found";
  }
  saveConfig();
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
