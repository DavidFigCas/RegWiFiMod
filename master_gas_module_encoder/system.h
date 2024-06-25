#ifndef SYSTEM_H
#define SYSTEM_H


#define   PRESS   LOW
//#define LIST_SIZE   4096
//#define LOG_SIZE   4096

#define WDT_TIMEOUT     15
#define FILE_SIZE       1024
#define LIST_SIZE       3048
#define LOG_SIZE        3048
#define STATUS_SIZE     2048
#define BT_REPORT       0

//#define SCREEN_WIDTH 128 // OLED display width, in pixels
//#define SCREEN_HEIGHT 32 // OLED display height, in pixels
//#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
//#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
// ----------------- LCD
#define BLACK 0
#define WHITE 1

// --------------------------------sd card
//#define UART_BAUD           9600
//#define PIN_DTR             25
//#define PIN_TX              27  
//#define PIN_RX              26
//#define PWR_PIN             4

// --------------------------- SD CARD PINS
#define SD_MISO             2
#define SD_MOSI             15
#define SD_SCLK             14
#define SD_CS               13

// ------------------------- LCD
#define SHARP_SCK  18
#define SHARP_MOSI 23
#define SHARP_SS   5

//SYSTEM
#include <Arduino.h>
#include <esp_task_wdt.h>

//BT
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

//JSON
#include <ArduinoJson.h>

//WIFI
#include <WiFi.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager


//MQTT
#include <PubSubClient.h>


//CLOCK NTP
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "time.h"


//CLOCK RTC
#include "RTClib.h"


//FILESYSTEM
#include "FS.h"
#include "SPIFFS.h"


//SD CARD
#include "SD.h"
#include "SPI.h"


// I2C COMUNICATION
#include <Wire.h>


//GPS
#include <TinyGPSPlus.h>

//FIREBASE
//#include <Firebase_ESP_Client.h>
//#include "firebasedb.h"



// ------------------ LOCAL
#include "version.h"
#include "wifiservice.h"
#include "clock.h"
#include "mqttservice.h"
#include "wireservice.h"
#include "filespiffs.h"
#include  "printerservice.h"
#include "sd_card_service.h"
#include "clock.h"
#include "gps_service.h"
//#include "bt_service.h"
#include "encoder_service.h"


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

extern unsigned long serialRefresh;
extern unsigned long serialTime;


// --------------------------------- printer
extern const char  end1;
extern const char  end2;
extern uint8_t tempVar;
extern char tempChar;
extern uint8_t resultadoBytes[200];
extern uint32_t pendingPrint;

extern char resultado[400];

//extern const char* unidades[];
//extern const char* decenas[];
//extern const char* especiales[];
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
extern uint32_t reporte;
extern uint32_t litros_suma;
extern uint32_t servicios;
extern uint32_t total_ventas;
extern uint32_t folio_ini;
extern uint32_t folio_fin;
extern uint32_t litros_ini;
extern uint32_t litros_fin;


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
extern volatile bool on_service;

extern volatile uint32_t pesos;

extern volatile bool updated;

extern String cadenaTeclas;
extern bool clear_key;

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
void Serial_CMD();


#endif
