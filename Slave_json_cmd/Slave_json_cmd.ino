// SLAVE (La Pico W solo funciona como esclavo)

#define SDA_MAIN    16
#define SCL_MAIN    17

#include <ArduinoJson.h>
#include <Wire.h>

static int ctr;
static char buff[200];
static char resp[200];
String jsonStr;

StaticJsonDocument<200> doc;  // Asegúrate de que el tamaño sea suficiente para tu objeto JSON
StaticJsonDocument<200> doc_aux;  // Crea un documento JSON con espacio para 200 bytes

void setup() {
  Serial.begin(115200);
  delay(2500);
  Wire.setSDA(SDA_MAIN);
  Wire.setSCL(SCL_MAIN);
  Wire.begin(0x5A);
  Wire.onReceive(recv);
  Wire.onRequest(req);


  doc["nombre"] = "John";
  doc["litros"] = 30;
  doc["precio de las tortillas"] = 9.5;

  // Serializar el objeto JSON en la variable resp
  serializeJson(doc, resp);

  // Ahora resp contiene el objeto JSON como una cadena
  Serial.println(resp);  // Salida: {"name":"John","age":30,"city":"New York"}
}

void loop()
{

  memset(resp, 0, sizeof(resp));
  Serial.printf("Slave Buff: '%s'\r\n", buff);

  //deserializeJson(doc_aux, jsonStr);  // (FUNCIONA)Serializa el documento JSON a una cadena
  //buff = jsonStr;
  //deserializeJson(doc_aux, buff);
  //Serial.println(buff);  // Salida: {"name":"John","age":30,"city":"New York"}

  //doc["precio"] = doc_aux["precio"];     //Commands
  //serializeJson(doc, resp);
  //Serial.println(resp);  // Salida: {"name":"John","age":30,"city":"New York"}
  delay(1000);


}

// These are called in an **INTERRUPT CONTEXT** which means NO serial port
// access (i.e. Serial.print is illegal) and no memory allocations, etc.

// Called when the I2C slave gets written to
void recv(int len)
{
  int i;
  memset(buff, 0, sizeof(buff));
  // Just stuff the sent bytes into a global the main routine can pick up and use
  for (i = 0; i < len; i++)
  {
    buff[i] = Wire.read();
  }
  //buff[i] = 0;
}

// Called when the I2C slave is read from
void req()
{
 // Wire.write(resp, 199);
 Wire.write((const uint8_t *)resp, 199);
}
