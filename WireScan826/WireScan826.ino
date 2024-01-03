#include "Wire.h"

#include <SoftwareSerial.h>
#define RX_PIN        -1
#define TX_PIN        PIN_PA4

SoftwareSerial mySerial(RX_PIN, TX_PIN); // Reemplaza RX_PIN y TX_PIN con los números de pin reales

void setup() {
  mySerial.begin(9600); // para depurar
  pinMode(PIN_PB1, INPUT_PULLUP);
  pinMode(PIN_PB0, INPUT_PULLUP);
  pinMode(PIN_PB3, OUTPUT);
  digitalWrite(PIN_PB3,HIGH);
  Wire.begin();
}

void loop() {
  byte error, address;
  int nDevices = 0;

  delay(5000);

  mySerial.println("Scanning for I2C devices ...");
  for(address = 0x01; address < 0x7f; address++){
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0){
      mySerial.printf("I2C device found at address 0x%02X\n", address);
      nDevices++;
    } else if(error != 2){
      mySerial.printf("Error %d at address 0x%02X\n", error, address);
    }
  }
  if (nDevices == 0){
    mySerial.println("No I2C devices found");
  }
}
