#include "system.h"





// ------------------------------------------------------ (CONFIG) setup
void setup()
{
  system_init();
  search_nclient(0);
  //saveNewlog();
  Serial1.begin(9600, SERIAL_8N1);  // Inicializa UART1 con 9600 baudios
}


// ------------------------------------------------------ loop
void loop()
{
  if (millis() - mainRefresh > mainTime)
  {
    mainRefresh = millis();
    gps_update();

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

    Serial.print("STATE: ");
    Serial.println(STATE, BIN);
    Serial.print("ToDO: ");
    Serial.println(todo_byte, BIN);
    Serial.print("Client: ");
    for (int i = 0; i < 4; i++)
      Serial.print(nclient_data[i]);
    Serial.println();

  }


  // ---------------------------------------------- I2C new command
  if (newcommand)
  {
    Serial.print("New Command ToDo: "); Serial.println(todo_byte, BIN);


    if (todo_byte & (1 << 6)) {  // ---- Search Client
      //new_nclient = true;
      //if (new_nclient)
      //{
      nclient = 0;
      nclient |= (uint32_t)nclient_data[0] << 24; // Byte más significativo
      nclient |= (uint32_t)nclient_data[1] << 16;
      nclient |= (uint32_t)nclient_data[2] << 8;
      nclient |= (uint32_t)nclient_data[3];
      search_nclient(nclient);
      todo_byte &= ~(1 << 6);  // Reset ToDo bit
      //}
    }

    if (todo_byte & (1 << 5)) {  // New LOG
      //new_nclient = true;
      //if (new_nclient)
      //{
      saveNewlog();
      todo_byte &= ~(1 << 5);  // Reset ToDo bit
      //}
    }
    newcommand = 0;
    todo_byte = 0;
  }

  if (new_litros)
  {
    Serial.println("NEW Litros: ");
    //Serial.println(nclient);
    for (int i = 0; i < 4; i++)
    {
      Serial.println(litros_num[i]);
    }
    new_litros = 0;
  }

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
