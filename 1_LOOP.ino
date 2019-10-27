void loop()
{
  cbpiEventSystem(EM_WEB);  // Webserver handle
  cbpiEventSystem(EM_WLAN); // Check WLAN
  if (startTEL)
    cbpiEventSystem(EM_TELNET); // TELNET

  if (millis() > (lastToggledSys + SYS_UPDATE))
  {
    cbpiEventSystem(EM_MQTT); // Check MQTT
    cbpiEventSystem(EM_MDNS); // MDNS handle
    lastToggledSys = millis();
  }

  if (millis() > (lastToggledSen + SEN_UPDATE))
  {
    cbpiEventSensors(sensorsStatus); // Sensor handle
    lastToggledSen = millis();
  }
  if (millis() > (lastToggledAct + ACT_UPDATE))
  {
    cbpiEventActors(actorsStatus); // Actor handle
    lastToggledAct = millis();
  }
  if (millis() > (lastToggledInd + IND_UPDATE))
  {
    cbpiEventInduction(inductionStatus); // Induction handle
    lastToggledInd = millis();
  }

  if (useDisplay)
  {
    if (millis() > (lastToggledDisp + DISP_UPDATE))
    {
      cbpiEventSystem(EM_DISPUP); // Display Update
      lastToggledDisp = millis();
    }
  }

  if (startOTA)
    cbpiEventSystem(EM_OTA); // OTA handle

  if (startTCP)
  {
    if (millis() > (lastToggledTCP + TCP_UPDATE))
    {
      cbpiEventSystem(EM_TCP); // TCP Server Update
      lastToggledTCP = millis();
    }
  }

  // Simulation - ignore!
  if (sim_mode != SIM_NONE)
    simCheck();

  // Debug output event queue
  // if (millis() > (lastToggledEvent + 5000))
  // {
  //   DBG_PRINT("Event queue status: ");
  //   DBG_PRINTLN(gEM.getNumEventsInQueue());
  //   lastToggledEvent = millis();
  // }
  //gEM.processAllEvents();
  
  while (gEM.getNumEventsInQueue()) // Eventmanager process queued events
  {
    gEM.processEvent();
  }
}
