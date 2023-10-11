#include "system.h"


// ------------------------------------------------------ (CONFIG) setup
void setup()
{
  system_init();
}


// ------------------------------------------------------ loop
void loop()
{
  if (millis() - mainRefresh > mainTime)
  {
    mainRefresh = millis();
    Serial.println("Runing");




    // ----------------------------------------- check internet
    if (wifi_check())
    {
      update_clock();
      read_clock();
      if ( mqtt_check())
        mqtt_send();

    }


  }


  if (newcommand)
  {
    Serial.print("New Command ToDo: "); Serial.println(todo_byte, BIN);
    //nclient |= nclient_data[0] << 24; // Byte más significativo
    //nclient |= nclient_data[1] << 16;
    //nclient |= nclient_data[2] << 8;
    //nclient |= nclient_data[3];
    Serial.print("Client: ");
    Serial.println(nclient);
    for (int i = 0; i < 4; i++)
    {
      Serial.println(nclient_data[i]);
    }
    newcommand = 0;
    todo_byte = 0;
  }

  if (new_litros)
  {
    //Serial.print("New Command ToDo: "); Serial.println(todo_byte, BIN);
    //nclient |= nclient_data[0] << 24; // Byte más significativo
    //nclient |= nclient_data[1] << 16;
    //nclient |= nclient_data[2] << 8;
    //nclient |= nclient_data[3];
    Serial.println("NEW Litros: ");
    //Serial.println(nclient);
    for (int i = 0; i < 4; i++)
    {
      Serial.println(litros_num[i]);
    }
    new_litros = 0;
  }






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
