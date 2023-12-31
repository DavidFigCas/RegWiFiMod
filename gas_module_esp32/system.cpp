#include "system.h"

#define   PRESS   LOW
//pinMode(ONDDEMANDPIN, INPUT_PULLUP);

bool factory_press = false;
unsigned long factory_time = 0;
unsigned long prev_factory_time = 0;
bool reset_time = false;
int buttonState = 0;
volatile bool found_client = false;


unsigned long mainRefresh = obj["mainTime"].as<uint32_t>();
unsigned long mainTime = 1000;

const uint32_t connectTimeoutMs = 10000;
unsigned long  s_timestamp;

//volatile uint32_t nclient_data; // nclient_data[4]
//volatile uint8_t price_data[2], litro_data[4], factor_data[2], name_data[42];


// ------------------------------------------------------------ register_client
void register_client()
{ // Guarda todos los datos de ese cliente en los registros correspondientes


  //serializeJson(obj_in["nombre"], Serial);


  uint32_t litros = obj_in["litros"];
  uint32_t uprice = (obj_in["precio"].as<float>())*100;
  uint32_t factor = (obj_in["factor"].as<float>())*100;
  const char* client_name = obj_in["nombre"].as<const char*>();
  int len = strlen(client_name);

  Serial.println();
  Serial.print("NAME: ");
  for (int i = 0; i < len && i < 42; i++) {
    name_data[i] = (uint8_t)client_name[i];
  }

  // Si la longitud de la cadena es menor que 42, rellena el resto del array con 0s
  for (int i = len; i < 42; i++) {
    name_data[i] = 0;
  }

  //Serial.print("NAME2: ");
  for (int i = 0; i < 42; i++) {
    Serial.print(char(name_data[i]));
  }
  Serial.println();

  Serial.print("Litros: ");
  Serial.println(litros);
  litros = litros * 100;
  for (int i = 0; i < 4; i++) {
    litros_num[i] = (litros >> (8 * i)) & 0xFF;
    Serial.println(litros_num[i]);
  }
  Serial.println();

  Serial.print("uPrice: ");
  Serial.println(uprice);
  //litros = litros * 100;
  for (int i = 0; i < 2; i++) {
    uprice_data[i] = (uprice >> (8 * i)) & 0xFF;
    Serial.println(uprice_data[i]);
  }
  Serial.println();

  Serial.print("Factor: ");
  Serial.println(factor);
  //litros = litros * 100;
  for (int i = 0; i < 2; i++) {
    factor_data[i] = (factor >> (8 * i)) & 0xFF;
    Serial.println(factor_data[i]);
  }
  Serial.println();

}

// -------------------------------------------------------------- search_nclient
void search_nclient(uint32_t aux_client)
{
  //for (int i = 0; i < 4; i++) {
  //  Serial.println(nclient_data[i]);
  //}


  Serial.print("{\"search_client\":"); Serial.print(aux_client); Serial.println("}");

  new_nclient = 0;
  found_client = false;

  // Buscar el valor de nclient en el array
  for (JsonArray::iterator it = obj_list.begin(); it != obj_list.end(); ++it) {
    obj_in = *it;
    //Serial.println(obj_in["nombre"].as<String>());
    if (obj_in["cliente"].as<uint32_t>() == aux_client) { //-------------------- Cliente encontrado
      Serial.println("{\"client_found\": true}");
      if (obj["test"].as<bool>())
        serializeJson(obj_in, Serial);
      register_client();
      found_client = true;
      break;  // Rompe el bucle una vez que encuentres una coincidencia
    }
  }
  if (!found_client) // ------------- Cliente no encontrado se Busca otra vez el 1
  {
    Serial.println("{\"client_found\": false}");
    for (JsonArray::iterator it = obj_list.begin(); it != obj_list.end(); ++it) {
      obj_in = *it;
      //Serial.println(obj_in["nombre"].as<String>());
      if (obj_in["cliente"].as<uint32_t>() == 0) { //-------------------- Cliente Publico encontrado
        Serial.println("{\"Using Public Client\": true}");
        serializeJson(obj_in, Serial);
        register_client();
        //found_client = true;
        break;  // Rompe el bucle una vez que encuentres una coincidencia
      }
    }
  }
}

// ----------------------------------------------------------- init
void system_init()
{
  Serial.begin(115200);
  I2C_Init(); Serial.println("i2c0_Init");// Slave mode
  Serial.println("gps-test");
  Serial.print("Version:"); Serial.println(VERSION);

  if (spiffs_init())
  {
    loadConfig();       // Load and update behaivor of system
    mqtt_init();
    wifi_init();
    mqtt_check();
    rtcUpdated = false;
    ntpConnected = false;
    init_clock();        // I2C for clock
  }
  gps_init();
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
  //WiFi.disconnect();
  //WiFi.mode(WIFI_OFF);
  //WiFi.mode(WIFI_STA);

  //obj["ssid"] = "";
  //obj["pass"] = "";
  //obj["enable_wifi"] = true;
  //obj["count_wifi"] = 0;
  //obj["registered_wifi"] = false;
  obj = getJSonFromFile(&doc, filedefault);
  Serial.println(saveJSonToAFile(&obj, filename) ? "{\"factory_reset\":true}" : "{\"factory_reset\":false}");
  delay(2000);
  //ESP.restart();
  rp2040.reboot();

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
  Serial.println("{\"loadConfig\":true}");



  //--------------- LOAD REGISTERS
  String email = obj["email"].as<String>(); // Suponiendo que obj es un objeto JSON válido
  size_t length = email.length();

  /* if (length <= sizeof(name_data)) {
     strncpy((char*)name_data, email.c_str(), sizeof(name_data));
     name_data[sizeof(name_data) - 1] = '\0'; // Asegura que la cadena esté terminada correctamente
    } else {
     Serial.println("La longitud del correo electrónico es demasiado larga para name_data");
    }*/


  // ------------- ID
  String s_aux = obj["id"].as<String>();
  int len = s_aux.length();
  // check for id or mac is the config.json file
  if ((len == 0))
  {
    //(i_aux == 0)
    Serial.println("{\"update_id\":true}");
    Serial.print("{\"ID\":\"");
    //Serial.print(WiFi.macAddress());
    Serial.println("\"}");

    //obj["id"].set( WiFi.macAddress());

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
