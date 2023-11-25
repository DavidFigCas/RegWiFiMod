#include "firebasedb.h"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;
//FirebaseJson events_json;
FirebaseJson conf;
FirebaseJson data_json;
String nodeName;
volatile bool updated = true;
volatile bool dataChanged = false;
volatile bool nullData = false;
//volatile bool saveConfig = false;
volatile bool fire_stream = false;
volatile bool update_events = false;
String route = "/panels/" + obj["id"].as<String>();// + "/actual";
FirebaseData stream;

// ----------------------------------- OTA The Firebase Storage download callback function
void fcsDownloadCallback(FCS_DownloadStatusInfo info)
{
  esp_task_wdt_reset();

  if (info.status == fb_esp_fcs_download_status_init)
  {
    Serial.printf("update %s (%d)\n", info.remoteFileName.c_str(), info.fileSize);
    obj["updated"] = true;
    obj["registered"] = false;
    saveConfig = true;
    //json.clear();
    //json.set("updated", true);
    Serial.println("{\"update_firmware\":true}");
  }
  else if (info.status == fb_esp_fcs_download_status_download)
  {
    Serial.printf("Done %d%s, Time %d ms\n", (int)info.progress, "%", info.elapsedTime);
    char bufferf[100];
    sprintf(bufferf, "Done %d%s", (int)info.progress, "%");
    oled_display_text(bufferf);
  }
  else if (info.status == fb_esp_fcs_download_status_complete)
  {
    Serial.println("Completed.");
    Serial.println();

    Serial.println("{\"save_last_config\":true}");
    obj["updated"] = true;
    obj["registered"] = false;
    saveConfig = true;
    saveConfigData();
    //loadConfig();
    Serial.println("Restarting...\n\n");
    ESP.restart();

  }
  else if (info.status == fb_esp_fcs_download_status_error)
  {
    Serial.printf("Update fail, %s\n", info.errorMsg.c_str());
  }
}



// -------------------------------------------------------------------------------------- SendData
void SendData()
{
  /* if (Firebase.ready() && (WiFi.status() == WL_CONNECTED))
    {
     //json.set("updatedBySelf", true);
     prepareData();

     // ------------------------------------- ergo
     if (obj["type"].as<String>() == "ergo")
     {

     }
     // ------------------------------------- cruz
     else if (obj["type"].as<String>() == "cruz")
     {
       //------------------ automatic update events, new month
       if (update_events)
       {
         json.clear();
         String jsonString = "{\"events\":\"32\":0}";
         FirebaseJson firebaseJson;
         firebaseJson.setJsonData(jsonString);
         json.set("events", firebaseJson);
         json.set("mes_prev", mes);
         Serial.println("{\"update_events\":true}");
         if (Firebase.RTDB.updateNode(&fbdo, route + "/config", &json) == false)
           Serial.printf("%s\n", fbdo.errorReason().c_str());
         //else
         update_events = false;

       }*/

  // ------------------------------------- response for new firmware
  /*if (obj["updated"].as<bool>() == false)
    {
    obj["updated"] = true;
    obj["registered"] = false;
    saveConfig = true;
    json.clear();
    json.set("updated", true);
    Serial.println("{\"update_firmware\":true}");
    if (Firebase.RTDB.updateNode(&fbdo, route + "/config", &json) == false)
      Serial.printf("%s\n", fbdo.errorReason().c_str());
    //else

    }*/

  // ------------------------------------- response for new firmware
  /*if (obj["restart"].as<bool>() == true)
    {
    json.clear();
    json.set("restart", false);
    if (Firebase.RTDB.updateNode(&fbdo, route + "/config", &json) == false)
      Serial.printf("%s\n", fbdo.errorReason().c_str());
    }

    json.remove("updated");
    json.remove("restart");
    json.remove("mes_prev");
    if (Firebase.RTDB.updateNode(&fbdo, route + "/actual", &json) == false)
    Serial.printf("%s\n", fbdo.errorReason().c_str());


    json.remove("Ts/.sv");
    json.remove("time");
    json.remove("mes_prev");
    if (Firebase.RTDB.updateNode(&fbdo, route + "/data/" + String(now.year()) + "_" + String(now.month()), &json) == false)
    {
    Serial.printf("%s\n", fbdo.errorReason().c_str());
    }
    }

    }
    else if ((obj["enable_wifi"].as<bool>() == true) && (WiFi.status() == WL_CONNECTED))
    {
    connectFirebase();
    }*/
}


// ---------------------------------------------------------------------------------------------------- streamCallback
void streamCallback(FirebaseStream data)
{
  // Add config from spiff file
  /*
    Serial.println("{\"stream\":true}");

    if (obj["test"].as<bool>() == true)
    {
      Serial.printf("stream path: %s\nevent path: %s\ndata type: %s\nevent type: %s\ndata:  %s\n\n",
                    data.streamPath().c_str(),
                    data.dataPath().c_str(),
                    data.dataType().c_str(),
                    data.eventType().c_str(),
                    data.payload().c_str());
    }


    if ((strcmp(data.dataPath().c_str(), "/") == 0) && (strcmp(data.eventType().c_str(), "patch") != 0))
    {
      if (strcmp(data.dataType().c_str(), "null") != 0)
      {
        //Serial.println("All obj");
        // Llamar a deserializeJson() para decodificar la respuesta
        deserializeJson(doc, data.payload().c_str());
        if (obj["test"].as<bool>() == true)
          serializeJson(obj, Serial);
        saveConfig = true;
        //saveConfigData();
        //loadConfig();
      }
      else
      {
        obj["registered"] = false;
        saveConfigData();
        loadConfig();
        ESP.restart();
        //SendData();
      }

    }
    else  if ((strcmp(data.dataPath().c_str(), "/") == 0) && (strcmp(data.eventType().c_str(), "patch") == 0))
    {
      if (strcmp(data.dataType().c_str(), "null") != 0)
      {
        DynamicJsonDocument doc_patch(1024);
        deserializeJson(doc_patch, data.payload().c_str());

        if (doc_patch.containsKey("defColor"))
        {
          obj["defColor"] = doc_patch["defColor"];
          saveConfig = true;
        }

        else if (doc_patch.containsKey("last_ac"))
        {
          obj["last_ac"] = doc_patch["last_ac"];
          saveConfig = true;
        }
        else if (doc_patch.containsKey("registered"))
        {
          obj["registered"] = doc_patch["registered"];
          if (obj["registered"] == false)
            obj["restart"] = true;
          saveConfig = true;
        }
      }
    }
    else  if ((strcmp(data.dataPath().c_str(), "/events") == 0) && (strcmp(data.eventType().c_str(), "patch") == 0))
    {
      if (strcmp(data.dataType().c_str(), "null") != 0)
      {
        DynamicJsonDocument doc_patch(1024);
        deserializeJson(doc_patch, data.payload().c_str());

        //Serial.println("Fast Up Events");
        //Serial.println();
        //serializeJson(doc_patch, Serial);
        //Serial.println();

        // Combinar los objetos JSON
        for (const auto& kv : doc_patch.as<JsonObject>())
        {
          obj["events"][kv.key()] = kv.value();
        }

        serializeJson(obj["events"], Serial);
        Serial.println();
        saveConfig = true;

      }
    }
  */
}


// ------------------------------------------------------------------------------------------------------- prepareData
void prepareData()
{
  // ------------------------------------------ all

  //json.set("sensors", obj["sensors"]);
  //json.set("h", h);
  //json.set("uv", uv);
  //json.set("db", db);
  //json.set("lux", lux);
  //json.set("ppm", ppm);

  //json.set("sensors", t);
  // now we will set the timestamp value at Ts
  //json.set("Ts/.sv", "timestamp"); // .sv is the required place holder for sever value which currently supports only string "timestamp" as a value
  //nodeName = String(millis());;
  // Set data with timestamp
  //Serial.printf("%s\n", Firebase.RTDB.updateNode(&fbdo, "/panels/01/actual", &json) ? /*fbdo.to<FirebaseJson>().raw()*/"" : fbdo.errorReason().c_str());
}


//------------------------------------------------------------------ connectFirebase
void connectFirebase()
{
  // Firebase
  esp_task_wdt_reset();
  if (!Firebase.ready()) // Add more filters
  {


    // Timeout, prevent to program halt
    config.timeout.wifiReconnect = 5 * 1000; // 10 Seconds to 2 min (10 * 1000)
    config.timeout.socketConnection = 30 * 1000; // 1 sec to 1 min (30 * 1000)
    config.timeout.sslHandshake = 2 * 60 * 1000; // 1 sec to 2 min (2 * 60 * 1000)
    config.timeout.rtdbKeepAlive = 20 * 1000;    // 20 sec to 2 min (45 * 1000)
    config.timeout.rtdbStreamReconnect = 1 * 1000;  //1 sec to 1 min (1 * 1000)
    config.timeout.rtdbStreamError = 3 * 1000;    // 3 sec to 30 sec (3 * 1000)
    config.timeout.serverResponse = 1 * 1000;    //Server response read timeout in ms 1 sec - 1 min ( 10 * 1000).
    /* Assign the api key (required) */
    // s_aux = obj["key"].as<String>();
    //len = s_aux.length();
    //--------------- LOAD REGISTERS
    //String email = obj["email"].as<String>(); // Suponiendo que obj es un objeto JSON válido
    //size_t length = email.length();

    /* if (length <= sizeof(name_data)) {
       strncpy((char*)name_data, email.c_str(), sizeof(name_data));
       name_data[sizeof(name_data) - 1] = '\0'; // Asegura que la cadena esté terminada correctamente
      } else {
       Serial.println("La longitud del correo electrónico es demasiado larga para name_data");
      }*/

    //blk = false;
    Serial.println("{\"firebase_init\":true}");
    config.api_key = obj["key"].as<String>();

    /* Assign the user sign in credentials */
    auth.user.email = obj["email"].as<String>();
    auth.user.password = obj["firepass"].as<String>();

    /* Assign the RTDB URL (required) */
    config.database_url = obj["url"].as<String>();
    //esp_task_wdt_init(WDT_TIMEOUT*3, true);  //enable panic so ESP32 restarts

    Serial.println("{\"connecting_firebase\":true}");
    Firebase.begin(&config, &auth);
    esp_task_wdt_reset();
    Firebase.reconnectWiFi(true);

    unsigned long startTime = millis();
    esp_task_wdt_reset();
    while (!Firebase.ready())
      //if (!Firebase.ready())
    {
      if (millis() - startTime > mainTime)
      {
        Serial.println("Failed to connect to Firebase within timeout period");
        break; // Salir del bucle si no se puede conectar a Firebase después de TIMEOUT_DURATION milisegundos
      }
      delay(100); // Esperar un poco antes de comprobar de nuevo, para no bloquear completamente el bucle
    }

  }


  if (!updated)
  {
    updated = true;
    String storage_id = obj["storage_id"].as<String>();
    //SendData();
    Serial.println("{\"new_firmware\":true}");
    //delay(2000);

    // In ESP8266, this function will allocate 16k+ memory for internal SSL client.
    if (!Firebase.Storage.downloadOTA(&fbdo, storage_id/* Firebase Storage bucket id */, "master_gas_module.ino.esp32da.bin" /* path of firmware file stored in the bucket */, fcsDownloadCallback /* callback function */))
      Serial.println(fbdo.errorReason());
  }
}

// ---------------------------------------------------------------------------------------------------- streamTimeoutCallback
void streamTimeoutCallback(bool timeout)
{
  if (!stream.httpConnected())
    Serial.printf("{\"stream_error\":\"%s\"}\n\n", stream.errorReason().c_str());
}


// ----------------------------------------------------------------------------------------------------- copyJsonObject
void copyJsonObject(FirebaseJson & firebaseJson, JsonObject & jsonObject)
{
  /*for (JsonObject::iterator it = jsonObject.begin(); it != jsonObject.end(); ++it) {
    if (it->value().is<JsonObject>()) {
      FirebaseJson nestedFirebaseJson;
      JsonObject nestedJsonObject = it->value().as<JsonObject>();
      copyJsonObject(nestedFirebaseJson, nestedJsonObject);
      firebaseJson.set(it->key().c_str(), nestedFirebaseJson);
    }
    else if (it->value().is<String>()) {
      firebaseJson.set(it->key().c_str(), it->value().as<String>());
    }
    else if (it->value().is<int>()) {
      firebaseJson.set(it->key().c_str(), it->value().as<int>());
    }
    else if (it->value().is<float>()) {
      firebaseJson.set(it->key().c_str(), it->value().as<float>());
    }
    else if (it->value().is<bool>()) {
      firebaseJson.set(it->key().c_str(), it->value().as<bool>());
    }
    else if (it->value().is<long>()) {
      firebaseJson.set(it->key().c_str(), it->value().as<long>());
    }
    else if (it->value().is<double>()) {
      firebaseJson.set(it->key().c_str(), it->value().as<double>());
    }
    else {
      Serial.printf("Unsupported data type for key: %s\n", it->key().c_str());
    }
    }*/
}
