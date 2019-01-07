void loop() {
  // set device name
  snprintf(mqtt_clientid, 25, "ESP8266-%08X", mqtt_chip_key);
  
  // WiFi Status prüfen, ggf. Reconnecten
  if (WiFi.status() != WL_CONNECTED) {
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

  // mDNS aktualisieren
  MDNS.update();
  
  // Sensoren aktualisieren
  handleSensors();

  // Aktoren aktualisieren
  handleActors();

  // Induktionskochfeld
  handleInduction();

  delay(100);
}
