void drawDisplayContent(void) {
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(2, 7);
  char buf[4];
  dtostrf(ptSensors[0].value, 2, 2, buf);
  display.print(buf);
  display.setTextSize(1);
  display.print(" ");
  display.setTextSize(2);
  display.write(247); // deg symbol
  display.setTextSize(3);
  display.print("C");
  display.display();
}
