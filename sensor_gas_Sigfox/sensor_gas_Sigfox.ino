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

#define RX_PIN        -1
//#define RX_PIN        PIN_PB4
//#define TX_PIN      PIN_PA0
#define TX_PIN        PIN_PA4
#define LED_1         PIN_PA3
#define RESET_RADIO   PIN_PB4
#define BAT_PIN       -1

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
  //pinMode(TX_PIN, OUTPUT);
  //pinMode(BAT_PIN, INPUT);

  mySerial.begin(9600);
  Serial1.begin(9600); // START UART

  Wire.begin();
  mySerial.println("GasSensor Init"); // Enviar el carácter 'A'



  encoder.setPowerMode(3);
  encoder.setFastFilter(0);
  encoder.setSlowFilter(3);

  parpadeo(3, 100);

  //if (resetRadio())
  //{
    //mySerial.println("Radio connected");
    //parpadeo(3, 100);
  //}

  //else
  //{
    //mySerial.print("Radio not detected");
    //parpadeo(5, 500);
  //}
  resetRadio();



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

  analogReference(INTERNAL2V048);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
}

// --------------------------------------------------------------------- loop
void loop()
{


  //bat = analogRead(BAT_PIN);
  p = 0;
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
    parpadeo(2, 50);
    resetRadio();
    SendHEXdata();
    sleepRadio();
    espera_larga(150); // 15 = 1.5 min, 150 = 15min
  }
  else
  {
    mySerial.print("Sensor no colocado");
    espera_larga(15); // 15 = 1.5 min, 150 = 15min
    //espera_larga();
  }

  
}


// ----------------------------------------------------------- sendHexData
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


}


//---------------------------------------------- espera_larga
void espera_larga(uint32_t espera)
{
  for (uint32_t i = 0; i < espera; i++)
  {

    mySerial.println(i);
    delay(6000);

    // Cada 10 iteraciones, 10*6 segundos = 60 segundos = 1 minuto
    if (i % 10 == 0)
    {
      parpadeo(1, 100);
    }
  }

  mySerial.println();
}


// ----------------------------------------------- resetRadio
void resetRadio()
{
  mySerial.println("Reseting Radio");
  bool response = false;
  pinMode(RESET_RADIO, OUTPUT);

  digitalWrite(RESET_RADIO, HIGH);    // Reset Radio;
  delay(30);
  digitalWrite(RESET_RADIO, LOW);    // Radio OK;
  pinMode(RESET_RADIO, INPUT);
  delay(30);

  mySerial.print("AT:");
  Serial1.print("AT\r");
  delay(100);
  //while (!Serial1.available());
  while (Serial1.available())
  { // Verificar si hay datos disponibles en Serial1
    char data = Serial1.read(); // Leer un byte desde Serial1
    mySerial.write(data); // Enviar ese byte a mySerial
    response = true;
  }
  mySerial.println();

  if(response == true)
  {
    mySerial.println("Radio Ready");
  }

  //return response;
}


//-------------------------------------- sleepRadio
void sleepRadio()
{
  //delay(120000);
  mySerial.println("Radio Deep Sleep");

  // Radio a bajo consumo
  Serial1.print ("AT$P=2");
  Serial1.print("\r");

  //delay(10);
  //while(!Serial1.available())
  //while (Serial1.available()) { // Verificar si hay datos disponibles en Serial1
  //char data = Serial1.read(); // Leer un byte desde Serial1
  //mySerial.write(data); // Enviar ese byte a mySerial
  //}

  //espera_larga();
  //sleep_enable();
}


//------------------------------------------ parpadeo()
void parpadeo(uint16_t cantidad, uint32_t ms)
{
  // Parpadeo del LED
  pinMode(LED_1, OUTPUT);
  for (uint16_t i = 0; i <= cantidad; i++)
  {
    digitalWrite(LED_1, HIGH);
    delay(ms); // LED encendido durante 500 ms
    digitalWrite(LED_1, LOW);
    delay(ms); // LED apagado durante 500 ms
  }

}
