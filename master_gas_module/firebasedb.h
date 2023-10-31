#ifndef  FIREBASEDB_H
#define FIREBASEDB_H

#include <Firebase_ESP_Client.h>
//#include <addons/TokenHelper.h>         // Provide the token generation process info.
//#include <addons/RTDBHelper.h>          // Provide the RTDB payload printing info and other helper functions.
#include "system.h"
#include "version.h"
//#include "sensors.h"

// Define Firebase Data object
extern FirebaseData fbdo;
extern FirebaseAuth auth;
extern FirebaseConfig config;
extern FirebaseJson json;
//extern FirebaseJson events_json;
extern FirebaseJson conf;
extern FirebaseJson data_json;
extern String nodeName;
extern volatile bool updated;
extern volatile bool dataChanged;
extern volatile bool nullData;
//extern volatile bool saveConfig;
extern volatile bool fire_stream;
extern volatile bool update_events;
extern FirebaseData stream;
extern String route;


void fcsDownloadCallback(FCS_DownloadStatusInfo info);
void SendData();
void streamCallback(FirebaseStream data);
void prepareData();
void streamTimeoutCallback(bool timeout);
void connectFirebase();
void copyJsonObject(FirebaseJson& firebaseJson, JsonObject& jsonObject);
//void merge(JsonVariant dst, JsonVariantConst src);

#endif  // FIREBASEDB_H
