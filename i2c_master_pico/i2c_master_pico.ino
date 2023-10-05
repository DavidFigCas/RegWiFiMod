//#include <Wire.h>
//#define  LED_BUILTIN 25
#include "pico/stdlib.h"
#include "hardware/i2c.h"
volatile uint8_t REG_TODO = 1;

uint8_t SENSOR_ADDRESS = 0x5E; // Адрес датчика на шине I2C
const uint8_t client_data[4] = {10, 11, 12, 13};
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
  Serial.print("STATE<---: ");
  i2c_write_blocking(i2c0, 0x5E, &STATE, 1, true);
  i2c_read_blocking(i2c0, 0x5E, readValue, 1, false);
  Serial.println(readValue[0],BIN);
   delay(2000);

  Serial.println("CLIENT--->: ");
  i2c_write_blocking(i2c0, 0x5E, &CLIENT, 1, true);
  i2c_write_blocking(i2c0, 0x5E, client_data, 4, false);
  for (int i = 0; i < 4; i++)
    Serial.println(client_data[i]);

  Serial.println();
  digitalWrite(LED_BUILTIN, LOW);

  delay(5000);

}
