
#include <Wire.h>
#include <stdio.h>
//static int p;
char b[200];
const char  end1 = '\r';
const char  end2 = '\n';
uint8_t tempVar = 0;
char tempChar;
uint8_t resultadoBytes[200];

char resultado[200];

const char* unidades[] = {"", "uno", "dos", "tres", "cuatro", "cinco", "seis", "siete", "ocho", "nueve"};
const char* decenas[] = {"", "diez", "veinte", "treinta", "cuarenta", "cincuenta", "sesenta", "setenta", "ochenta", "noventa"};
const char* especiales[] = {"diez", "once", "doce", "trece", "catorce", "quince"};
uint32_t unitprice = 950;

void setup()
{
  Serial.begin(115200);
  delay(500);
  //Wire.setSDA(SDA_MAIN);
  //Wire.setSCL(SCL_MAIN);
  Wire.begin();
  delay(1000);

}


void loop()
{

  printCheck(999, 141, 1, 11, 23, 14, 45, 6);

 
  delay(10000);
}

// printCheck worked. A ticket was printed
// the function i2c_write_blocking is for RP2040 (RPi Pico)
//Numero       letra          dia          mes       año       hora       minuto
void printCheck (uint32_t num, uint32_t ltr, uint8_t d, uint8_t m, uint8_t y, uint8_t h, uint8_t mn, uint8_t f) {
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
  else if (num > 100){
    strcat(resultado, "ciento ");
    //resultado += "ciento ";
    num %= 100;
  }
  else if (num==100){
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
    if (num > 0){
      strcat(resultado, "y ");
      //resultado += "y ";
    }
  }
  else if (num > 20){
    strcat(resultado, "veinti");
    //resultado += "veinti";
    num %= 10;
  }
  else if (num==20){
    strcat(resultado, "veinte");
    //resultado += "veinte";
    num %= 10;
  }
  else if (num > 15){
    strcat(resultado, "dieci");
    strcat(resultado, unidades[num - 10]);
    //resultado += "dieci";
    //resultado +=  unidades[num - 10];
    num %= 10;
  }
  else if (num >=11){
    strcat(resultado, especiales[num - 10]);
    //strcat(resultado, " ");
    //resultado += especiales[num - 10];
    num %= 10;
  }
  else if (num==10){
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
  uint8_t tempnum = unitprice/100;
  
  Serial.print("number:");Serial.println(tempnum);
  sprintf(resultado, "%u", tempnum);
  strcat(resultado, ".");
  Serial.print("string:");Serial.println(resultado);
  size = strlen(resultado);
  strncpy((char*)resultadoBytes, resultado, size);
  
  Wire.beginTransmission(0x5D);
  Wire.write(resultadoBytes, size);
  Wire.endTransmission();
  
  strcpy(resultado, " ");
  tempnum = unitprice%100;
  Serial.print("number:");Serial.println(tempnum);
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
