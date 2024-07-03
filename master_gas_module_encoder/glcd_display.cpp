#include "glcd_display.h"

// -------------------------------------------------- updateDisplayTask
void updateDisplayTask(void * parameter) {
  for (;;) {

    // Actualización del documento a enviar al display
    doc_aux.clear();
    doc_aux["wifi"] = true;
    doc_aux["litros"] = ceil(litros);
    doc_aux["precio"] = int(precio);
    doc_aux["STATE"] = STATE_DISPLAY;
    doc_aux["time"] = now.unixtime();
    doc_aux["folio"] = folio;
    doc_aux["sd"] = sd_ready;
    doc_aux["valve"] = valve_state;
    //doc["valve"] = !((digitalRead(SOLENOID)));
    doc_aux["bt"] = obj["enable_bt"].as<bool>();
    if (clear_key) {
      doc_aux["k"] = 0;
      clear_key = false;
    }
    serializeJson(doc_aux, b);

    // Enviar los datos al display
    Wire.beginTransmission(DISPLAY_ADD);
    Wire.write((const uint8_t*)b, strlen(b));
    Wire.endTransmission();

    // Esperar 41 ms antes de la siguiente ejecución
    vTaskDelay(30 / portTICK_PERIOD_MS);

    // --------------------- leer display
    // Read from the slave and print out
    Wire.requestFrom(DISPLAY_ADD, 199);
    memset(buff, 0, sizeof(buff));
    i = 0;
    while (Wire.available())
    {
      buff[i] = Wire.read();
      //Serial  .print((char)buff[i]);
      i++;
    }
    //Serial.println();

    jsonStr =  buff;
    //Serial.println(jsonStr);
    deserializeJson(doc_display, jsonStr);

    if (doc_display.isNull())
    {
      status_doc["display"] = false;
      pinMode(RESET_DISPLAY,OUTPUT);
      digitalWrite(RESET_DISPLAY, HIGH);  // Asegúrate de que el pin esté en estado bajo antes de enviar el pulso
      vTaskDelay(100 / portTICK_PERIOD_MS);  // Esperar 100 ms
      digitalWrite(RESET_DISPLAY, LOW);  // Enviar el pulso (cambio de estado)
      vTaskDelay(100 / portTICK_PERIOD_MS);  // Esperar 100 ms
      digitalWrite(RESET_DISPLAY, HIGH);  // Asegurar que el pin vuelva a estado bajo
    }
    else
    {
      status_doc["display"] = true;
      print_report = doc_display["print_report"];
      if (doc_display["enable_ap"])
      {
        obj["enable_ap"] = !(obj["enable_ap"]);
        //saveConfig = true;
      }


    }
  }
}
