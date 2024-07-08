#include "Arduino.h"
#include "hardware/watchdog.h"
#include "pico/stdlib.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <UnixTime.h>
#include <ArduinoJson.h>
#include <cmath>
#include <Keypad.h>
#include "hardware/uart.h"

//#define SHARP_SCK  2
//#define SHARP_MOSI 3
//#define SHARP_SS   1
//#define RESTART_PIN 15

#define SHARP_SCK  2
#define SHARP_MOSI 3
#define SHARP_SS   4
#define RESTART_PIN 15

#define BLACK 0
#define WHITE 1

int16_t x_lit = 450;
int16_t y_lit = 125;
int16_t x_pes = 450;
int16_t y_pes = 212;

uint16_t w;
uint16_t h;
uint16_t w2;
uint16_t h2;

const uint8_t FILAS = 4;
const uint8_t COLUMNAS = 4;
char tecla;

uint8_t pinesFilas[FILAS] = {11, 12, 13, 14};
uint8_t pinesColumnas[COLUMNAS] = {18, 19, 20, 21};

char teclas[FILAS][COLUMNAS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};


Keypad teclado = Keypad(makeKeymap(teclas), pinesFilas, pinesColumnas, FILAS, COLUMNAS);

Adafruit_SharpMem display(SHARP_SCK, SHARP_MOSI, SHARP_SS, 320, 240);
U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;
UnixTime stamp(0);

bool flag_img = true;
bool flag_msg = true;
bool flag_med = true;
const char* txt;
const char* txt_med_1;
const char* txt_med_2;
static int txt_size = 0;
static int txt_x;
static int txt_y;

static char buffx[2000];
StaticJsonDocument<2000> doc;
StaticJsonDocument<2000> doc_aux;
String jsonStr;
const char* aux_char;

const unsigned long intervalo = 1000;
unsigned long tiempoAnterior = 0;
unsigned long tiempoActual;

const unsigned long intervalo2 = 250;
unsigned long tiempoAnterior2 = 0;
unsigned long tiempoActual2;

volatile uint32_t litros, print_litros, print_pesos, pesos, litros_target;

uint8_t STATE = 0;

volatile boolean  shown = 0;
volatile uint32_t number = 0;
uint32_t unixtime, client;
uint32_t prevTime;
unsigned int desconex_count;
bool buttonState;
bool prevButtonState;
String cadenaNumeros = "";
String cadenaLetras = "";
bool open_valve = false;
bool close_valve = false;
bool valve_state = false;
bool print_report = false;
bool enable_ap = false;
int64_t current;

// Imagen en formato de matriz de bits
const uint8_t myBitmap[] PROGMEM = {
  // Aquí va la matriz de bits de tu imagen
  // Ejemplo de 128x64 píxeles (1024 bytes)
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  // ...continúa con los datos de la imagen
};

//UART Serial2(8,9);
// --------------------------------------------------------------------- SETUP
void setup()
{
  delay(2000);
  Serial.begin(115200);
  //Serial1.setTX(4);  // Establecer el pin TX para Serial1
  //Serial1.setRX(5);  // Establecer el pin RX para Serial1


  Serial.println("Init Display");

  pinMode(28, OUTPUT);
  digitalWrite(28, 0);
  pinMode(27, OUTPUT);
  digitalWrite(27, 0);
  teclado.setDebounceTime(10);

  setup1();

  // Mostrar la imagen al inicio
  //displayImage(myBitmap);
}

// --------------------------------------------------------------------- LOOP
void loop() {
  tecla = teclado.getKey();
  if (tecla) {
    // Crear JSON con la tecla presionada
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["method"] = "key_press";
    jsonDoc["params"]["key"] = String(tecla);
    String jsonStr;
    serializeJson(jsonDoc, jsonStr);
    Serial.println(jsonStr);
    Serial1.println(jsonStr);
  }

  // Leer y procesar datos del puerto serie principal PC
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    Serial.print("PC: ");
    Serial.println(input);

    // Deserializar JSON
    DeserializationError error = deserializeJson(doc, input);
    if (!error) {
      const char* method = doc["method"];
      if (strcmp(method, "msg") == 0) {
        const char* message = doc["params"]["payload"];
        displayMessage(message);
      }
    } else {
      //Serial.print("Error de parseo JSON: ");
      Serial.println(error.c_str());
    }
  }


  if (flag_msg)
  {
    displayMessage(txt);
    flag_msg = false;
  }
  
  if (flag_med)
  {
    displayMedidor(txt_med_1,txt_med_2);
    flag_msg = false;
  }


}

// ----------------------------------------------------------------- SETUP1
void setup1()
{
  delay(2000);
  Serial1.begin(115200); // Inicializar segundo puerto serie
  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);

  display.begin();
  display.clearDisplay();
  u8g2_for_adafruit_gfx.begin(display);

  u8g2_for_adafruit_gfx.setForegroundColor(BLACK); // apply Adafruit GFX color
  u8g2_for_adafruit_gfx.setBackgroundColor(WHITE); // apply Adafruit GFX color
  //u8g2_for_adafruit_gfx.setFont(u8g2_font_lubB19_tr); // extended font
  u8g2_for_adafruit_gfx.setFont(u8g2_font_logisoso78_tn);
  u8g2_for_adafruit_gfx.setCursor(0, 90); // start writing at this position
  u8g2_for_adafruit_gfx.print("Hola");

  display.refresh();

  digitalWrite(25, LOW);
}

void loop1()
{
  // Leer y procesar datos del segundo puerto serie ENCODER
  while (Serial1.available() > 0)
  {
    String input = Serial1.readStringUntil('\n');
    //Serial.print("E: ");
    Serial.println(input);

    // Deserializar JSON
    DeserializationError error = deserializeJson(doc, input);
    if (!error) {
      const char* method = doc["method"];

      if (strcmp(method, "msg") == 0)
      {
        flag_msg = true;
        if (!doc["params"]["txt"].isNull())
        {
          txt  = doc["params"]["txt"];
        }
        if (!doc["params"]["size"].isNull())
        {
          txt_size = doc["params"]["size"];
        }
        if (!doc["params"]["txt_x"].isNull())
        {
          txt_x = doc["params"]["txt_x"];
        }
        if (!doc["params"]["txt_y"].isNull())
        {
          txt_y = doc["params"]["txt_y"];
        }
      }
      else if (strcmp(method, "med") == 0)
      {
        flag_med = true;
        if (!doc["params"]["1"].isNull())
        {
          txt_med_1  = doc["params"]["1"];
        }
        if (!doc["params"]["2"].isNull())
        {
          txt_med_2 = doc["params"]["2"];
        }
        if (!doc["params"]["size"].isNull())
        {
          txt_size = doc["params"]["size"];
        }
        if (!doc["params"]["txt_x"].isNull())
        {
          txt_x = doc["params"]["txt_x"];
        }
        if (!doc["params"]["txt_y"].isNull())
        {
          txt_y = doc["params"]["txt_y"];
        }
      }

    } else {
      //Serial.print("Error de parseo JSON: ");
      Serial.println(error.c_str());
    }
  }


  while (!Serial1.available());
}

// Función para mostrar un mensaje en el display
void displayMessage(const char* message)
{
  display.fillRect(0, 0, 320, 240, WHITE); // Llenar la pantalla de blanco

  if (txt_size == 0)
    u8g2_for_adafruit_gfx.setFont(u8g2_font_profont22_tf);  // 14 pixels
  else
    u8g2_for_adafruit_gfx.setFont(u8g2_font_logisoso78_tn);

  u8g2_for_adafruit_gfx.setCursor(txt_x, txt_y); // Ajustar posición según sea necesario
  u8g2_for_adafruit_gfx.print(message);
  display.refresh();
}

// Función para mostrar el valor de current en el display
void displayMedidor(const char* message1,const char* message2)
{
  display.fillRect(0, 0, 320, 240, WHITE); // Llenar la pantalla de blanco
  u8g2_for_adafruit_gfx.setFont(u8g2_font_logisoso78_tn);
  u8g2_for_adafruit_gfx.setCursor(0, 100); // Ajustar posición según sea necesario
  u8g2_for_adafruit_gfx.print(message1);
  u8g2_for_adafruit_gfx.setCursor(0, 200); // Ajustar posición según sea necesario
  u8g2_for_adafruit_gfx.print(message2);
  display.refresh();
}

// Función para mostrar una imagen en el display
void displayImage(const uint8_t* bitmap)
{
  //display.clearDisplay();
  //u8g2_for_adafruit_gfx.drawBitmap(0, 0, 128, 64, bitmap);
  //display.refresh();
}
