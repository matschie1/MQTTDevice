void millis2wait(int value)
{
  unsigned long pause = millis();
  while (millis() < pause + value)
  {
    //wait approx. [period] ms
    yield();
  }
}
void DBG_PRINT(String value)
{
  if (setDEBUG) Serial.print(value);
}
void DBG_PRINT(int value)
{
  if (setDEBUG) Serial.print(value);
}
void DBG_PRINT(unsigned int value)
{
  if (setDEBUG) Serial.print(value);
}
void DBG_PRINT(long unsigned int value)
{
  if (setDEBUG) Serial.print(value);
}

void DBG_PRINT(float value)
{
  if (setDEBUG) Serial.print(value);
}
void DBG_PRINTHEX(int value)
{
  if (setDEBUG) Serial.print(value, HEX);
}
void DBG_PRINTLN(String value)
{
  if (setDEBUG) Serial.println(value);
}
void DBG_PRINTLN(int value)
{
  if (setDEBUG) Serial.println(value);
}
void DBG_PRINTLN(unsigned int value)
{
  if (setDEBUG) Serial.println(value);
}
void DBG_PRINTLN(long unsigned int value)
{
  if (setDEBUG) Serial.println(value);
}

void DBG_PRINTLN(float value)
{
  if (setDEBUG) Serial.println(value);
}
void DBG_PRINTLNHEX(int value)
{
  if (setDEBUG) Serial.println(value, HEX);
}
void DBG_PRINTLNTS(unsigned long value) // Timestamp
{
  value = value / 1000;
  if (setDEBUG) {
    Serial.print((value / 3600) % 24); // Stunden
    Serial.print(":");
    Serial.print((value / 60) % 60); // Minuten
    Serial.print(":");
    Serial.println(value % 60); // Sekunden
  }
}

String decToHex(unsigned char decValue, unsigned char desiredStringLength)
{
  String hexString = String(decValue, HEX);
  while (hexString.length() < desiredStringLength) hexString = "0" + hexString;

  return "0x" + hexString;
}

unsigned char convertCharToHex(char ch)
{
  unsigned char returnType;
  switch (ch)
  {
    case '0':
      returnType = 0;
      break;
    case  '1' :
      returnType = 1;
      break;
    case  '2':
      returnType = 2;
      break;
    case  '3':
      returnType = 3;
      break;
    case  '4' :
      returnType = 4;
      break;
    case  '5':
      returnType = 5;
      break;
    case  '6':
      returnType = 6;
      break;
    case  '7':
      returnType = 7;
      break;
    case  '8':
      returnType = 8;
      break;
    case  '9':
      returnType = 9;
      break;
    case  'A':
      returnType = 10;
      break;
    case  'B':
      returnType = 11;
      break;
    case  'C':
      returnType = 12;
      break;
    case  'D':
      returnType = 13;
      break;
    case  'E':
      returnType = 14;
      break;
    case  'F' :
      returnType = 15;
      break;
    default:
      returnType = 0;
      break;
  }
  return returnType;
}
