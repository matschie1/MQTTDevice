void rebootDevice() {
  server.send(200, "text/plain", "rebooting...");
  delay(1000);
  // CAUTION: known (library) issue: only works if you (hardware)button-reset once after flashing the device
  ESP.restart();
}

// TODO: Implement
void turnMqttOff() {
  mqttCommunication = false;
  server.send(200, "text/plain", "CAUTION! I don't work yet: turned off, please reboot to turn on again...");
}
