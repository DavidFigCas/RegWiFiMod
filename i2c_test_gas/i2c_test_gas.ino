#include <Wire.h>

uint8_t SENSOR_ADDRESS=0x5E; // Адрес датчика на шине I2C

void setup() {
  Wire.begin(); // Инициализация I2C
  Serial.begin(115200); // Инициализация Serial
  delay(500); // Пауза для устойчивости
  Serial.println("Setup");
  
}

void loop() {
  uint8_t readValue;

  // Отправляем байт 0x01 и читаем один байт
  Wire.beginTransmission(SENSOR_ADDRESS);
  Wire.write(0x01);
  Wire.endTransmission(false); // Не закрывать сессию передачи
  //delay(1000);
  //Serial.println("try read");
  Wire.requestFrom(SENSOR_ADDRESS, 1);
  //uint8_t b = Wire.available();
  //Serial.println(b);
 // if (b !=0) {
    //Serial.println("try read");
    readValue = Wire.read();
    Serial.print("Read one byte: ");
    Serial.println(readValue, BIN);
 // }
  //delay(1000);
  Wire.endTransmission();
  delay(1000);
  // Отправляем байт 0x03 и читаем 4 байта
 
  Wire.beginTransmission(SENSOR_ADDRESS);
  Wire.write(0x07);
  Wire.endTransmission(false); // Не закрывать сессию передачи
  Wire.requestFrom(SENSOR_ADDRESS, 42);
  Serial.print("Read 42 bytes: ");
  while (Wire.available()) {
    readValue = Wire.read();
    Serial.print(char (readValue));
  }
  Wire.endTransmission();
  Serial.println();

  delay(5000); // Пауза между итерациями
}
