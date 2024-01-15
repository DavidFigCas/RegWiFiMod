#include "Wire.h"
#include <SoftwareSerial.h>

#define RX_PIN        -1
#define TX_PIN        PIN_PA4
const byte MLX90393_ADDRESS = 0x0F;

//SoftwareSerial Serial1(RX_PIN, TX_PIN); // Reemplaza RX_PIN y TX_PIN con los números de pin reales

void setup() {
  delay(3000); // Espera un segundo para la próxima lectura
 Serial1.begin(9600 ); // para depurar
  Serial1.println("MLX Starting");
  pinMode(PIN_PB1, INPUT_PULLUP);
  pinMode(PIN_PB0, INPUT_PULLUP);
  pinMode(PIN_PB3, OUTPUT);


  digitalWrite(PIN_PB3, HIGH);
  Wire.begin();

  // Configura el MLX90393
  Wire.beginTransmission(MLX90393_ADDRESS);
  Wire.write(0x60); // Comando para configurar el sensor
  Wire.write(0x00); // Comando para configurar el sensor
  Wire.write(0x70); // Comando para configurar el sensor
  Wire.write(0x00); // Comando para configurar el sensor
  // Envía otros bytes de configuración según sea necesario
  Wire.endTransmission();
  delay(50);

  // Configura el MLX90393
  Wire.beginTransmission(MLX90393_ADDRESS);
  Wire.write(0x3F); // Comando para configurar el sensor
  Wire.endTransmission();
  Wire.requestFrom(MLX90393_ADDRESS, 4);
  while (Wire.available())
  {
    Serial1.println(Wire.read());
  }
  delay(50);

  // Configura el MLX90393
  Wire.beginTransmission(MLX90393_ADDRESS);
  Wire.write(0x4F); // Comando para configurar el sensor
  Wire.endTransmission();
  // Lee los datos
  Wire.requestFrom(MLX90393_ADDRESS, 32); // Solicita 6 bytes (2 por eje)
  while (Wire.available())
  {
    Serial1.println(Wire.read());
  }
  //delay(50);
}

uint8_t posture[30];
int posture_length = 0;
void loop() {

  // Configura el MLX90393
  Wire.beginTransmission(MLX90393_ADDRESS);
  Wire.write(0x3E); // Comando para configurar el sensor
  Wire.endTransmission();
  Wire.requestFrom(MLX90393_ADDRESS, 4);
  while (Wire.available())
  {
    //Serial1.println(Wire.read());
    Wire.read();
  }
  //Serial1.println("\n ");
  delay(50);

  // Configura el MLX90393
  Wire.beginTransmission(MLX90393_ADDRESS);
  Wire.write(0x4E); // Comando para configurar el sensor
  Wire.endTransmission();
  // Lee los datos
  Wire.requestFrom(MLX90393_ADDRESS, 10); // Solicita 6 bytes (2 por eje)
  int i = -1;  // Comienza con -1 para ignorar el primer byte
  posture_length = 0;
  while (Wire.available())
  {
    byte data = Wire.read();  // Lee el dato actual
    posture[posture_length] = data;
    posture_length++;

  }

  int16_t x = (int16_t)posture[1] << 8 | posture[2];
  int16_t y = (int16_t)posture[3] << 8 | posture[4];
  int16_t z = (int16_t)posture[5] << 8 | posture[6];

  Serial1.print(x);
  Serial1.print("\t");
  Serial1.print(y);
  Serial1.print("\t");
  Serial1.print(z);

  Serial1.println();
  delay(100); // Espera un segundo para la próxima lectura
}
