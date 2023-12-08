#include "printerservice.h"

// --------------------------------------------------------------------------- printRepot emty
void printReport(void)
{
  Serial.print("NO HAY REPORTES: ");


  setPrintMode(0); // Configurar modo de impresión
  printString("   NO HAY REPORTES");
  printString("\n\r");
  //printDateTime(dia_hoy, mes, anio, hora, minuto);
  endPrint();

  //reporte++;
  //obj["reporte"] = reporte;
  //status_doc["reporte"] = reporte;
  //saveConfig = true;

}

// --------------------------------------------------------------------------- printRepot
// printCheck worked. A ticket was printed
// the function i2c_write_blocking is for RP2040 (RPi Pico)
//Numero       letra          dia          mes       año       hora       minuto
void printReport (uint32_t servicios, uint32_t ltr, uint32_t total, uint32_t reporte, uint32_t acumulado_litros, uint32_t folio_ini, uint32_t folio_fin, uint32_t litros_ini, uint32_t litros_fin)
{

  // Numero de reporte
  //reporte = obj["reporte"].as<uint32_t>();
  //status_doc["reporte"] = reporte;
  //Serial.print("Reporte: ");
  //Serial.println(reporte);

  Serial.print("REPORTE: ");
  Serial.println(ltr);
  Serial.print("LITROS: ");
  Serial.println(ltr);
  Serial.print("SERVICIOS: ");
  Serial.println(servicios);
  Serial.print("LITROS GLOBALES: ");
  Serial.println(acumulado_litros);

  setPrintMode(0); // Configurar modo de impresión
  printString("        REPORTE #:");
  printNumber(reporte);
  printString("\n\r");
  printString("VERSION: ");
  printString(VERSION);
  printString("\n\r");

  // Imprimir número de unidad y folio
  printString("EQUIPO: ");
  printNumber(obj["id"]);
  printString("\n\r");
  setPrintMode(48); // Configurar modo de impresión
  printString("LITROS: ");
  printNumber(ltr);
  printString("\n\r");

  printString("GLOBAL: ");
  printNumber(acumulado_litros);
  printString("\n\r");

  printString("SERVICIOS: ");
  printNumber(servicios);
  printString("\n\r");
  printString("VENTA: ");
  printNumber(total);
  printString("\n\r");
  setPrintMode(0); // Configurar modo de impresión

  printString("LITROS INICIAL: ");
  printNumber(litros_ini);
  printString("\n\r");

  printString("LITROS FINAL: ");
  printNumber(litros_fin);
  printString("\n\r");

  printString("FOLIO INICIAL: ");
  printNumber(folio_ini);
  printString("\n\r");

  printString("FOLIO FINAL: ");
  printNumber(folio_fin);
  printString("\n\r");

  printString("PROXIMO SERVICIO: ");
  printNumber(folio_fin + 1);
  printString("\n\r");


  printString("\n\r");
  // Imprimir fecha y hora
  printString("FECHA:");
  DateTime now;
  now = rtc.now();
  //now = DateTime(end_timestamp);
  int mes = now.month();
  int anio = now.year();
  int dia_hoy = now.day();
  int hora = now.hour();
  int minuto = now.minute();
  printDateTime(dia_hoy, mes, (anio - 2000), hora, minuto);
  //printDateTime(dia_hoy, mes, anio, hora, minuto);
  endPrint();
  //printString("\n\r");

  reporte++;
  obj["reporte"] = reporte;
  status_doc["reporte"] = reporte;
  saveConfig = true;


}

// --------------------------------------------------------------------------printCheck
//void printCheck(uint32_t num, uint32_t ltr, uint32_t unitprice, uint8_t d, uint8_t m, uint8_t y, uint8_t h, uint8_t mn, uint8_t f)
void printCheck(uint32_t num, uint32_t ltr, uint32_t unitprice, uint32_t f, uint32_t start_timestamp, uint32_t end_timestamp)
{
  //char resultado[150];
  //const char end1 = '\r';
  //const char end2 = '\n';

  // Imprimir Folio
  Serial.print("FOLIO: ");
  Serial.println(f);

  setPrintMode(0); // Configurar modo de impresión
  printString("VERACRUZANA DE GAS S.A. DE C.V.\n\r");

  // Imprimir número de unidad y folio
  //String id_str = obj["id"].as<String>();
  const char * id_str = obj["id"].as<const char *>();
  Serial.print("ID: ");
  Serial.println(id_str);

  printString("EQUIPO: ");
  //printString(id_str.c_str());
  printString(id_str);





  printString("\n\r");
  printString("NO. DE SERVICIO: ");
  printNumber(f);
  printString("\n\r");
  // Imprimir fecha y hora
  printString("FECHA:");

  DateTime now;
  //now = rtc.now();
  now = DateTime(end_timestamp);
  int mes = now.month();
  int anio = now.year();
  int dia_hoy = now.day();
  int hora = now.hour();
  int minuto = now.minute();

  Serial.print("Time: ");
  Serial.println(now.unixtime());


  printDateTime(dia_hoy, mes, (anio - 2000), hora, minuto);
  printString("\n\r");
  //printString("Duración del servicio:");

  // Imprimir precio unitario
  printString("PRECIO UNITARIO: $");
  printPrice(unitprice);
  printString("\n\r");

  // Imprimir litros
  setPrintMode(48); // Configurar modo de impresión
  printString("LITROS: ");
  printNumber(ltr);
  printString("Lts\n\r");

  Serial.print("Litros: ");
  Serial.println(ltr);

  // Imprimir número total
  printString("TOTAL:  $");
  printNumber(num);
  printString("\n\r");

  Serial.print("Total: ");
  Serial.println(num);

  // Convertir y imprimir número en palabras
  setPrintMode(0); // Configurar modo de impresión
  convertNumberToWords(num, resultado);
  printString("\n\r");
  Serial.println(resultado);
  printString(resultado);
  printString("\n\r");


  // Resetear tamaño de texto y finalizar impresión
  setPrintMode(0);
  printString("GRACIAS POR SU PREFERENCIA");
  endPrint();

  return;
}


// --------------------------------------------------------------------------setPrintMode
void setPrintMode(uint8_t mode) {
  Wire.beginTransmission(0x5D);
  Wire.write(0x1B);
  Wire.write(0x21);
  Wire.write(mode);
  Wire.endTransmission();
  delay(100);
}

// --------------------------------------------------------------------------printString
void printString(const char* str) {
  Wire.beginTransmission(0x5D);
  //Wire.write(str, strlen(str));
  Wire.write(reinterpret_cast<const uint8_t*>(str), strlen(str));
  Wire.endTransmission();
}


// --------------------------------------------------------------------------printNumber
void printNumber(uint32_t num) {
  char buffer[20];
  sprintf(buffer, "%u", num);
  printString(buffer);
}


// --------------------------------------------------------------------------convertNumberToWords
void convertNumberToWords(uint32_t num, char* resultado) 
{
  if (num == 0) {
    strcpy(resultado, "CERO");
    return;
  } else {
    strcpy(resultado, "");
  }

  if (num >= 1000) {
    int miles = num / 1000;
    strcat(resultado, unidades[miles]);
    strcat(resultado, " MIL ");
    num %= 1000;
  }

  if (num >= 200) {
    int centenas = num / 100;
    strcat(resultado, unidades[centenas]);
    strcat(resultado, " CIENTOS ");
    num %= 100;
  } else if (num > 100) {
    strcat(resultado, "CIENTO ");
    num %= 100;
  } else if (num == 100) {
    strcat(resultado, "CIEN");
    num %= 100;
  }

  if (num > 29) {
    int decena = num / 10;
    strcat(resultado, decenas[decena]);
    if (num % 10 > 0) {
      strcat(resultado, " Y ");
    }
    num %= 10;
  } else if (num > 20) {
    strcat(resultado, "VEINTI");
    num %= 10;
  } else if (num == 20) {
    strcat(resultado, "VEINTE");
    num = 0;
  } else if (num > 15) {
    strcat(resultado, "DIECI");
    num %= 10;
  } else if (num >= 11) {
    strcat(resultado, especiales[num - 10]);
    num = 0;
  } else if (num == 10) {
    strcat(resultado, "DIEZ");
    num = 0;
  }

  if (num > 0) {
    strcat(resultado, unidades[num]);
  }
}

//-------------------------------------------- print price
void printPrice(uint32_t price) {
  char buffer[20];
  sprintf(buffer, "%u.%02u", price / 100, price % 100);
  printString(buffer);
}


// -------------------------------------------- printDateTime
void printDateTime(uint8_t d, uint8_t m, uint8_t y, uint8_t h, uint8_t mn) {
  char buffer[30];

  if (mn >= 10)
    sprintf(buffer, "%u/%u/%u %u:%u", d, m, y, h, mn);
  else
    sprintf(buffer, "%u/%u/%u %u:0%u", d, m, y, h, mn);


  printString(buffer);
}

// -------------------------------------------- endPrint
void endPrint() {
  Wire.beginTransmission(0x5D);
  Wire.write(0x1B);
  Wire.write(0x64);
  Wire.write(0x08);
  Wire.endTransmission();
}
