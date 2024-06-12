#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/power.h>

// Definir el pin de interrupción
#define INTERRUPT_PIN PIN_PB0

void setup() {
  // Configurar el pin de interrupción como entrada
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);

  // Habilitar interrupciones en el pin
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), wakeUp, LOW);

  // Habilitar interrupciones globales
  sei();
}

void loop() {
  // Configurar el microcontrolador para dormir en modo de bajo consumo
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();

  // Deshabilitar el ADC para ahorrar más energía
  ADC0.CTRLA &= ~ADC_ENABLE_bm;

  
  pinMode(PIN_PA3, INPUT);
  pinMode(PIN_PA4, INPUT);
  pinMode(PIN_PA5, INPUT);
  pinMode(PIN_PA6, INPUT);
  pinMode(PIN_PA7, INPUT);


  //pinMode(PIN_PB0, INPUT);
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

  //digitalWrite(PIN_PB0, HIGH);
  digitalWrite(PIN_PB1, HIGH);
  digitalWrite(PIN_PB2, HIGH);
  digitalWrite(PIN_PB3, HIGH);
  digitalWrite(PIN_PB4, HIGH);
  digitalWrite(PIN_PB5, HIGH);

  digitalWrite(PIN_PC0, HIGH);
  digitalWrite(PIN_PC1, HIGH);
  digitalWrite(PIN_PC2, HIGH);
  digitalWrite(PIN_PC3, HIGH);

  // Dormir hasta que ocurra una interrupción
  sleep_mode();

  // El microcontrolador se despierta aquí después de una interrupción

  // Rehabilitar el ADC
  ADC0.CTRLA |= ADC_ENABLE_bm;

  // Deshabilitar el modo de dormir
  sleep_disable();

  // Realizar las acciones necesarias después de despertar
  // ...

  // Volver a habilitar el modo de dormir
  sleep_enable();
  while(1);
}

// Rutina de servicio de interrupción
void wakeUp() {
  // Esta función es llamada cuando ocurre la interrupción
}
