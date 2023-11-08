#ifndef SYSTEM_H
#define SYSTEM_H

#define   PRESS   LOW
//#define LIST_SIZE   4096
//#define LOG_SIZE   4096

#define WDT_TIMEOUT     150
#define FILE_SIZE       1024
#define LIST_SIZE       3048
#define LOG_SIZE        3048
#define BT_REPORT       0

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#include <Arduino.h>
#include <ArduinoJson.h>
//#include <vector>
#include <PubSubClient.h>
#include <WiFi.h>
//#include <LittleFS.h>
#include "FS.h"
#include "SPIFFS.h"
#include <Wire.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "time.h"
#include "RTClib.h"
#include <Wire.h>
#include <TinyGPSPlus.h>
#include <Firebase_ESP_Client.h>
#include <esp_task_wdt.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "version.h"
#include "wifiservice.h"
//#include "pins.h"
#include "filespiffs.h"
#include "mqttservice.h"
#include "wireservice.h"
#include "clock.h"
#include "gps_service.h"
#include  "printerservice.h"
#include "firebasedb.h"


//15 seconds WDT
extern bool buttonState;
extern bool lastButtonState;
extern unsigned long buttonPressTime;
extern const unsigned long longPressDuration;

extern unsigned long startTime;

extern bool factory_press;
extern unsigned long factory_time;
extern unsigned long prev_factory_time;
//extern bool reset_time;
//extern byte localAddress;     // address of this device
extern unsigned long mainRefresh;
extern unsigned long mainTime;
extern const uint32_t connectTimeoutMs;
extern unsigned long  s_timestamp;
extern volatile bool found_client;


// --------------------------------- printer
extern const char  end1;
extern const char  end2;
extern uint8_t tempVar;
extern char tempChar;
extern uint8_t resultadoBytes[200];
extern uint32_t pendingPrint;

extern char resultado[200];

extern const char* unidades[];
extern const char* decenas[];
extern const char* especiales[];
//uint32_t unitprice;


extern const unsigned long intervalo;
extern unsigned long tiempoAnterior;
extern unsigned long tiempoActual;

extern const unsigned long intervalo2;
extern unsigned long tiempoAnterior2;
extern unsigned long tiempoActual2;
extern volatile bool startCounting2;


extern uint32_t start_process_time;
extern uint32_t litros;
extern unsigned int pulsos_litro;
extern uint32_t precio;
extern float uprice; //price of 1 litre
extern uint32_t litros_check;
extern uint32_t precio_check;

extern int folio;
extern char b[200];
extern char buff[200];
extern int i;
extern String jsonStr;
extern unsigned int STATE_DISPLAY;

extern volatile bool display_reset;
extern volatile bool encoder_reset;
extern volatile bool start_print;
extern volatile bool startCounting;
extern volatile bool startFlowing;
extern volatile bool stopFlowing;
extern volatile bool readyToPrint;


extern volatile uint32_t pesos;

//void IRAM_ATTR factory_reset3();
void reset_config();
//void check_reset();
bool strToBool(String str);
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max);
void loadConfig();
void system_init();
void search_nclient(uint32_t aux_client);
void register_client();
#endif
