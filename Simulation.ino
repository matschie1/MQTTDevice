void startSIM()
{
  if (sim_mode == SIM_NONE)
  {
    DBG_PRINTLN("*********************");
    DBG_PRINTLN("SIM: Start simulation");
    DBG_PRINTLN("*********************");
    sim_mode = SIM_SEN_ERR;
    sim_counter = 0;

    // start actors and induction
    cbpiEventActors(SIM_ACT); // Switch on all actors
    cbpiEventInduction(SIM_IND); // Switch on Induction
  }
}

void simCheck()
{
  if (sim_counter < 10)
    sim_mode = SIM_SEN_ERR;
  else if (sim_counter >= 10 && sim_counter <= 20)
    sim_mode = SIM_MQTT;
  else if (sim_counter > 20 && sim_counter <= 30)
    sim_mode = SIM_WLAN;
  else if (sim_counter > 30)
    endSIM();
  else
    sim_mode = SIM_NONE;
}

void endSIM()
{
  DBG_PRINTLN("*******************");
  DBG_PRINTLN("SIM: End simulation");
  DBG_PRINTLN("*******************");
  cbpiEventActors(EM_ACTOFF); // Switch on all actors
  cbpiEventInduction(EM_INDOFF); // Switch on Induction
  sim_counter = 0;
  sim_mode = SIM_NONE;
}

void simWLAN()
{
  if (sim_counter == 21) {
    DBG_PRINTLN("SIM: Start WLAN simulation");
    DBG_PRINTLN("SIM: WLAN disabled");
  }
  if (sim_counter == 28)
    DBG_PRINTLN("SIM: WLAN enabled");

  if (sim_counter > 20 && sim_counter <= 27)
  {
    oledDisplay.wlanOK = false;
    oledDisplay.mqttOK = false;
    wlan_state = false;
    mqtt_state = false;
    cbpiEventSystem(EM_WLANER);
  }
  else if (sim_counter > 27 && sim_counter <= 30)
  {
    oledDisplay.wlanOK = true;
    oledDisplay.mqttOK = true;
    retriesWLAN = 1;
    mqtt_state = true;
    wlan_state = true;
    //cbpiEventSystem(EM_MQTTSUB); // MQTT subscribe
    cbpiEventSystem(EM_MQTTRES);
    DBG_PRINT("SIM: WLAN loop ");
    DBG_PRINTLN(sim_counter);
    sim_counter++;
  }
}
void simMQTT()
{
  if (sim_counter == 10)
    DBG_PRINTLN("SIM: Start MQTT simulation");
  if (sim_counter == 18) {
    DBG_PRINTLN("SIM: MQTT switched on");
  }
  if (sim_counter >= 10 && sim_counter <= 17)
  {
    oledDisplay.mqttOK = false;
    wlan_state = true;
    mqtt_state = false;
    cbpiEventSystem(EM_MQTTER);
  }
  else if (sim_counter > 17 && sim_counter <= 20)
  {
    oledDisplay.mqttOK = true;
    retriesMQTT = 1;
    mqtt_state = true;
    wlan_state = true;
    //cbpiEventSystem(EM_MQTTSUB); // MQTT subscribe
    cbpiEventSystem(EM_MQTTRES);
    DBG_PRINT("SIM: MQTT loop ");
    DBG_PRINTLN(sim_counter);
    sim_counter++;
  }
}
bool simSenChange(bool val) // val hat aktuellen sensor status
{
  val = true;

  if (sim_mode == SIM_SEN_ERR) // FlipFlop Sensor Status nach 20sek
  {
    if (sim_counter == 0)     // sen sim starts all actors and induction on
      DBG_PRINTLN("SIM: Start sensor simulation");

    if (sim_counter > 0 && sim_counter < 6) // create timestamp on loop 1. send EM_OK after loop 7
      val = false;

    if ( lastSenAct == 0 && lastSenInd == 0 )  // kein timestamp
    {
      DBG_PRINT("SIM: sensors loop ");
      DBG_PRINTLN(sim_counter);
      sim_counter++;
    }
    else if ( lastSenAct > 0 || lastSenInd > 0 ) // timestamp
    {
      if (millis() > (lastSenInd + wait_on_Sensor_error_actor) || millis() > (lastSenAct + wait_on_Sensor_error_induction) ) // next loop after wait on sensor
      {
        DBG_PRINT("SIM: sensors loop ");
        DBG_PRINTLN(sim_counter);
        sim_counter++;
      }
    }
  }
  return val;
}
