#include "pins.h"

void pins_init()
{
  pinMode(cont_monedas, INPUT_PULLUP);
  pinMode(cont_premios, INPUT_PULLUP);
  pinMode(I_maq_onoff, INPUT);
  pinMode(ONDDEMANDPIN, INPUT_PULLUP);

  // WatchDog Timer
  //esp_task_wdt_init(WDT_TIMEOUT, true);  //enable panic so ESP32 restarts
  //esp_task_wdt_add(NULL);                //add current thread to WDT watch

  attachInterrupt(cont_monedas, botonpress_monedas, FALLING);
  attachInterrupt(cont_premios, botonpress_premios, FALLING);
}
