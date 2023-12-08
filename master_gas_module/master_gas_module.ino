#include "system.h"





// ------------------------------------------------------ (CONFIG) setup
void setup()
{
  system_init();
  search_nclient(0);
  //saveNewlog();
  //Serial1.begin(9600, SERIAL_8N1);  // Inicializa UART1 con 9600 baudios

  buttonState = LOW;
  lastButtonState = HIGH;



  // Reset Display state
  doc_aux["STATE"] = 0;
  doc_aux["time"] = now.unixtime();
  serializeJson(doc_aux, b);
  Wire.beginTransmission(DISPLAY_ADD);
  Wire.write((const uint8_t*)b, (strlen(b)));
  Wire.endTransmission();
  delay(TIME_SPACE);

  oled_display_number(0);    // Draw 'stylized' characters
  //printCheck(uint32_t (precio_check), uint32_t(litros_check), uint32_t (uprice * 100), folio, uint32_t(now.unixtime()), uint32_t(now.unixtime()));
}


// ------------------------------------------------------ loop
void loop()
{
  // PRead button for report
  buttonState = digitalRead(BT_REPORT);
  // ----------------------------------------------- leer

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
    status_doc["display"] = false;
  else
    status_doc["display"] = true;



  delay(TIME_SPACE);

  // --------------------- leer encoder
  // Read from the slave and print out
  Wire.requestFrom(ENCODE_ADD, 199);
  memset(buff, 0, sizeof(buff));
  i = 0;
  while (Wire.available())
  {
    buff[i] = Wire.read();
    //Serial.print((char)buff[i]);
    i++;
  }
  //Serial.println();

  jsonStr =  buff;
  //Serial.println(jsonStr);
  deserializeJson(doc_encoder, jsonStr);

  if (doc_encoder.isNull())
    status_doc["encoder"] = false;
  else
    status_doc["encoder"] = true;



  // ----------------------------------- Serial Monitor

  //Serial.print("Display: ");
  //serializeJson(doc_display, Serial);
  //Serial.println();

  //Serial.print("Encoder: ");
  //serializeJson(doc_encoder, Serial);
  //Serial.println();

  //Serial.print("main_status: ");
  //serializeJson(status_doc, Serial);
  //Serial.println();

  delay(TIME_SPACE);

  // ----------------------------------------------- procesar
  //litros = ((doc_encoder["current"].as<unsigned int>()) / pulsos_litro);
  // Encoder value is ready and not null
  if (!doc_encoder["current"].isNull())
  {
    litros = (doc_encoder["current"].as<uint32_t>() / (pulsos_litro));
    precio = litros * uprice;
  }

  display_reset = false;





  // ------------------------------------- encoder Read and stop
  //if (!doc_encoder["STATE"].isNull())
  {
    if (doc_encoder["STATE"] == 1)
    {
      if (!startFlowing)
      {
        Serial.println("--------------------START FLOWING-----------------");
        read_clock();
        start_process_time = now.unixtime();
        startFlowing = true;
        stopFlowing = false;
        lcd.backlight();
      }
      encoder_reset = false;

      //displays
      oled_display_number(litros);
      lcd.setCursor(0, 0); // Establecer cursor en la primera línea
      lcd.print("LITROS:  "); // Escribir en la primera línea
      lcd.print(litros); // Escribir en la primera línea

    }
    else if (doc_encoder["STATE"] == 3)
    {
      encoder_reset = true;
      if (!stopFlowing)
      {
        // Detectado por primera vez
        tiempoAnterior = millis();
        startCounting = true;
        startFlowing = false;
        stopFlowing = true;
        Serial.println("--------------------STOP FLOWING-----------------");
        STATE_DISPLAY = 2;
        litros_check = ceil(litros);
        precio = litros_check * uprice;
        precio_check = precio;
        //encoder_reset = true;
        read_clock();
        saveNewlog();

      }
      oled_display_number(litros_check);
    }
    else if (doc_encoder["STATE"] == 0)
    {
      // Si STATE no es 3, resetear el conteo
      // start_process_time
      //startCounting = false;
      encoder_reset = false;
      startFlowing = false;
      readyToPrint = false;

    }
    //else
    //{

    //}
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

      //if (millis() - startTimeToPrint >= 1000)
      //{ // Han pasado 10 segundos
      //printCheck(uint32_t (precio_check), uint32_t(litros_check), uint32_t (uprice * 100), dia_hoy, mes, (anio - 2000), hora, minuto, folio);
      printCheck(uint32_t (precio_check), uint32_t(litros_check), uint32_t (uprice * 100), folio - 1, uint32_t(now.unixtime()), uint32_t(now.unixtime()));
      readyToPrint = false;
      STATE_DISPLAY = 0;
      //saveConfig = true;
      //new_log = true;
      Serial.println("###################      Done reset    #########################");
      startTimeToPrint = 0; // Resetea el tiempo de inicio para la próxima vez
      oled_display_number(0);
      lcd.noBacklight();
    }
  }


  // ----------------------------------------------- enviar


  // ---------------------- display doc
  doc_aux.clear();

  if ((!doc_display["STATE"].isNull()) && (doc_display["STATE"] == 0))
  {
    doc_aux["valve"] = doc_encoder["valve_open"].as<bool>();
    doc_aux["wifi"] = true;
    doc_aux["gps"] = false;
    doc_aux["clock"] = true;
    doc_aux["printer"] = true;
    doc_aux["paper"] = true;
    STATE_DISPLAY = 1;

  }
  else
  {
    doc_aux["flow"] = doc_encoder["flow"].as<bool>();
    doc_aux["litros"] = litros;
    doc_aux["litros_check"] = litros_check;
    doc_aux["precio"] = precio;
    doc_aux["precio_check"] = precio_check;
    doc_aux["uprice"] = uprice;
  }

  doc_aux["STATE"] = STATE_DISPLAY;
  doc_aux["time"] = now.unixtime();
  serializeJson(doc_aux, b);

  //Serial.print("Master to display: ");
  //serializeJson(doc, Serial);
  //Serial.println();


  Wire.beginTransmission(DISPLAY_ADD);
  Wire.write((const uint8_t*)b, (strlen(b)));
  Wire.endTransmission();
  delay(TIME_SPACE);

  // ---------------------- encoder doc
  doc_aux.clear();
  doc_aux["reset"] = encoder_reset;
  doc_aux["litros"] = litros;
  serializeJson(doc_aux, b);
  //Serial.print("Master to encoder: ");
  //serializeJson(doc, Serial);
  //Serial.println();

  Wire.beginTransmission(ENCODE_ADD);
  Wire.write((const uint8_t*)b, (strlen(b)));
  Wire.endTransmission();

  delay(TIME_SPACE);

  // ------------------------------------------- Clear Log
  if (clear_log == true)
  {
    //obj_log.clear();
    //Serial.println(saveJSonArrayToAFile(SD, &obj_log, filelog) ? "{\"log_clear_spiffs\":true}" : "{\"log_clear_spiffs\":false}");
    // if (saveJSonArrayToAFile(SD, &obj_log, filelog.c_str()))
    //{
    Serial.println("{\"log_clear_SD\":true}");
    //clear_log = false;
    //ESP.restart();
    //}
    //else
    //{
    //Serial.println("{\"log_clear_SD\":false}");
    //sd_ready = false;

    //}
    clear_log = false;
  }


  // ------------------------------------------- Print LOG
  if (print_log == true)
  {
    read_logs(consult_filelog);
    print_log = false;
  }


  // ---------------------------------------------------------------- MAIN TIME
  if (millis() - mainRefresh > mainTime)
  {
    mainRefresh = millis();

    read_clock();
    gps_update();
    save_gps_log();




    // -------------------------------------------solo si no esta en proceso de surtido

    if (((doc_encoder["STATE"] == 0)) || (doc_encoder["STATE"].isNull()))
    {

      // ----------------------------------------- check internet

      if (!sd_ready)
        SD_Init();


      if (wifi_check())
      {
        update_clock();
        if (mqtt_check())
        {
          // ------------------------------------------- Send Log
          if (send_log == true)
          {
            mqtt_send_file(file_to_send);
          }

          // ------------------------------------------- Send STATUS
          //if (send_log == true)
          {
            Serial.println("{\"mqtt_status\":\"sending\"}");

            //saveNewlog();

            strcpy(buffer_union_publish, obj["id"].as<const char*>());
            strcat(buffer_union_publish, publish_topic);
            strcat(buffer_union_publish, status_topic);

            //JsonArray logObject = obj_log;
            //size_t serializedLength = measureJson(logObject) + 1;
            char tempBuffer[STATUS_SIZE];
            serializeJson(status_doc, tempBuffer);
            strcpy(buffer_msg_status, tempBuffer);

            Mclient.publish(buffer_union_publish, buffer_msg_status);
            //send_log = false;
          }

          // ------------------------------------------- Send LIST
          if (send_list == true)
          {
            mqtt_send_list();
            send_list = false;
          }
        }
      }

    }

  }




  // ----------------------------------------- save new List
  if (flag_new_list == true)
  {
    flag_new_list = false;

    Serial.print("Saving List on Loop: ");
    //serializeJson(doc_list,Serial);
    //Serial.println();
    //saveListData();
  }



  // ----------------------------------------- save new data
  if (saveConfig == true)  // Data change
  {
    saveConfig = false;

    //Serial.println("{\"upload_config_from_loop\":true}");
    //saveConfigData();

    //Serial.println("saving config");


    //DynamicJsonDocument json(1024);

    //for (WiFiManagerParameter* p : customParams) {
    //  doc[p->getID()] = p->getValue();
    //}

    //File configFile = SPIFFS.open("/config.json", "w");
    //if (!configFile) {
    //  Serial.println("failed to open config file for writing");
    //}

    //serializeJson(obj, Serial);
    //serializeJson(obj, configFile);
    //configFile.close();
    //end save
    //ESP.restart();

    //loadConfig();

    // ----------------------------------------- save new data
    //if (flag_newList)
    //{
    //Serial.println("{\"upload_list\":true}");
    //saveListData();
    //flag_newList = false;
    //loadConfig();
    //saveConfig = false;
    //ESP.restart();
    //return;
    //}
    //else
    //{
    Serial.println("{\"upload_config\":true}");
    saveConfigData();
    loadConfig();
    //
    //}

  }


  // leer boton para imprimir reporte diario
  // Si el botón cambia de no presionado a presionado
  if (lastButtonState == HIGH && buttonState == LOW)
  {
    Serial.println("PUSH");
    buttonPressTime = millis();

  }

  // Si el botón cambia de presionado a no presionado
  if (lastButtonState == LOW && buttonState == HIGH)
  {
    if (millis() - buttonPressTime < longPressDuration)
    {
      Serial.println("Short press detected!");
      read_clock();
      consult_filelog = "/logs/" + String(anio) + "_" + String(mes) + "_" + String(dia_hoy) + ".json";
      print_log = true;
    }
    else if (millis() - buttonPressTime >= (4 * longPressDuration)) // Reinicio de fabrica
    {
      Serial.println("Super Long press detected!");

      obj.clear();
      obj = getJSonFromFile(SPIFFS, &doc, filedefault);


      //print_log = false;
      //clear_log = true;
      //folio = 1;
      //obj["folio"] = folio;

      consult_filelog = "/logs/" + String(anio) + "_" + String(mes) + "_" + String(dia_hoy) + ".json";
      deleteFile(SD,consult_filelog.c_str());

      
      saveConfig = true;
      Serial.println("{\"default_config\":true}");
      saveConfigData();
      ESP.restart();

    }
    else if (millis() - buttonPressTime >= longPressDuration) // Reinicia el log
    {

      Serial.println("Long press detected!");
      wifiAP(true);
    }


  }

  lastButtonState = buttonState;

  esp_task_wdt_reset();
}
