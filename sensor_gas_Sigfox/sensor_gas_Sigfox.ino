/*
  Sensor Gas 826 SigFox MLX
*/


#include "Wire.h"
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <megaTinyCore.h>     // Librería megaTinyCore para leer Vdd fácilmente (ver ejemplo "readTempVcc")

#define RESET_RADIO   PIN_PA6

#define INICIO    0
#define PROCESO   1
#define ESPERA    2

unsigned int STATE = 0;
uint16_t bat; //voltaje de la batería (Vdd)
volatile uint32_t countRTC_CLK = 0;
volatile uint32_t count_DELTA = 0;

volatile uint32_t sleepTime  =  36; //  3600 TIEMPO DORMIDO
volatile uint32_t deltaTime  =  6;   //  60  TIEMPO PARA LEER Y ENVIAR SI HAY CAMBIO BRUSCO
int delta = 5;                         // GRADOS DE CAMBIO PARA QUE SEA BRUSCO

const byte MLX90393_ADDRESS = 0x0F;
double x, y, phaseShift = 0;      // phaseSift = 105
int angulo, angulo_anterior;
byte tipo_cambio;




// --------------------------------------------------------------------- setup
void setup()
{
  //pinMode(PIN_PA0, INPUT_PULLUP);
  RTC_init();
  resetRadio();
  initRadio();
  initSensor();
  configMLX();
  delay(100);
  analogReference(INTERNAL1V024); //INTERNAL2V048
  // descartar primera lectura para mejor medición
  readSupplyVoltage();
  //parpadeo(3,100);
  bat = readSupplyVoltage() - 60; //error de 60 mV aprox.
  leerSensor();
  resetRadio();
  SendHEXdata();
}

// --------------------------------------------------------------------- loop
void loop()
{


  switch (STATE)
  {
    //----------------------------------------------------------- Leer sensores
    case INICIO:
      tipo_cambio = 0;
      initSensor();
      bat = readSupplyVoltage() - 60; //error de 60 mV aprox.
      leerSensor();
      STATE = PROCESO;
      break;


    //----------------------------------------------------------- Procesa y envia los datos
    case PROCESO:


      if ((angulo_anterior - angulo) > delta)
      {
        tipo_cambio = 1;
        resetRadio();
        SendHEXdata();
        //sleepRadio();
      }
      else if (((angulo - angulo_anterior) > delta))
      {
        tipo_cambio = 2;
        resetRadio();
        SendHEXdata();
        //sleepRadio();
      }

      if (countRTC_CLK == 0)
      {

        //tipo_cambio = 0;
        resetRadio();
        SendHEXdata();
        //sleepRadio();
      }

      STATE = ESPERA;
      break;

    case ESPERA:
      angulo_anterior = angulo;
      sleepRadio();
      espera_larga();
      //analogReference(INTERNAL1V024); //INTERNAL2V048
      // descartar primera lectura para mejor medición
      //readSupplyVoltage();
      STATE = INICIO;
      break;
  }
}


// ----------------------------------------------------------- sendHexData
void SendHEXdata()
{

  //mySerial.println("SendHEX");

  //mySerial.print("RESET:");
  Serial1.print("AT$RC\n");
  delay(50);
  //while (!Serial1.available());
  //while (Serial1.available())
  //{ // Verificar si hay datos disponibles en Serial1
  //char data = Serial1.read(); // Leer un byte desde Serial1
  //mySerial.write(data); // Enviar ese byte a mySerial
  //response = true;
  //}
  uint8_t txData[8];

  int aux_angulo = angulo + (tipo_cambio * 4096);

  // x = (int16_t)posture[1] << 8 | posture[2];
  // y = (int16_t)posture[3] << 8 | posture[4];

  //  txData[0] = (aux_angulo >> 8) & 0xFF;
  //  txData[1] = aux_angulo & 0xFF;
  //  txData[2] = (bat >> 8) & 0xFF;
  //  txData[3] = bat  & 0xFF;

  int x_int = static_cast<int>(x);
  int y_int = static_cast<int>(y);

  txData[0] = (x_int >> 8) & 0xFF;
  txData[1] = x_int & 0xFF;
  txData[2] = (y_int >> 8) & 0xFF;
  txData[3] = y_int & 0xFF;
  txData[5] = (aux_angulo >> 8) & 0xFF;
  txData[5] = aux_angulo & 0xFF;
  txData[6] = (bat >> 8) & 0xFF;
  txData[7] = bat  & 0xFF;



  Serial1.print ("AT$SF=");
  if (txData[0] < 0x10) Serial1.print("0");
  Serial1.print(txData[0], HEX);
  if (txData[1] < 0x10) Serial1.print("0");
  Serial1.print(txData[1], HEX);
  if (txData[2] < 0x10) Serial1.print("0");
  Serial1.print(txData[2], HEX);
  if (txData[3] < 0x10) Serial1.print("0");
  Serial1.print(txData[3], HEX);
  if (txData[4] < 0x10) Serial1.print("0");
  Serial1.print(txData[4], HEX);
  if (txData[5] < 0x10) Serial1.print("0");
  Serial1.print(txData[5], HEX);
  if (txData[6] < 0x10) Serial1.print("0");
  Serial1.print(txData[6], HEX);
  if (txData[7] < 0x10) Serial1.print("0");
  Serial1.print(txData[7], HEX);
  Serial1.print("\r");

  delay(50);
  //while (!Serial1.available());
  //while (Serial1.available())
  //{ // Verificar si hay datos disponibles en Serial1
  //char data = Serial1.read(); // Leer un byte desde Serial1
  //mySerial.write(data); // Enviar ese byte a mySerial
  //response = true;
  //}
  //mySerial.println();

}


//---------------------------------------------- espera_larga
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

  sleep_disable(); // Deshabilitar modo de sueño después de despertar
  power_all_enable();
  ADC0.CTRLA |= ADC_ENABLE_bm;

}


// ----------------------------------------------- resetRadio
void resetRadio()
{
  Serial1.begin(9600 );
  bool response = false;
  pinMode(RESET_RADIO, OUTPUT);
  digitalWrite(RESET_RADIO, HIGH);    // Reset Radio;
  delay(50);
  digitalWrite(RESET_RADIO, LOW);    // Radio OK;
  pinMode(RESET_RADIO, INPUT);
  delay(50);

  Serial1.print("AT\r");
  delay(50);
  //while (!Serial1.available());
  //while (Serial1.available())
  //{ // Verificar si hay datos disponibles en Serial1
  //char data = Serial1.read(); // Leer un byte desde Serial1
  //mySerial.write(data); // Enviar ese byte a mySerial
  //response = true;
  //}
  //mySerial.println();

  //if (response == true)
  //{
  //mySerial.println("Radio Ready");
  //}

  //return response;
  delay(50);
  //initRadio();
}


//-------------------------------------- sleepRadio
void sleepRadio()
{
  //delay(120000);
  //mySerial.println("Radio Deep Sleep");

  // Radio a bajo consumo
  Serial1.print ("AT$P=2");
  Serial1.print("\r");

  delay(50);
  //while(!Serial1.available())
  //while (Serial1.available()) { // Verificar si hay datos disponibles en Serial1
  //char data = Serial1.read(); // Leer un byte desde Serial1
  //mySerial.write(data); // Enviar ese byte a mySerial
  //}

  //espera_larga();
  //sleep_enable();
}

//------------------------------------------ initRadio
void initRadio()
{
  //Serial1.begin(9600 ); // para depurar
  //mySerial.print("ID:");
  Serial1.print("AT$I=10\r");
  delay(50);
  //while (Serial1.available())
  //{ // Verificar si hay datos disponibles en Serial1
  //char data = Serial1.read(); // Leer un byte desde Serial1
  //mySerial.write(data); // Enviar ese byte a mySerial
  //}
  //mySerial.println();NJ


  //mySerial.print("PAC:");
  Serial1.print("AT$I=11\r");
  delay(50);

  //while (Serial1.available()) { // Verificar si hay datos disponibles en Serial1
  //char data = Serial1.read(); // Leer un byte desde Serial1
  //mySerial.write(data); // Enviar ese byte a mySerial
  //}
  //mySerial.println();
}


//------------------------------------------ parpadeo()
void parpadeo(uint16_t cantidad, uint32_t ms)
{
  // Parpadeo del LED
  //pinMode(LED_1, OUTPUT);
  for (uint16_t i = 0; i < cantidad; i++)
  {
    //digitalWrite(LED_1, HIGH);
    //mySerial.print("OFF:");
    Serial1.print("AT:P4=0\r");
    delay(50);
    //mySerial.print("OFF:");
    Serial1.print("AT:P4=0\r");
    delay(ms); // LED encendido durante 500 ms
    //mySerial.print("ON:");
    Serial1.print("AT:P4=1\r");
    //digitalWrite(LED_1, LOW);
    delay(ms); // LED apagado durante 500 ms
  }
  //pinMode(LED_1, INPUT);
  //mySerial.print("OFF:");
  Serial1.print("AT:P4=0\r");
  delay(50);

  //mySerial.println();

}

// --------------------------------------------------------------------- RTC_init
void RTC_init(void)
{
  /* Initialize RTC: */

  //CCP = 0xD8;
  //CLKCTRL.MCLKCTRLA = 1;
  //CLKCTRL.OSC32KCTRLA = 1;

  cli(); // Desactivar interrupciones globales
  while (RTC.STATUS > 0)
  {
    ;                                   /* Wait for all register to be synchronized */
  }


  RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;    /* 32.768kHz Internal Ultra-Low-Power Oscillator (OSCULP32K) */

  RTC.CTRLA = RTC_PRESCALER_DIV1024_gc   // Configurar el prescaler del RTC
              | RTC_RTCEN_bm               // Habilitar el RTC
              | RTC_RUNSTDBY_bm;           // RTC activo en modo standby
  RTC.PITINTCTRL = RTC_PI_bm;           /* PIT Interrupt: enabled */
  RTC.PITCTRLA = RTC_PERIOD_CYC32768_gc /* RTC Clock Cycles 32768, resulting in 32.768kHz/32768 = 1Hz */
                 | RTC_PITEN_bm;                       /* Enable PIT counter: enabled */



  //RTC.PER = 1023;  // Establecer el período del RTC (1023 + 1 ciclos)
  //RTC.INTCTRL = RTC_OVF_bm;  // Habilitar la interrupción por desbordamiento del RTC

  sei(); // Activar interrupciones globales
}

// --------------------------------------------------------------------- ISR
ISR(RTC_PIT_vect)
{
  RTC.PITINTFLAGS = RTC_PI_bm;          /* Clear interrupt flag by writing '1' (required) */
  countRTC_CLK++;
  count_DELTA++;

}


// ----------------------------------------------------------- initSensor
void initSensor()
{
  pinMode(PIN_PB1, INPUT_PULLUP);
  pinMode(PIN_PB0, INPUT_PULLUP);
  Wire.begin();
  delay(50);
}

void configMLX()
{
  // Configura el MLX90393

  Wire.beginTransmission(MLX90393_ADDRESS);
  Wire.write(0x60); // Comando para configurar el sensor
  Wire.write(0x00); // Comando para configurar el sensor
  Wire.write(0x70); // Comando para configurar el sensor
  Wire.write(0x00); // Comando para configurar el sensor
  // Envía otros bytes de configuración según sea necesario
  Wire.endTransmission();
  delay(50);
  Wire.beginTransmission(MLX90393_ADDRESS);
  Wire.write(0x4F); // Comando par  a configurar el sensor
  Wire.endTransmission();
  // Lee los datos
  Wire.requestFrom(MLX90393_ADDRESS, 32); // Solicita 6 bytes (2 por eje)
  while (Wire.available())
  {
    Wire.read();
    //mySerial.println();
  }

  delay(150);
}

// ----------------------------------------------------------- calcularAngulo
double calcularAngulo(double x, double y)
{
  x = x * (-1);
  double a = atan2(y, x); // Calcula el ángulo en radianes
  a = a * (180.0 / M_PI); // Convierte de radianes a grados
  a += phaseShift; // Agrega o resta el defase en grados

  // Normaliza el ángulo para que esté en el rango 0-360
  if (a < 0) {
    a += 360;
  } else if (a >= 360) {
    a -= 360;
  }
  return a;
}

// ----------------------------------------------------------- leerSensor
void leerSensor()
{
  // Configura el MLX90393

  uint8_t posture[30];
  int posture_length = 0;
  Wire.beginTransmission(MLX90393_ADDRESS);
  Wire.write(0x3E); // Comando para configurar el sensor
  Wire.endTransmission();
  Wire.requestFrom(MLX90393_ADDRESS, 4);
  while (Wire.available())
  {
    //mySerial.println(Wire.read());
    Wire.read();
  }
  //mySerial.println("\n ");
  delay(50);

  // Configura el MLX90393
  Wire.beginTransmission(MLX90393_ADDRESS);
  Wire.write(0x4E); // Comando para configurar el sensor
  Wire.endTransmission();
  // Lee los datos
  Wire.requestFrom(MLX90393_ADDRESS, 10); // Solicita 6 bytes (2 por eje)
  int i = -1;  // Comienza con -1 para ignorar el primer byte
  posture_length = 0;
  while (Wire.available())
  {
    byte data = Wire.read();  // Lee el dato actual
    posture[posture_length] = data;
    posture_length++;

  }
  x = (int16_t)posture[1] << 8 | posture[2];
  y = (int16_t)posture[3] << 8 | posture[4];
  
  //if ((abs(x) + abs(y)) < 1000)
  //  angulo = 0;
  //else
    angulo = static_cast<int>(calcularAngulo(x, y));

   Serial1.print("{\"x\":");
     Serial1.print(x);
     Serial1.print(",\"y\":");
     Serial1.print(y);
     Serial1.print(",\"a\":");
     Serial1.print(angulo);
     Serial1.print(",\"v\":");
     Serial1.print(bat);
     Serial1.print("}");
     Serial1.println();
  //delay(100); // Espera un segundo para la próxima lectura

}


// --------------------------------- enterSleep
void enterSleep()
{

  power_all_disable();

  //Serial1.end();
  //Wire.end();
  //pinMode(PIN_PA0, INPUT);
  //pinMode(PIN_PA1, INPUT);
  //pinMode(PIN_PA2, INPUT);
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

  //digitalWrite(PIN_PA0, HIGH);
  //digitalWrite(PIN_PA1, HIGH);
  //digitalWrite(PIN_PA2, HIGH);
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
  //Serial1.end();

  ADC0.CTRLA &= ~ADC_ENABLE_bm;


  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Modo de sueño más bajo
  sleep_enable(); // Habilitar el modo de sueño
  sleep_cpu();    // Poner al MCU en modo de sueño


}
