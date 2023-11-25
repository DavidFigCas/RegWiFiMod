#include "wireservice.h"

void I2C_Init()
{
  //Wire.setSDA(SDA_MAIN);
  //Wire.setSCL(SCL_MAIN);
  Wire.begin();
  //delay(5000);
  //Serial.println("Main Logic START");
}


void I2C_Get()
{
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
}


void I2C_Put()
{
  // ----------------------------------------------- enviar


  // ---------------------- display doc
  doc_aux.clear();

  if ((!doc_display["STATE"].isNull()) && (doc_display["STATE"] == 0))
  {
    doc_aux["valve"] = doc_encoder["valve_open"].as<bool>();
    doc_aux["wifi"] = true;
    doc_aux["gps"] = false;
    doc_aux["clock"] = true;
    doc_aux["printer"] = true;
    doc_aux["paper"] = true;
    STATE_DISPLAY = 1;

  }
  else
  {
    doc_aux["flow"] = doc_encoder["flow"].as<bool>();
    doc_aux["litros"] = litros;
    doc_aux["precio"] = precio;
  }
  doc_aux["STATE"] = STATE_DISPLAY;
  serializeJson(doc, b);
  //Serial.print("Master to display: ");
  //serializeJson(doc, Serial);
  //Serial.println();


  Wire.beginTransmission(DISPLAY_ADD);
  Wire.write((const uint8_t*)b, (strlen(b)));
  Wire.endTransmission();
  delay(TIME_SPACE);

  // ---------------------- encoder doc
  doc_aux.clear();
  doc_aux["reset"] = display_reset;
  doc_aux["litros"] = litros;
  serializeJson(doc, b);
  //Serial.print("Master to encoder: ");
  //serializeJson(doc, Serial);
  //Serial.println();

  Wire.beginTransmission(ENCODE_ADD);
  Wire.write((const uint8_t*)b, (strlen(b)));
  Wire.endTransmission();

  delay(TIME_SPACE);
}
