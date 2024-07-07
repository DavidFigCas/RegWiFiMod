#include "hardware/watchdog.h"
#include "pico/stdlib.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <UnixTime.h>
#include <ArduinoJson.h>
#include <cmath>
#include <Keypad.h>

#define SHARP_SCK  2
#define SHARP_MOSI 3
#define SHARP_SS   1
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

bool flag_print = true;

static char buffx[200];
StaticJsonDocument<200> doc;
StaticJsonDocument<200> doc_aux;
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

// Imagen en formato de matriz de bits
const uint8_t myBitmap[] PROGMEM = {
  // Aquí va la matriz de bits de tu imagen
  // Ejemplo de 128x64 píxeles (1024 bytes)
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  // ...continúa con los datos de la imagen
};

// --------------------------------------------------------------------- SETUP
void setup()
{
  delay(2000);
  Serial.begin(115200);
  Serial1.begin(115200); // Inicializar segundo puerto serie

  Serial.println("Init Display");

  pinMode(28, OUTPUT);
  digitalWrite(28, 0);
  pinMode(27, OUTPUT);
  digitalWrite(27, 0);
  teclado.setDebounceTime(10);

  setup1();

  // Mostrar la imagen al inicio
  displayImage(myBitmap);
}

// --------------------------------------------------------------------- LOOP
void loop() {
  tecla = teclado.getKey();
  if (tecla) {
    //Serial.print("Tecla presionada: ");
    //Serial.println(tecla);
    //Serial1.print("Tecla presionada: ");
    //Serial1.println(tecla);

    // Crear JSON con la tecla presionada
    StaticJsonDocument<200> jsonDoc;
    //jsonDoc["jsonrpc"] = "2.0";
    jsonDoc["method"] = "key_press";
    jsonDoc["params"]["key"] = String(tecla);
    String jsonStr;
    serializeJson(jsonDoc, jsonStr);
    Serial.println(jsonStr);
    Serial1.println(jsonStr);
  }

  // Leer y procesar datos del puerto serie principal
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    //Serial.print("Mensaje recibido: ");
    //Serial.println(input);
    //Serial1.print("Mensaje recibido: ");
    //Serial1.println(input);

    // Deserializar JSON
    DeserializationError error = deserializeJson(doc, input);
    if (!error) {
      const char* method = doc["method"];
      if (strcmp(method, "print_message") == 0) {
        const char* message = doc["params"]["msg.payload"];
        displayMessage(message);
      }
    } else {
      Serial.print("Error de parseo JSON: ");
      Serial.println(error.c_str());
      Serial1.print("Error de parseo JSON: ");
      Serial1.println(error.c_str());
    }
  }

  // Leer y procesar datos del segundo puerto serie
  if (Serial1.available() > 0) {
    String input = Serial1.readStringUntil('\n');
    Serial.print("Mensaje recibido en Serial1: ");
    Serial.println(input);

    // Deserializar JSON
    DeserializationError error = deserializeJson(doc, input);
    if (!error) {
      const char* method = doc["method"];
      if (strcmp(method, "print_message") == 0) {
        const char* message = doc["params"]["msg.payload"];
        displayMessage(message);
      }
    } else {
      Serial.print("Error de parseo JSON: ");
      Serial.println(error.c_str());
    }
  }
}

// ----------------------------------------------------------------- SETUP1
void setup1()
{
  delay(2000);
  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);

  display.begin();
  display.clearDisplay();
  u8g2_for_adafruit_gfx.begin(display);

  u8g2_for_adafruit_gfx.setForegroundColor(BLACK); // apply Adafruit GFX color
  u8g2_for_adafruit_gfx.setBackgroundColor(WHITE); // apply Adafruit GFX color
  u8g2_for_adafruit_gfx.setFont(u8g2_font_lubB19_tr); // extended font
  u8g2_for_adafruit_gfx.setCursor(5, 19 + 10); // start writing at this position
  u8g2_for_adafruit_gfx.print("Hola");

  display.refresh();

  digitalWrite(25, LOW);
}

// Función para mostrar un mensaje en el display
void displayMessage(const char* message)
{
  display.fillRect(0, 0, 320, 240, WHITE); // Llenar la pantalla de blanco
  u8g2_for_adafruit_gfx.setCursor(0, 30); // Ajustar posición según sea necesario
  u8g2_for_adafruit_gfx.print(message);
  display.refresh();
}

// Función para mostrar una imagen en el display
void displayImage(const uint8_t* bitmap)
{
  //display.clearDisplay();
  //u8g2_for_adafruit_gfx.drawBitmap(0, 0, 128, 64, bitmap);
  //display.refresh();
}
