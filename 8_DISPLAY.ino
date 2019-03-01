void drawDisplayContent(void) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(F("Temperatur"));
  display.setTextColor(BLACK, WHITE); // Draw 'inverse' text
  display.print(ptSensors[0].value);
  display.print(" C");
  display.display();
}
