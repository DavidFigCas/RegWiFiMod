//#include <Wire.h>
//#define  LED_BUILTIN 25
#include "pico/stdlib.h"
#include "hardware/i2c.h"
volatile uint8_t REG_TODO = 1;

uint8_t SENSOR_ADDRESS = 0x5E; // Адрес датчика на шине I2C
uint8_t client_data[4] = {1,2,3,4};
uint8_t readValue[4];
uint32_t nclient = 10000001;
uint8_t todo = 0b11000000;
//uint8_t readValue;
const uint8_t STATE = 1;
const uint8_t CLIENT = 3;
#define SDA_MAIN    4
#define SCL_MAIN    5

void setup() {
  //Wire.setClock(50000);
  pinMode(LED_BUILTIN, OUTPUT);
  //Wire.begin(); // Инициализация I2C
  //i2c_begin();
  i2c_init(i2c0, 100 * 1000);
  gpio_set_function(4, GPIO_FUNC_I2C);
  gpio_set_function(5, GPIO_FUNC_I2C);
  gpio_pull_up(4);
  gpio_pull_up(5);
  Serial.begin(115200); // Инициализация Serial
  delay(500); // Пауза для устойчивости
  Serial.println("Setup");



  // Отправляем байт 0x01 и читаем один байт


}

void loop()
{

  // ASK STATE
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.print("STATE: ");
  i2c_write_blocking(i2c0, 0x5E, &STATE, 1, true);
  i2c_read_blocking(i2c0, 0x5E, readValue, 1, false);
  Serial.println(readValue[0]);

  i2c_write_blocking(i2c0, 0x5E, &CLIENT, 1, true);
  i2c_write_blocking(i2c0, 0x5E, &client_data, 4, true);
  
  digitalWrite(LED_BUILTIN, LOW);
  
  delay(5000);






  
  //i2c_read_blocking(i2c0, 0x5A, buffer, 1, false);
  //Wire.beginTransmission(SENSOR_ADDRESS);
  //Wire.write(0x01);
  //int error = Wire.endTransmission(false);

  //if (error != 0)
  //{
    //Serial.print("error 1: ");
    //Serial.print(error);
    //Serial.print("\t");
  //}

  
  //Wire.requestFrom(SENSOR_ADDRESS, 1);
  //readValue = Wire.read();
  //Wire.endTransmission();
  //error = Wire.endTransmission(true);
  //if (error != 0)
  //{
    //Serial.print("error 2: ");
    //Serial.print(error);
    //Serial.println("\t");
  //}
  //else
  //Serial.println(readValue, BIN);

  //delay(1000);


  // ASK NAME
  /*Serial.print("Read 42 bytes: ");
    Wire.beginTransmission(SENSOR_ADDRESS);
    Wire.write(0x07);
    error = Wire.endTransmission(false);
    if (error != 0)
    {
    Serial.print("error: ");
    Serial.print(error);
    Serial.print("\t");
    }
    Wire.requestFrom(SENSOR_ADDRESS, 42);
    //while (Wire.available())
    for (int i = 0; i < 42; i++ )
    {
    readValue = Wire.read();
    Serial.print(char (readValue));
    }
    Wire.endTransmission();
    Serial.println();
    delay(10000); // Пауза между итерациями*/

  // WRITE CLIENT
//  Serial.print("Write client: ");
//  Wire.beginTransmission(SENSOR_ADDRESS);
//  Wire.write(0x03);
//  error = Wire.endTransmission(false);
//  if (error != 0)
//  {
//    Serial.print("error 3: ");
//    Serial.print(error);
//    Serial.print("\t");
//  }
//  //else
//  //{
//  for (int i = 0; i <= 3; i++)
//  {
//    uint8_t byteToSend = (nclient >> (8 * i)) & 0xFF; // Obtener el byte correspondiente
//    Wire.write(byteToSend); // Enviar el byte
//    Serial.print(byteToSend, BIN);
//  }
//  //}
//
//  //Wire.endTransmission(true);
//  Serial.println();
//  error = Wire.endTransmission(true);
//  if (error != 0)
//  {
//    Serial.print("error 4: ");
//    Serial.print(error);
//    Serial.print("\t");
//  }
//  Serial.println("...end ");
//  Serial.println();
//  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
//  delay(10000);


  /*
    Serial.print("Write ToDo: ");
    Wire.beginTransmission(SENSOR_ADDRESS);
    Wire.write(0x02);
    //Wire.endTransmission(false); // Не закрывать сессию
    //for(int i = 0; i <= 3; i++)
    {
    Wire.write(todo);
    Serial.print(todo, BIN);
    }
    Wire.endTransmission();
    Serial.println();
    delay(1000);*/


}
