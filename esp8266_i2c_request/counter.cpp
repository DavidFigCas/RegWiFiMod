#include "counter.h"


long flag_moneda;
long flag_premio;
long flag_bolsa;


// ----------------------------------------- coin_cointer
void IRAM_ATTR botonpress_monedas() {
  flag_moneda++;
  obj["total"] = flag_moneda;
  flag_bolsa = (obj["total"].as<long>()) - (obj["init_bag"].as<long>());
  //obj["bag"] = flag_bolsa;
  //Serial.print("M");Serial.println(flag_moneda);
  saveConfig = true;
  return;
}


// ----------------------------------------- gift_cointer
void IRAM_ATTR botonpress_premios() {
  flag_premio++;
  obj["gift"] = flag_premio;
  //Serial.print("P");Serial.println(flag_premio);
  saveConfig = true;
  return;
}
