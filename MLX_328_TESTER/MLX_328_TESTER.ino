#include <Wire.h>
#include "Adafruit_MLX90393.h"
#include <avr/io.h>

Adafruit_MLX90393 sensor = Adafruit_MLX90393();
uint8_t angulo;
float x, y, z;

void setup() {
  // put your setup code here, to run once:
  Wire.begin(21, 22); // SDA en GPIO21, SCL en GPIO22
  Serial1.begin(9600);

  if (!sensor.begin_I2C(0x0F)) 
  { // hardware I2C mode, can pass in address & alt Wire
    //Serial1.println("No");
    while (1) {
      delay(10);
    }
  }
  //Serial.println("Found");

}

void loop() {

  // get X Y and Z data at once
  if (sensor.readData(&x, &y, &z))
  {
    //Serial.print("X: "); Serial.print(x, 4); Serial.print("\t");
    //Serial.print("Y: "); Serial.print(y, 4); Serial.print("\t");
    //Serial.print("Z: "); Serial.print(z, 4); Serial.print("\t\t");
    //calcularAngulo();
    angulo = atan2(y, x); // Calcula el ángulo en radianes
    angulo = angulo * (180.0 / M_PI); // Convierte de radianes a grados
    if (angulo < 0) {
      angulo += 360;
    } else if (angulo >= 360) {
      angulo -= 360;
    }
    
    convertirYEnviarAngulo(angulo);
  }

}


void convertirYEnviarAngulo(uint8_t angulo) {
    char buffer[4]; // Un número máximo de 255 más el carácter nulo final
    int i = 0;

    if (angulo == 0) {
        buffer[i++] = '0'; // Manejar explícitamente el caso de 0
    } else {
        // Extraer cada dígito empezando por el más significativo
        int hundreds = (angulo / 100) % 10; // Centenas
        int tens = (angulo / 10) % 10;      // Decenas
        int ones = angulo % 10;             // Unidades

        if (hundreds > 0) { // Solo agregar centenas si es necesario
            buffer[i++] = hundreds + '0';
        }
        if (tens > 0 || hundreds > 0) { // Solo agregar decenas si es necesario
            buffer[i++] = tens + '0';
        }
        buffer[i++] = ones + '0'; // Siempre agregamos el dígito de las unidades
    }

    buffer[i] = '\0'; // Terminar la cadena

    // Enviar la cadena por el puerto serial
    //Serial1.println(buffer);
}
