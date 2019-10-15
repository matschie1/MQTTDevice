void loop()
{
  cbpiEventSystem(EM_WEB);  // Webserver handle

  if (millis() > (lastToggledSys + SYS_UPDATE))
  {
    cbpiEventSystem(EM_WLAN); // Check WLAN
    cbpiEventSystem(EM_MQTT); // Loop or Check MQTT
    cbpiEventSystem(EM_MDNS); // MDNS handle
    if (startTEL)
      cbpiEventSystem(EM_TELNET); // TELNET
    if (startOTA)
      cbpiEventSystem(EM_OTA); // OTA handle

    // Simulation - ignore!
    if (sim_mode != SIM_NONE)
      simCheck();
      
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
  while (gEM.getNumEventsInQueue()) // Eventmanager process queued events
  {
    gEM.processEvent();
  }
}
