#include "Wire.h"

void setup() {
  Serial.begin(115200);
  //#define SDA_MAIN    16
//#define SCL_MAIN    17
 Wire.setClock(100000);
  Wire.setSDA(16);
  Wire.setSCL(17);
  Wire.begin();

}

void loop() {
  byte error, address;
  int nDevices = 0;

  delay(5000);

  Serial.println("Scanning for I2C devices ...");
  for (address = 0x01; address < 0x7f; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    delay(100);
    if (error == 0) {
      Serial.printf("I2C device found at address 0x%02X\n", address);
      nDevices++;
    } else if (error != 2) {
      Serial.printf("Error %d at address 0x%02X\n", error, address);
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found");
  }
}
