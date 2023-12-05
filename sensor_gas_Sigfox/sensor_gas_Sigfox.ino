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
// Librería megaTinyCore para leer Vdd fácilmente (ver ejemplo "readTempVcc")
#include <megaTinyCore.h>

#define RX_PIN        -1
//#define RX_PIN        PIN_PB4
//#define TX_PIN      PIN_PA0
#define TX_PIN        PIN_PA4
#define LED_1         PIN_PA3
//#define LED_1         PIN_PC3
#define RESET_RADIO   PIN_PB4
#define BAT_PIN       -1

#define INICIO    0
#define PROCESO   1
#define ESPERA    2
#define F_CPU 32768UL // Define la frecuencia del reloj como 32.768 kHz

int contador;
long p; //promedio?
uint16_t bat; //voltaje de la batería (Vdd)
long angulo;
long promedio_angulo;
long promedio_angulo_anterior;
uint8_t txData[4];
unsigned int STATE = 0;

SoftwareSerial mySerial(RX_PIN, TX_PIN); // Reemplaza RX_PIN y TX_PIN con los números de pin reales
AS5600 encoder;


// --------------------------------------------------------------------- setup
void setup()
{
  //F_CPU;
  //CCP = 0xD8;
  //CLKCTRL.MCLKCTRLA = 1;
  //CLKCTRL.OSC32KCTRLA = 1;
  

 mySerial.begin(9600); // para depurar
  Serial1.begin(9600); // START UART para el módulo Sigfox

  Wire.begin();
  mySerial.println("GasSensor Init"); // Enviar el carácter 'A'


  // Sensor en bajo consumo
  encoder.setPowerMode(3);
  encoder.setFastFilter(0);
  encoder.setSlowFilter(3);

  parpadeo(3, 10);

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

  // config ADC para leer voltaje interno
  analogReference(INTERNAL1V024); //INTERNAL2V048
  // se iba a implementar escribiendo directamente en los registros,
  // afortunadamente ya existe una librería para eso
  //VREF.CTRLA = 1;//VREF_ADC0REFSEL_1V1_gc;
  //ADC0.MUXPOS = 1;//ADC_MUXPOS_INTREF_gc;
  // descartar primera lectura para mejor medición
  readSupplyVoltage();



  //set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  //sleep_enable();
  //sleep_mode(); 
}

// --------------------------------------------------------------------- loop
void loop()
{


  switch (STATE)
  {
    //----------------------------------------------------------- Leer sensores
    case INICIO:
      //bat = analogRead(BAT_PIN);
      bat = readSupplyVoltage() - 60; //error de 60 mV aprox.

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



      mySerial.print("Sensor: ");
      mySerial.print(p);
      mySerial.print("\t\t");
      mySerial.print("Battery: ");
      mySerial.println(bat);
      STATE = PROCESO;
      break;


    //----------------------------------------------------------- Procesa y envia los datos
    case PROCESO:
      if ((p > 200) && (p < 3830))
      {
        resetRadio();
        mySerial.println("Sensor OK");
        parpadeo(2, 50);
        SendHEXdata();
        sleepRadio();
      }
      else
      {
        resetRadio();
        p = 0;
        mySerial.print("Sensor no colocado");
        SendHEXdata();
        sleepRadio();

      }
      STATE = ESPERA;
      break;

    case ESPERA:
      espera_larga(150); // 15 = 1.5 min, 150 = 15mi
      STATE = INICIO;
      break;
  }
}


// ----------------------------------------------------------- sendHexData
void SendHEXdata() {

  mySerial.println("SendHEX");

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

  delay(20);
  //while (!Serial1.available());
  while (Serial1.available())
  { // Verificar si hay datos disponibles en Serial1
    char data = Serial1.read(); // Leer un byte desde Serial1
    mySerial.write(data); // Enviar ese byte a mySerial
    //response = true;
  }
  mySerial.println();

}


//---------------------------------------------- espera_larga
void espera_larga(uint32_t espera)
{
  for (uint32_t i = 0; i < espera; i++)
  {

    mySerial.println(espera - i);
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
  delay(50);

  mySerial.print("AT:");
  Serial1.print("AT\r");
  delay(10);
  //while (!Serial1.available());
  while (Serial1.available())
  { // Verificar si hay datos disponibles en Serial1
    char data = Serial1.read(); // Leer un byte desde Serial1
    mySerial.write(data); // Enviar ese byte a mySerial
    response = true;
  }
  mySerial.println();

  if (response == true)
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
  for (uint16_t i = 0; i < cantidad; i++)
  {
    digitalWrite(LED_1, HIGH);
    delay(ms); // LED encendido durante 500 ms
    digitalWrite(LED_1, LOW);
    delay(ms); // LED apagado durante 500 ms
  }
  //pinMode(LED_1, INPUT);
}
