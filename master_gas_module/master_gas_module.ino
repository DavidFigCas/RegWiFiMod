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
  oled_display_text(VERSION);    // Draw 'stylized' characters
  oled_display_number(88888);    // Draw 'stylized' characters
  printCheck(uint32_t (precio_check), uint32_t(litros_check), uint32_t (uprice * 100), dia_hoy, mes, (anio - 2000), hora, minuto, folio);
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


  // ----------------------------------- Serial Monitor

  //Serial.print("Display: ");
  //serializeJson(doc_display, Serial);
  //Serial.println();


  Serial.print("Encoder: ");
  serializeJson(doc_encoder, Serial);
  Serial.println();

  delay(TIME_SPACE);

  // ----------------------------------------------- procesar
  //litros = ((doc_encoder["current"].as<unsigned int>()) / pulsos_litro);
  // Encoder value is ready and not null
  if (!doc_encoder["current"].isNull())
  {
    litros = (doc_encoder["current"].as<uint32_t>() / pulsos_litro);
    precio = litros * uprice;
  }

  oled_display_number(litros); 
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
      }
      encoder_reset = false;

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
        litros_check = litros;
        precio_check = precio;
        //encoder_reset = true;
        read_clock();
        saveNewlog();
      }

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
    if (startTime == 0)
    { // Si es la primera vez que entras al estado
      startTime = millis();
      Serial.println("Display on 3, reset");
    }

    if (millis() - startTime >= 10000)
    { // Han pasado 10 segundos
      printCheck(uint32_t (precio_check), uint32_t(litros_check), uint32_t (uprice * 100), dia_hoy, mes, (anio - 2000), hora, minuto, folio);
      readyToPrint = false;
      STATE_DISPLAY = 0;
      saveConfig = true;
      //new_log = true;
      Serial.println("Done reset");
      startTime = 0; // Resetea el tiempo de inicio para la próxima vez
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
    doc_aux["precio"] = precio;
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
    obj_log.clear();
    Serial.println(saveJSonArrayToAFile(&obj_log, filelog) ? "{\"log_clear_spiffs\":true}" : "{\"log_clear_spiffs\":false}");
    clear_log = false;
  }


  // ------------------------------------------- Print LOG
  if (print_log == true)
  {
    printing_logs();
    print_log = false;
  }


  // ---------------------------------------------------------------- internet
  if (((millis() - mainRefresh > mainTime) && ((doc_encoder["STATE"] == 0)) || (doc_encoder["STATE"].isNull())))
  {
    mainRefresh = millis();
    //gps_update();

    // ----------------------------------------- check internet
    if (wifi_check())
    {
      update_clock();
      read_clock();
      if (mqtt_check())
      {
        // ------------------------------------------- Send Log
        if (send_log == true)
        {
          Serial.println("mqtt sending");

          //saveNewlog();

          strcpy(buffer_union_publish, obj["id"].as<const char*>());
          strcat(buffer_union_publish, publish_topic);
          strcat(buffer_union_publish, log_topic);

          JsonArray logObject = obj_log;
          size_t serializedLength = measureJson(logObject) + 1;
          char tempBuffer[serializedLength];
          serializeJson(logObject, tempBuffer, serializedLength);
          strcpy(buffer_msg, tempBuffer);

          Mclient.publish(buffer_union_publish, buffer_msg);
          send_log = false;
        }
      }
    }


  }


  // ---------------------------------------------- I2C new command


  //  if (new_litros)
  //{
  //Serial.println("NEW Litros: ");
  //Serial.println(nclient);
  //for (int i = 0; i < 4; i++)
  //{
  //Serial.println(litros_num[i]);
  //}
  //new_litros = 0;
  //}

  // ------------------------------- gps




  // ----------------------------------------- save new List
  //if(flag_new_list == true)
  //{
  //flag_new_list = false;
  //Serial.print("Saving List on Loop: ");
  //serializeJson(doc_list,Serial);
  //Serial.println();
  //saveListData();
  //}



  // ----------------------------------------- save new data
  if (saveConfig)  // Data change
  {
    //saveConfig = false;
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
    //ESP.restart();
    //}

    saveConfig = false;
  }


  // leer boton para imprimir reporte diario
  // Si el botón cambia de no presionado a presionado
  if (lastButtonState == HIGH && buttonState == LOW) {
    Serial.println("PUSH");
    buttonPressTime = millis();
  }

  // Si el botón cambia de presionado a no presionado
  if (lastButtonState == LOW && buttonState == HIGH) {
    if (millis() - buttonPressTime < longPressDuration) {
      Serial.println("Short press detected!");
      print_log = true;
    } else {
      Serial.println("Long press detected!");
      print_log = false;
      clear_log = true;
      folio = 0;
      obj["folio"] = folio;
      saveConfig = true;
      saveNewlog();
    }
  }

  lastButtonState = buttonState;

  //if (digitalRead(BT_REPORT) == LOW)
  //{
  //Serial.println("PUSH");

  //print_log = true;
  //}


  esp_task_wdt_reset();
}
