#include "wifiservice.h"

bool correct = false;
int wifi_trys;
boolean isSaved = false;
bool ALLOWONDEMAND   = true; // enable on demand
bool WMISBLOCKING    = true;
WiFiManager wifiManager;
std::vector<WiFiManagerParameter*> customParams;


void wifi_init()
{
  if (obj["enable_wifi"].as<bool>() == true && (WiFi.status() != WL_CONNECTED))
  {
    /*WiFi.mode(WIFI_STA);
      String auxssid = obj["ssid"].as<String>();
      String auxpass = obj["pass"].as<String>();

      WiFi.begin(obj["ssid"].as<const char*>(), obj["pass"].as<const char*>());
      Serial.print("{\"wifi\":{\"ssid\":\"");
      Serial.print(obj["ssid"].as<const char*>());
      Serial.println("\"}}");*/

    Serial.println("{\"wifi\":\"init\"}");

    // ---- load parameter for config portal

    for (JsonPair kv : doc.as<JsonObject>()) {
      String keyString = kv.key().c_str();
      char* key = new char[keyString.length() + 1];
      strcpy(key, keyString.c_str());

      String value = kv.value().as<String>();
      char* valueCStr = new char[value.length() + 1];
      strcpy(valueCStr, value.c_str());

      WiFiManagerParameter* p = new WiFiManagerParameter(key, key, valueCStr, value.length() + 1);
      customParams.push_back(p);
      wifiManager.addParameter(p);
    }

    //WiFiManager
    if (!WMISBLOCKING) {
      wifiManager.setConfigPortalBlocking(false);
    }

    //set config save notify callback
    wifiManager.setSaveParamsCallback(saveConfigCallback);
    wifiManager.setSaveConfigCallback(saveWifiCallback);
    wifiManager.setWebServerCallback(bindServerCallback);
    wifiManager.setBreakAfterConfig(true); // needed to use saveWifiCallback

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
    //wifiManager.setTimeout(120);

    //fetches ssid and pass and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
    if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
      Serial.println("failed to connect previous network and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      //ESP.restart();
      //delay(5000);
    }
    else
    {
      //if you get here you have connected to the WiFi
      Serial.println("{\"wifi\":\"first connection\"}");

      Serial.print("{\"ip\":\"");
      Serial.print(WiFi.localIP());
      Serial.println("\"}");
    }

  }
  else if (obj["enable_wifi"].as<bool>() == false)
  {
    //
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("{\"wifi\":{\"enable\":false}}");
  }


}





// ------------------------------------------------------------------------------------------------------- checkServer
bool wifi_check()
{
  // put your main code here, to run repeatedly:
  if (!WMISBLOCKING) {
    Serial.println("{\"wifi\":\"manager process\"}");
    wifiManager.process();
  }
  // is configuration portal requested?
  if (ALLOWONDEMAND && digitalRead(ONDDEMANDPIN) == LOW )
  {
    delay(100);
    if ( digitalRead(ONDDEMANDPIN) == LOW )
    {
      Serial.println("BUTTON PRESSED");

      // button reset/reboot
      // wm.resetSettings();
      // wm.reboot();
      // delay(200);
      // return;


      wifiManager.setSaveParamsCallback(saveConfigCallback);
      wifiManager.setSaveConfigCallback(saveWifiCallback);
      wifiManager.setWebServerCallback(bindServerCallback);
      wifiManager.setConfigPortalTimeout(140);
      wifiManager.setBreakAfterConfig(true); // needed to use saveWifiCallback
      //wifiManager.setParamsPage(true); // move params to seperate page, not wifi, do not combine with setmenu!

      // disable captive portal redirection
      // wm.setCaptivePortalEnable(false);

      if (!wifiManager.startConfigPortal("OnDemandAP", "12345678")) {
        Serial.println("failed to connect on demand and hit timeout");
        //delay(3000);
      }
    }
    else {
      //if you get here you have connected to the WiFi
      Serial.println("{\"new_wifi\":\"connected\"}");
      //getTime();
    }
  }


  if (obj["enable_wifi"].as<bool>())
  {

    //if ((millis() - s_timestamp) >= connectTimeoutMs) // check to an interval of time
    //if (millis() - mainRefresh > mainTime)
    //mainRefresh = millis();





    if ((millis() - s_timestamp) >= connectTimeoutMs) // check to an interval of time
    {
      s_timestamp = millis();
      //String auxssid = obj["ssid"].as<String>();


      // ------------------ Wifi Connected
      if (WiFi.status() == WL_CONNECTED)
      {
        Serial.println("{\"check_wifi\":\"connected\"}");
        //if (obj["enable_rtc"].as<bool>())
        //  update_clock();

        return true;

      }

      else //wifi not connected
      {
        Serial.println("{\"wifi\":\"disconnected\"}");
        return false;
      }


    }

  }
  else
    return false;
}



// ------------------------- callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  saveConfig = true;
  //return;
}

void bindServerCallback() {
  wifiManager.server->on("/custom", handleRoute); // this is now crashing esp32 for some reason
  // wm.server->on("/info",handleRoute); // you can override wm!
}

void handleRoute() {
  Serial.println("[HTTP] handle route");
  wifiManager.server->send(200, "text/plain", "hello from user code");
}

void saveWifiCallback() {
  Serial.println("[CALLBACK] saveCallback fired");
}
