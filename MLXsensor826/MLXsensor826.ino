#include "Wire.h"
#include <SoftwareSerial.h>

#define RX_PIN        -1
#define TX_PIN        PIN_PA1
const byte MLX90393_ADDRESS = 0x0F;
double x, y, z, a, phaseShift = 90;

SoftwareSerial mySerial(RX_PIN, TX_PIN); // Reemplaza RX_PIN y TX_PIN con los números de pin reales


double calcularAngulo(double x, double y)
{
  x = x * (-1);
  double angulo = atan2(y, x); // Calcula el ángulo en radianes
  angulo = angulo * (180.0 / M_PI); // Convierte de radianes a grados
  angulo += phaseShift; // Agrega o resta el defase en grados

  // Normaliza el ángulo para que esté en el rango 0-360
  if (angulo < 0) {
    angulo += 360;
  } else if (angulo >= 360) {
    angulo -= 360;
  }
  return angulo;
}

void setup()
{
  delay(3000); // Espera un segundo para la próxima lectura
  mySerial.begin(9600 ); // para depurar
  mySerial.println("MLX Starting");
  //pinMode(PIN_PB1, INPUT_PULLUP);
  //pinMode(PIN_PB0, INPUT_PULLUP);
  //pinMode(PIN_PA6, OUTPUT);
  delay(3000);

  //digitalWrite(PIN_PA6, HIGH);
  delay(3000);
  Wire.begin();
  delay(1000);

  byte error, address;
  int nDevices = 0;

  delay(5000);

  mySerial.println("Scanning for I2C devices ...");
  for (address = 0x01; address < 0x7f; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      mySerial.printf("I2C device found at address 0x%02X\n", address);
      nDevices++;
    } else if (error != 2) {
      mySerial.printf("Error %d at address 0x%02X\n", error, address);
    }
  }
  if (nDevices == 0) {
    mySerial.println("No I2C devices found");
  }
  // Configura el MLX90393
  Wire.beginTransmission(MLX90393_ADDRESS);
  Wire.write(0x60); // Comando para configurar el sensor
  Wire.write(0x00); // Comando para configurar el sensor
  Wire.write(0x70); // Comando para configurar el sensor
  Wire.write(0x00); // Comando para configurar el sensor
  // Envía otros bytes de configuración según sea necesario
  Wire.endTransmission();
  delay(50);

  Wire.beginTransmission(0x0C);
  Wire.write(0x60); // Comando para configurar el sensor
  Wire.write(0x00); // Comando para configurar el sensor
  Wire.write(0x70); // Comando para configurar el sensor
  Wire.write(0x00); // Comando para configurar el sensor
  // Envía otros bytes de configuración según sea necesario
  Wire.endTransmission();
  delay(50);

  // Configura el MLX90393
  //Wire.beginTransmission(MLX90393_ADDRESS);
  //Wire.write(0x3F); // Comando para configurar el sensor
  //Wire.endTransmission();
  //Wire.requestFrom(MLX90393_ADDRESS, 4);
  //while (Wire.available())
  //{
  //mySerial.println(Wire.read());
  //}
  delay(50);

  // Configura el MLX90393
  //Wire.beginTransmission(MLX90393_ADDRESS);
  //Wire.write(0x4F); // Comando par  a configurar el sensor
  //Wire.endTransmission();
  // Lee los datos
  //Wire.requestFrom(MLX90393_ADDRESS, 32); // Solicita 6 bytes (2 por eje)
  //while (Wire.available())
  //{
  //mySerial.println(Wire.read());
  //}
  delay(50);
}

uint8_t posture[30];
int posture_length = 0;
void loop()
{

  // Configura el MLX90393
  Wire.beginTransmission(MLX90393_ADDRESS);
  Wire.write(0x3E); // Comando para configurar el sensor
  Wire.endTransmission();
  Wire.requestFrom(MLX90393_ADDRESS, 4);
  while (Wire.available())
  {
    //mySerial.println(Wire.read());
    Wire.read();
  }
  //mySerial.println("\n ");
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

  mySerial.print("{\"x\":");
  x = (int16_t)posture[1] << 8 | posture[2];
  mySerial.print(x);
  mySerial.print(",\"y\":");
  y = (int16_t)posture[3] << 8 | posture[4];
  mySerial.print(y);
  mySerial.print(",\"a\":");
  a = calcularAngulo(x, y);
  if ((abs(x) + abs(y)) >= 1700)
    mySerial.print(a);
  else
    mySerial.print(-1);
  mySerial.print("}");


  mySerial.println();

  
  delay(100); // Espera un segundo para la próxima lectura

  // Configura el MLX90393
  Wire.beginTransmission(0x0C);
  Wire.write(0x3E); // Comando para configurar el sensor
  Wire.endTransmission();
  Wire.requestFrom(0x0C, 4);
  while (Wire.available())
  {
    //mySerial.println(Wire.read());
    Wire.read();
  }
  //mySerial.println("\n ");
  delay(50);

  // Configura el MLX90393
  Wire.beginTransmission(0x0C);
  Wire.write(0x4E); // Comando para configurar el sensor
  Wire.endTransmission();
  // Lee los datos
  Wire.requestFrom(0x0C, 10); // Solicita 6 bytes (2 por eje)
  posture_length = 0;
  while (Wire.available())
  {
    byte data = Wire.read();  // Lee el dato actual
    posture[posture_length] = data;
    posture_length++;

  }

  mySerial.print("{\"x\":");
  x = (int16_t)posture[1] << 8 | posture[2];
  mySerial.print(x);
  mySerial.print(",\"y\":");
  y = (int16_t)posture[3] << 8 | posture[4];
  mySerial.print(y);
  mySerial.print(",\"a\":");
  a = calcularAngulo(x, y);
  if ((abs(x) + abs(y)) >= 1700)
    mySerial.print(a);
  else
    mySerial.print(-1);
  mySerial.print("}");


  mySerial.println();
  delay(2000); // Espera un segundo para la próxima lectura
}
