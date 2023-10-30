// MASTER (La Pico W no funciona como master)

#define SDA_MAIN    16
#define SCL_MAIN    17
#define ENCODE_ADD  0x5C
#define DISPLAY_ADD  0x5A


#include <Wire.h>
#include <ArduinoJson.h>

static int p;
char b[200];
static char buff[200];
int i;
volatile bool display_reset = false;
StaticJsonDocument<200> doc;  // Asegúrate de que el tamaño sea suficiente para tu objeto JSON
StaticJsonDocument<200> doc_aux;  // Crea un documento JSON con espacio para 200
StaticJsonDocument<200> doc_display;  // Crea un documento JSON con espacio para 200
StaticJsonDocument<200> doc_encoder;  // Crea un documento JSON con espacio para 200
String jsonStr;

void setup()
{
  Serial.begin(115200);
  delay(5000);
  //Wire.setSDA(SDA_MAIN);
  //Wire.setSCL(SCL_MAIN);
  Wire.begin();

  doc["precio"] = 9.5;
}


void loop()
{

  // Write a value over I2C to the slave
  //Serial.println("Sending...");
  //doc["precio"] = p++;
  //doc["litros"] = p++;






  // --------------------- leer encoder
  // Read from the slave and print out
  Serial.print("Encoder: ");
  Wire.requestFrom(ENCODE_ADD, 199);
  memset(buff, 0, sizeof(buff));
  i = 0;
  while (Wire.available())
  {
    buff[i] = Wire.read();
    //Serial.print((char)buff[i]);
    i++;
  }
  //Serial.println();

  jsonStr =  buff;
  //Serial.println(jsonStr);
  deserializeJson(doc_encoder, jsonStr);
  serializeJson(doc_encoder, Serial);
  Serial.println();
  delay(100);


  // --------------------- leer display
  // Read from the slave and print out
  Serial.print("Display: ");
  Wire.requestFrom(DISPLAY_ADD, 199);
  memset(buff, 0, sizeof(buff));
  i = 0;
  while (Wire.available())
  {
    buff[i] = Wire.read();
    //Serial.print((char)buff[i]);
    i++;
  }
  //Serial.println();

  jsonStr =  buff;
  //Serial.println(jsonStr);
  deserializeJson(doc_display, jsonStr);
  serializeJson(doc_display, Serial);
  Serial.println();
  delay(100);


  p = doc_encoder["pulses"].as<int>();


  if (doc_encoder["STATE"] ==  3)
  {
    Serial.println("Running");
    Serial.println("Litros: ");
    p = doc_encoder["current"].as<int>();
    Serial.println(p);
    display_reset = true;

  }


  // ---------------------- display doc
  //doc["litros"] = p;
  doc.clear();
  doc["wifi"] = true;
  doc["valve"] = doc_encoder["valve_open"].as<bool>();
  doc["gps"] = false;
  doc["clock"] = true;
  doc["printer"] = true;
  doc["paper"] = true;
  doc["litros"] = p;
  serializeJson(doc, b);
  Serial.print("Master to display: ");
  serializeJson(doc, Serial);
  Serial.println();


  Wire.beginTransmission(DISPLAY_ADD);
  Wire.write((const uint8_t*)b, (strlen(b)));
  Wire.endTransmission();
  delay(100);

  // ---------------------- encoder doc
  doc.clear();
  doc["reset"] = display_reset;
  doc["litros"] = p;
  serializeJson(doc, b);
  Serial.print("Master to encoder: ");
  serializeJson(doc, Serial);
  Serial.println();

  Wire.beginTransmission(ENCODE_ADD);
  Wire.write((const uint8_t*)b, (strlen(b)));
  Wire.endTransmission();

  delay(100 );
}
