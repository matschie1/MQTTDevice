void loop() {
  // set device name
  snprintf(mqtt_clientid, 25, "ESP8266-%08X", mqtt_chip_key);

  // WiFi Status prüfen, ggf. Reconnecten
  if (WiFi.status() != WL_CONNECTED) {
    cbpiEventSystem(1);
    wifiManager.autoConnect("MQTTDevice");
  }

  // OTA
  ArduinoOTA.handle();

  // MQTT Status prüfen
  if (!client.connected()) {
    mqttreconnect();
  }
  client.loop();

  // Webserver prüfen
  server.handleClient();

   //mDNS aktualisieren
   MDNS.update();

  // Sensoren aktualisieren
  cbpiEventSensors(0);
  //handleSensors();
  
  // Aktoren aktualisieren
  cbpiEventActors(0);
  //handleActors();

  // Induktionskochfeld
  cbpiEventInduction(0);
  //handleInduction();
  
  // Eventmanager
  while(gEM.getNumEventsInQueue()) // process all queued events
  {
    gEM.processEvent();
  }
  delay(100);
}
