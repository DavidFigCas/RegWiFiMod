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
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define RX_PIN   PIN_PB4
//#define TX_PIN   PIN_PA3
#define TX_PIN   PIN_PA5

int contador;
long p, bat;
long angulo;
long promedio_angulo;
long promedio_angulo_anterior;
uint8_t txData[4];

SoftwareSerial mySerial(RX_PIN, TX_PIN); // Reemplaza RX_PIN y TX_PIN con los números de pin reales
AS5600 encoder;


// --------------------------------------------------------------------- setup
void setup()
{

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(PIN_PA3, OUTPUT);
  //pinMode(PIN_PA7, OUTPUT);
  //digitalWrite(PIN_PA7, LOW);

  mySerial.begin(9600);
  Serial1.begin(9600); // START UART

  Wire.begin();
  mySerial.println("Encoder Init"); // Enviar el carácter 'A'
  digitalWrite(PIN_PA3, LOW);    // turn the LED off by making the voltage LOW
  delay(100);
  digitalWrite(PIN_PA3, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);
  digitalWrite(PIN_PA3, LOW);    // turn the LED off by making the voltage LOW
  delay(100);
  digitalWrite(PIN_PA3, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);
  digitalWrite(PIN_PA3, LOW);    // turn the LED off by making the voltage LOW
  delay(100);

  encoder.setPowerMode(3);
  encoder.setFastFilter(0);
  encoder.setSlowFilter(3);

  mySerial.print("AT:");
  Serial1.print("AT\r");
  delay(10);
  while (Serial1.available()) { // Verificar si hay datos disponibles en Serial1
    char data = Serial1.read(); // Leer un byte desde Serial1
    mySerial.write(data); // Enviar ese byte a mySerial
  }
  mySerial.println();


  mySerial.print("ID:");
  Serial1.print("AT$I=10\r");
  delay(10);
  while (Serial1.available()) { // Verificar si hay datos disponibles en Serial1
    char data = Serial1.read(); // Leer un byte desde Serial1
    mySerial.write(data); // Enviar ese byte a mySerial
  }
  mySerial.println();


  mySerial.print("PAC:");
  Serial1.print("AT$I=11\r");
  delay(10);
  while (Serial1.available()) { // Verificar si hay datos disponibles en Serial1
    char data = Serial1.read(); // Leer un byte desde Serial1
    mySerial.write(data); // Enviar ese byte a mySerial
  }
  mySerial.println();

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
}

// --------------------------------------------------------------------- loop
void loop()
{

  mySerial.print("AT:");
  Serial1.print("AT\r");
  delay(10);
  while (Serial1.available()) { // Verificar si hay datos disponibles en Serial1
    char data = Serial1.read(); // Leer un byte desde Serial1
    mySerial.write(data); // Enviar ese byte a mySerial
  }
  mySerial.println();

  bat = analogRead(PIN_PA4);
  if (encoder.magnetTooWeak()) //encoder.detectMagnet()
  {

    for (int i = 0; i < 10; i++)
    {
      angulo = encoder.readAngle();
      p = p + angulo;
      mySerial.println(angulo); // Enviar el carácter 'A'// wait for a second
      delay(210);
    }
    p = p / 10;

  }
  else if (encoder.detectMagnet())
  {
    p = encoder.readAngle();
  }



  mySerial.print("Sensor: "); // Enviar el carácter 'A'// wait for a second
  mySerial.println(p);

  if (p > 200)
  {
    mySerial.println("Sensor OK");
    digitalWrite(PIN_PA3, HIGH);    // turn the LED off by making the voltage LOW
    delay(100);
    digitalWrite(PIN_PA3, LOW);   // turn the LED on (HIGH is the voltage level)
    delay(100);
    SendHEXdata();

  }
  else
  {
    mySerial.print("Sensor no colocado");
  }

  p = 0;
  delay(500);

  /*else
    {
    if (encoder.magnetTooWeak())
    {
      digitalWrite(PIN_PA3, LOW);    // turn the LED off by making the voltage LOW

    }
    else
      mySerial.println("Sensor min"); // Enviar el carácter 'A'// wait for a second
    delay(100);
    }*/





  //byte error, address;
  //int nDevices = 0;

  //delay(500);

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

void SendHEXdata() {

  txData[0] = (p >> 8) & 0xFF;
  txData[1] = p & 0xFF;
  txData[2] = (bat >> 8) & 0xFF;
  txData[3] = bat  & 0xFF;


  Serial1.print ("AT$SF=");
  if (txData[0] < 0x10) Serial1.print("0");
  Serial1.print(txData[0], HEX);
  if (txData[1] < 0x10) Serial1.print("0");
  Serial1.print(txData[1], HEX);
  if (txData[2] < 0x10) Serial1.print("0");
  Serial1.print(txData[2], HEX);
  if (txData[3] < 0x10) Serial1.print("0");
  Serial1.print(txData[3], HEX);
  Serial1.print("\r");

  //delay(120000);
  mySerial.println("Radio Deep Sleep");

  // Radio a bajo consumo
  //Serial1.print ("AT$P=2");
  //Serial1.print("\r");
  //delay(10);
  //while(!Serial1.available())
  //while (Serial1.available()) { // Verificar si hay datos disponibles en Serial1
  //char data = Serial1.read(); // Leer un byte desde Serial1
  //mySerial.write(data); // Enviar ese byte a mySerial
  //}

  for (int i = 0; i < 15 ; i++)
  {
    //Serial1.print ("AT$P=2");
    //Serial1.print("\r");

    mySerial.println(i);
    delay(6000);

    // Cada 10 iteraciones, 10*6 segundos = 60 segundos = 1 minuto
    if (i % 10 == 0) {
      // Parpadeo del LED
      digitalWrite(PIN_PA3, HIGH);
      delay(100); // LED encendido durante 500 ms
      digitalWrite(PIN_PA3, LOW);
      //delay(100); // LED apagado durante 500 ms
    }
  }

  mySerial.println();
  sleep_enable();
}
