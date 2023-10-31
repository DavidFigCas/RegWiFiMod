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
#include <stdio.h>
//#include "clock.h"

const char  end1 = '\r';
const char  end2 = '\n';
uint8_t tempVar = 0;
char tempChar;
uint8_t resultadoBytes[200];

char resultado[200];

const char* unidades[] = {"", "uno", "dos", "tres", "cuatro", "cinco", "seis", "siete", "ocho", "nueve"};
const char* decenas[] = {"", "diez", "veinte", "treinta", "cuarenta", "cincuenta", "sesenta", "setenta", "ochenta", "noventa"};
const char* especiales[] = {"diez", "once", "doce", "trece", "catorce", "quince"};
//uint32_t unitprice;


const unsigned long intervalo = 10000;
unsigned long tiempoAnterior = 0;
unsigned long tiempoActual;

const unsigned long intervalo2 = 10000;
unsigned long tiempoAnterior2 = 0;
unsigned long tiempoActual2;
volatile bool startCounting2 = false;


uint32_t litros;
unsigned int pulsos_litro = 10;
uint32_t precio;
float uprice = 9.8; //price of 1 litre
uint32_t litros_check;
uint32_t precio_check;

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

  //printCheck(432, 123, uint32_t (uprice * 100), 1, 11, 23, 14, 45, 6);

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


  //litros = ((doc_encoder["current"].as<unsigned int>()) / pulsos_litro);
  litros = (doc_encoder["current"].as<uint32_t>() / pulsos_litro);
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
    printCheck(uint32_t (precio_check), uint32_t(litros_check), uint32_t (uprice * 100), 1, 11, 23, 14, 45, 6);
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
        litros_check = litros;
        precio_check = precio;
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


// --------------------------------------------------------------------------- printCheck
// printCheck worked. A ticket was printed
// the function i2c_write_blocking is for RP2040 (RPi Pico)
//Numero       letra          dia          mes       año       hora       minuto
void printCheck (uint32_t num, uint32_t ltr, uint32_t unitprice, uint8_t d, uint8_t m, uint8_t y, uint8_t h, uint8_t mn, uint8_t f) {
  //char* resultado = "";
  char resultado[150]; // ??
  //const char* Total = "TOTAL $"; // header of ticket
  const char  end1 = '\r'; // chars used many times
  const char  end2 = '\n';
  uint8_t resultadoBytes[100]; // ??
  uint8_t tempVar = 0; // hold bytes for commands
  char tempChar; // ??
  //  uint32_t tempnum = 0; // ??

  //Set text double size
  // first command ESC ! <1B>H<21>H<n> Set print mode '0'
  Serial.println("Printing...");
  tempVar = 0x1B;
  //i2c_write_blocking(i2c0, 0x5D, (const uint8_t *)&tempVar, 1, false);
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempVar, 1);
  Wire.endTransmission();
  tempVar = 0x21;
  //i2c_write_blocking(i2c0, 0x5D, (const uint8_t *)&tempVar, 1, false);
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempVar, 1);
  Wire.endTransmission();
  tempVar = 48; // char '0'
  //i2c_write_blocking(i2c0, 0x5D, (const uint8_t *)&tempVar, 1, false);
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempVar, 1);
  Wire.endTransmission();
  delay(100);
  //Send "Total   $" (9 chars)
  // "converting" data type (chars to bytes)
  //Serial.println(Total);
  //strncpy((char*)resultadoBytes, Total, 8);
  //Serial.print(resultadoBytes);
  //resultadoBytes[BUFFER_SIZE - 1] = '\0';
  //i2c_write_blocking(i2c0, 0x5D, resultadoBytes, 9, false);
  //Wire.beginTransmission(0x5D);
  //Wire.write((const uint8_t *)&tempVar, 8);
  //Wire.endTransmission();

  strcpy(resultado, "TOTAL   $");
  size_t size = strlen(resultado);
  strncpy((char*)resultadoBytes, resultado, size);

  Wire.beginTransmission(0x5D);
  Wire.write(resultadoBytes, size);
  Wire.endTransmission();

  // num is the total cost of gas
  sprintf(resultado, "%u", num);
  size = strlen(resultado);
  strncpy((char*)resultadoBytes, resultado, size);
  //i2c_write_blocking(i2c0, 0x5D, resultadoBytes, size, false);
  Wire.beginTransmission(0x5D);
  Wire.write(resultadoBytes, size);
  Wire.endTransmission();
  //sending end of string
  tempChar = end1;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempChar, 1);
  Wire.endTransmission();
  tempChar = end2;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempChar, 1);
  Wire.endTransmission();
  //Reset double size
  tempVar = 0x1B;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempVar, 1);
  Wire.endTransmission();
  tempVar = 0x21;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempVar, 1);
  Wire.endTransmission();
  tempVar = 0;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempVar, 1);
  Wire.endTransmission();
  //////////////////// convert numbers to words: ////////////////////////////////////
  if (num == 0) {
    strcpy(resultado, "cero");
    //resultado = "cero";
  }
  else strcpy(resultado, " ");
  if (num >= 1000) {
    int miles = num / 1000;
    strcat(resultado, unidades[miles]);
    strcat(resultado, " mil ");
    //resultado += unidades[miles];
    //resultado += " mil ";
    num %= 1000;
  }
  // Обработка сотен
  if (num >= 200) {
    int centenas = num / 100;
    strcat(resultado, unidades[centenas]);
    strcat(resultado, " cientos ");
    //resultado += unidades[centenas];
    //  resultado += " cientos ";
    num %= 100;
  }
  else if (num > 100) {
    strcat(resultado, "ciento ");
    //resultado += "ciento ";
    num %= 100;
  }
  else if (num == 100) {
    strcat(resultado, "cien");
    //resultado += "cien";
    num %= 100;
  }

  // Обработка десятков
  if (num > 29) {
    int decena = num / 10;
    strcat(resultado, decenas[decena]);
    strcat(resultado, " ");
    //resultado += decenas[decena];
    //resultado += " ";
    num %= 10;
    if (num > 0) {
      strcat(resultado, "y ");
      //resultado += "y ";
    }
  }
  else if (num > 20) {
    strcat(resultado, "veinti");
    //resultado += "veinti";
    num %= 10;
  }
  else if (num == 20) {
    strcat(resultado, "veinte");
    //resultado += "veinte";
    num %= 10;
  }
  else if (num > 15) {
    strcat(resultado, "dieci");
    strcat(resultado, unidades[num - 10]);
    //resultado += "dieci";
    //resultado +=  unidades[num - 10];
    num %= 10;
  }
  else if (num >= 11) {
    strcat(resultado, especiales[num - 10]);
    //strcat(resultado, " ");
    //resultado += especiales[num - 10];
    num %= 10;
  }
  else if (num == 10) {
    strcat(resultado, "diez");
    //resultado += "diez";
    num %= 10;
  }
  Serial.println (num);
  // Обработка единиц
  if (num > 0) {
    strcat(resultado, unidades[num]);
    //resultado += unidades[num];
  }
  Serial.println(resultado);
  size = strlen(resultado);
  strncpy((char*)resultadoBytes, resultado, size);
  Wire.beginTransmission(0x5D);
  Wire.write(resultadoBytes, size);
  Wire.endTransmission();
  //sending end of string
  tempChar = end1;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempChar, 1);
  Wire.endTransmission();
  tempChar = end2;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempChar, 1);
  Wire.endTransmission();
  delay(200);
  ////////////// end of conversion /////////////////////

  ////////////// Print unit  price /////////////////////
  strcpy(resultado, "Precio U. $");
  size = strlen(resultado);
  strncpy((char*)resultadoBytes, resultado, size);

  Wire.beginTransmission(0x5D);
  Wire.write(resultadoBytes, size);
  Wire.endTransmission();
  //resultado = "Precio U.   $";
  strcpy(resultado, " ");
  uint8_t tempnum = unitprice / 100;
  //uint8_t tempnum = uprice;

  Serial.print("number:"); Serial.println(tempnum);
  sprintf(resultado, "%u", tempnum);
  strcat(resultado, ".");
  Serial.print("string:"); Serial.println(resultado);
  size = strlen(resultado);
  strncpy((char*)resultadoBytes, resultado, size);

  Wire.beginTransmission(0x5D);
  Wire.write(resultadoBytes, size);
  Wire.endTransmission();

  strcpy(resultado, " ");
  tempnum = unitprice % 100;
  Serial.print("number:"); Serial.println(tempnum);
  sprintf(resultado, "%u", tempnum);
  Serial.print("string:"); Serial.println(resultado);
  size = strlen(resultado);
  strncpy((char*)resultadoBytes, resultado, size);

  Wire.beginTransmission(0x5D);
  Wire.write(resultadoBytes, size);
  Wire.endTransmission();
  //sending end of string
  tempChar = end1;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempChar, 1);
  Wire.endTransmission();
  tempChar = end2;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempChar, 1);
  Wire.endTransmission();

  ////////////// Print date and time ////////////////////////////////
  strcpy(resultado, "");
  sprintf(resultado, "%u", d);
  size = strlen(resultado);
  strncpy((char*)resultadoBytes, resultado, size);

  Wire.beginTransmission(0x5D);
  Wire.write(resultadoBytes, size);
  Wire.endTransmission();

  strcpy(resultado, "/");
  size = strlen(resultado);
  strncpy((char*)resultadoBytes, resultado, size);

  Wire.beginTransmission(0x5D);
  Wire.write(resultadoBytes, size);
  Wire.endTransmission();

  strcpy(resultado, "");
  sprintf(resultado, "%u", m);
  size = strlen(resultado);
  strncpy((char*)resultadoBytes, resultado, size);

  Wire.beginTransmission(0x5D);
  Wire.write(resultadoBytes, size);
  Wire.endTransmission();

  strcpy(resultado, "/");
  size = strlen(resultado);
  strncpy((char*)resultadoBytes, resultado, size);

  Wire.beginTransmission(0x5D);
  Wire.write(resultadoBytes, size);
  Wire.endTransmission();

  strcpy(resultado, "");
  sprintf(resultado, "%u", y);
  size = strlen(resultado);
  strncpy((char*)resultadoBytes, resultado, size);

  Wire.beginTransmission(0x5D);
  Wire.write(resultadoBytes, size);
  Wire.endTransmission();

  strcpy(resultado, " ");
  size = strlen(resultado);
  strncpy((char*)resultadoBytes, resultado, size);

  Wire.beginTransmission(0x5D);
  Wire.write(resultadoBytes, size);
  Wire.endTransmission();

  strcpy(resultado, "");
  sprintf(resultado, "%u", h);
  size = strlen(resultado);
  strncpy((char*)resultadoBytes, resultado, size);

  Wire.beginTransmission(0x5D);
  Wire.write(resultadoBytes, size);
  Wire.endTransmission();

  strcpy(resultado, ":");
  size = strlen(resultado);
  strncpy((char*)resultadoBytes, resultado, size);

  Wire.beginTransmission(0x5D);
  Wire.write(resultadoBytes, size);
  Wire.endTransmission();

  strcpy(resultado, "");
  sprintf(resultado, "%u", mn);
  size = strlen(resultado);
  strncpy((char*)resultadoBytes, resultado, size);

  Wire.beginTransmission(0x5D);
  Wire.write(resultadoBytes, size);
  Wire.endTransmission();

  tempChar = end1;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempChar, 1);
  Wire.endTransmission();
  tempChar = end2;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempChar, 1);
  Wire.endTransmission();

  ////////////// Print unit number ///////////////////////////////////////

  strcpy(resultado, "Unit 002");
  size = strlen(resultado);
  strncpy((char*)resultadoBytes, resultado, size);

  Wire.beginTransmission(0x5D);
  Wire.write(resultadoBytes, size);
  Wire.endTransmission();

  tempChar = end1;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempChar, 1);
  Wire.endTransmission();
  tempChar = end2;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempChar, 1);
  Wire.endTransmission();

  ///////// Print folio number ///////////////////////////////////////////

  strcpy(resultado, "Folio  ");
  //resultado = "Folio    1";
  size = strlen(resultado);
  strncpy((char*)resultadoBytes, resultado, size);

  Wire.beginTransmission(0x5D);
  Wire.write(resultadoBytes, size);
  Wire.endTransmission();

  sprintf(resultado, "%u", f);
  size = strlen(resultado);
  strncpy((char*)resultadoBytes, resultado, size);

  Wire.beginTransmission(0x5D);
  Wire.write(resultadoBytes, size);
  Wire.endTransmission();

  tempChar = end1;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempChar, 1);
  Wire.endTransmission();
  tempChar = end2;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempChar, 1);
  Wire.endTransmission();

  ///////////////// Set double size and print litros //////////////////////

  tempVar = 0x1B;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempVar, 1);
  Wire.endTransmission();
  tempVar = 0x21;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempVar, 1);
  Wire.endTransmission();
  tempVar = 48;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempVar, 1);
  Wire.endTransmission();

  strcpy(resultado, "LITROS   ");
  //resultado = "LITROS   ";
  size = strlen(resultado);
  strncpy((char*)resultadoBytes, resultado, size);

  Wire.beginTransmission(0x5D);
  Wire.write(resultadoBytes, size);
  Wire.endTransmission();

  sprintf(resultado, "%u", ltr);
  size = strlen(resultado);
  strncpy((char*)resultadoBytes, resultado, size);

  Wire.beginTransmission(0x5D);
  Wire.write(resultadoBytes, size);
  Wire.endTransmission();

  tempChar = end1;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempChar, 1);
  Wire.endTransmission();
  tempChar = end2;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempChar, 1);
  Wire.endTransmission();

  /////////// Reset double size  ///////////////////////////////
  tempVar = 0x1B;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempVar, 1);
  Wire.endTransmission();
  tempVar = 0x21;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempVar, 1);
  Wire.endTransmission();
  tempVar = 0;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempVar, 1);
  Wire.endTransmission();

  strcpy(resultado, "Gracias por su preferencia");
  //resultado = "Gracias por su preferencia";
  size = strlen(resultado);
  strncpy((char*)resultadoBytes, resultado, size);

  Wire.beginTransmission(0x5D);
  Wire.write(resultadoBytes, size);
  Wire.endTransmission();

  tempChar = end1;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempChar, 1);
  Wire.endTransmission();
  tempChar = end2;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempChar, 1);
  Wire.endTransmission();

  /////////// End printing /////////////////////////////////



  //end print
  /*
    tempVar = 0x1D;
    Wire.beginTransmission(0x5D);
    Wire.write((const uint8_t *)&tempVar, 1);
    Wire.endTransmission();
    tempVar = (uint8_t)('V');
    Wire.beginTransmission(0x5D);
    Wire.write((const uint8_t *)&tempVar, 1);
    Wire.endTransmission();
    tempVar = 0x66;
    Wire.beginTransmission(0x5D);
    Wire.write((const uint8_t *)&tempVar, 1);
    Wire.endTransmission();
    tempVar = 0xA;
    Wire.beginTransmission(0x5D);
    Wire.write((const uint8_t *)&tempVar, 1);
    Wire.endTransmission();*/
  //<1B>H<64>H<n>
  tempVar = 0x1B;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempVar, 1);
  Wire.endTransmission();
  tempVar = 0x64;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempVar, 1);
  Wire.endTransmission();
  tempVar = 0x08;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempVar, 1);
  Wire.endTransmission();
  tempChar = end1;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempChar, 1);
  Wire.endTransmission();
  tempChar = end2;
  Wire.beginTransmission(0x5D);
  Wire.write((const uint8_t *)&tempChar, 1);
  Wire.endTransmission();

}
