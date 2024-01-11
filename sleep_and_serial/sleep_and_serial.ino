#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/power.h>

#define LED_PIN PIN_PA3

void setup()
{


  // Configurar el RTC

  RTC_init();


  delay(50);


}

void enterSleep()
{
  Serial1.end();
  power_all_disable();
  Serial1.end();
  //for (int i = 0; i < NUM_DIGITAL_PINS; i++)
  //{
  //pinMode(i, INPUT);
  //digitalWrite(i, HIGH);
  //}

  pinMode(PIN_PA0, INPUT);
  pinMode(PIN_PA1, INPUT);
  pinMode(PIN_PA2, INPUT);
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

  digitalWrite(PIN_PA0, HIGH);
  digitalWrite(PIN_PA1, HIGH);
  digitalWrite(PIN_PA2, HIGH);
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
  Serial1.end();

  ADC0.CTRLA &= ~ADC_ENABLE_bm;


  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Modo de sueño más bajo
  sleep_enable(); // Habilitar el modo de sueño
  sleep_cpu();    // Poner al MCU en modo de sueño

  // La ejecución se detiene aquí hasta que ocurre una interrupción
  sleep_disable(); // Deshabilitar modo de sueño después de despertar
  power_all_enable();
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  Serial1.begin(9600);
  Serial1.println("AT\r");
  delay(100);
  Serial1.print ("AT$P=2\r");
  delay(500);
  delay(500);

  enterSleep();
  enterSleep();
  enterSleep();


  // Entrar en modo de sueño
  enterSleep();
  enterSleep();
}

//ISR(RTC_CNT_vect) {
//RTC.INTFLAGS = RTC_OVF_bm; // Limpiar la bandera de interrupción del RTC
//}

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

ISR(RTC_PIT_vect)
{
  RTC.PITINTFLAGS = RTC_PI_bm;          /* Clear interrupt flag by writing '1' (required) */
  //countRTC_CLK++;
}
