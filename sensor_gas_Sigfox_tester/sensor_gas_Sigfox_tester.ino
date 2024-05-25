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

volatile uint32_t sleepTime  =  60; //  3600 TIEMPO DORMIDO
volatile uint32_t deltaTime  =  30;   //  600  TIEMPO PARA LEER Y ENVIAR SI HAY CAMBIO BRUSCO
int delta = 15;                         // GRADOS DE CAMBIO PARA QUE SEA BRUSCO

const byte MLX90393_ADDRESS = 0x0F;
double z, phaseShift = 0;     // phaseSift = 105
int x, y;
int angulo, angulo_anterior;
double a, rad;
byte tipo_cambio;

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
  delay(100);
  analogReference(INTERNAL1V024);
  readSupplyVoltage();
  bat = readSupplyVoltage() - 60;
  leerSensor();
  resetRadio();
  SendHEXdata();
  sleepRadio();
}

void loop()
{
  switch (STATE)
  {
    case INICIO:
      tipo_cambio = 0;
      initSensor();
      bat = readSupplyVoltage() - 60;
      leerSensor();
      
      printData(); //Consume eenergía?
      
      STATE = PROCESO;
      break;

    case PROCESO:
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

      if (countRTC_CLK == 0)
      {
        resetRadio();
        SendHEXdata();
      }

      STATE = ESPERA;
      break;

    case ESPERA:
      angulo_anterior = angulo;
      sleepRadio();
      espera_larga();
      STATE = INICIO;
      break;
  }
}

void SendHEXdata()
{
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
  for (int i = 0; i < 8; i++) {
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
  bool response = false;
  pinMode(RESET_RADIO, OUTPUT);
  digitalWrite(RESET_RADIO, HIGH);
  delay(50);
  digitalWrite(RESET_RADIO, LOW);
  pinMode(RESET_RADIO, INPUT);
  delay(50);

  mySerial.print("AT\r");
  delay(50);
  while (mySerial.available()) {
    char data = mySerial.read();
    response = true;
  }
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

void parpadeo(uint16_t cantidad, uint32_t ms)
{
  for (uint16_t i = 0; i < cantidad; i++)
  {
    mySerial.print("AT:P4=0\r");
    delay(ms);
    mySerial.print("AT:P4=1\r");
    delay(ms);
  }
  mySerial.print("AT:P4=0\r");
  delay(50);
}

void RTC_init(void) {
  cli(); // Deshabilita las interrupciones globales para evitar conflictos durante la configuración

  while (RTC.STATUS > 0) {
    ; // Espera a que todos los registros del RTC se sincronicen
  }

  // Selecciona el reloj de 32.768 kHz interno de ultra bajo consumo (OSCULP32K) para el RTC
  RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;

  // Configura el RTC:
  RTC.CTRLA = RTC_PRESCALER_DIV1024_gc  // Establece el prescaler en 1024 para reducir la frecuencia del reloj
              | RTC_RTCEN_bm            // Habilita el RTC
              | RTC_RUNSTDBY_bm;        // Permite que el RTC funcione en modo de espera (standby)

  // Habilita la interrupción del temporizador del RTC
  RTC.PITINTCTRL = RTC_PI_bm;

  // Configura el temporizador del RTC para generar una interrupción cada 32768 ciclos de reloj
  RTC.PITCTRLA = RTC_PERIOD_CYC32768_gc // Establece el período del temporizador en 32768 ciclos (1 segundo si el reloj es de 32.768 kHz)
                 | RTC_PITEN_bm;        // Habilita el temporizador del RTC

  sei(); // Vuelve a habilitar las interrupciones globales
}

ISR(RTC_PIT_vect) {
  RTC.PITINTFLAGS = RTC_PI_bm; // Limpia la bandera de interrupción escribiendo un '1' en ella
  countRTC_CLK++;              // Incrementa el contador de tiempo de sueño
  count_DELTA++;               // Incrementa el contador de tiempo para el delta
}

void initSensor()
{
  pinMode(PIN_PB1, INPUT_PULLUP);
  pinMode(PIN_PB0, INPUT_PULLUP);
  Wire.begin();
  delay(50);
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

  for (int j = 0; j <= 9; j++)
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
    delay(50);

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

  angulo = prom / 10;
}

void enterSleep()
{
  power_all_disable();

  pinMode(PIN_PA3, INPUT);
  pinMode(PIN_PA4, INPUT);
  pinMode(PIN_PA5, INPUT);
  pinMode(PIN_PA6, INPUT);
  pinMode(PIN_PA7, INPUT);

  pinMode(PIN_PB0, INPUT);
  pinMode(PIN_PB1, INPUT);
  pinMode(PIN_PB2, INPUT);
  pinMode(PIN_PB3, INPUT);
  pinMode(PIN_PB4, INPUT);
  pinMode(PIN_PB5, INPUT);

  pinMode(PIN_PC0, INPUT);
  pinMode(PIN_PC1, INPUT);
  pinMode(PIN_PC2, INPUT);
  pinMode(PIN_PC3, INPUT);

  digitalWrite(PIN_PA3, HIGH);
  digitalWrite(PIN_PA4, HIGH);
  digitalWrite(PIN_PA5, HIGH);
  digitalWrite(PIN_PA6, LOW);
  digitalWrite(PIN_PA7, HIGH);

  digitalWrite(PIN_PB0, HIGH);
  digitalWrite(PIN_PB1, HIGH);
  digitalWrite(PIN_PB2, HIGH);
  digitalWrite(PIN_PB3, HIGH);
  digitalWrite(PIN_PB4, HIGH);
  digitalWrite(PIN_PB5, HIGH);

  digitalWrite(PIN_PC0, HIGH);
  digitalWrite(PIN_PC1, HIGH);
  digitalWrite(PIN_PC2, HIGH);
  digitalWrite(PIN_PC3, HIGH);

  ADC0.CTRLA &= ~ADC_ENABLE_bm;

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_cpu();
}

void printData()
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
  mySerial.println();
}
