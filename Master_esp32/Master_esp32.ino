// MASTER (La Pico W no funciona como master)

#define SDA_MAIN    16
#define SCL_MAIN    17
#define ENCODE_ADD  0x5C
#define DISPLAY_ADD  0x5A
#define TIME_SPACE  1000


#include <Wire.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "time.h"
#include "RTClib.h"
//#include "clock.h"
const unsigned long intervalo = 20000;
unsigned long tiempoAnterior = 0;
unsigned long tiempoActual;
unsigned int litros;

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
  delay(5000);

}


void loop()
{

  // Write a value over I2C to the slave
  //Serial.println("Sending...");
  //doc["precio"] = p++;
  //doc["litros"] = p++;




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
  delay(TIME_SPACE);

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
  delay(TIME_SPACE);


  litros = doc_encoder["current"].as<unsigned int>();
  display_reset = false;


  if (doc_encoder["STATE"] ==  3)
  {
    tiempoActual = millis();
    Serial.println("STOP FLOWING");
    Serial.print("Litros: ");
    Serial.println(litros);
    

    if (tiempoActual - tiempoAnterior >= intervalo) 
    {
      // Ha pasado 1 minuto
      tiempoAnterior = tiempoActual;  // Actualiza la última vez que se activó el retardo
      display_reset = true;
      Serial.println("Display Reset");
      
    }

  }


  // ---------------------- display doc
  doc.clear();
  doc["flow"] = doc_encoder["flow"].as<bool>();  
  doc["litros"] = litros;
  doc["valve"] = doc_encoder["valve_open"].as<bool>();
  doc["precio"] = 9.5;
  doc["wifi"] = true;
  doc["gps"] = false;
  doc["clock"] = true;
  doc["printer"] = true;
  doc["paper"] = true;
  
  
  
  serializeJson(doc, b);
  Serial.print("Master to display: ");
  serializeJson(doc, Serial);
  Serial.println();


  Wire.beginTransmission(DISPLAY_ADD);
  Wire.write((const uint8_t*)b, (strlen(b)));
  Wire.endTransmission();
  delay(TIME_SPACE);

  // ---------------------- encoder doc
  doc.clear();
  doc["reset"] = display_reset;
  doc["litros"] = litros;
  serializeJson(doc, b);
  Serial.print("Master to encoder: ");
  serializeJson(doc, Serial);
  Serial.println();

  Wire.beginTransmission(ENCODE_ADD);
  Wire.write((const uint8_t*)b, (strlen(b)));
  Wire.endTransmission();

  delay(TIME_SPACE);
}
