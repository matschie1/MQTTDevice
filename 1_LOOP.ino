void loop()
{
  cbpiEventSystem(EM_WEB);              // Webserver handle
  cbpiEventSystem(EM_MQTT);             // Check MQTT
  
  if (millis() > lastToggledSen + SEN_UPDATE)
  {
    cbpiEventSensors(0);                // Sensor handle
    lastToggledSen = millis();
  }
  if (millis() > lastToggledAct + ACT_UPDATE)
  {
    cbpiEventActors(0);                 // Actor handle
    lastToggledAct = millis();
  }
  if (millis() > lastToggledInd + IND_UPDATE)
  {
    cbpiEventInduction(0);              // Induction handle
    lastToggledInd = millis();
  }

  if (millis() > lastToggledSys + SYS_UPDATE)
  {
    cbpiEventSystem(EM_WLAN);             // Check WLAN
    cbpiEventSystem(EM_MDNS);             // MDNS handle
    cbpiEventSystem(EM_OTA);              // OTA handle
    lastToggledSys = millis();
  }
  
  if (millis() > lastToggledDisp + DISP_UPDATE)
  {
    cbpiEventSystem(EM_DISPUP);         // Display Update
    lastToggledDisp = millis();
  }

  while (gEM.getNumEventsInQueue())     // Eventmanager process all queued events
  {
    gEM.processEvent();
  }
  //delay(100);  // get rid off delay
}
