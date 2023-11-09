#include "printerservice.h"

void printing_logs()
{
  Serial.println("Print ALL LOGS");
  serializeJson(obj_log, Serial);

  uint32_t acumulado;

  // Iterar sobre cada objeto en el JsonArray
  for (JsonObject jsonObject : obj_log)
  {

    // Extraer los valores del objeto JSON
    uint32_t folio = jsonObject["folio"];
    uint32_t timestamp = jsonObject["timestamp"];
    uint32_t state = jsonObject["state"];
    uint32_t litros = jsonObject["litros"];
    uint32_t precio = jsonObject["precio"];
    uint32_t cliente = jsonObject["cliente"];
    float uprice = (float)precio / litros; // Asumiendo que uprice es el precio por litro

    // Convertir el timestamp a fecha y hora usando RTClib
    DateTime dt(timestamp);
    int dia_hoy = dt.day();
    int mes = dt.month();
    int anio = dt.year();
    int hora = dt.hour();
    int minuto = dt.minute();

    acumulado = acumulado + litros;

    // Llamar a la función printCheck con los valores extraídos
  }

  printReport(precio, acumulado, uint32_t(uprice * 100), dia_hoy, mes, (anio - 2000), hora, minuto, folio);
}

// --------------------------------------------------------------------------- printRepot
// printCheck worked. A ticket was printed
// the function i2c_write_blocking is for RP2040 (RPi Pico)
//Numero       letra          dia          mes       año       hora       minuto
void printReport (uint32_t num, uint32_t ltr, uint32_t unitprice, uint8_t d, uint8_t m, uint8_t y, uint8_t h, uint8_t mn, uint8_t f)
{
  
  Serial.print("LITROS: ");
  Serial.println(ltr);

  setPrintMode(0); // Configurar modo de impresión
  printString("        REPORTE \n\r");
  //printString("R.F.C.: GXA 550301 BP3\n\r");

  // Imprimir número de unidad y folio
  printString("EQUIPO: 002");
  printString("\n\r");
  setPrintMode(48); // Configurar modo de impresión
  printString("LITROS: ");
  printNumber(ltr);
  setPrintMode(0); // Configurar modo de impresión
  printString("\n\r");
  // Imprimir fecha y hora
  printString("FECHA DE IMPRESION:");
  DateTime now;
  now = rtc.now();
  //now = DateTime(end_timestamp);
  int mes = now.month();
  int anio = now.year();
  int dia_hoy = now.day();
  int hora = now.hour();
  int minuto = now.minute();
  printDateTime(dia_hoy, mes, anio, hora, minuto);
  printString("\n\r");
  //printDateTime(d, m, y, h, mn);
  printString("\n\r");

}

// --------------------------------------------------------------------------printCheck
//void printCheck(uint32_t num, uint32_t ltr, uint32_t unitprice, uint8_t d, uint8_t m, uint8_t y, uint8_t h, uint8_t mn, uint8_t f)
void printCheck(uint32_t num, uint32_t ltr, uint32_t unitprice, uint8_t f, uint32_t start_timestamp, uint32_t end_timestamp)
{
  //char resultado[150];
  //const char end1 = '\r';
  //const char end2 = '\n';

  // Imprimir Folio
  Serial.print("FOLIO: ");
  Serial.println(f);

  setPrintMode(0); // Configurar modo de impresión
  printString("VERACRUZANA DE GAS S.A. DE C.V.\n\r");
  //printString("R.F.C.: GXA 550301 BP3\n\r");

  // Imprimir número de unidad y folio
  printString("EQUIPO: 002");
  printString("\n\r");
  printString("NO. DE SERVICO: ");
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
  printDateTime(dia_hoy, mes, anio, hora, minuto);
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

  // Imprimir número total
  printString("TOTAL:  $");
  printNumber(num);
  printString("\n\r");

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
void convertNumberToWords(uint32_t num, char* resultado) {
  // Lógica para convertir números a palabras...
  // Utilizar sprintf o strcat según sea necesario
  //////////////////// convert numbers to words: ////////////////////////////////////
  if (num == 0) {
    strcpy(resultado, "CERO");
    //resultado = "cero";
  }
  else strcpy(resultado, " ");
  if (num >= 1000) {
    int miles = num / 1000;
    strcat(resultado, unidades[miles]);
    strcat(resultado, " MIL ");
    //resultado += unidades[miles];
    //resultado += " mil ";
    num %= 1000;
  }
  // Обработка сотен
  if (num >= 200) {
    int centenas = num / 100;
    strcat(resultado, unidades[centenas]);
    strcat(resultado, " CIENTOS ");
    //resultado += unidades[centenas];
    //  resultado += " cientos ";
    num %= 100;
  }
  else if (num > 100) {
    strcat(resultado, "CIENTO ");
    //resultado += "ciento ";
    num %= 100;
  }
  else if (num == 100) {
    strcat(resultado, "CIEN");
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
      strcat(resultado, "Y ");
      //resultado += "y ";
    }
  }
  else if (num > 20) {
    strcat(resultado, "VEINTI");
    //resultado += "veinti";
    num %= 10;
  }
  else if (num == 20) {
    strcat(resultado, "VEINTE");
    //resultado += "veinte";
    num %= 10;
  }
  else if (num > 15) {
    strcat(resultado, "DIECI");
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
    strcat(resultado, "DIEZ");
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
  uint32_t size = strlen(resultado);
  strncpy((char*)resultadoBytes, resultado, size);

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
  sprintf(buffer, "%u/%u/%u %u:%u", d, m, y, h, mn);
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