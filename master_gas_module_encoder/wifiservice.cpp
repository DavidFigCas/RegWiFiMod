#include "wifiservice.h"

bool correct = false;
int wifi_trys;
boolean isSaved = false;
bool ALLOWONDEMAND   = true; // enable on demand
bool WMISBLOCKING    = true;
WiFiManager wifiManager;
std::vector<WiFiManagerParameter*> customParams;
// Manejador de tarea para la tarea WiFi
TaskHandle_t wifiTaskHandle = NULL;
SemaphoreHandle_t wifiMutex;


// ---------------------------------------------------------- disableWiFi
void disableWiFi() 
{
  if (xSemaphoreTake(wifiMutex, portMAX_DELAY) == pdTRUE) {
    if (WiFi.status() == WL_CONNECTED) {
      WiFi.disconnect(true);
      delay(1000); // Esperar un segundo para asegurar la desconexión
    }

    WiFi.mode(WIFI_OFF);

    //esp_err_t err = esp_wifi_deinit();
    //if (err == ESP_OK) {
      //Serial.println("Wi-Fi deshabilitado completamente.");
    //} else {
      //Serial.printf("Error deshabilitando el Wi-Fi: %d\n", err);
    //}

    if (wifiTaskHandle != NULL) {
      vTaskDelete(wifiTaskHandle);
      wifiTaskHandle = NULL;
    }

    xSemaphoreGive(wifiMutex);
  }
}

// ---------------------------------------------------------- enableWiFi
bool enableWiFi()
{
  int retries = 0;

  if (xSemaphoreTake(wifiMutex, portMAX_DELAY) == pdTRUE) {
    WiFi.mode(WIFI_STA);
    const char * auxssid = obj["ssid"].as<const char *>();
    const char * auxpass = obj["pass"].as<const char *>();

    // Star WiFi connection
    WiFi.begin(auxssid, auxpass);

    while (WiFi.status() != WL_CONNECTED && retries < 10) {
      delay(500);
      Serial.print(".");
      retries++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("Wi-Fi conectado.");
      Serial.print("Dirección IP: ");
      Serial.println(WiFi.localIP());
      
      if (wifiTaskHandle == NULL) {
        xTaskCreatePinnedToCore(
          wifiTask,            // Función de la tarea
          "WiFiTask",          // Nombre de la tarea
          8192,                // Tamaño del stack
          NULL,                // Parámetro de entrada
          2,                   // Prioridad de la tarea
          &wifiTaskHandle,     // Manejar de la tarea
          1                    // Núcleo en el que se ejecutará la tarea
        );
      }

      xSemaphoreGive(wifiMutex);
      return true;
    } else {
      Serial.println("");
      Serial.println("No se pudo conectar al Wi-Fi.");
      disableWiFi();
      xSemaphoreGive(wifiMutex);
      return false;
    }
  }

  return false;
}


// ------------------------------------------------ wifiAP
bool wifiAP()
//bool wifiAP(bool force)
{

  //WiFi.disconnect(true);
 /* WiFi.mode(WIFI_AP_STA);
  bool force = true;
  server_running = false;
  const char * ap_ssid = obj["ap"].as<const char *>();
  const char * ap_pass = obj["ap_pass"].as<const char *>();

  Serial.print("{\"AP\": \"");
  Serial.print(ap_ssid);
  Serial.println("\"}");

  //WiFiManager
  wifiManager.setConfigPortalBlocking(false);
  // captive portal redirection
  //wifiManager.setCaptivePortalEnable(false);
  //wifiManager.setTimeout(120);

  //set config save notify callback
  wifiManager.setSaveParamsCallback(saveConfigCallback);
  wifiManager.setSaveConfigCallback(saveWifiCallback);
  wifiManager.setWebServerCallback(bindServerCallback);
  wifiManager.setBreakAfterConfig(true); // needed to use saveWifiCallback
  //wifiManager.setConfigPortalTimeout(140);
  //wifiManager.setParamsPage(true); // move params to seperate page, not wifi, do not combine with setmenu!

  //set static ip
  //wifiManager.setSTAStaticIPConfig(IPAddress(10, 0, 1, 99), IPAddress(10, 0, 1, 1), IPAddress(255, 255, 255, 0));

  //reset settings - for testing
  //wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds


  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration

  for (JsonPair kv : doc.as<JsonObject>())
  {
    String keyString = kv.key().c_str();
    char* key = new char[keyString.length() + 1];
    strcpy(key, keyString.c_str());

    String value = kv.value().as<String>();
    char* valueCStr = new char[value.length() + 1];
    strcpy(valueCStr, value.c_str());

    WiFiManagerParameter* p = new WiFiManagerParameter(key, key, valueCStr, value.length() + 10);
    customParams.push_back(p);
    wifiManager.addParameter(p);
  }

  if (!obj["ap"].isNull())
  {

    if (force == true)
    {
      wifiManager.startConfigPortal(ap_ssid, ap_pass);
      Serial.print("{\"Server_force\":");
      Serial.print("true");
      Serial.println("}");
      server_running = true;
    }
    else
    {

      if (wifiManager.autoConnect(ap_ssid, ap_pass))
        server_running = false;
      else
        server_running = true;

      Serial.print("{\"Server_force\":");
      Serial.print("false");
      Serial.println("}");
    }

  }
  else
  {
    if (force == true)
    {
      wifiManager.startConfigPortal("GasSolutions", "12345678");
      Serial.print("{\"Server_force_wdefault\":");
      Serial.print("true");
      Serial.println("}");
      server_running = true;
    }
    else
    {
      if ( wifiManager.autoConnect("GasSolutions", "12345678"))
        server_running = false;
      else
        server_running = true;


      Serial.print("{\"Server_force_wdefault\":");
      Serial.print("false");
      Serial.println("}");
    }

  }
  Serial.print("{\"server_running\":");
  Serial.print(bool(server_running));
  Serial.println("}");*/
  return server_running;
}

bool wifi_AP_END()
{
  /*if (WiFi.getMode() & WIFI_AP) {
    WiFi.softAPdisconnect(true);
    Serial.println("{\"AP\":\"OFF\"}");
  }
  server_running = false;

  // Limpia los parámetros personalizados añadidos al WiFiManager
  for (auto p : customParams) {
    delete p;
  }
  customParams.clear();

  // Opcionalmente, puedes resetear el WiFiManager para limpiar cualquier configuración guardada
  wifiManager.resetSettings();

  Serial.print("{\"server_running\":");
  Serial.print(bool(server_running));
  Serial.println("}");*/
}


// --------------------------------------------------- wifiINIT
void wifi_init()
{
  //Serial.println("{\"wifi\":{\"init\":true}}");
  /*if ((obj["enable_wifi"].as<bool>() == true && (WiFi.status() != WL_CONNECTED)) || (obj["enable_wifi"].isNull()))
  {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);

    const char * auxssid = obj["ssid"].as<const char *>();
    const char * auxpass = obj["pass"].as<const char *>();

    // Star WiFi connection
    WiFi.begin(auxssid, auxpass);

    //Serial.print("{\"wifi\":{\"ssid\":\"");
    //Serial.print(auxssid);
    //Serial.println("\"}}");
    //Serial.println("{\"wifi\":\"init\"}");

    // Check wifi connection or make AP
    //wifiAP(false);

  }
  else if (obj["enable_wifi"].as<bool>() == false)
  {
    //
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("{\"wifi\":{\"enable\":false}}");
  }

  if (obj["enable_ap"].as<bool>() == true)
  {
    //wifiAP();
  }
  else
  {
    //wifi_AP_END();
  }
*/

}





// ------------------------------------------------------------------------------------------------------- checkServers
bool wifi_check()
{
  bool flag;

  //if (WiFi.status() != WL_CONNECTED)
  //{
    //wifi_init();
  //}

  // -------------------------------------------- server is running
  //if (server_running)
  //{
    //Serial.println("{\"wifi\":\"manager process\"}");
    //wifiManager.process();
  //}

 if (obj["enable_wifi"].as<bool>())
  {

    // ------------------ Wifi Connected
    if (WiFi.status() == WL_CONNECTED)
    {
      if (flag == false)
      {
        Serial.print("{\"wifi_connected\": ");
        serializeJson(obj["ssid"], Serial);
        Serial.println("}");
        Serial.print("{\"ip\":\"");
        Serial.print(WiFi.localIP());
        Serial.println("\"}");
        status_doc["ssid"] = obj["ssid"];
        status_doc["ip"] = WiFi.localIP();

        flag = true;
        //STATE = 1;
        //STATE |= (1 << 6);
      }

    }

    else //wifi not connected
    {
      Serial.println("{\"wifi\":\"disconnected\"}");
      flag = false;

      //STATE &= ~(1 << 6);
      //if (server_running == false)
      //  wifiAP(true);         // run force server
      //else
      //  Serial.println("{\"server\":\"running\"}");
    }


  }

  //}
  //else
  //{
    //Serial.println("{\"wifi\":\"disabled\"}");
    //flag = false;
    //WiFi.disconnect(true);
    //WiFi.mode(WIFI_OFF);
  //}

  return flag;
}



// --------------------------------------------------------------saveConfigCallback
void saveConfigCallback () {
  Serial.println("Should save config");

  for (WiFiManagerParameter* p : customParams) {
    // Suponiendo que cada p->getID() es único y coincide con las claves en 'doc'
    const char* paramId = p->getID();
    const char* paramValue = p->getValue();

    // Actualizar o añadir el valor en el documento JSON
    doc[paramId] = paramValue;
  }
  saveConfig = true;
  //return;
}

void bindServerCallback() {
  //wifiManager.server->on("/custom", handleRoute); // this is now crashing esp32 for some reason
  // wm.server->on("/info",handleRoute); // you can override wm!
}

void handleRoute() {
  Serial.println("[HTTP] handle route");

}

void saveWifiCallback() {
  Serial.println("[CALLBACK] saveCallback fired");
  saveConfig = true;
}

// ------------------------------------------------------------------wifiTask
void wifiTask(void * parameter) {
  for (;;) {
    // Esperar 1 segundo antes de la ejecución
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // Tareas relacionadas con WiFi
    //if ((((doc_encoder["STATE"] == 0)) || (doc_encoder["STATE"].isNull())) && (doc_display["k"].isNull()))
    {
      //if (!sd_ready)
      //  SD_Init();

      if (wifi_check())
      {
        update_clock();

        // ------------------------ firebase connection
        if ((updated == true) && (on_service == false))
        {
          connectFirebase();
        }

        // ------------------------ mqtt connection
        if (mqtt_check())
        {
          if (send_file)
          {
            mqtt_send_file(file_to_send);
          }
          if (send_log)
          {
            mqtt_send_log();
          }
          if (send_event)
          {
            mqtt_send_event();
          }
          if (send_report)
          {
            file_to_send = "/logs/" + String(anio) + "_" + String(mes) + "_" + String(dia_hoy) + ".json";
            mqtt_send_report();
            send_file = true;
          }
          if (send_list)
          {
            mqtt_send_list();
            send_list = false;
          }
          if (send_gps)
          {
            mqtt_send_gps();
            send_gps = false;
          }
        }
      }
    }
  }
}
