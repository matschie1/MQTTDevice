void loop()
{
  cbpiEventSystem(EM_WEB);  // Webserver handle

  if (millis() > (lastToggledSen + SEN_UPDATE))
  {
    //      DBG_PRINT("Loop: event sensors SEN_UPDATE ");
    //      DBG_PRINT(SEN_UPDATE);
    //      DBG_PRINT(" lastToggledSen ");
    //      DBG_PRINTLN(lastToggledSen);
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

  if (millis() > (lastToggledSys + SYS_UPDATE))
  {
    cbpiEventSystem(EM_WLAN); // Check WLAN
    cbpiEventSystem(EM_MQTT); // Loop or Check MQTT
    cbpiEventSystem(EM_MDNS); // MDNS handle
    lastToggledSys = millis();
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
  {
    cbpiEventSystem(EM_OTA); // OTA handle
  }

  while (gEM.getNumEventsInQueue()) // Eventmanager process queued events
  {
    gEM.processEvent();
  }
}
