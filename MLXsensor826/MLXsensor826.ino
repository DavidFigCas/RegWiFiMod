#include "Wire.h"
//#include <SoftwareSerial.h>

//#define RX_PIN        -1
//#define TX_PIN        PIN_PA1
const byte MLX90393_ADDRESS = 0x0C;
double x, y, z, a, phaseShift = 90;

//SoftwareSerial mySerial(RX_PIN, TX_PIN); // Reemplaza RX_PIN y TX_PIN con los números de pin reales


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
  //delay(3000); // Espera un segundo para la próxima lectura
  Serial1.begin(9600 ); // para depurar
  Serial1.println("MLX Starting");
  //pinMode(PIN_PB1, INPUT_PULLUP);
  //pinMode(PIN_PB0, INPUT_PULLUP);
  //pinMode(PIN_PA6, OUTPUT);
  delay(3000);

  //digitalWrite(PIN_PA6, HIGH);
  //delay(3000);
  Wire.begin();
  //delay(1000);

  byte error, address;
  int nDevices = 0;

  //delay(5000);

  /*Serial1.println("Scanning for I2C devices ...");
  for (address = 0x01; address < 0x7f; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial1.printf("I2C device found at address 0x%02X\n", address);
      nDevices++;
    } else if (error != 2) {
      Serial1.printf("Error %d at address 0x%02X\n", error, address);
    }
  }
  if (nDevices == 0) {
    Serial1.println("No I2C devices found");
  }*/
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
  Wire.beginTransmission(MLX90393_ADDRESS);
  Wire.write(0x4F); // Comando par  a configurar el sensor
  Wire.endTransmission();
  // Lee los datos
  Wire.requestFrom(MLX90393_ADDRESS, 32); // Solicita 6 bytes (2 por eje)
  while (Wire.available())
  {
    Wire.read();
  //mySerial.println();
  }
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

  Serial1.print("{\"x\":");
  x = (int16_t)posture[1] << 8 | posture[2];
  Serial1.print(x);
  Serial1.print(",\"y\":");
  y = (int16_t)posture[3] << 8 | posture[4];
  Serial1.print(y);
  Serial1.print(",\"a\":");
  a = calcularAngulo(x, y);
  if ((abs(x) + abs(y)) >= 1700)
    Serial1.print(a);
  else
    Serial1.print(-1);
  Serial1.print("}");


  Serial1.println();

  
  delay(1000); // Espera un segundo para la próxima lectura

}
