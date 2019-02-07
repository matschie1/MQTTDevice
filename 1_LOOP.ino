void loop()
{

  // Webserver prüfen
  server.handleClient();

  // Sys Update
  if (millis() > lastToggledSys + SYS_UPDATE)
  {
    // WiFi Status prüfen, ggf. Reconnecten
    if (WiFi.status() != WL_CONNECTED)
    {
      wifiManager.autoConnect("MQTTDevice");
    }

    // OTA
    ArduinoOTA.handle();

    // MQTT Status prüfen
    if (!client.connected())
    {
      mqttreconnect();
    }
    client.loop();
    lastToggledSys = millis();
  }

  if (millis() > lastToggled + UPDATE)
  {
    // Sensoren aktualisieren
    handleSensors();
    // Aktoren aktualisieren
    handleActors();
    // Induktionskochfeld
    handleInduction();
    lastToggled = millis();
  }
}