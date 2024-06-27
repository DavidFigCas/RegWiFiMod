
#include "icons.h"
#include "pico/stdlib.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>
//#include <Fonts/FreeMonoBold24pt7b.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <UnixTime.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <cmath> // 
#include <Keypad.h>

#define SHARP_SCK  2
#define SHARP_MOSI 3
#define SHARP_SS   1

#define BLACK 0
#define WHITE 1

#define BUF_LEN         0x100
#define I2C_SLAVE_ADDRESS  0x5A

#define SDA_MAIN    16
#define SCL_MAIN    17
#define buttonPin   15

int16_t x_lit = 450;   //(display.width() - tbw) / 2;
int16_t y_lit = 125;  //(display.height() - tbh) / 2;
int16_t x_pes = 450;   //(display.width() - tbw) / 2;
int16_t y_pes = 212;  //(display.height() - tbh) / 2;


uint16_t w; // Un poco de margen
uint16_t h;
uint16_t w2; // Un poco de margen
uint16_t h2;

const uint8_t FILAS = 4; // Cuatro filas
const uint8_t COLUMNAS = 4; // Cuatro columnas
char tecla;

// Definir las conexiones de las filas y columnas
uint8_t pinesFilas[FILAS] = {11, 12, 13, 14}; // Ajustar estos pines según tu conexión
uint8_t pinesColumnas[COLUMNAS] = {18, 19, 20, 21}; // Ajustar estos pines según tu conexión

// Definir la disposición de caracteres del teclado
char teclas[FILAS][COLUMNAS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Crear la instancia del teclado
Keypad teclado = Keypad(makeKeymap(teclas), pinesFilas, pinesColumnas, FILAS, COLUMNAS);


Adafruit_SharpMem display(SHARP_SCK, SHARP_MOSI, SHARP_SS, 320, 240);
U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;
UnixTime stamp(0);

bool flag_print = true;

static char buffx[200];
static char respx[200];
StaticJsonDocument<200> doc;  // Asegúrate de que el tamaño sea suficiente para tu objeto JSON
StaticJsonDocument<200> doc_aux;  // Crea un documento JSON con espacio para 200
String jsonStr;
const char* aux_char;

const unsigned long intervalo = 1000;  // Intervalo de tiempo (1 minuto en milisegundos)
unsigned long tiempoAnterior = 0;
unsigned long tiempoActual;

const unsigned long intervalo2 = 250;  // Intervalo de tiempo (1 minuto en milisegundos)
unsigned long tiempoAnterior2 = 0;
unsigned long tiempoActual2;

volatile uint32_t litros, print_litros, print_pesos, pesos, litros_target;
//volatile float litros;

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


// Called when the I2C slave gets written to
// ---------------------------------------------------------------------------- recv
void recv(int len)
{
  int i;
  memset(buffx, 0, sizeof(buffx));
  // Just stuff the sent bytes into a global the main routine can pick up and use
  for (i = 0; i < len; i++)
  {
    buffx[i] = Wire.read();
  }
  //newcommand = true;
  jsonStr = buffx;
  DeserializationError error = deserializeJson(doc_aux, jsonStr);
  if (error) {
    //Serial.print(F("deserializeJson() failed: "));
    //Serial.println(error.f_str());
  }

  if (!doc_aux["litros"].isNull())
  {
    litros = doc_aux["litros"];

    //print_litros = ceil(litros);
    print_litros = litros;
    pesos = doc_aux["precio"].as<uint32_t>();
    print_pesos = pesos;
  }

  //if(!doc_aux["valve"].isNull())
  valve_state = doc_aux["valve"];


}

// Called when the I2C slave is read from
// ---------------------------------------------------------------------------- req
void req()
{
  doc.clear();
  //doc["precio"] = doc_aux["precio"];     //Commands
  doc["STATE"] = STATE;     //Commands

  if (open_valve) ////AQUIE ESYOY TRABAJANDO
  {
    doc["open"] = open_valve;
    open_valve = false;
  }
    
  if(close_valve)
  {
     doc["close"] = close_valve;
     close_valve = false;
  }
   

  if ((litros_target > 0))
    doc["litros_target"] = litros_target;

  if (print_report)
  {
    doc["print_report"] = print_report;
    print_report = false;
  }

  if (enable_ap)
  {
    doc["enable_ap"] = enable_ap;
    enable_ap = false;
  }


  //serializeJson(doc, respx);

  char temp[200];
  size_t len = serializeJson(doc, temp);
  memset(respx, 0, sizeof(respx));

  // Copiar solo los bytes útiles al buffer 'resp'
  memcpy(respx, temp, len);


  serializeJson(doc, respx);

  Wire.write(respx, len);
  //cadenaTeclas = "";

}



// --------------------------------------------------------------------- SETUP
void setup()
{


  Serial.begin(115200);
  delay(2000);
  Serial.println("Init Display");

  pinMode(28, OUTPUT);
  digitalWrite(28, 0);
  pinMode(27, OUTPUT);
  digitalWrite(27, 0);

  Wire.setClock(400000);
  Wire.setSDA(SDA_MAIN);
  Wire.setSCL(SCL_MAIN);
  Wire.begin(I2C_SLAVE_ADDRESS);
  Wire.onReceive(recv);
  Wire.onRequest(req);

  //delay(2000);
  Serial.println("I2C Ready");

  pinMode(buttonPin, INPUT_PULLUP);
  teclado.setDebounceTime(10);  // setDebounceTime(mS)

}


// --------------------------------------------------------------------- LOOP
void loop() {

  //Serial.println(STATE);
  //memset(respx, 0, sizeof(respx));

  tiempoActual = millis();

  if (tiempoActual - tiempoAnterior >= intervalo)
  {
    // Ha pasado 1 minuto
    tiempoAnterior = tiempoActual;

    //Serial.printf("Display Read Buffer: '%s'\r\n", buffx);
    //Serial.println();


    //Serial.println(jsonStr);

    //if (doc_conf["test"] == true)
    {
      Serial.print("master: ");
      serializeJson(doc_aux, Serial);
      Serial.println();
      Serial.print("DISPLAY: ");
      Serial.println(respx);
    }


    /*desconex_count++;

      if (desconex_count > 15) // More than one minute desconected from main board
      {
      desconex_count = 0;

      if (prevTime > doc_aux["time"])
      {
        doc_aux.clear();
      }
      prevTime = doc_aux["time"];
      }*/

  }



  tecla = teclado.getKey();
  if (tecla)
  {
    // Verifica si es un número
    if (isDigit(tecla) && cadenaNumeros.length() <= 10)
    {
      cadenaNumeros += tecla;
      //Serial.print("Cadena de Números acumulada: ");
      Serial.println(cadenaNumeros);
    }
    // Verifica si es una letra (A, B, C, D, *, #)
    else if (isLetter(tecla) && cadenaLetras.length() <= 10)
    {
      cadenaLetras = tecla;
      //Serial.print("Cadena de Letras acumulada: ");
      Serial.println(cadenaLetras);
      if (tecla == '*')
      {
        cadenaLetras = "";
        cadenaNumeros = "";
        litros_target = 0;
        close_valve = true;
      }

      if (tecla == '#')
      {
        cadenaLetras = "";
        litros_target = cadenaNumeros.toInt();;
        open_valve = true;
      }
      if (tecla == 'A')
      {
        print_report = true;
      }

      if (tecla == 'C')
      {
        enable_ap = true;
      }
    }

    // Si es un número, actualizar litros_target
    if (cadenaNumeros.toInt() > 0) {
      litros_target = cadenaNumeros.toInt();
      //Serial.print("Litros: ");
      Serial.println(litros_target);
    }
  }



}


// ----------------------------------------------------------------- SETUP1
void setup1()
{
  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);

  display.begin();
  display.clearDisplay();
  //delay(100);
  //display.setRotation(1);
  u8g2_for_adafruit_gfx.begin(display);

  //u8g2_for_adafruit_gfx.setForegroundColor(BLACK);      // apply Adafruit GFX color
  //u8g2_for_adafruit_gfx.setBackgroundColor(WHITE);      // apply Adafruit GFX color
  //u8g2_for_adafruit_gfx.setFont(u8g2_font_lubB19_tr);  // extended font
  //u8g2_for_adafruit_gfx.setCursor(5, 19 + 10);             // start writing at this position
  //u8g2_for_adafruit_gfx.print("Hola");

  display.refresh();
  //delay(2000);
  //display.clearDisplay();

  //u8g2_for_adafruit_gfx.setFont(u8g2_font_7x13_te);

  //u8g2_for_adafruit_gfx.setFontDirection(1);            // left to right (this is default)
  //u8g2_for_adafruit_gfx.setForegroundColor(BLACK);      // apply Adafruit GFX color

  //u8g2_for_adafruit_gfx.setFont(u8g2_font_siji_t_6x10);  // icon font
  //u8g2_for_adafruit_gfx.setFontMode(1);                 // use u8g2 transparent mode (this is default)
  //u8g2_for_adafruit_gfx.drawGlyph(0, 10, 0x0e200);  // Power Supply
  //u8g2_for_adafruit_gfx.drawGlyph(12, 10, 0x0e201);  // Charging
  //u8g2_for_adafruit_gfx.drawGlyph(24, 10, 0x0e10a);  // Right Arrow
  //u8g2_for_adafruit_gfx.drawGlyph(36, 10, 0x0e24b);  // full Battery

  //u8g2_for_adafruit_gfx.setFont(u8g2_font_7x13_te);  // extended font
  //u8g2_for_adafruit_gfx.setFontMode(1);                 // use u8g2 transparent mode (this is default)
  //u8g2_for_adafruit_gfx.setCursor(0, 40);               // start writing at this position
  //u8g2_for_adafruit_gfx.print("<Ȧǀʘ>");            // UTF-8 string: "<" 550 448 664 ">"
  //display.refresh();                                    // make everything visible
  //delay(2000);

  //display.setFont(&FreeMono24pt7b);

  digitalWrite(25, LOW);
  //display.fillScreen(WHITE);
}


// ----------------------------------------------------------------- LOOP1
void loop1()
{

  //Serial.println();

  print_icons();


  // ----------------------------------------------- Contadores
  // --------------------- Error no hay tarjeta principal
  if (doc_aux.isNull())
  {

    //display.fillRect(0, 22, 340, 180, WHITE);

    //u8g2_for_adafruit_gfx.setForegroundColor(BLACK);      // apply Adafruit GFX color
    //u8g2_for_adafruit_gfx.setBackgroundColor(WHITE);      // apply Adafruit GFX color
    //u8g2_for_adafruit_gfx.setFont(u8g2_font_logisoso92_tn );  // extended font
    //u8g2_for_adafruit_gfx.setFont(u8g2_font_logisoso38_tr);
    //u8g2_for_adafruit_gfx.setCursor(10, 78 + 30);             // start writing at this position
    //u8g2_for_adafruit_gfx.print("Error Tarjeta Principal");
  }

  // ----------------------- Despliega los contadores
  else
  {
    switch (STATE)
    {
      // -------------------------------------------------------- display icons
      case 0:

        
        digitalWrite(25, HIGH);

        display.fillRect(0, 22, 340, 180, WHITE);

        u8g2_for_adafruit_gfx.setForegroundColor(BLACK);      // apply Adafruit GFX color
        u8g2_for_adafruit_gfx.setBackgroundColor(WHITE);      // apply Adafruit GFX color
        //u8g2_for_adafruit_gfx.setFont(u8g2_font_logisoso92_tn );  // extended font
        u8g2_for_adafruit_gfx.setFont(u8g2_font_logisoso78_tn);
        u8g2_for_adafruit_gfx.setCursor(5, 78 + 30);             // start writing at this position

        if (litros_target < 10)
          u8g2_for_adafruit_gfx.print("     ");
        else if (litros_target < 100)
          u8g2_for_adafruit_gfx.print("    ");
        else if (litros_target < 1000)
          u8g2_for_adafruit_gfx.print("   ");
        else if (litros_target < 10000)
          u8g2_for_adafruit_gfx.print("  ");
        else if (litros_target < 100000)
          u8g2_for_adafruit_gfx.print(" ");
        u8g2_for_adafruit_gfx.print(litros_target);

        u8g2_for_adafruit_gfx.setCursor(5, (78 * 2) + 40);           // start writing at this position
        if (pesos < 10)
          u8g2_for_adafruit_gfx.print("     ");
        else if (pesos < 100)
          u8g2_for_adafruit_gfx.print("    ");
        else if (pesos < 1000)
          u8g2_for_adafruit_gfx.print("   ");
        else if (pesos < 10000)
          u8g2_for_adafruit_gfx.print("  ");
        else if (pesos < 100000)
          u8g2_for_adafruit_gfx.print(" ");

        u8g2_for_adafruit_gfx.print(pesos);

        if(litros > 0)
        {
          STATE = 1;
        }


        break;

      // -------------------------------------------------------- display litros
      case 1:

        //display.clearDisplay();


        digitalWrite(27, LOW);
        digitalWrite(28, LOW);


        print_litros = ceil(litros);
        print_pesos = pesos;
        litros_target = 0;
        //open_valve = false;

        /* Serial.print("Litros: ");
          Serial.print(litros);
          Serial.print("\t");
          Serial.print("Print_Litros: ");
          Serial.print(print_litros);
          Serial.print("\t");
          Serial.print("precio: ");
          Serial.print(pesos);
          Serial.print("\t");
          Serial.print("Print_Precio: ");
          Serial.println(pesos);*/

        //print_litros

        //new_litros = false;
        //newcommand = false;


        //if (shown == 1)
        {
          digitalWrite(25, !digitalRead(25));
          if (litros > 0)
            digitalWrite(27, !digitalRead(27));

          //Show litros
          String litStr = String(print_litros);  // Convierte el número a String
          int16_t tbx, tby; uint16_t tbw, tbh;
          // Obtener las dimensiones del texto
          //display.setTextColor(BLACK);
          //display.setFont(&CodenameCoderFree4F_Bold40pt7b);
          //display.getTextBounds(litStr, 0, 0, &tbx, &tby, &tbw, &tbh);


          w = tbw + 10; // Un poco de margen
          h = tbh + 10;

          //display.setPartialWindow(x_lit, y_lit, w, h);
          //display.firstPage();


          //do {
          //display.setCursor(x_lit - tbx, y_lit - tby); // Ajustar la posición del cursor
          //display.setTextSize(5);
          //display.setTextColor(WHITE);

          // Dibuja un cuadro en blanco
          int x = 10; // Posición X inicial del cuadro
          int y = 10; // Posición Y inicial del cuadro
          int width = 50; // Ancho del cuadro
          int height = 30; // Altura del cuadro

          //display.drawRect(x, y, width, height, BLACK); // Dibuja un rectángulo (cuadro) en blanco

          //display.refresh();

          //delay(100);

          //display.setTextSize(0);
          //display.setTextColor(BLACK);
          //display.setCursor(10, 100);
          //if (print_litros < 100)
          //  display.print(" ");
          //if (print_litros < 10)
          //  display.print(" ");

          display.fillRect(0, 22, 340, 180, WHITE);

          u8g2_for_adafruit_gfx.setForegroundColor(BLACK);      // apply Adafruit GFX color
          u8g2_for_adafruit_gfx.setBackgroundColor(WHITE);      // apply Adafruit GFX color
          //u8g2_for_adafruit_gfx.setFont(u8g2_font_logisoso92_tn );  // extended font
          u8g2_for_adafruit_gfx.setFont(u8g2_font_logisoso78_tn);
          u8g2_for_adafruit_gfx.setCursor(5, 78 + 30);             // start writing at this position

          if (litros < 10)
            u8g2_for_adafruit_gfx.print("     ");
          else if (litros < 100)
            u8g2_for_adafruit_gfx.print("    ");
          else if (litros < 1000)
            u8g2_for_adafruit_gfx.print("   ");
          else if (litros < 10000)
            u8g2_for_adafruit_gfx.print("  ");
          else if (litros < 100000)
            u8g2_for_adafruit_gfx.print(" ");
          u8g2_for_adafruit_gfx.print(litros);

          u8g2_for_adafruit_gfx.setCursor(5, (78 * 2) + 40);           // start writing at this position
          if (pesos < 10)
            u8g2_for_adafruit_gfx.print("     ");
          else if (pesos < 100)
            u8g2_for_adafruit_gfx.print("    ");
          else if (pesos < 1000)
            u8g2_for_adafruit_gfx.print("   ");
          else if (pesos < 10000)
            u8g2_for_adafruit_gfx.print("  ");
          else if (pesos < 100000)
            u8g2_for_adafruit_gfx.print(" ");

          u8g2_for_adafruit_gfx.print(pesos);

          //display.print(litros);
          //display.refresh();
          //delay(100);
          //} while (display.nextPage());


          //Show price
          //display.setTextColor(GxEPD_BLACK);
          //display.setFont(&CodenameCoderFree4F_Bold40pt7b);
          //display.fillRect(450, 212, 250, 50, GxEPD_WHITE);
          //display.setCursor(450, 256);
          //display.print(print_pesos/100);
          //display.displayWindow(450, 212, 250, 50);
          //shown = 0;
          //digitalWrite(28, 0);
        }

        shown = true;
        //print_litros = ceil(litros);

        break;

      // ---------------------------------------------------------- Final price
      case 2:
        digitalWrite(28, LOW);
        Serial.println("STATE 2");
        tiempoActual2 = millis();

        if (tiempoActual2 - tiempoAnterior2 >= intervalo2)
        {
          // Ha pasado 1 minuto
          tiempoAnterior2 = tiempoActual2;
          digitalWrite(27, !digitalRead(27));

        }



        //litros = ((uint32_t)litros_num[0] << 24) | ((uint32_t)litros_num[1] << 16) | ((uint32_t)litros_num[2] << 8) | litros_num[3];
        //pesos = ((uint32_t)pesos_num[0] << 24) | ((uint32_t)pesos_num[1] << 16) | ((uint32_t)pesos_num[2] << 8) | pesos_num[3];

        if (shown == true)
        {
          Serial.println("Final Numbers");
          //Serial.print("Litros: ");
          //Serial.print(litros);

          //Serial.print("\t");

          //print_litros = ceil(litros);
          Serial.print("Print_Litros: ");
          Serial.print(print_litros);

          Serial.print("\t");

          Serial.print("precio: ");
          Serial.println(print_pesos);


          String litStr = String(print_litros);  // Convierte el número a String
          int16_t tbx, tby; uint16_t tbw, tbh;
          // Obtener las dimensiones del texto
          display.setTextColor(BLACK);
          // display.setFont(&CodenameCoderFree4F_Bold40pt7b);
          display.getTextBounds(litStr, 0, 0, &tbx, &tby, &tbw, &tbh);


          w = tbw + 10; // Un poco de margen
          h = tbh + 10;

          //display.setPartialWindow(x_lit, y_lit, w, h);
          //display.firstPage();


          //do {
          //display.setCursor(x_lit - tbx, y_lit - tby); // Ajustar la posición del cursor
          //display.print(litStr);
          //} while (display.nextPage());


          String pesosStr = String(print_pesos);  // Convierte el número a String
          display.getTextBounds(pesosStr, 0, 0, &tbx, &tby, &tbw, &tbh);

          w2 = tbw + 10; // Un poco de margen
          h2 = tbh + 10;

          //display.setPartialWindow(x_pes, y_pes, w2, h2);
          //display.firstPage();
          //do {
          //display.setCursor(x_pes - tbx, y_pes - tby); // Ajustar la posición del cursor
          //display.print(pesosStr);
          //} while (display.nextPage());


          /*display.setTextColor(GxEPD_BLACK);
            display.setFont(&CodenameCoderFree4F_Bold40pt7b);
            display.fillRect(450, 125, 250, 50, GxEPD_WHITE);
            display.setCursor(450, 169);
            display.print(print_litros);
            display.displayWindow(450, 125, 250, 50);

            //Show price
            display.setTextColor(GxEPD_BLACK);
            display.setFont(&CodenameCoderFree4F_Bold40pt7b);
            display.fillRect(450, 212, 250, 50, GxEPD_WHITE);
            display.setCursor(450, 256);
            display.print(print_pesos);
            display.displayWindow(450, 212, 250, 50);*/
          Serial.println("Show price");
          //delay(5000);
          //Serial.println("goto STATE 3");
          //STATE = 3;
          shown = false;
        }
        flag_print = true;
        //delay(10);
        break;

      // ---------------------------------------------------------- Bing Printer
      case 3:

        //digitalWrite(28, HIGH);
        Serial.println("STATE 3");
        digitalWrite(27, LOW);
        digitalWrite(25, LOW);
        tiempoActual2 = millis();

        if (tiempoActual2 - tiempoAnterior2 >= intervalo2)
        {
          // Ha pasado 1 minuto
          tiempoAnterior2 = tiempoActual2;
          digitalWrite(28, !digitalRead(28));

        }

        if (flag_print == true)
        {
          flag_print = false;
          //display.setFullWindow();  // Establece el área de dibujo para toda la pantalla
          //display.firstPage();
          //do {
          //  display.fillScreen(GxEPD_WHITE);  // Llena la pantalla de blanco (borra todo)
          //} while (display.nextPage());

          //display.setFullWindow();  // Establece el área de dibujo para toda la pantalla

          //display.firstPage();
          //do {
          //display.setFullWindow();
          //display.drawImage(BitmapPrinter, 300, 140, 200, 200, false, false, true);
          //} while (display.nextPage());

          //display.powerOff();
        }


        //STATE = 0;
        //Serial.println("goto STATE 0");
        //delay(10);
        break;
      default:
        break;
    }
  }
  display.refresh();

  // ------------------------------------------- take STATE from master
  if (!doc_aux["STATE"].isNull())
  {
    STATE = doc_aux["STATE"];
  }

}

// ---------------------------------------------------------------- print icons
void print_icons()
{
  // ------------------------------------------------------ menu bar

  //display.fillScreen(WHITE);



  display.drawLine(0, 20, 320, 20, BLACK);  // Dibuja la línea horizontal
  //display.drawLine(65, 20, 65, 0, BLACK);  // Dibuja la línea horizontal
  display.drawLine(0, 204, 320, 204, BLACK);  // Dibuja la línea horizontal
  //display.drawLine(195, 20, 195, 0, BLACK);  // Dibuja la línea horizontal

  unixtime = doc_aux["time"].as<uint32_t>();
  //serializeJson(doc_aux["time"],Serial);
  stamp.getDateTime(unixtime);


  u8g2_for_adafruit_gfx.setForegroundColor(BLACK);      // apply Adafruit GFX color
  u8g2_for_adafruit_gfx.setBackgroundColor(WHITE);      // apply Adafruit GFX color
  //u8g2_for_adafruit_gfx.setFont(u8g2_font_ncenB12_tr);  // extended font
  u8g2_for_adafruit_gfx.setFont(u8g2_font_profont22_tf);  // 14 pixels




  // -------------------------------------------------- NOT CLOCK
  if (stamp.year < 2000)
  {

    /*

        // ----------- hora
        u8g2_for_adafruit_gfx.setCursor(0, 15);             // start writing at this position
        u8g2_for_adafruit_gfx.print(" 0");

        if ((millis() / 1000) % 2 == 0)
          u8g2_for_adafruit_gfx.print(":");
        else
          u8g2_for_adafruit_gfx.print(" ");
        u8g2_for_adafruit_gfx.print("00");

        // ------------ fecha
        u8g2_for_adafruit_gfx.setCursor(90, 15);             // start writing at this position
        u8g2_for_adafruit_gfx.print("00");
        u8g2_for_adafruit_gfx.print("/");
        u8g2_for_adafruit_gfx.print("00");
        u8g2_for_adafruit_gfx.print("/");
        u8g2_for_adafruit_gfx.print("0000");

        u8g2_for_adafruit_gfx.setCursor(235, 15);             // start writing at this position
        u8g2_for_adafruit_gfx.print("0000000");*/
  }

  // --------------------------------------------- Display time/date
  else
  {

    display.fillRect(0, 0, 320, 20, WHITE);
    u8g2_for_adafruit_gfx.setCursor(0, 15);             // start writing at this position

    if (stamp.hour < 10)
      u8g2_for_adafruit_gfx.print(" ");
    u8g2_for_adafruit_gfx.print(stamp.hour);

    if ((millis() / 1000) % 2 == 0)
      u8g2_for_adafruit_gfx.print(":");
    else
      u8g2_for_adafruit_gfx.print(" ");

    if (stamp.minute < 10)
      u8g2_for_adafruit_gfx.print("0");
    u8g2_for_adafruit_gfx.print(stamp.minute);


    u8g2_for_adafruit_gfx.setCursor(90, 15);             // start writing at this positiont
    if (stamp.day < 10)
      u8g2_for_adafruit_gfx.print(" ");
    u8g2_for_adafruit_gfx.print(stamp.day);
    u8g2_for_adafruit_gfx.print("/");

    if (stamp.month < 10)
      u8g2_for_adafruit_gfx.print("0");
    u8g2_for_adafruit_gfx.print(stamp.month);
    u8g2_for_adafruit_gfx.print("/");
    u8g2_for_adafruit_gfx.print(stamp.year);


    u8g2_for_adafruit_gfx.setCursor(235, 15);             // start writing at this position
    if (doc_aux["folio"] < 10)
    {
      u8g2_for_adafruit_gfx.print("      ");
    }
    else if (doc_aux["folio"] < 100)
    {
      u8g2_for_adafruit_gfx.print("     ");
    }
    else if (doc_aux["folio"] < 1000)
    {
      u8g2_for_adafruit_gfx.print("    ");
    }
    else if (doc_aux["folio"] < 10000)
    {
      u8g2_for_adafruit_gfx.print("   ");
    }
    else if (doc_aux["folio"] < 100000)
    {
      u8g2_for_adafruit_gfx.print("  ");
    }
    else if (doc_aux["folio"] < 1000000)
    {
      u8g2_for_adafruit_gfx.print(" ");
    }
    u8g2_for_adafruit_gfx.print(doc_aux["folio"].as<String>());



  }

  // ------------------------------------------ Barra de iconos

  if (doc_aux["wifi"] == true)
  {
    display.drawBitmap(0, 240 - 32, wifi_on_small, 32, 32, WHITE, BLACK);
  }
  else
  {
    display.drawBitmap(0, 240 - 32, wifi_off_small, 32, 32, WHITE, BLACK);
  }

  if (doc_aux["sd"] == true)
  {
    display.drawBitmap(32 + 5, 240 - 32, sd_on_small, 32, 32, WHITE, BLACK);
  }
  else
  {
    display.drawBitmap(32 + 5, 240 - 32, sd_off_small, 32, 32, WHITE, BLACK);
  }

  if (doc_aux["valve"] == true)
  {
    if ((millis() / 1000) % 2 == 0)
      display.drawBitmap(32 * 2 + 5, 240 - 32, valve_on_small, 32, 32, WHITE, BLACK);
    else
      //display.fillRect(32 * 2 + 5, 240 - 32, 32, 32, BLACK); energy_small
      display.drawBitmap(32 * 2 + 5, 240 - 32, energy_small, 32, 32, WHITE, BLACK);

  }
  else
  {
    display.drawBitmap(32 * 2 + 5, 240 - 32, valve_off_small, 32, 32, WHITE, BLACK);
  }

  if (STATE >= 2)
  {
    if ((millis() / 1000) % 2 == 0)
      display.drawBitmap(32 * 3 + 5, 240 - 32, printer_on_small, 32, 32, WHITE, BLACK);
    else
      display.fillRect(32 * 3 + 5, 240 - 32, 32, 32, BLACK);
  }
  else
  {
    display.drawBitmap(32 * 3 + 5, 240 - 32, printer_off_small, 32, 32, WHITE, BLACK);
  }

  if (doc_aux["gps"] == true)
  {
    display.drawBitmap(32 * 4 + 5, 240 - 32, gps_on_small, 32, 32, WHITE, BLACK);
  }
  else
  {
    display.drawBitmap(32 * 4 + 5, 240 - 32, gps_off_small, 32, 32, WHITE, BLACK);
  }

  if (doc_aux["bt"] == true)
  {
    if ((millis() / 1000) % 2 == 0)
      display.drawBitmap(32 * 5 + 5, 240 - 32, I2, 32, 32, WHITE, BLACK);
    else
      display.fillRect(32 * 5 + 5, 240 - 32, 32, 32, BLACK);
  }
  else
  {
    display.drawBitmap(32 * 5 + 5, 240 - 32, I1, 32, 32, WHITE, BLACK);
  }


  //display.refresh();
}


// Función para verificar si el carácter es una letra específica (A, B, C, D, *, #)
bool isLetter(char c) {
  return c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == '*' || c == '#';
}
