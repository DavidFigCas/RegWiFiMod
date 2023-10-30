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
StaticJsonDocument<200> doc;  // Asegúrate de que el tamaño sea suficiente para tu objeto JSON
StaticJsonDocument<200> doc_aux;  // Crea un documento JSON con espacio para 200
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
  doc["wifi"] = true;
  doc["valve"] = false;
  doc["gps"] = false;
  doc["clock"] = true;
  doc["printer"] = true;
  doc["paper"] = true;


  serializeJson(doc, b);
  //Serial.println(b);

  //Wire.beginTransmission(0x5C);
  //Wire.write((const uint8_t*)b, (strlen(b)));
  //Wire.endTransmission();

  // Ensure the slave processing is done and print it out
  //delay(10);
  //Serial.printf("Master Send: '%s'\r\n", b);

  // Read from the slave and print out
  //Wire.requestFrom(0x5C, 199);

  //Serial.print("\nrecv: '");

  //while (Wire.available())
  //{
  //Serial.print((char)Wire.read());
  //}

  //Serial.println("'");
  //delay(1000);

  


  // --------------------- leer encoder
  // Read from the slave and print out
  Wire.requestFrom(ENCODE_ADD, 199);
  memset(buff, 0, sizeof(buff));
  int i;
  while (Wire.available())
  {
    buff[i] = Wire.read();
    //Serial.print((char)buff[i]);
    i++;
  }
  Serial.println();

  jsonStr =  buff;
  //Serial.println(jsonStr);
  deserializeJson(doc_aux, jsonStr);
  serializeJson(doc_aux, Serial);
  Serial.println();

  if(doc_aux["STATE"] ==  3)
  {
    Serial.println("Running");
    Serial.println("Litros: ");
    p = doc_aux["current"].as<int>();
    Serial.println(p);
    doc["reset"] = true;
    doc["litros"] = p;

    
    serializeJson(doc, b);
    
    

    Wire.beginTransmission(DISPLAY_ADD);
    Wire.write((const uint8_t*)b, (strlen(b)));
    Wire.endTransmission();

    Wire.beginTransmission(ENCODE_ADD);
    Wire.write((const uint8_t*)b, (strlen(b)));
    Wire.endTransmission();
    
  }

  
  delay(1000);
}
