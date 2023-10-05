#include "system.h"

#define   PRESS   LOW

bool factory_press = false;
unsigned long factory_time = 0;
unsigned long prev_factory_time = 0;
bool reset_time = false;
int buttonState = 0;


unsigned long mainRefresh = obj["mainTime"].as<uint32_t>();
unsigned long mainTime = 1000;

const uint32_t connectTimeoutMs = 10000;
unsigned long  s_timestamp;

// ----------------------------------------------------------- init
void system_init()
{
  Serial.begin(115200);
  Serial.println("gps-test");
  Serial.print("Version:"); Serial.println(VERSION);

  if (spiffs_init())
  {
    loadConfig();       // Load and update behaivor of system
    wifi_init();
    I2C_Init();
    connectFirebase();
  }
  pins_init();
}

// ----------------------------------------------------------------------------------------------- factory_reset3 change
/*void IRAM_ATTR factory_reset3()
  {
  if ((factory_press == false) && (digitalRead(FACTORY_BT) == PRESS))
  {
    Serial.println("{\"R_bt\":\"push\"}");
    factory_press = true;
    factory_time = millis();
    return;

  }
  else if((digitalRead(FACTORY_BT) == !PRESS))
  {
    prev_factory_time = millis();
    reset_time = true;
    //Serial.println("{\"reset_button\":\"released\"}");
    return;
  }

  }*/


// --------------------------------------------------------------------------------------------- check_reset
/*void check_reset()
  {
  // Force Factory to input
  pinMode(FACTORY_BT, INPUT);

  if (reset_time)
  {
    Serial.print("{\"reset_time\":"); Serial.print(prev_factory_time - factory_time); Serial.println("}");
    if ((prev_factory_time - factory_time) > 5000)
    {
      reset_config();
    }
    else
      Serial.println("{\"reset\":\"fail\"}");
    factory_press = false;
    reset_time = false;
  }

  // ------------------------------------------------------reboot time es en horas
  int reboot_time = obj["reboot_time"].as<unsigned int>();
  if (reboot_time < 1)
    reboot_time = 24;
  // Si han pasado más de 24 horas del reset anterior o el tiempo en reboot time
  if (millis() - tiempoInicio >=  (reboot_time * 60 * 60 * 1000))
    //if (millis() - tiempoInicio >=  (reboot_time  * 1000))
  { // Comparar el tiempo actual con el tiempo de inicio
    Serial.print("{\"reboot_time\":"); Serial.print(obj["reboot_time"].as<unsigned int>()); Serial.println("}");
    tiempoInicio = millis();  // Actualizar el tiempo de inicio
    ESP.restart();  // Reiniciar el ESP32
  }

  //------------------------------------------------------ restart from command
  if (obj["restart"].as<bool>())
  {
    Serial.println("{\"reboot\":true}");
    SendData();
    obj["restart"] = false;
    Serial.println(saveJSonToAFile(&obj, filename) ? "{\"reboot_save\":true}" : "{\"reboot_save\":false}");
    //delay(2000);
    ESP.restart();
  }

  }*/

//----------------------------------------------------------------------------------------------------------- reset_config
void reset_config()
{
  WiFi.disconnect();
  //WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);

  //obj["ssid"] = "";
  //obj["pass"] = "";
  //obj["enable_wifi"] = true;
  //obj["count_wifi"] = 0;
  //obj["registered_wifi"] = false;
  obj = getJSonFromFile(&doc, filedefault);
  Serial.println(saveJSonToAFile(&obj, filename) ? "{\"factory_reset\":true}" : "{\"factory_reset\":false}");
  delay(2000);
  ESP.restart();

}


// ------------------------------------------- strtoBool
/*bool strToBool(String str)
  {
  if (str == "true" || str == "1") {
    return true;
  } else if (str == "false" || str == "0") {
    return false;
  } else {
    // handle invalid input
    return false;
  }
  }*/

// ---------------------------------------------------------------------------------------------------- loadConfig
// Update a new config in the file an change behivor
void loadConfig()
{
  // ----------- Load Counters
  Serial.println("{\"loadConfig\":\"flags\"}");
  flag_moneda = obj["total"].as<long>();
  flag_premio = obj["gift"].as<long>();
  //flag_bolsa = obj["bag"];
  flag_bolsa = (obj["total"].as<long>()) - (obj["init_bag"].as<long>());

  // ------------- ID
  String s_aux = obj["id"].as<String>();
  int len = s_aux.length();
  // check for id or mac is the config.json file
  if ((len == 0))
  {
    //(i_aux == 0)
    Serial.println("{\"update_id\":true}");
    Serial.print("{\"ID\":\"");
    Serial.print(WiFi.macAddress());
    Serial.println("\"}");

    obj["id"].set( WiFi.macAddress());

    Serial.println(saveJSonToAFile(&obj, filename) ? "{\"id_file_saved\":true}" : "{\"id_file_saved\":false}" );
  }



  //----------------- RTC
  // if (obj["enable_rtc"].as<bool>())
  // {
  //   rtcUpdated = false;
  //   ntpConnected = false;
  //   init_clock();
  //}





  // -------------------------------- mainTime
  // refres time
  JsonVariant objTime = obj["mainTime"];
  if (objTime.isNull())
  {
    Serial.println("{\"mainTime\":NULL}");
    mainTime = 1000;
  }
  else
  {
    mainTime = obj["mainTime"].as<uint32_t>();
    Serial.print("{\"mainTime\":");
    Serial.print(mainTime);
    Serial.println("}");
  }
  mainRefresh = mainTime + 1;


  Serial.println("{\"config\":true}");

}
