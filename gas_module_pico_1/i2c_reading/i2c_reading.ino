#include "Wire.h"

byte read_value;


void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(10000);
  Wire.setSDA(16);
  Wire.setSCL(17);
  Wire.begin();


}

void loop()
{
  // put your main code here, to run repeatedly:
  Serial.println("Starnting");
  Wire.beginTransmission(0X5C);
  Wire.write(0x03);
  Wire.endTransmission(false); // Не закрывать сессию передач
  Wire.requestFrom(0X5C, 4);
  //while(Wire.available())
  for(int i = 0;i<4;i++)
  {
    read_value = Wire.read();
    Serial.println(read_value,BIN);
  }
  Wire.endTransmission(true);
  Serial.println("end");
  delay(500);
}
