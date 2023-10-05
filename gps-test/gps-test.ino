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

    Serial.print(flag_premio);
    Serial.print("\t");
    Serial.print(flag_bolsa);
    Serial.print("\t");
    Serial.print(flag_moneda);
    Serial.println("\t");


    // ----------------------------------------- check internet
    if (wifi_check())
      //wifi_check();
    {
      //if ( mqtt_check())
      //mqtt_send();
    }


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
    if (flag_newList)
    {
      Serial.println("{\"upload_list\":true}");
      saveListData();
      flag_newList = false;
      //loadConfig();
      //saveConfig = false;
      //ESP.restart();
      return;
    }
    else
    {
      Serial.println("{\"upload_config\":true}");
      saveConfigData();
      loadConfig();
      //ESP.restart();
    }

    saveConfig = false;
  }


}
