class TCPServer
{
public:
    int temperature = 0;
    int target_temp = 0;    // Quelle MQTT
    int powerlevel = 0;     // Quelle Aktoren und Induktion
    String kettle_id = "0"; // Quelle Sensoren
    String tcpTopic;
    String ID = "";
    String name = "";

    TCPServer(String new_kettle_id)
    {
        change(new_kettle_id);
    }

    void change(String new_kettle_id)
    {
        if (kettle_id != new_kettle_id)
        {
            mqtt_unsubscribe();
            kettle_id = new_kettle_id;
            setTopic(kettle_id);
            mqtt_subscribe();
        }
    }
    void mqtt_subscribe()
    {
        if (client.connected())
        {
            char subscribemsg[50];
            tcpTopic.toCharArray(subscribemsg, 50);
            DBG_PRINT("TCP: ");
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
            tcpTopic.toCharArray(subscribemsg, 50);
            DBG_PRINT("TCP: ");
            DBG_PRINT("Unsubscribing from ");
            DBG_PRINTLN(subscribemsg);
            client.unsubscribe(subscribemsg);
        }
    }

    void handlemqtt(char *payload)
    {
        StaticJsonDocument<128> doc;
        DeserializationError error = deserializeJson(doc, (const char *)payload);
        if (error)
        {
            DBG_PRINT("TCP: handlemqtt deserialize Json error ");
            DBG_PRINTLN(error.c_str());
            return;
        }
        target_temp = doc["target_temp"];
    }
};

// Erstelle Array mit Anzahl maxSensoren == max Anzahl TCP
// kettle_id, name, id, act_temp, target_temp, powerlevel
TCPServer tcpServer[numberOfSensorsMax] = {
    TCPServer("0"),
    TCPServer("0"),
    TCPServer("0"),
    TCPServer("0"),
    TCPServer("0"),
    TCPServer("0"),
    TCPServer("0"),
    TCPServer("0"),
    TCPServer("0"),
    TCPServer("0")};

void publishTCP()
{
    for (int i = 1; i < numberOfSensorsMax; i++)
    {
        if (tcpServer[i].kettle_id == "0")
            continue;
        tcpClient.connect(tcpHost, tcpPort);
        if (tcpClient.connect(tcpHost, tcpPort))
        {
            StaticJsonDocument<256> doc;
            doc["name"] = tcpServer[i].name;
            doc["ID"] = tcpServer[i].ID;
            doc["temperature"] = tcpServer[i].temperature;
            doc["temp_units"] = "C";
            doc["RSSI"] = WiFi.RSSI();
            doc["interval"] = TCP_UPDATE;
            // Send additional but sensless data to act as an iSpindle device
            // json from iSpindle:
            // Input Str is now:{"name":"iSpindle","ID":1234567,"angle":22.21945,"temperature":15.6875,
            // "temp_units":"C","battery":4.207508,"gravity":1.019531,"interval":900,"RSSI":-59}

            doc["angle"] = tcpServer[i].target_temp;
            doc["battery"] = tcpServer[i].powerlevel;
            doc["gravity"] = 0;
            char jsonMessage[256];
            serializeJson(doc, jsonMessage);
            tcpClient.write(jsonMessage);
            DBG_PRINT("TCP: TCP message ");
            DBG_PRINTLN(jsonMessage);
        }
    }
}

void setTCPConfig()
{
    // Init TCP array
    // Das Array ist nicht lin aufsteigend sortiert
    // Das Array beginnt bei 1 mit der ersten ID aus MQTTPub (CBPi Plugin)
    // das Array Element 0 ist unbelegt
    for (int i = 0; i < numberOfSensors; i++)
    {
        if (sensors[i].kettle_id.toInt() > 0)
        {
            // Setze Kettle ID
            tcpServer[sensors[i].kettle_id.toInt()].kettle_id = sensors[i].kettle_id;
            // Setze MQTTTopic
            tcpServer[sensors[i].kettle_id.toInt()].tcpTopic = "MQTTDevice/kettle/" + sensors[i].kettle_id;
            tcpServer[sensors[i].kettle_id.toInt()].name = sensors[i].sens_name;
            tcpServer[sensors[i].kettle_id.toInt()].ID = SensorAddressToString(sensors[i].sens_address);
            tcpServer[sensors[i].kettle_id.toInt()].temperature = (sensors[i].sens_value + sensors[i].sens_offset);
        }
    }
    for (int i = 1; i < numberOfSensorsMax; i++)
    {
        if (tcpServer[i].kettle_id == "0")
            break;
        DBG_PRINT("TCP Server: ");
        DBG_PRINT(tcpServer[i].kettle_id);
        DBG_PRINT(" Topic ");
        DBG_PRINT(tcpServer[i].tcpTopic);
        DBG_PRINT(" Powerlevel ");
        DBG_PRINTLN(tcpServer[i].powerlevel);
    }
}

void setTopic(String id)
{
    tcpServer[id.toInt()].tcpTopic = "MQTTDevice/kettle/" + id;
    DBG_PRINT("TCP: topic ");
    DBG_PRINT(id);
    DBG_PRINT(" set ");
    DBG_PRINTLN(tcpServer[id.toInt()].tcpTopic);
}

void setTCPTemp(String id, float temp)
{
    tcpServer[id.toInt()].temperature = temp;
}

void setTCPTemp()
{
    for (int i = 0; i < numberOfSensors; i++)
    {
        if (sensors[i].kettle_id.toInt() > 0)
            tcpServer[sensors[i].kettle_id.toInt()].temperature = (sensors[i].sens_value + sensors[i].sens_offset);
    }
}

void setTCPPowerAct(String id, int power)
{
    tcpServer[id.toInt()].powerlevel = power;
    // DBG_PRINT("TCP: set actor power for kettle ");
    // DBG_PRINT(actors[i].kettle_id.toInt());
    // DBG_PRINT(" Power ");
    // DBG_PRINTLN(actors[i].power_actor);
}
void setTCPPowerAct()
{
    for (int i = 0; i < numberOfActors; i++)
    {
        if (actors[i].kettle_id.toInt() > 0)
        {
            tcpServer[actors[i].kettle_id.toInt()].powerlevel = actors[i].power_actor;
            // DBG_PRINT("TCP: set actor power for kettle ");
            // DBG_PRINT(actors[i].kettle_id.toInt());
            // DBG_PRINT(" Power ");
            // DBG_PRINTLN(actors[i].power_actor);
        }
    }
}
void setTCPPowerInd(String id, int power)
{
    tcpServer[id.toInt()].powerlevel = power;
    // DBG_PRINT("TCP: set induction power for kettle ");
    // DBG_PRINT(inductionCooker.kettle_id.toInt());
    // DBG_PRINT(" Power ");
    // DBG_PRINTLN(inductionCooker.power);
}
void setTCPPowerInd()
{
    if (inductionCooker.kettle_id.toInt() > 0)
    {
        tcpServer[inductionCooker.kettle_id.toInt()].powerlevel = inductionCooker.power;
        // DBG_PRINT("TCP: set induction power for kettle ");
        // DBG_PRINT(inductionCooker.kettle_id.toInt());
        // DBG_PRINT(" Power ");
        // DBG_PRINTLN(inductionCooker.power);
    }
}