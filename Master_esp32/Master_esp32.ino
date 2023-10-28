// MASTER (La Pico W no funciona como master)

#define SDA_MAIN    16
#define SCL_MAIN    17


#include <Wire.h>
#include <ArduinoJson.h>

static int p;
char b[200];
StaticJsonDocument<200> doc;  // Asegúrate de que el tamaño sea suficiente para tu objeto JSON
StaticJsonDocument<200> doc_aux;  // Crea un documento JSON con espacio para 200

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
  
  Wire.beginTransmission(0x5A);
  Wire.write((const uint8_t*)b, (strlen(b)));
  Wire.endTransmission();

  // Ensure the slave processing is done and print it out
  delay(10);
  Serial.printf("Master Send: '%s'\r\n", b);

  // Read from the slave and print out
  Wire.requestFrom(0x5A, 199);

  Serial.print("\nrecv: '");

  while (Wire.available())
  {
    Serial.print((char)Wire.read());
  }

  Serial.println("'");
  delay(10);
}
