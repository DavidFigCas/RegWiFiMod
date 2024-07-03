#include "system.h"





// ------------------------------------------------------ (CONFIG) setup
void setup()
{
  delay(2500);
  system_init();
  //search_nclient(0);
  //saveNewlog();
}


// ------------------------------------------------------ loop
void loop()
{

  Serial_CMD();

  // ----------------------------------------- save new data
  if (saveConfig == true)  // Data change
  {
    saveConfig = false;
    //Serial.println("{\"upload_config\":true}");
    saveConfigData();
    loadConfig();
  }




  // --------------------------------- proces stop, display liters and wait for icon
  if (startCounting)
  {
    // Ya se ha detectado antes, verificar el intervalo
    tiempoActual = millis();
    if (tiempoActual - tiempoAnterior >= intervalo)
    {
      // Ha pasado 1 minuto
      display_reset = true;
      startCounting = false;  // Detener el conteo
      //if (STATE_DISPLAY == 3)
      STATE_DISPLAY = 3;
      Serial.println("Display Bing Printer");
      startCounting = false;
      readyToPrint = true;
    }
  }


  // ------------------------------------- printer ready to print
  // Debe depender del encoder
  if (readyToPrint == true)
  {
    if (startTimeToPrint == 0)
    { // Si es la primera vez que entras al estado
      startTimeToPrint = millis();
      Serial.println("Display on 3, reset");
      //}


      
      printCheck(uint32_t (precio_check), uint32_t(litros_check), uint32_t (uprice * 100), folio - 1, uint32_t(now.unixtime()), uint32_t(now.unixtime()));
      
      readyToPrint = false;
      STATE_DISPLAY = 1;
      clear_key = true;
      //saveConfig = true;
      //new_log = true;
      Serial.println("###################      Done reset    #########################");
      startTimeToPrint = 0; // Resetea el tiempo de inicio para la próxima vez
      //oled_display_number(0);
      //lcd.noBacklight();
    }
  }

  
  // ------------------------------------------- Print LOG
  if (print_report == true)
  {
    consult_filelog = "/logs/" + String(anio) + "_" + String(mes) + "_" + String(dia_hoy) + ".json";
    read_logs(consult_filelog);
    print_report = false;
    send_report = true;

  }

  


  // ---------------------------------------------------------------- MAIN TIME

  //esp_task_wdt_reset();
}
