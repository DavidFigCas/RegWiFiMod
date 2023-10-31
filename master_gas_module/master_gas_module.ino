#include "system.h"





// ------------------------------------------------------ (CONFIG) setup
void setup()
{
  system_init();
  search_nclient(0);
  //saveNewlog();
  //Serial1.begin(9600, SERIAL_8N1);  // Inicializa UART1 con 9600 baudios
}


// ------------------------------------------------------ loop
void loop()
{

  // Write a value over I2C to the slave
  //Serial.println("Sending...");
  //doc["precio"] = p++;
  //doc["litros"] = p++;


  // ----------------------------------------------- leer

  // --------------------- leer display
  // Read from the slave and print out
  Wire.requestFrom(DISPLAY_ADD, 199);
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

  Serial.print("Display: ");
  serializeJson(doc_display, Serial);
  Serial.println();


  Serial.print("Encoder: ");
  serializeJson(doc_encoder, Serial);
  Serial.println();

  delay(TIME_SPACE);

  // ----------------------------------------------- procesar


  //litros = ((doc_encoder["current"].as<unsigned int>()) / pulsos_litro);
  litros = (doc_encoder["current"].as<uint32_t>() / pulsos_litro);
  precio = litros * uprice;
  display_reset = false;


  // if ((doc_display["STATE"] ==  0) || (doc_display["STATE"].inNull()))


  //if (doc_display["STATE"] ==  0)
  //{
  //  STATE_DISPLAY = 1;
  //}
  //else
  //if (!doc_display["STATE"].isNull())
  //{
  //  if (doc_display["STATE"] >  0)
  //    STATE_DISPLAY = doc_display["STATE"];

  //if(STATE_DISPLAY == 1)
  //}

  // ------------------------------------- printer
  if (STATE_DISPLAY == 3)
  {
    Serial.println("Display on 3, reset");
    /* if (!startCounting2)
      {
       // Detectado por primera vez
       tiempoAnterior2 = millis();
       startCounting2 = true;
       Serial.println("Printer START");
       //Serial.print("Litros: ");
       //Serial.println(litros);
       //STATE_DISPLAY = 2;
      }
      else
      {
       // Ya se ha detectado antes, verificar el intervalo
       tiempoActual2 = millis();
       if (tiempoActual2 - tiempoAnterior2 >= intervalo2)
       {
         // Ha pasado 1 minuto
         //display_reset = true;
         startCounting2 = false;  // Detener el conteo
         //if (STATE_DISPLAY == 3)
           STATE_DISPLAY = 0;
         Serial.println("Printer Finish");
       }
      }*/
    delay(10000);
    printCheck(uint32_t (precio_check), uint32_t(litros_check), uint32_t (uprice * 100), 1, 11, 23, 14, 45, 6);
    STATE_DISPLAY = 0;
    Serial.println("Done reset");
  }
  else
  {
    // Si STATE no es 3, resetear el conteo
    // startCounting = false;
  }


  // ------------------------------------- encoder Read and stop
  if (doc_encoder["STATE"] == 3)
  {
    if (!startCounting)
    {
      // Detectado por primera vez
      tiempoAnterior = millis();
      startCounting = true;
      Serial.println("STOP FLOWING");
      Serial.print("Litros: ");
      Serial.println(litros);
      STATE_DISPLAY = 2;
    }
    else
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
        litros_check = litros;
        precio_check = precio;
      }
    }
  }
  else
  {
    // Si STATE no es 3, resetear el conteo
    startCounting = false;
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
  }
  doc_aux["STATE"] = STATE_DISPLAY;
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
  doc_aux["reset"] = display_reset;
  doc_aux["litros"] = litros;
  serializeJson(doc_aux, b);
  //Serial.print("Master to encoder: ");
  //serializeJson(doc, Serial);
  //Serial.println();

  Wire.beginTransmission(ENCODE_ADD);
  Wire.write((const uint8_t*)b, (strlen(b)));
  Wire.endTransmission();

  delay(TIME_SPACE);


  
  if (millis() - mainRefresh > mainTime)
  {
    mainRefresh = millis();
    //gps_update();

    // ----------------------------------------- check internet
    //if (wifi_check())
    {
      //update_clock();
      read_clock();
      /*if (mqtt_check())
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
      }*/
    }

    // ------------------------------------------- Clear Log
    if (clear_log == true)
    {
      obj_log.clear();
      Serial.println(saveJSonArrayToAFile(&obj_log, filelog) ? "{\"log_clear_spiffs\":true}" : "{\"log_clear_spiffs\":false}");
      clear_log = false;
    }


    // ------------------------------------------- MQTT new LOG
    if (new_log == true)
    {
      saveNewlog();
      new_log = false;
    }

    //Serial.print("STATE: ");
    //Serial.println(STATE, BIN);
    //Serial.print("ToDO: ");
    //Serial.println(todo_byte, BIN);
    //Serial.print("Client: ");
    //for (int i = 0; i < 4; i++)
    //  Serial.print(nclient_data[i]);
    //Serial.println();

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

}
