#ifndef SYSTEM_H
#define SYSTEM_H

#include <Arduino.h>
#include <ArduinoJson.h>
//#include <vector>
#include <PubSubClient.h>
#include <WiFi.h>
//#include <LittleFS.h>


#define   PRESS   LOW
//#define LIST_SIZE   4096
//#define LOG_SIZE   4096

#define WDT_TIMEOUT     150
#define FILE_SIZE       1024
#define LIST_SIZE       3048
#define LOG_SIZE        3048
#define STATUS_SIZE     2048
#define BT_REPORT       0

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

// --------------------------------sd card
//#define UART_BAUD           9600
//#define PIN_DTR             25
//#define PIN_TX              27  
//#define PIN_RX              26
//#define PWR_PIN             4

#define SD_MISO             2
#define SD_MOSI             15
#define SD_SCLK             14
#define SD_CS               13

#include <Arduino.h>
#include <ArduinoJson.h>
//#include <vector>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

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
#include <LiquidCrystal_I2C.h>
#include <cmath> // 
#include "SD.h"
#include "SPI.h"


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
#include "oled_display.h"
#include  "glcd_display.h"
#include  "sd_card_service.h"


//15 seconds WDT

// sd card
extern bool sd_ready;

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

extern char resultado[400];

extern const char* unidades[];
extern const char* decenas[];
extern const char* especiales[];
//uint32_t unitprice;
extern uint32_t startTimeToPrint;


extern const unsigned long intervalo;
extern unsigned long tiempoAnterior;
extern unsigned long tiempoActual;

extern const unsigned long intervalo2;
extern unsigned long tiempoAnterior2;
extern unsigned long tiempoActual2;
extern volatile bool startCounting2;

// ----------------------------------------GPS intervalos para gps
extern unsigned long previousMillisGPS;  // Variable para almacenar la última vez que se ejecutó el evento
extern const long intervalGPS;  // Intervalo en milisegundos (60,000 milisegundos = 1 minuto)
extern unsigned long currentMillisGPS; 

// ------------------------------------- wifi flag
extern bool server_running;

extern String gps_name_file;
extern String gps_str;


extern uint32_t start_process_time;
extern float litros;
extern uint32_t acumulado_litros;
extern float pulsos_litro;
extern float precio;
extern float uprice; //price of 1 litre
extern uint32_t litros_check;
extern uint32_t precio_check;

extern uint32_t folio;
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
/*static*/ void Cfg_get(/*struct jsonrpc_request * r*/);
void system_init();
void search_nclient(uint32_t aux_client);
void register_client();
void saveNewlog();
void read_logs(String consult);


#endif
