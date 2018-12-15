void loop() {
  // WiFi Status prüfen, ggf. Reconnecten
  if (WiFi.status() != WL_CONNECTED) {  wifiManager.autoConnect("MQTTDevice"); }

  // MQTT Status prüfen
  if (!client.connected()) { mqttreconnect(); }
  client.loop();

  // Webserver prüfen
  server.handleClient();

  // Sensoren aktualisieren 
  handleSensors();

  // Aktoren aktualisieren
//  handleActors();

  // Induktionskochfeld
  handleInduction();

  delay(100);
}
