void loop()
{
  cbpiEventSystem(EM_WEB);  // Webserver handle
  cbpiEventSystem(EM_MQTT); // Loop or Check MQTT
  
  if (millis() > (lastToggledSen + SEN_UPDATE))
  {
    DBG_PRINT("Loop: event sensors SEN_UPDATE ");
    DBG_PRINT(SEN_UPDATE);
    DBG_PRINT(" lastToggledSen ");
    DBG_PRINTLN(lastToggledSen);
    cbpiEventSensors(sensorsStatus); // Sensor handle
    lastToggledSen = millis();
  }
  if (millis() > (lastToggledAct + ACT_UPDATE))
  {
    DBG_PRINT("Loop: event actors ACT_UPDATE ");
    DBG_PRINT(ACT_UPDATE);
    DBG_PRINT(" lastToggledAct ");
    DBG_PRINTLN(lastToggledAct);
    cbpiEventActors(actorsStatus); // Actor handle
    lastToggledAct = millis();
  }
  if (millis() > (lastToggledInd + IND_UPDATE))
  {
    DBG_PRINT("Loop: event induction IND_UPDATE ");
    DBG_PRINT(IND_UPDATE);
    DBG_PRINT(" lastToggledInd ");
    DBG_PRINTLN(lastToggledInd);
    cbpiEventInduction(inductionStatus); // Induction handle
    lastToggledInd = millis();
  }

  if (millis() > (lastToggledSys + SYS_UPDATE))
  {
    cbpiEventSystem(EM_WLAN); // Check WLAN
    cbpiEventSystem(EM_MDNS); // MDNS handle
    lastToggledSys = millis();
  }

  if (millis() > (lastToggledDisp + DISP_UPDATE))
  {
    DBG_PRINT("Loop: event display DISP_UPDATE ");
    DBG_PRINT(DISP_UPDATE);
    DBG_PRINT(" lastToggledDisp ");
    DBG_PRINTLN(lastToggledDisp);
    cbpiEventSystem(EM_DISPUP); // Display Update
    lastToggledDisp = millis();
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
