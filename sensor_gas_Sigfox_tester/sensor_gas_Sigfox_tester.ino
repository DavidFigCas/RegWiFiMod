#include <SoftwareSerial.h>
#include "Wire.h"
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <megaTinyCore.h>     // Librería megaTinyCore para leer Vdd fácilmente (ver ejemplo "readTempVcc")

#define RESET_RADIO   PIN_PA6

#define INICIO    0
#define PROCESO   1
#define ESPERA    2

#define MIN_TESLA   700     //abajo de ese valor de la suma de x y, se considera que no hay iman

unsigned int STATE = 0;
uint16_t bat; //voltaje de la batería (Vdd)
volatile uint32_t countRTC_CLK = 0;
volatile uint32_t count_DELTA = 0;

volatile uint32_t sleepTime  =  10; //  3600 TIEMPO DORMIDO
volatile uint32_t deltaTime  =  1;   //  600  TIEMPO PARA LEER Y ENVIAR SI HAY CAMBIO BRUSCO
int delta = 15;                         // GRADOS DE CAMBIO PARA QUE SEA BRUSCO

const byte MLX90393_ADDRESS = 0x0F;
double z, phaseShift = 0;     // phaseSift = 105
int x, y;
int angulo, angulo_anterior;
double a, rad;
byte tipo_cambio;
bool sleep_radio = false;
bool response = false;

// Define los pines para SoftwareSerial
const int txPin = PIN_A1; // El pin que actuará como TX
const int rxPin = PIN_A2; // El pin que actuará como RX

// Crea el objeto SoftwareSerial con los pines invertidos
SoftwareSerial mySerial(rxPin, txPin); // RX, TX

void setup()
{
  mySerial.begin(9600);
  RTC_init();
  resetRadio();
  initRadio();
  initSensor();
  configMLX();
  delay(10);
  analogReference(INTERNAL1V024);
  readSupplyVoltage();
  bat = readSupplyVoltage() - 60;
  leerSensor();
  resetRadio();
  SendHEXdata();
  sleepRadio();

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

void loop()
{
  switch (STATE)
  {
    case INICIO:
      tipo_cambio = 0;
      sleep_radio = false;
      initSensor();
      bat = readSupplyVoltage() - 60;
      leerSensor();


      mySerial.print("{\"x\":");
      mySerial.print(x);
      mySerial.print(",\"y\":");
      mySerial.print(y);
      mySerial.print(",\"z\":");
      mySerial.print(z);
      mySerial.print(",\"a\":");
      mySerial.print(angulo);
      mySerial.println("}");
      //mySerial.println(" ");

      STATE = PROCESO;
      break;

    case PROCESO:

      if ((angulo > (delta + 1)) && (angulo < (360 - (delta + 1))) )
      {
        if ((angulo_anterior - angulo) > delta)
        {
          tipo_cambio = 1;
          resetRadio();
          SendHEXdata();
        }
        else if (((angulo - angulo_anterior) > delta))
        {
          tipo_cambio = 2;
          resetRadio();
          SendHEXdata();
        }
      }

      if (countRTC_CLK == 0)
      {
        resetRadio();
        SendHEXdata();
      }

      STATE = ESPERA;
      break;

    case ESPERA:
      angulo_anterior = angulo;
      if (sleep_radio == true)
        sleepRadio();
      espera_larga();
      STATE = INICIO;
      break;
  }
}

void SendHEXdata()
{
  Serial1.print("AT$RC\r");
  delay(50);
  sleep_radio = true;
  reset_radio_counter();


  uint8_t txData[8];
  int aux_angulo = angulo + (tipo_cambio * 4096);

  txData[0] = (aux_angulo >> 8) & 0xFF;
  txData[1] = aux_angulo & 0xFF;
  txData[2] = (bat >> 8) & 0xFF;
  txData[3] = bat  & 0xFF;
  txData[4] = (x >> 8) & 0xFF;
  txData[5] = x  & 0xFF;
  txData[6] = (y >> 8) & 0xFF;
  txData[7] = y  & 0xFF;

  mySerial.print("AT$SF=");
  for (int i = 0; i < 4; i++) {
    if (txData[i] < 0x10) mySerial.print("0");
    mySerial.print(txData[i], HEX);
  }
  mySerial.print("\r");
}

void espera_larga()
{
  while ((countRTC_CLK < sleepTime) && (count_DELTA < deltaTime))
  {
    enterSleep();
  }

  if (count_DELTA >= deltaTime)
  {
    count_DELTA = 0;
  }

  if (countRTC_CLK >= sleepTime)
  {
    countRTC_CLK = 0;
  }

  sleep_disable();
  power_all_enable();
  ADC0.CTRLA |= ADC_ENABLE_bm;
}

void resetRadio()
{
   response = false;
  mySerial.begin(9600);

  pinMode(RESET_RADIO, OUTPUT);
  digitalWrite(RESET_RADIO, HIGH);
  delay(5);
  digitalWrite(RESET_RADIO, LOW);
  pinMode(RESET_RADIO, INPUT);
  delay(5);

  mySerial.print("AT\r\n");
  delay(50);
  //while(!mySerial.available());
  while (mySerial.available()) {
    char data = mySerial.read();
    response = true;
  }
}

void reset_radio_counter()
{
  Serial1.print("AT$RC\n");
  delay(50);
  while (mySerial.available()) {
    char data = mySerial.read();
    response = true;
  }
  //return 0;
}

void sleepRadio()
{
  mySerial.print("AT$P=2\r");
}

void initRadio()
{
  mySerial.print("AT$I=10\r");
  mySerial.print("AT$I=11\r");
}



void RTC_init(void) {
  cli();
  while (RTC.STATUS > 0) ; // Espera a que los registros del RTC estén listos

  RTC.CLKSEL = RTC_CLKSEL_INT32K_gc; // Reloj interno de 32.768 kHz
  RTC.CTRLA = RTC_PRESCALER_DIV1024_gc | RTC_RTCEN_bm | RTC_RUNSTDBY_bm; // Configura prescaler y habilita RTC

  //RTC.CTRLA = RTC_PRESCALER_DIV1024_gc | RTC_RTCEN_bm; // Configura prescaler y habilita RTC

  RTC.PITINTCTRL = RTC_PI_bm; // Habilita interrupción periódica
  RTC.PITCTRLA = RTC_PERIOD_CYC32768_gc | RTC_PITEN_bm; // Configura el periodo de la interrupción

  sei();
}


ISR(RTC_PIT_vect) {
  RTC.PITINTFLAGS = RTC_PI_bm; // Limpia la bandera de interrupción escribiendo un '1' en ella
  countRTC_CLK++;              // Incrementa el contador de tiempo de sueño
  count_DELTA++;               // Incrementa el contador de tiempo para el delta
}

void initSensor()
{
  //pinMode(PIN_PB1, INPUT_PULLUP);
  //pinMode(PIN_PB0, INPUT_PULLUP);

  //  pinMode(PIN_PB1, INPUT);
  //pinMode(PIN_PB0, INPUT);


  Wire.begin();
  delay(10);
}

void configMLX()
{
  Wire.beginTransmission(MLX90393_ADDRESS);
  Wire.write(0x60);
  Wire.write(0x00);
  Wire.write(0x70);
  Wire.write(0x00);
  Wire.endTransmission();
}

double calcularAngulo(double x, double y)
{
  rad = atan2(y, x);
  a = rad * (180.0 / M_PI);

  if (a < 0) {
    a += 360;
  } else if (a >= 360) {
    a -= 360;
  }
  return a;
}

void leerSensor()
{
  int a_aux = 0;
  int32_t prom = 0;
  int num_red = 3;

  for (int j = 0; j < num_red; j++)
  {
    uint8_t posture[30];
    int posture_length = 0;
    Wire.beginTransmission(MLX90393_ADDRESS);
    Wire.write(0x3E);
    Wire.endTransmission();
    Wire.requestFrom(MLX90393_ADDRESS, 4);
    while (Wire.available()) {
      Wire.read();
    }
    delay(5);

    Wire.beginTransmission(MLX90393_ADDRESS);
    Wire.write(0x4E);
    Wire.endTransmission();
    Wire.requestFrom(MLX90393_ADDRESS, 10);
    int i = -1;
    posture_length = 0;
    while (Wire.available()) {
      byte data = Wire.read();
      posture[posture_length] = data;
      posture_length++;
    }
    x = (int16_t)posture[1] << 8 | posture[2];
    y = (int16_t)posture[3] << 8 | posture[4];
    z = (int16_t)posture[5] << 8 | posture[6];

    a_aux = static_cast<int>(calcularAngulo(x, y));

    if ((abs(x) + abs(y)) < MIN_TESLA) {
      a_aux = 0;
    }

    prom = prom + a_aux;
  }

  angulo = prom / num_red;
}

void enterSleep()
{
  power_all_disable();

  Wire.end();

  //pinMode(PIN_PA1, INPUT_PULLUP);
  //pinMode(PIN_PA2, INPUT_PULLUP);
  pinMode(PIN_PA3, INPUT_PULLUP);
  pinMode(PIN_PA4, INPUT_PULLUP);
  pinMode(PIN_PA5, INPUT_PULLUP);
  pinMode(PIN_PA6, INPUT_PULLUP);
  pinMode(PIN_PA7, INPUT_PULLUP);

  pinMode(PIN_PB0, INPUT_PULLUP);
  pinMode(PIN_PB1, INPUT_PULLUP);
  pinMode(PIN_PB2, INPUT_PULLUP);
  pinMode(PIN_PB3, INPUT_PULLUP);
  pinMode(PIN_PB4, INPUT_PULLUP);
  pinMode(PIN_PB5, INPUT_PULLUP);

  pinMode(PIN_PC0, INPUT_PULLUP);
  pinMode(PIN_PC1, INPUT_PULLUP);
  pinMode(PIN_PC2, INPUT_PULLUP);
  pinMode(PIN_PC3, INPUT_PULLUP);

  //digitalWrite(PIN_PA1, LOW);
  //digitalWrite(PIN_PA2, LOW);
  digitalWrite(PIN_PA3, LOW);
  digitalWrite(PIN_PA4, LOW);
  digitalWrite(PIN_PA5, LOW);
  digitalWrite(PIN_PA6, LOW);
  digitalWrite(PIN_PA7, LOW);

  digitalWrite(PIN_PB0, HIGH);
  digitalWrite(PIN_PB1, HIGH);
  digitalWrite(PIN_PB2, LOW);
  digitalWrite(PIN_PB3, LOW);
  digitalWrite(PIN_PB4, LOW);
  digitalWrite(PIN_PB5, LOW);

  digitalWrite(PIN_PC0, LOW);
  digitalWrite(PIN_PC1, LOW);
  digitalWrite(PIN_PC2, LOW);
  digitalWrite(PIN_PC3, LOW);

  ADC0.CTRLA &= ~ADC_ENABLE_bm;


  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_cpu();
  sleep_enable();

}

/*void printSensor()
  {
  mySerial.print("{\"x\":");
  mySerial.print(x);
  mySerial.print(",\"y\":");
  mySerial.print(y);
  mySerial.print(",\"z\":");
  mySerial.print(z);
  mySerial.print(",\"r\":");
  mySerial.print(rad);
  mySerial.print(",\"a\":");
  mySerial.print(angulo);
  mySerial.print(",\"v\":");
  mySerial.print(bat);
  mySerial.print("}");
  mySerial.println(" ");
  }*/

int calcularDiferenciaAngular(int angulo_anterior, int angulo_actual) {
  int diferencia = angulo_actual - angulo_anterior;
  if (diferencia > 180) {
    diferencia -= 360;
  } else if (diferencia < -180) {
    diferencia += 360;
  }
  return diferencia;
}
