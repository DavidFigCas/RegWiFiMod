#include <Wire.h>;
#include <LiquidCrystal_I2C.h>;


//0x27 esla direccion I2C comunmente arduno detecta la interfaz, en caso de no funcionar
//la pantalla intentar con a direccion 0x20
LiquidCrystal_I2C lcd(0x27,20,4); //16,4 es el tamaño de la pantalla


void setup()
{
lcd.backlight(); //inicio de fondo retroalimentado
lcd.init(); //inicializacion de la pantalla
lcd.clear();//limpieza de la pantalla
}

void loop()
{
//lcd.clear(); //limpiar la lcd antes de escribir

lcd.setCursor(0,0);//posicionamiento en la primera linea
lcd.print("0"); //escritura en la primera linea
delay(2000);

lcd.setCursor(-3,1); //salto a la segunda linea
lcd.print("1"); //escritura en la segunda linea
delay(2000);

/*
lcd.setCursor(2,2); //salto a la tercera linea
lcd.print("2"); //escritura en la tercera linea
delay(2000);

lcd.setCursor(3,3); //salto a la cuarta linea
lcd.print("3"); //escritura en la cuarta linea
delay(2000);
*/
delay(2000); //tiempo de espera para que reinicie el ciclo
}
