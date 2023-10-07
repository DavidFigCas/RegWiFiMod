#ifndef SYSTEM_H
#define SYSTEM_H

#include <Arduino.h>
#include <ArduinoJson.h>
//#include <vector>
#include <PubSubClient.h>
#include <WiFi.h>
#include <LittleFS.h>
//#include <Wire.h>
#include "i2c_fifo.h"
#include "i2c_slave.h"
#include "pico/stdlib.h"
//#include <WiFiManager_RP2040W.h>  

#include "version.h"
#include "wifiservice.h"
//#include "pins.h"
#include "filespiffs.h"
#include "mqttservice.h"
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
//extern volatile uint32_t nclient_data; // nclient_data[4]
//extern volatile uint8_t price_data[2], litro_data[4], factor_data[2], name_data[42];

//void IRAM_ATTR factory_reset3();
void reset_config();
//void check_reset();
bool strToBool(String str);
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max);
void loadConfig();
void system_init();

#endif
