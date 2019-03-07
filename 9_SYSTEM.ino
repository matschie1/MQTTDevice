
void DBG_PRINT(String value)
{
  if (setDEBUG) Serial.print(value);
}
void DBG_PRINT(int value)
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

String decToHex(byte decValue, byte desiredStringLength)
{
  String hexString = String(decValue, HEX);
  while (hexString.length() < desiredStringLength) hexString = "0" + hexString;

  return "0x" + hexString;
}
