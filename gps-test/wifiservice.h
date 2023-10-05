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

//void Wifi_disconnected(WiFiEvent_t event, WiFiEventInfo_t info);
bool wifi_check();
void wifi_init();


//callback notifying us of the need to save config
void saveConfigCallback ();
void bindServerCallback();
void handleRoute();
void saveWifiCallback();

//void neoConfig();

#endif 
