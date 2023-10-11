#include <Wire.h>
//#define  LED_BUILTIN 25
//#include "pico/stdlib.h"
//#include "hardware/i2c.h"
volatile uint8_t REG_TODO = 1;

uint8_t SENSOR_ADDRESS = 0x5E; // Адрес датчика на шине I2C
uint8_t client_data[4] = {110, 110, 111, 112};
uint32_t valor = 10032;

uint8_t readValue[4];
uint32_t nclient = 10000001;
uint8_t todo = 0b11000000;
//uint8_t readValue;
const uint8_t STATE = 1;
const uint8_t CLIENT = 0x03;
const uint8_t TODO = 2;
#define SDA_MAIN    16
#define SCL_MAIN    17

void setup() {
  //Wire.setClock(50000);
  pinMode(LED_BUILTIN, OUTPUT);
  //Wire.begin(); // Инициализация I2C
  //i2c_begin();
  i2c_init(i2c0, 100 * 1000);
  gpio_set_function(SDA_MAIN, GPIO_FUNC_I2C);
  gpio_set_function(SCL_MAIN, GPIO_FUNC_I2C);
  gpio_pull_up(SDA_MAIN);
  gpio_pull_up(SCL_MAIN);
  Serial.begin(115200); // Инициализация Serial
  delay(5000); // Пауза для устойчивости
  Serial.println("Setup");

  // ASK STATE
  digitalWrite(LED_BUILTIN, HIGH);

  

  // Отправляем байт 0x01 и читаем один байт


}

void loop()
{

  //10032
  digitalWrite(LED_BUILTIN, HIGH);
  client_data[0] = (valor >> 24) & 0xFF; // Byte más significativo
  client_data[1] = (valor >> 16) & 0xFF;
  client_data[2] = (valor >> 8) & 0xFF;
  client_data[3] = valor & 0xFF;        // Byte menos significativo


  Serial.print("nClient--->: "); Serial.println(valor);
  i2c_write_blocking(i2c0, 0x5E, &CLIENT, 1, true);
  i2c_write_blocking(i2c0, 0x5E, client_data, 4, false);
  for (int i = 0; i < 4; i++)
    Serial.println(client_data[i]);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("endC");
  Serial.println();
  delay(5000);
  valor++;

  /*digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("ToDo--->: ");
  i2c_write_blocking(i2c0, 0x5E, &TODO, 1, true);
  i2c_write_blocking(i2c0, 0x5E, &todo, 1, false);
  Serial.println(todo, BIN);

  Serial.println();
  digitalWrite(LED_BUILTIN, LOW);
  readValue[0] = 0b11111111;
  delay(5000);*/

}
