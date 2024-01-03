#include "Wire.h"
#include <SoftwareSerial.h>

#define RX_PIN        -1
#define TX_PIN        PIN_PA4
const byte MLX90393_ADDRESS = 0x0C;

SoftwareSerial mySerial(RX_PIN, TX_PIN); // Reemplaza RX_PIN y TX_PIN con los números de pin reales

void setup() {
  mySerial.begin(9600 ); // para depurar
  //pinMode(PIN_PB1, INPUT_PULLUP);
  //pinMode(PIN_PB0, INPUT_PULLUP);
  pinMode(PIN_PB3, OUTPUT);

  
  digitalWrite(PIN_PB3,HIGH);
  Wire.begin();

   // Configura el MLX90393
  //Wire.beginTransmission(MLX90393_ADDRESS);
  //Wire.write(0x60); // Comando para configurar el sensor
  // Envía otros bytes de configuración según sea necesario
  //Wire.endTransmission();
}

void loop() {

   Wire.beginTransmission(MLX90393_ADDRESS);
  Wire.write(0x4E); // Comando para leer los datos del sensor
  Wire.endTransmission();

  // Lee los datos
  Wire.requestFrom(MLX90393_ADDRESS, 6); // Solicita 6 bytes (2 por eje)
  if (Wire.available() == 6) {
    int x = Wire.read() | Wire.read() << 8;
    int y = Wire.read() | Wire.read() << 8;
    int z = Wire.read() | Wire.read() << 8;

    // Muestra los valores
    mySerial.print("X: "); mySerial.print(x);
    mySerial.print(" Y: "); mySerial.print(y);
    mySerial.print(" Z: "); mySerial.println(z);
  }

  delay(1000); // Espera un segundo para la próxima lectura
}
