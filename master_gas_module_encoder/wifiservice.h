#ifndef WIFISERVICE_H
#define WIFISERVICE_H

#include "system.h"

extern bool correct;
extern int wifi_trys;
extern boolean isSaved;

extern WiFiManager wifiManager;
extern bool ALLOWONDEMAND; // enable on demand
extern bool WMISBLOCKING;
extern std::vector<WiFiManagerParameter*> customParams;
extern SemaphoreHandle_t wifiMutex;


//void Wifi_disconnected(WiFiEvent_t event, WiFiEventInfo_t info);
bool wifi_check();
void wifi_init();
void disableWiFi();
bool enableWiFi();


//callback notifying us of the need to save config
void saveConfigCallback ();
void bindServerCallback();
void handleRoute();
void saveWifiCallback();
//bool wifiAP(bool);
bool wifiAP();
bool wifi_AP_END();
void wifiTask(void * parameter);

//void neoConfig();

#endif 
