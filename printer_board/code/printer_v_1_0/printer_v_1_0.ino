#include <Wire.h>

byte RxByte;

#define ADDRESS 0x5D
uint8_t stat=248;
void I2C_RxHandler(int numBytes)
{
  while(Wire.available()) {  // Read Any Received Data
    RxByte = Wire.read();
    Serial.write(RxByte);
  }
}

void I2C_TxHandler(void)
{
  Wire.write(stat);
}

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_PA3, OUTPUT);
  digitalWrite(PIN_PA3, 0);
  Wire.begin(ADDRESS);
  Wire.onReceive(I2C_RxHandler);
  Wire.onRequest(I2C_TxHandler);
  Serial.begin(38400);
  Serial.write(0x10);    
  Serial.write(0x04);  
  Serial.write(1);
  delay(200);
  int result=-1;
  if (Serial.available() > 0) {
    // read the incoming byte:
    result = Serial.read();
  }
  if (result<0){
    stat &= ~(1 << 6);// 0 in 5 bite of stat register
    digitalWrite(PIN_PA3, 0);
  }
  else {
    stat |= (1 << 6);
    digitalWrite(PIN_PA3, 1);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
