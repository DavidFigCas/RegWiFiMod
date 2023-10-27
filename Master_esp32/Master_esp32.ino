// MASTER (La Pico W no funciona como master)

#define SDA_MAIN    16
#define SCL_MAIN    17


#include <Wire.h>

static int p;
char b[200];

void setup() {
  Serial.begin(115200);
  delay(5000);
  //Wire.setSDA(SDA_MAIN);
  //Wire.setSCL(SCL_MAIN);
  Wire.begin();
}


void loop() { 

  // Write a value over I2C to the slave
  Serial.println("Sending...");
  Wire.beginTransmission(0x5A);
  sprintf(b, "{\"key\":%d}", p++);
  Wire.write((const uint8_t*)b, strlen(b));
  Wire.endTransmission();

  // Ensure the slave processing is done and print it out
  delay(10);
  Serial.printf("Master Send: '%s'\r\n", b);

  // Read from the slave and print out
  Wire.requestFrom(0x30, 199);
  
  Serial.print("\nrecv: '");
  
  while (Wire.available()) 
  {
    Serial.print((char)Wire.read());
  }
  
  Serial.println("'");
  delay(10);
}
