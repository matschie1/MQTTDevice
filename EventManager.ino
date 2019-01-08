void listenerSystem( int event, int parm )                           // System event listener
{
  // 1:= WLAN Error
  // 2:= MQTT Error
  if ( ( millis() - lastToggledSys ) > onErrorInterval )
  {
    switch (parm) {
      case 1:
        //Serial.printf("Sys Even1t: %i, Parameter: %i%\r\n", event, parm);
        // Stop actors
        for (int i = 0; i < numberOfActors; i++) {
          if (actors[i].isOn) {
            Serial.printf("Set actor %i to off due to WLAN error%\r\n", i);
            actors[i].isOn = false;
            //actors[i].power_actor = 0;
            actors[i].Update();
          }
        }
        // Stop induction
        if (inductionCooker.isInduon) {
          inductionCooker.isInduon = false;
          //inductionCooker.newPower = 0;
          inductionCooker.Update();
        }
        break;
      case 2:
        //Serial.printf("Sys Event2: %i, Parameter: %i%\r\n", event, parm);
        // Stop actors
        for (int i = 0; i < numberOfActors; i++) {
          if (actors[i].isOn) {
            Serial.printf("Set actor %i off due to MQTT error%\r\n", i);
            actors[i].isOn = false;
            actors[i].Update();
          }
          //actors[i].power_actor = 0;

        }
        // Stop induction
        if (inductionCooker.isInduon) {
          inductionCooker.isInduon = false;
          //inductionCooker.newPower = 0;
          inductionCooker.Update();
        }
        break;
      default: // brewing ...
        break;
    }
    lastToggledSys = millis();
  }
}
void listenerSensors( int event, int parm )                           // Sensor event listener
{
  // 1:= Sensor on Err
  if ( ( millis() - lastToggledSys ) > onErrorInterval )
  {
    switch (parm) {
      case 1:
        //Serial.printf("Sensor Event1: %i, Parameter: %i%\r\n", event, parm);
        // Stop actors
        for (int i = 0; i < numberOfActors; i++) {
          if (actors[i].isOn) {
            Serial.printf("Set actor %i off due to Sensor error%\r\n", i);
            actors[i].isOn = false;
            actors[i].Update();
          }
        }
        // Stop induction
        if (inductionCooker.isInduon) {
          inductionCooker.isInduon = false;
          inductionCooker.Update();
        }
        break;
      case 2:
        Serial.printf("Sensor Event2: %i, Parameter: %i%\r\n", event, parm);
        break;
      default: // brewing ...
        handleSensors();
        break;
    }
    lastToggledSen = millis();
  }
}
void listenerActors( int event, int parm )                           // Actor event listener
{
  if ( ( millis() - lastToggledSys ) > onErrorInterval )
  {
    switch (parm) {
      case 1:
        Serial.printf("Actor Event1: %i, Parameter: %i%\r\n", event, parm);
        for (int i = 0; i < numberOfActors; i++) {
          actors[i].isOn = false;
          //actors[i].power_actor = 0;
          actors[i].Update();
        }
        break;
      case 2:
        Serial.printf("Actor Event2: %i, Parameter: %i%\r\n", event, parm);
        break;
      default: // brewing ...
        handleActors();
        break;
    }
    lastToggledAct = millis();
  }
}
void listenerInduction( int event, int parm )                           // Induction event listener
{
  if ( ( millis() - lastToggledSys ) > onErrorInterval )
  {
    switch (parm) {
      case 1:
        Serial.printf("Induction Event1: %i, Parameter: %i%\r\n", event, parm);
        if (inductionCooker.isInduon) {
          inductionCooker.isInduon = false;
          //inductionCooker.newPower = 0;
          inductionCooker.Update();
        }
        break;
      case 2:
        Serial.printf("Induction Event2: %i, Parameter: %i%\r\n", event, parm);
        break;
      default: // brewing ...
        handleInduction();
        break;
    }
    lastToggledInd = millis();
  }
}

void cbpiEventSystem(int parm)                                   // System events
{
  //Serial.printf("System: %i%\r\n", parm);
  gEM.queueEvent( EventManager::cbpiEventSystem, parm );
}

void cbpiEventSensors(int parm)                                   // Sensor events
{
  //Serial.printf("Sensors: %i%\r\n", parm);
  gEM.queueEvent( EventManager::cbpiEventSensors, parm );
}
void cbpiEventActors(int parm)                                   // Actor events
{
  //Serial.printf("Actors: %i%\r\n", parm);
  gEM.queueEvent( EventManager::cbpiEventActors, parm );
}
void cbpiEventInduction(int parm)                                // Induction events
{
  //Serial.printf("Induction: %i%\r\n", parm);
  gEM.queueEvent( EventManager::cbpiEventInduction, parm );
}
