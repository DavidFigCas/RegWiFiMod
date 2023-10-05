#ifndef SYSTEM_H
#define SYSTEM_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_task_wdt.h>
#include <vector>
//#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiManager.h> 
#include <Firebase_ESP_Client.h>
#include "SPIFFS.h"
#include "FS.h"
#include "esp_attr.h"  
#include <Wire.h>

#include "counter.h"
#include "version.h"
#include "wifiservice.h"
#include "pins.h"
#include "filespiffs.h"
#include "firebasedb.h"
#include "wireservice.h"


//15 seconds WDT
#define WDT_TIMEOUT 15

extern bool factory_press;
extern unsigned long factory_time;
extern unsigned long prev_factory_time;
//extern bool reset_time;
//extern byte localAddress;     // address of this device
extern unsigned long mainRefresh;
extern unsigned long mainTime;
extern const uint32_t connectTimeoutMs;
extern unsigned long  s_timestamp;
extern int buttonState;

//void IRAM_ATTR factory_reset3();
void reset_config();
//void check_reset();
bool strToBool(String str);
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max);
void loadConfig();
void system_init();

#endif
