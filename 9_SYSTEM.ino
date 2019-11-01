void millis2wait(const int &value)
{
  unsigned long pause = millis();
  while (millis() < pause + value)
  {
    //wait approx. [period] ms
    yield();
  }
}
bool isValidInt(const String &str)
{
  for (int i = 0; i < str.length(); i++)
  {
    if (isdigit(str.charAt(i)))
      continue;
    if (str.charAt(i) == '.')
      return false;
    return false;
  }
  return true;
}

bool isValidFloat(const String &str)
{
  for (int i = 0; i < str.length(); i++)
  {
    if (str.charAt(i) == '.')
      continue;
    if (isdigit(str.charAt(i)))
      continue;
    return false;
  }
  return true;
}

void DBG_PRINT(const String &value)
{
  if (setDEBUG)
  {
    if (Telnet.connected())
      Telnet.print(value);
    else
      Serial.print(value);
  }
}
void DBG_PRINT(const int &value)
{
  if (setDEBUG)
  {
    if (Telnet.connected())
      Telnet.print(value);
    else
      Serial.print(value);
  }
}
void DBG_PRINT(const unsigned int &value)
{
  if (setDEBUG)
  {
    if (Telnet.connected())
      Telnet.print(value);
    else
      Serial.print(value);
  }
}
void DBG_PRINT(const long unsigned int &value)
{
  if (setDEBUG)
  {
    if (Telnet.connected())
      Telnet.print(value);
    else
      Serial.print(value);
  }
}
void DBG_PRINT(const long &value)
{
  if (setDEBUG)
  {
    if (Telnet.connected())
      Telnet.print(value);
    else
      Serial.print(value);
  }
}
void DBG_PRINTLN(const long &value)
{
  if (setDEBUG)
  {
    if (Telnet.connected())
      Telnet.println(value);
    else
      Serial.println(value);
  }
}
void DBG_PRINT(const float &value)
{
  if (setDEBUG)
  {
    if (Telnet.connected())
      Telnet.print(value);
    else
      Serial.print(value);
  }
}
void DBG_PRINTHEX(const int &value)
{
  if (setDEBUG)
  {
    if (Telnet.connected())
      Telnet.print(value, HEX);
    else
      Serial.print(value, HEX);
  }
}
void DBG_PRINTLN(const String &value)
{
  if (setDEBUG)
  {
    if (Telnet.connected())
      Telnet.println(value);
    else
      Serial.println(value);
  }
}
void DBG_PRINTLN(const int &value)
{
  if (setDEBUG)
  {
    if (Telnet.connected())
      Telnet.println(value);
    else
      Serial.println(value);
  }
}
void DBG_PRINTLN(const unsigned int &value)
{
  if (setDEBUG)
  {
    if (Telnet.connected())
      Telnet.println(value);
    else
      Serial.println(value);
  }
}
void DBG_PRINTLN(const long unsigned int &value)
{
  if (setDEBUG)
  {
    if (Telnet.connected())
      Telnet.println(value);
    else
      Serial.println(value);
  }
}
void DBG_PRINTLN(const float &value)
{
  if (setDEBUG)
  {
    if (Telnet.connected())
      Telnet.println(value);
    else
      Serial.println(value);
  }
}
void DBG_PRINTLNHEX(const int &value)
{
  if (setDEBUG)
  {
    if (Telnet.connected())
      Telnet.println(value, HEX);
    else
      Serial.println(value, HEX);
  }
}
void DBG_PRINTLNTS(unsigned long value) // Timestamp
{
  value = value / 1000;
  if (setDEBUG)
  {
    if (Telnet.connected())
    {
      Telnet.print((value / 3600) % 24); // Stunden
      Telnet.print(":");
      Telnet.print((value / 60) % 60); // Minuten
      Telnet.print(":");
      Telnet.println(value % 60); // Sekunden
    }
    else
    {
      Serial.print((value / 3600) % 24); // Stunden
      Serial.print(":");
      Serial.print((value / 60) % 60); // Minuten
      Serial.print(":");
      Serial.println(value % 60); // Sekunden
    }
  }
}

String decToHex(unsigned char decValue, unsigned char desiredStringLength)
{
  String hexString = String(decValue, HEX);
  while (hexString.length() < desiredStringLength)
    hexString = "0" + hexString;

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
  case '1':
    returnType = 1;
    break;
  case '2':
    returnType = 2;
    break;
  case '3':
    returnType = 3;
    break;
  case '4':
    returnType = 4;
    break;
  case '5':
    returnType = 5;
    break;
  case '6':
    returnType = 6;
    break;
  case '7':
    returnType = 7;
    break;
  case '8':
    returnType = 8;
    break;
  case '9':
    returnType = 9;
    break;
  case 'A':
    returnType = 10;
    break;
  case 'B':
    returnType = 11;
    break;
  case 'C':
    returnType = 12;
    break;
  case 'D':
    returnType = 13;
    break;
  case 'E':
    returnType = 14;
    break;
  case 'F':
    returnType = 15;
    break;
  default:
    returnType = 0;
    break;
  }
  return returnType;
}
