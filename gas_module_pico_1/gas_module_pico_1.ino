#include "system.h"





// ------------------------------------------------------ (CONFIG) setup
void setup()
{
  system_init();
  search_nclient();
  Serial1.begin(9600, SERIAL_8N1);  // Inicializa UART1 con 9600 baudios
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
      mqtt_check();
    }


  }


  if (newcommand)
  {
    Serial.print("New Command ToDo: "); Serial.println(todo_byte, BIN);
    

    if (todo_byte & (1 << 6)) {  // Verifica si el bit 6 está en alto
      //new_nclient = true;
      //if (new_nclient)
      //{
      search_nclient();
      todo_byte &= ~(1 << 6);  // Reset ToDo
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
   while (Serial1.available()) {  // Mientras haya datos disponibles desde el GPS
        char c = Serial1.read();  // Lee un carácter desde el GPS
        Serial.print(c);  // Imprime el carácter en el puerto serie
    }
    //Serial.println();









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
