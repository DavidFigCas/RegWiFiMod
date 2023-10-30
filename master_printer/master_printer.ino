// MASTER (La Pico W no funciona como master)

#define SDA_MAIN    16
#define SCL_MAIN    17


#include <Wire.h>
#include <ArduinoJson.h>

static int p;
char b[200];
StaticJsonDocument<200> doc;  // Asegúrate de que el tamaño sea suficiente para tu objeto JSON
StaticJsonDocument<200> doc_aux;  // Crea un documento JSON con espacio para 200
const char  end1 = '\r';
const char  end2 = '\n';
uint8_t tempVar = 0;
char tempChar;
uint8_t resultadoBytes[100];
uint32_t Num = 1234;
char resultado[150];

void setup()
{
  Serial.begin(115200);
  delay(5000);
  //Wire.setSDA(SDA_MAIN);
  //Wire.setSCL(SCL_MAIN);
  Wire.begin();

  //doc["lit"] = 0;
  doc["precio"] = 9.5;
}


void loop()
{

  // Write a value over I2C to the slave
  Serial.println("Sending...");
  doc["precio"] = p++;
  doc["otro"] = p++;
  doc["wifi"] = false;
  doc["valve"] = false;

  serializeJson(doc, b);
  Serial.println(b);

  Wire.beginTransmission(0x5D);
  //Wire.write((const uint8_t*)b, (strlen(b)));
  //Wire.endTransmission();

  tempVar = 0x1B;
  //i2c_write_blocking(i2c0, 0x5D, &tempVar, 1, false);
  Wire.write(&tempVar, 1);
  Wire.endTransmission();
  tempVar = 0x21;
  //i2c_write_blocking(i2c0, 0x5D, &tempVar, 1, false);
  Wire.beginTransmission(0x5D);
  Wire.write(&tempVar, 1);
  Wire.endTransmission();
  tempVar = 48;
  //i2c_write_blocking(i2c0, 0x5D, &tempVar, 1, false);
  //Send  "Total"

  Wire.beginTransmission(0x5D);
  Wire.write(&tempVar, 1);
  Wire.endTransmission();

  const char* Total = "TOTAL   $";

  //strncpy((char*)resultadoBytes, Total, 9);
  //resultadoBytes[BUFFER_SIZE - 1] = '\0';
  //i2c_write_blocking(i2c0, 0x5D, resultadoBytes, 9, false);
  Wire.beginTransmission(0x5D);
  //Wire.write(resultadoBytes, 9);
  Wire.write((const uint8_t*)Total, 9);
  Wire.endTransmission();

  Wire.beginTransmission(0x5D);
  //Wire.write(resultadoBytes, 9);
  //Wire.write((uint8_t*)&Num, sizeof(Num));
  //Wire.endTransmission();
  sprintf(resultado, "%u", Num);
  size_t size = strlen(resultado);
  strncpy((char*)resultadoBytes, resultado, size);
  Wire.write(resultadoBytes, size);
  //i2c_write_blocking(i2c0, 0x5D, resultadoBytes, size, false);
  Wire.endTransmission();

  // Ensure the slave processing is done and print it out

  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t*)&end1, 1);
  Wire.endTransmission();

  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t*)&end2, 1);
  Wire.endTransmission();
  
  //Reset double size
  tempVar = 0x1B;
  Wire.beginTransmission(0x5D);
  Wire.write(&tempVar, 1);
  Wire.endTransmission();

  tempVar = 0x21;
  Wire.beginTransmission(0x5D);
  Wire.write(&tempVar, 1);
  Wire.endTransmission();

  tempVar = 0;
  Wire.beginTransmission(0x5D);
  Wire.write(&tempVar, 1);
  Wire.endTransmission();


  //tempChar = end2;
  //i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);
  /*Wire.beginTransmission(0x5D);
  Wire.write(end1, 1);
  Wire.endTransmission();
  //Wire.beginTransmission(0x5D);
  //Wire.write(end2, 1);
  Wire.endTransmission();*/

  delay(10);
  Serial.printf("Master Send: '%s'\r\n", b);

  // Read from the slave and print out
  //Wire.requestFrom(0x5A, 199);

  //Serial.print("\nrecv: '");

  //while (Wire.available())
  //{
  // Serial.print((char)Wire.read());
  //}

  Serial.println("'");
  delay(10);
}
