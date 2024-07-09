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
#include <LittleFS.h>
#include "icons.h"

#define BLACK 0
#define WHITE 1
#define FILAS 4
#define COLUMNAS 4

// -------------------------------- pines
#define SHARP_SCK   2
#define SHARP_MOSI  3
#define SHARP_SS    4
#define LED_1       25
#define LED_2       5
uint8_t pinesFilas[FILAS] = {11, 12, 13, 14};
uint8_t pinesColumnas[COLUMNAS] = {18, 19, 20, 21};



int16_t x_lit = 450;
int16_t y_lit = 125;
int16_t x_pes = 450;
int16_t y_pes = 212;

uint16_t w;
uint16_t h;
uint16_t w2;
uint16_t h2;


char tecla;



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
bool flag_msg = false;
bool flag_med = false;
bool flag_time = true;
char txt[200];
char txt_med_1[100];
char txt_med_2[100];
char txt_med_u1[50];
char txt_med_u2[50];
char txt_hour[6] = "04:20";
char txt_day[12] = "05/Sep/1988";
char txt_folio[20] = "";
int img_pos_x = 0;
int img_pos_y = 240 - 32;
int light_state = 1;

char txt_name[100] = "valve.raw";
unsigned long lastTimeCheck = 0; // Variable para guardar el último tiempo de verificación
bool show_colon = true; // Variable para alternar el parpadeo

int txt_size = 0;
int txt_x = 0;
int txt_y = 15;
int pos_x, pos_y, back, front;
int size_x, size_y = 32;

char buffx[2000];
StaticJsonDocument<2000> doc;
//String jsonStr;
const char* aux_char;

// --------------------------------------------------------------------- SETUP TECLADO
void setup() {
  delay(2000);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  digitalWrite(LED_1, light_state);
  digitalWrite(LED_2, light_state);
  Serial.begin(115200);
  Serial.println("Init Display");

  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
    return;
  }
  Serial.println("LittleFS mounted successfully");
  teclado.setDebounceTime(10);

  setup1();
}

// --------------------------------------------------------------------- LOOP TECLADO Y DISPLAY
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
    get_cmd(input);
  }

  if (flag_med) {
    displayMedidor(txt_med_1, txt_med_2);
    flag_med = false;
  }

  if (flag_msg) {
    displayMessage(txt);
    flag_msg = false;
  }

  if (flag_time) {
    displayTime();
    flag_time = false;
  }
  if (flag_img) {
    displayImage(img_pos_x, img_pos_y, txt_name, 32, 32, WHITE, BLACK); // Llama a la imagen desde LittleFS
    flag_img = false;
  }
  // Llamar a displayTime cada segundo
  if (millis() - lastTimeCheck >= 1000) {
    displayTime();
    lastTimeCheck = millis();
  }
}

// ----------------------------------------------------------------- SETUP1 DISPLAY
void setup1() {
  delay(2000);
  Serial1.begin(115200); // Inicializar segundo puerto serie
  display.begin();
  display.clearDisplay();
  u8g2_for_adafruit_gfx.begin(display);

  u8g2_for_adafruit_gfx.setForegroundColor(BLACK); // apply Adafruit GFX color
  u8g2_for_adafruit_gfx.setBackgroundColor(WHITE); // apply Adafruit GFX color

  display.drawLine(0, 20, 320, 20, BLACK);  // Dibuja la línea horizontal
  display.drawLine(0, 204, 320, 204, BLACK);  // Dibuja la línea horizontal
  u8g2_for_adafruit_gfx.setFont(u8g2_font_profont22_tf);  // 14 pixels
  u8g2_for_adafruit_gfx.setCursor(0, 90); // start writing at this position
  u8g2_for_adafruit_gfx.print("MED1");

  display.refresh();

  digitalWrite(25, LOW);
}

// ----------------------------------------------------- LOOP1  SERIAL DISPLAY
void loop1() {
  // Leer y procesar datos del segundo puerto serie ENCODER
  String input = "";
  while (Serial1.available() > 0) {
    input = Serial1.readStringUntil('\n');
    Serial.println(input);
    get_cmd(input);
  }
}

// ----------------------------------------------------- MESSAGE
void displayMessage(const char* message) {
  u8g2_for_adafruit_gfx.setFont(u8g2_font_profont22_tf);  // 14 pixels

  uint16_t w = u8g2_for_adafruit_gfx.getUTF8Width(message);
  uint16_t h = u8g2_for_adafruit_gfx.getFontAscent() - u8g2_for_adafruit_gfx.getFontDescent();

  display.fillRect(txt_x, txt_y - h, w, h, WHITE);

  u8g2_for_adafruit_gfx.setCursor(txt_x, txt_y);
  u8g2_for_adafruit_gfx.print(message);
  display.refresh();
}

// ---------------------------------------------------- MEDIDOR
void displayMedidor(const char* message1, const char* message2) {
  int screenWidth = 340; // Ancho de la pantalla
  int cursorX1, cursorX2;

  display.fillRect(0, 22, screenWidth, 180, WHITE);
  u8g2_for_adafruit_gfx.setFont(u8g2_font_logisoso78_tn);

  int widthMessage1 = u8g2_for_adafruit_gfx.getUTF8Width(message1);
  int widthMessage2 = u8g2_for_adafruit_gfx.getUTF8Width(message2);

  cursorX1 = screenWidth - widthMessage1 - 35; // 5 es un margen arbitrario
  cursorX2 = screenWidth - widthMessage2 - 35; // 5 es un margen arbitrario

  u8g2_for_adafruit_gfx.setCursor(cursorX1, 78 + 30);
  u8g2_for_adafruit_gfx.print(message1);

  u8g2_for_adafruit_gfx.setCursor(cursorX2, (78 * 2) + 40);
  u8g2_for_adafruit_gfx.print(message2);

  display.refresh();
  u8g2_for_adafruit_gfx.setFont(u8g2_font_profont22_tf);  // 14 pixels
}

// ----------------------------------------------------- TIME
void displayTime() {
  u8g2_for_adafruit_gfx.setFont(u8g2_font_profont22_tf);  // 14 pixels

  uint16_t w = u8g2_for_adafruit_gfx.getUTF8Width(txt_hour);
  uint16_t h = u8g2_for_adafruit_gfx.getFontAscent() - u8g2_for_adafruit_gfx.getFontDescent();

  // Limpiar el área que ocupa txt_hour
  display.fillRect(txt_x, txt_y - h, w, h, WHITE);

  if (show_colon) {
    txt_hour[2] = ':'; // Mostrar dos puntos
  } else {
    txt_hour[2] = ' '; // Ocultar dos puntos
  }
  show_colon = !show_colon; // Alternar el estado del parpadeo

  // Dibujar txt_hour
  u8g2_for_adafruit_gfx.setCursor(txt_x, txt_y);  //0,15
  u8g2_for_adafruit_gfx.print(txt_hour);

  int txt_x2 = txt_x + w + 35; // 15 es un margen

  w = u8g2_for_adafruit_gfx.getUTF8Width(txt_day);

  // Limpiar el área que ocupa txt_day
  display.fillRect(txt_x2, txt_y - h, w, h + 4, WHITE);
  u8g2_for_adafruit_gfx.setCursor(txt_x2, txt_y);
  u8g2_for_adafruit_gfx.print(txt_day);

  int txt_x3 = txt_x2 + w + 75; // 15 es un margen

  w = u8g2_for_adafruit_gfx.getUTF8Width(txt_folio);

  // Limpiar el área que ocupa txt_folio
  display.fillRect(txt_x3, txt_y - h, w, h + 4, WHITE);
  u8g2_for_adafruit_gfx.setCursor(txt_x3, txt_y);
  u8g2_for_adafruit_gfx.print(txt_folio);

  display.refresh();
}

// ----------------------------------------------------- IMAGE
void displayImage(int pos_x, int pos_y, const char* filename, int size_x, int size_y, int back, int front) {
  File file = LittleFS.open(filename, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  // Assuming the image is a 32x32 bitmap in raw format
  uint8_t image[size_x * size_y / 8]; // 32x32 pixels, 1 bit per pixel
  file.read(image, sizeof(image));
  file.close();

  //dispdisplay.clearDisplay();
  display.drawBitmap(pos_x, pos_y, image, size_x, size_y, back, front);
  display.refresh();
  Serial.println(filename);
  Serial.println("IAMGEN DISPLAY");
}


// ------------------------------------------------ get_cmd
void get_cmd(String input)
{
  // Deserializar JSON
  doc.clear();
  DeserializationError error = deserializeJson(doc, input);
  if (!error) {
    const char* method = doc["method"];

    if (strcmp(method, "msg") == 0) {
      flag_msg = true;
      if (!doc["params"]["txt"].isNull()) {
        strncpy(txt, doc["params"]["txt"], sizeof(txt) - 1);
        txt[sizeof(txt) - 1] = '\0';
      }
      if (!doc["params"]["size"].isNull()) {
        txt_size = doc["params"]["size"];
      }
      if (!doc["params"]["txt_x"].isNull()) {
        txt_x = doc["params"]["txt_x"];
      }
      if (!doc["params"]["txt_y"].isNull()) {
        txt_y = doc["params"]["txt_y"];
      }
    } else if (strcmp(method, "med") == 0) {
      flag_med = true;

      JsonObject params = doc["params"];
      int count = 1;
      for (JsonPair kv : params) {
        if (count == 1) {
          strncpy(txt_med_1, kv.value().as<const char*>(), sizeof(txt_med_1) - 1);
          txt_med_1[sizeof(txt_med_1) - 1] = '\0';
          strncpy(txt_med_u1, kv.key().c_str(), sizeof(txt_med_u1) - 1);
          txt_med_u1[sizeof(txt_med_u1) - 1] = '\0';
        } else if (count == 2) {
          strncpy(txt_med_2, kv.value().as<const char*>(), sizeof(txt_med_2) - 1);
          txt_med_2[sizeof(txt_med_2) - 1] = '\0';
          strncpy(txt_med_u2, kv.key().c_str(), sizeof(txt_med_u2) - 1);
          txt_med_u2[sizeof(txt_med_u2) - 1] = '\0';
        }
        count++;
      }
    }

    if (strcmp(method, "time") == 0) {
      flag_time = true;
      if (!doc["params"]["hour"].isNull()) {
        strncpy(txt_hour, doc["params"]["hour"], sizeof(txt_hour) - 1);
        txt_hour[sizeof(txt_hour) - 1] = '\0'; // Asegurar que esté terminada en nulo
      }
      if (!doc["params"]["day"].isNull()) {
        strncpy(txt_day, doc["params"]["day"], sizeof(txt_day) - 1);
        txt_day[sizeof(txt_day) - 1] = '\0';
      }
      if (!doc["params"]["folio"].isNull()) {
        strncpy(txt_folio, doc["params"]["folio"], sizeof(txt_folio) - 1);
        txt_folio[sizeof(txt_folio) - 1] = '\0';
      }
    }
    if (strcmp(method, "img") == 0) {
      Serial.println("IAMGEN");
      flag_img = true;
      if (!doc["params"]["name"].isNull()) {
        strncpy(txt_name, doc["params"]["name"], sizeof(txt_name) - 1);
        txt_name[sizeof(txt_name) - 1] = '\0'; // Asegurar que esté terminada en nulo
      }
      if (!doc["params"]["pos_x"].isNull()) {
        img_pos_x = doc["params"]["pos_x"];
      }
      if (!doc["params"]["pos_y"].isNull()) {
        img_pos_y = doc["params"]["pos_y"];
      }
    }
    if (strcmp(method, "light") == 0) {
      if (!doc["params"]["state"].isNull()) {
        light_state = doc["params"]["state"];
        digitalWrite(LED_1, light_state);
        digitalWrite(LED_2, light_state);
      }
    }
  } else {
    Serial.println(error.c_str());
  }
}
