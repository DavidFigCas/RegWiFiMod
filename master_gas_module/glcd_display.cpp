#include "glcd_display.h"

//LiquidCrystal_I2C lcd(0x27, 16, 4); // Ajusta a 20,4 si es una pantalla de 20x4 caracteres.
Adafruit_SharpMem display(SHARP_SCK, SHARP_MOSI, SHARP_SS, 320, 240);


void init_glcd()
{
  Serial.println("LCD Init");
  display.begin();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0, 0);
  display.println("Hello, world!");
  Serial.println("Ready");
}
