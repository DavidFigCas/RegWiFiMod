/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink
*/

#include <SoftwareSerial.h>
#include "Wire.h"
#include <AS5600.h>

#define RX_PIN   PIN_PB4
#define TX_PIN   PIN_PA5
//#define TX_PIN   PIN_PA0

int contador;
uint16_t p;

SoftwareSerial mySerial(RX_PIN, TX_PIN); // Reemplaza RX_PIN y TX_PIN con los números de pin reales
AS5600 encoder; 


// the setup function runs once when you press reset or power the board
void setup() {

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(PIN_PA3, OUTPUT);
  mySerial.begin(9600);
  Wire.begin();
  mySerial.write('A'); // Enviar el carácter 'A'

}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(PIN_PA3, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);                       // wait for a second
  digitalWrite(PIN_PA3, LOW);    // turn the LED off by making the voltage LOW
  delay(100);    
  mySerial.print("Sensor: "); // Enviar el carácter 'A'// wait for a second  
  p = encoder.getAngle();
  mySerial.println(p); // Enviar el carácter 'A'// wait for a second

  //byte error, address;
  //int nDevices = 0;

  delay(500);

 /* mySerial.println("Scanning for I2C devices ...");
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
  }*/
}
