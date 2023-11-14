#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// 0x27 es la dirección I2C comúnmente utilizada. Si no funciona, intenta con 0x20.
LiquidCrystal_I2C lcd(0x27, 20, 4); // Ajusta a 20,4 si es una pantalla de 20x4 caracteres.

void setup() {
  lcd.init();  // Inicialización de la pantalla
  lcd.backlight(); // Encender el fondo retroiluminado
  lcd.clear(); // Limpiar la pantalla
}

void loop() {
  // Primera Línea
  lcd.setCursor(0, 0); // Establecer cursor en la primera línea
  lcd.print("Modulo I2C LCD"); // Escribir en la primera línea
  delay(2000);

  // Segunda Línea
  lcd.clear(); // Limpiar la pantalla antes de escribir en la segunda línea
  lcd.setCursor(0, 2); // Establecer cursor en la segunda línea
  lcd.print("Serial I2C"); // Escribir en la segunda línea
  delay(2000);

  // Tercera Línea
  lcd.clear(); // Limpiar la pantalla antes de escribir en la tercera línea
  lcd.setCursor(0, 4); // Establecer cursor en la tercera línea
  lcd.print("y LCD"); // Escribir en la tercera línea
  delay(2000);

  // Cuarta Línea
  lcd.clear(); // Limpiar la pantalla antes de escribir en la cuarta línea
  lcd.setCursor(0, 5); // Establecer cursor en la cuarta línea
  lcd.print("LCM12864"); // Escribir en la cuarta línea
  delay(2000);
}
