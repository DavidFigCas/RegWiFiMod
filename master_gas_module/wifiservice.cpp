#include "wifiservice.h"

bool correct = false;
int wifi_trys;
boolean isSaved = false;
bool ALLOWONDEMAND   = true; // enable on demand
bool WMISBLOCKING    = true;
WiFiManager wifiManager;
std::vector<WiFiManagerParameter*> customParams;



// ------------------------------------------------ wifiAP
bool wifiAP(bool force)
{

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
  wifiManager.setTimeout(120);

  //set config save notify callback
  wifiManager.setSaveParamsCallback(saveConfigCallback);
  wifiManager.setSaveConfigCallback(saveWifiCallback);
  wifiManager.setWebServerCallback(bindServerCallback);
  wifiManager.setBreakAfterConfig(true); // needed to use saveWifiCallback
  wifiManager.setConfigPortalTimeout(140);
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
  Serial.println("}");
  return server_running;
}


// --------------------------------------------------- wifiINIT
void wifi_init()
{

  if ((obj["enable_wifi"].as<bool>() == true && (WiFi.status() != WL_CONNECTED)) || (obj["enable_wifi"].isNull()))
  {
    WiFi.mode(WIFI_STA);

    const char * auxssid = obj["ssid"].as<const char *>();
    const char * auxpass = obj["pass"].as<const char *>();

    // Star WiFi connection
    WiFi.begin(auxssid, auxpass);

    Serial.print("{\"wifi\":{\"ssid\":\"");
    Serial.print(auxssid);
    Serial.println("\"}}");
    Serial.println("{\"wifi\":\"init\"}");

    // Check wifi connection or make AP
    wifiAP(false);

  }
  else if (obj["enable_wifi"].as<bool>() == false)
  {
    //
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("{\"wifi\":{\"enable\":false}}");
  }


}





// ------------------------------------------------------------------------------------------------------- checkServers
bool wifi_check()
{
  bool flag;

  // -------------------------------------------- server is running
  if (server_running)
  {
    Serial.println("{\"wifi\":\"manager process\"}");
    wifiManager.process();
  }

  //if (obj["enable_wifi"].as<bool>())
  {

    //if ((millis() - s_timestamp) >= connectTimeoutMs) // check to an interval of time
    //{
    //s_timestamp = millis();

    // ------------------ Wifi Connected
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.print("{\"wifi_connected\": ");
      serializeJson(obj["ssid"],Serial);
      Serial.println("}");
      Serial.print("{\"ip\":\"");
      Serial.print(WiFi.localIP());
      Serial.println("\"}");
      status_doc["ssid"] = obj["ssid"];
      status_doc["ip"] = WiFi.localIP();

      flag = true;
      //STATE = 1;
      STATE |= (1 << 6);

      // ------------------------ firebase connection
      if (updated == false)
      {
        oled_display_text("VERSION \n NUEVA");
        connectFirebase();
      }


    }

    else //wifi not connected
    {
      Serial.println("{\"wifi\":\"disconnected\"}");
      flag = false;
      //STATE = 0;
      STATE &= ~(1 << 6);
      if (server_running == false)
        wifiAP(true);         // run force server
      else
        Serial.println("{\"server\":\"running\"}");
    }


  }

  //}
  //else

  return flag;
}



// ------------------------- callback notifying us of the need to save config
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
