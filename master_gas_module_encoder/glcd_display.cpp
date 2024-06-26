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
        doc_aux["valve"] = doc_encoder["valve"].as<bool>();
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
    }
}
