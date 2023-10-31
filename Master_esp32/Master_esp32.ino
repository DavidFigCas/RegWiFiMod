// MASTER (La Pico W no funciona como master)

#define SDA_MAIN    16
#define SCL_MAIN    17
#define ENCODE_ADD  0x5C
#define DISPLAY_ADD  0x5A
#define TIME_SPACE  10


#include <Wire.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "time.h"
#include "RTClib.h"
//#include "clock.h"


const unsigned long intervalo = 10000;
unsigned long tiempoAnterior = 0;
unsigned long tiempoActual;

const unsigned long intervalo2 = 10000;
unsigned long tiempoAnterior2 = 0;
unsigned long tiempoActual2;
volatile bool startCounting2 = false;


unsigned int litros;
unsigned int pulsos_litro = 10;
unsigned int precio;
float uprice = 9.8; //price of 1 litre

static int p;
char b[200];
static char buff[200];
int i;
volatile bool display_reset = false;
volatile bool start_print = false;
volatile bool startCounting = false;


volatile uint32_t pesos;


StaticJsonDocument<200> doc;  // Asegúrate de que el tamaño sea suficiente para tu objeto JSON
StaticJsonDocument<200> doc_aux;  // Crea un documento JSON con espacio para 200
StaticJsonDocument<200> doc_display;  // Crea un documento JSON con espacio para 200
StaticJsonDocument<200> doc_encoder;  // Crea un documento JSON con espacio para 200
String jsonStr;
unsigned int STATE_DISPLAY = 1;

void setup()
{
  delay(100);
  Serial.begin(115200);
  delay(5000);
  //Wire.setSDA(SDA_MAIN);
  //Wire.setSCL(SCL_MAIN);
  Wire.begin();
  delay(5000);
  Serial.println("Main Logic START");

}


void loop()
{

  // Write a value over I2C to the slave
  //Serial.println("Sending...");
  //doc["precio"] = p++;
  //doc["litros"] = p++;


  // ----------------------------------------------- leer

  // --------------------- leer display
  // Read from the slave and print out
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


  delay(TIME_SPACE);

  // --------------------- leer encoder
  // Read from the slave and print out
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


  // ----------------------------------- Serial Monitor

  Serial.print("Display: ");
  serializeJson(doc_display, Serial);
  Serial.println();


  Serial.print("Encoder: ");
  serializeJson(doc_encoder, Serial);
  Serial.println();

  delay(TIME_SPACE);

  // ----------------------------------------------- procesar


  litros = ((doc_encoder["current"].as<unsigned int>()) / pulsos_litro);
  precio = litros * uprice;
  display_reset = false;


  // if ((doc_display["STATE"] ==  0) || (doc_display["STATE"].inNull()))


  //if (doc_display["STATE"] ==  0)
  //{
  //  STATE_DISPLAY = 1;
  //}
  //else
  //if (!doc_display["STATE"].isNull())
  //{
  //  if (doc_display["STATE"] >  0)
  //    STATE_DISPLAY = doc_display["STATE"];

    //if(STATE_DISPLAY == 1)
  //}

  // ------------------------------------- printer
  if (STATE_DISPLAY == 3)
  {
   Serial.println("Display on 3, reset");
   /* if (!startCounting2)
    {
      // Detectado por primera vez
      tiempoAnterior2 = millis();
      startCounting2 = true;
      Serial.println("Printer START");
      //Serial.print("Litros: ");
      //Serial.println(litros);
      //STATE_DISPLAY = 2;
    }
    else
    {
      // Ya se ha detectado antes, verificar el intervalo
      tiempoActual2 = millis();
      if (tiempoActual2 - tiempoAnterior2 >= intervalo2)
      {
        // Ha pasado 1 minuto
        //display_reset = true;
        startCounting2 = false;  // Detener el conteo
        //if (STATE_DISPLAY == 3)
          STATE_DISPLAY = 0;
        Serial.println("Printer Finish");
      }
    }*/
    delay(10000);
    STATE_DISPLAY = 0;
    Serial.println("Done reset");
  }
  else
  {
    // Si STATE no es 3, resetear el conteo
   // startCounting = false;
  }

   
  // ------------------------------------- encoder Read and stop
  if (doc_encoder["STATE"] == 3)
  {
    if (!startCounting)
    {
      // Detectado por primera vez
      tiempoAnterior = millis();
      startCounting = true;
      Serial.println("STOP FLOWING");
      Serial.print("Litros: ");
      Serial.println(litros);
      STATE_DISPLAY = 2;
    }
    else
    {
      // Ya se ha detectado antes, verificar el intervalo
      tiempoActual = millis();
      if (tiempoActual - tiempoAnterior >= intervalo)
      {
        // Ha pasado 1 minuto
        display_reset = true;
        startCounting = false;  // Detener el conteo
        //if (STATE_DISPLAY == 3)
          STATE_DISPLAY = 3;
        Serial.println("Display Bing Printer");
      }
    }
  }
  else
  {
    // Si STATE no es 3, resetear el conteo
    startCounting = false;
  }

  



  // ----------------------------------------------- enviar


  // ---------------------- display doc
  doc.clear();
  
  if ((!doc_display["STATE"].isNull()) && (doc_display["STATE"] == 0))
  {
    doc["valve"] = doc_encoder["valve_open"].as<bool>();
    doc["wifi"] = true;
    doc["gps"] = false;
    doc["clock"] = true;
    doc["printer"] = true;
    doc["paper"] = true;
    STATE_DISPLAY = 1;
    
  }
  else
  {
    doc["flow"] = doc_encoder["flow"].as<bool>();
    doc["litros"] = litros;
    doc["precio"] = precio;
  }
  doc["STATE"] = STATE_DISPLAY;
  serializeJson(doc, b);
  //Serial.print("Master to display: ");
  //serializeJson(doc, Serial);
  //Serial.println();


  Wire.beginTransmission(DISPLAY_ADD);
  Wire.write((const uint8_t*)b, (strlen(b)));
  Wire.endTransmission();
  delay(TIME_SPACE);

  // ---------------------- encoder doc
  doc.clear();
  doc["reset"] = display_reset;
  doc["litros"] = litros;
  serializeJson(doc, b);
  //Serial.print("Master to encoder: ");
  //serializeJson(doc, Serial);
  //Serial.println();

  Wire.beginTransmission(ENCODE_ADD);
  Wire.write((const uint8_t*)b, (strlen(b)));
  Wire.endTransmission();

  delay(TIME_SPACE);
}
