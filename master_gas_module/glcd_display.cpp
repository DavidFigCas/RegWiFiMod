#include "glcd_display.h"

LiquidCrystal_I2C lcd(0x27, 16, 4); // Ajusta a 20,4 si es una pantalla de 20x4 caracteres.


void init_glcd()
{
  lcd.init();  // Inicialización de la pantalla
  lcd.clear(); // Limpiar la pantalla
  lcd.noBacklight();
  lcd.clear(); // Limpiar la pantalla
  lcd.backlight(); // Encender el fondo retroiluminado
  lcd.clear(); // Limpiar la pantalla
  lcd.setCursor(0, 0); // Establecer cursor en la primera línea
  lcd.print("Ver: "); // Escribir en la primera línea
  //lcd.setCursor(0, 0); // Establecer cursor en la primera línea
  lcd.print(VERSION); // Escribir en la primera línea
  //lcd.clear(); // Limpiar la pantalla
}
