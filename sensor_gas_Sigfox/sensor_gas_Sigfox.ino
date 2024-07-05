/*
  Sensor Gas 826 SigFox MLX
*/

#include "Inventoteca_MLX90393.h"
#include "Wire.h"
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <megaTinyCore.h>     // Librería megaTinyCore para leer Vdd fácilmente (ver ejemplo "readTempVcc")



#define RESET_RADIO   PIN_PA6

#define INICIO    0
#define PROCESO   1
#define ENVIAR    2
#define ESPERA    3

#define MIN_TESLA   0     //abajo de ese valor de la suma de x y, se considera que no hay iman

unsigned int STATE = 0;
uint16_t bat; //voltaje de la batería (Vdd)
volatile uint32_t countRTC_CLK = 0;
volatile uint32_t count_DELTA = 0;

volatile uint32_t sleepTime  =  3600*4; //  3600*4 (4 horas) TIEMPO DORMIDO
volatile uint32_t deltaTime  =  3600;   //  3600 (1 hora)  TIEMPO PARA LEER Y ENVIAR SI HAY CAMBIO BRUSCO
int delta = 15;                         // GRADOS DE CAMBIO PARA QUE SEA BRUSCO

const byte MLX90393_ADDRESS = 0x0F;
int angulo, angulo_anterior;
byte tipo_cambio;
bool on_send = false;

Adafruit_MLX90393 sensor = Adafruit_MLX90393();


// --------------------------------------------------------------------- setup
void setup()
{
  //initExtras();
  RTC_init();
  resetRadio();
  initRadio();
  //initSensor();
  //delay(100);


  // descartar primera lectura para mejor medición
  //readSupplyVoltage();
  //parpadeo(3,100);
  //bat = readSupplyVoltage() - 60; //error de 60 mV aprox.
}

// --------------------------------------------------------------------- loop
void loop()
{

  //espera_larga();

  switch (STATE)
  {
    //----------------------------------------------------------- Leer sensores
    case INICIO:
      tipo_cambio = 0;
      initSensor();

      //bat = readSupplyVoltage() - 60; //error de 60 mV aprox.
      //bat = analogRead(PIN_PB4);

      configMLX();
      leerSensor();


      STATE = PROCESO;
      break;


    //----------------------------------------------------------- Procesa
    case PROCESO:
      if (((angulo - angulo_anterior) > delta))
      {
        tipo_cambio = 2;
        STATE = ENVIAR;
      }
      else if ((angulo_anterior - angulo) > delta)
      {
        tipo_cambio = 1;
        STATE = ENVIAR;
      }
      else if ((countRTC_CLK == 0) || (on_send == false))
        STATE = ENVIAR;
      else
        STATE = ESPERA;

      angulo_anterior = angulo;
      break;

    //----------------------------------------------------------- Envia los datos
    case ENVIAR:
      resetRadio();
      SendHEXdata();
      sleepRadio();
      STATE = ESPERA;
      break;

    //----------------------------------------------------------- Espera
    case ESPERA:
      espera_larga();
      initExtras();
      STATE = INICIO;
      break;
  }

}


// ----------------------------------------------------------- sendHexData
void SendHEXdata()
{

  //mySerial.println("SendHEX");

  //mySerial.print("RESET:");
  if (!on_send)
  {
    on_send = true;
    tipo_cambio = 3;
  }
  Serial1.print("AT$RC\n");
  delay(50);
  //while (!Serial1.available());
  while (Serial1.available())
  { // Verificar si hay datos disponibles en Serial1
    char data = Serial1.read(); // Leer un byte desde Serial1
    //mySerial.write(data); // Enviar ese byte a mySerial
    //response = true;
  }
  uint8_t txData[8];

  int aux_angulo = angulo + (tipo_cambio * 4096);

  // x = (int16_t)posture[1] << 8 | posture[2];
  // y = (int16_t)posture[3] << 8 | posture[4];

  txData[0] = (aux_angulo >> 8) & 0xFF;
  txData[1] = aux_angulo & 0xFF;
  txData[2] = (bat >> 8) & 0xFF;
  txData[3] = bat  & 0xFF;

  /*int x_int = static_cast<int>(x);
    int y_int = static_cast<int>(y);

    txData[0] = (x_int >> 8) & 0xFF;
    txData[1] = x_int & 0xFF;
    txData[2] = (y_int >> 8) & 0xFF;
    txData[3] = y_int & 0xFF;
    txData[5] = (aux_angulo >> 8) & 0xFF;
    txData[5] = aux_angulo & 0xFF;
    txData[6] = (bat >> 8) & 0xFF;
    txData[7] = bat  & 0xFF;*/



  Serial1.print ("AT$SF=");
  if (txData[0] < 0x10) Serial1.print("0");
  Serial1.print(txData[0], HEX);
  if (txData[1] < 0x10) Serial1.print("0");
  Serial1.print(txData[1], HEX);
  if (txData[2] < 0x10) Serial1.print("0");
  Serial1.print(txData[2], HEX);
  if (txData[3] < 0x10) Serial1.print("0");
  Serial1.print(txData[3], HEX);
  //if (txData[4] < 0x10) Serial1.print("0");
  //Serial1.print(txData[4], HEX);
  //if (txData[5] < 0x10) Serial1.print("0");
  //Serial1.print(txData[5], HEX);
  //if (txData[6] < 0x10) Serial1.print("0");
  //Serial1.print(txData[6], HEX);
  //if (txData[7] < 0x10) Serial1.print("0");
  //Serial1.print(txData[7], HEX);
  Serial1.print("\r");

  delay(50);
  //while (!Serial1.available());
  while (Serial1.available())
  { // Verificar si hay datos disponibles en Serial1
    char data = Serial1.read(); // Leer un byte desde Serial1
    //mySerial.write(data); // Enviar ese byte a mySerial
    //response = true;
  }
  //mySerial.println();

}


//---------------------------------------------- espera_larga
void espera_larga()
{
  //enterSleep();


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
  //power_all_enable();
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
  //delay(50);

  if (!sensor.begin_I2C(0x0F)) {
    Serial1.println("Error");
    //while (1) {
    //delay(10);
    //}
  }
  //writeProgmemString(sensorFoundMsg);
}

void configMLX()
{
  sensor.setGain(MLX90393_GAIN_5X);
  sensor.setResolution(MLX90393_X, MLX90393_RES_17);
  sensor.setResolution(MLX90393_Y, MLX90393_RES_17);
  sensor.setResolution(MLX90393_Z, MLX90393_RES_16);

  sensor.setOversampling(MLX90393_OSR_3);
  sensor.setFilter(MLX90393_FILTER_5);

  // Configura el MLX90393

}



// ----------------------------------------------------------- leerSensor
void leerSensor()
{
  float x, y, z;

  if (sensor.readData(&x, &y, &z))
  {
    //writeProgmemString(axisX); writeFloatToSerial(x);
    //writeProgmemString(axisY); writeFloatToSerial(y);
    //writeProgmemString(axisZ); writeFloatToSerial(z);
    //Serial.write("\n");
  } else {
    //writeProgmemString(unableToReadMsg);
  }

  //delay(500);

  // ------------------- calcularAngulo
  float rad = atan2(y, x);
  //float aux_an = rad * (180.0 / M_PI);
  angulo = rad * (180.0 / M_PI);

  if (angulo < 0) {
    angulo += 360;
  } else if (angulo >= 360) {
    angulo -= 360;
  }

  if ((abs(x) + abs(y)) < MIN_TESLA)
  {
    angulo = 0;
  }

  /*char buffer[10];

    itoa(x, buffer, 10);
    Serial1.write("\n");
    Serial1.write(buffer);
    Serial1.write("\t");
    itoa(y, buffer, 10);
    Serial1.write("\t");
    Serial1.write(buffer);
    Serial1.write("\t");
    itoa(angulo, buffer, 10);
    Serial1.write("\t");
    Serial1.write(buffer);
    Serial1.write("\n");*/

  //angulo = prom / 10;
  /*Serial1.print("{\"x\":");
    Serial1.print(x, 4);
    Serial1.print(",\"y\":");
    Serial1.print(y, 4);
    Serial1.print(",\"z\":");
    Serial1.print(z, 4);
    //Serial1.print(",\"r\":");
    //Serial1.print(rad, 4);
    Serial1.print(",\"a\":");
    Serial1.print(angulo);
    //Serial1.print(",\"v\":");
    //Serial1.print(bat);
    Serial1.print("}");
    Serial1.println();*/
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


// --------------------------------------- initExtras
void initExtras()
{
  // Configurar el ADC para leer la referencia de voltaje interna
  //VREF.CTRLA = VREF_ADC0REFSEL_1V1_gc; // Selecciona 1.1V como referencia para ADC0
  //ADC0.CTRLC = ADC_PRESC_DIV4_gc;      // Preescalador del ADC a 4
  //ADC0.MUXPOS = ADC_MUXPOS_INTREF_gc;  // Seleccionar referencia interna
  //ADC0.CTRLA = ADC_ENABLE_bm;          // Habilitar ADC

  //USART0.CTRLB |= (USART_RXEN_bm | USART_TXEN_bm); // Activa transmisor y receptor
  //Serial1.begin(9600);
  //pinMode(PIN_PA0, INPUT_PULLUP);

  //sleep_disable(); // Deshabilitar modo de sueño después de despertar
  //power_all_enable();
  //ADC0.CTRLA |= ADC_ENABLE_bm;
  //pinMode(PIN_PA0, INPUT_PULLUP);

  //TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; // Habilitar TCA0
  //TCB0.CTRLA |= TCB_ENABLE_bm; // Habilitar TCB0
  //TCB1.CTRLA |= TCB_ENABLE_bm; // Habilitar TCB1 si estás utilizando más de un TCB


  //analogReference(INTERNAL1V024); //INTERNAL2V048
  // descartar primera lectura para mejor medición
  //readSupplyVoltage();
  //analogReference(INTERNAL1V024);

}
