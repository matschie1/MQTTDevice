void listenerSystem( int event, int parm )                           // System event listener
{
  if (parm > 9) { lastToggledSys = 0; } // SysEvents > 9
  
  if ( ( millis() - lastToggledSys ) > onErrorInterval )
  {
    switch (parm) {
      case 1: // WLAN error
        // Stop actors
        for (int i = 0; i < numberOfActors; i++) {
          if (actors[i].isOn) {
            Serial.printf("Set actor %i to off due to WLAN error%\r\n", i);
            actors[i].isOn = false;
            //actors[i].power_actor = 0;
            actors[i].Update();
            actors[i].publishmqtt();
          }
        }
        // Stop induction
        if (inductionCooker.isInduon) {
          Serial.println("Set induction off due to WLAN error");
          inductionCooker.isInduon = false;
          //inductionCooker.newPower = 0;
          inductionCooker.Update();
          inductionCooker.publishmqtt();
        }
        break;
      case 2: // MQTT Error
        // Stop actors
        for (int i = 0; i < numberOfActors; i++) {
          if (actors[i].isOn) {
            Serial.printf("Set actor %i off due to MQTT error%\r\n", i);
            actors[i].isOn = false;
            actors[i].Update();
            actors[i].publishmqtt();
          }
        }
        // Stop induction
        if (inductionCooker.isInduon) {
          Serial.println("Set induction off due to MQTT error");
          inductionCooker.isInduon = false;
          inductionCooker.Update();
          inductionCooker.publishmqtt();
        }
        break;
      case 10: // Disable MQTT
        // Stop actors
        for (int i = 0; i < numberOfActors; i++) {
          if (actors[i].isOn) {
            Serial.printf("Set actor %i off due to MQTT disabled%\r\n", i);
            actors[i].isOn = false;
            actors[i].Update();
            actors[i].publishmqtt();
          }
        }
        // Stop induction
        if (inductionCooker.isInduon) {
          Serial.println("Set induction off due to MQTT disabled");
          inductionCooker.isInduon = false;
          inductionCooker.Update();
          inductionCooker.publishmqtt();
        }
        mqttCommunication = false;
        server.send(200, "text/plain", "CAUTION! I don't work yet: turned off, please reboot to turn on again...");
        break;
      case 11: // Reboot ESP
        // Stop actors
        for (int i = 0; i < numberOfActors; i++) {
          if (actors[i].isOn) {
            Serial.printf("Set actor %i off due to reboot%\r\n", i);
            actors[i].isOn = false;
            actors[i].Update();
            actors[i].publishmqtt();
          }
        }
        // Stop induction
        if (inductionCooker.isInduon) {
          Serial.println("Set induction off due to reboot");
          inductionCooker.isInduon = false;
          inductionCooker.Update();
          inductionCooker.publishmqtt();
        }
        server.send(200, "text/plain", "rebooting...");
        delay(1000);
        // CAUTION: known (library) issue: only works if you (hardware)button-reset once after flashing the device
        ESP.restart();
        break;
      default:
        break;
    }
    lastToggledSys = millis();
  }
}
void listenerSensors( int event, int parm )                           // Sensor event listener
{
  // 1:= Sensor on Err
  if (( millis() - lastToggledSen ) > onErrorInterval )
  {
    switch (parm) {
      case 1:
#ifdef StopActorsOnSensorError
        // Stop actors
        for (int i = 0; i < numberOfActors; i++) {
          if (actors[i].isOn) {
            Serial.printf("Set actor %i off due to Sensor error%\r\n", i);
            actors[i].isOn = false;
            actors[i].Update();
            actors[i].publishmqtt();
          }
        }
#endif
#ifdef StopInductionOnSensorError
        // Stop Induction
        if (inductionCooker.isInduon) {
          Serial.println("Set induction off due to sensor error");
          inductionCooker.isInduon = false;
          inductionCooker.Update();
          inductionCooker.publishmqtt();
        }
#endif
        break;
      default:
        break;
    }
    handleSensors();
    lastToggledSen = millis();
  }
}
void listenerActors( int event, int parm )                           // Actor event listener
{
  if ( ( millis() - lastToggledSys ) > onErrorInterval )
  {
    switch (parm) {
      case 1:
        for (int i = 0; i < numberOfActors; i++) {
          Serial.printf("Set actor %i off due to actor error%\r\n", i);
          actors[i].isOn = false;
          actors[i].Update();
          actors[i].publishmqtt();
        }
        break;
      default:
        break;
    }
    handleActors();
    lastToggledAct = millis();
  }
}
void listenerInduction( int event, int parm )                           // Induction event listener
{
  if ( ( millis() - lastToggledSys ) > onErrorInterval )
  {
    switch (parm) {
      case 1:
        if (inductionCooker.isInduon) {
          Serial.println("Set induction off due to induction error");
          inductionCooker.isInduon = false;
          //inductionCooker.newPower = 0;
          inductionCooker.Update();
          inductionCooker.publishmqtt();
        }
        break;
      default:
        break;
    }
    handleInduction();
    lastToggledInd = millis();
  }
}

void cbpiEventSystem(int parm)                                   // System events
{
  gEM.queueEvent( EventManager::cbpiEventSystem, parm );
}

void cbpiEventSensors(int parm)                                   // Sensor events
{
  gEM.queueEvent( EventManager::cbpiEventSensors, parm );
}
void cbpiEventActors(int parm)                                   // Actor events
{
  gEM.queueEvent( EventManager::cbpiEventActors, parm );
}
void cbpiEventInduction(int parm)                                // Induction events
{
  gEM.queueEvent( EventManager::cbpiEventInduction, parm );
}
