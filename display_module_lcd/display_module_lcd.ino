
#include "icons.h"
#include "pico/stdlib.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>
#include <UnixTime.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <cmath> // 

#define SHARP_SCK  2
#define SHARP_MOSI 3
#define SHARP_SS   1

#define BLACK 0
#define WHITE 1




#define BUF_LEN         0x100
#define I2C_SLAVE_ADDRESS  0x5A

#define SDA_MAIN    16
#define SCL_MAIN    17

int16_t x_lit = 450;   //(display.width() - tbw) / 2;
int16_t y_lit = 125;  //(display.height() - tbh) / 2;
int16_t x_pes = 450;   //(display.width() - tbw) / 2;
int16_t y_pes = 212;  //(display.height() - tbh) / 2;


uint16_t w; // Un poco de margen
uint16_t h;
uint16_t w2; // Un poco de margen
uint16_t h2;


Adafruit_SharpMem display(SHARP_SCK, SHARP_MOSI, SHARP_SS, 320, 240);
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

volatile uint32_t print_litros, print_pesos, pesos;
volatile float litros;

uint8_t STATE = 0;

volatile boolean  shown = 0;
volatile uint32_t number = 0;
uint32_t unixtime, client;



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

  litros = doc_aux["litros"];
  print_litros = doc_aux["litros_check"];
  pesos = doc["precio"].as<uint32_t>();
  print_pesos = doc_aux["precio_check"];
}

// Called when the I2C slave is read from
// ---------------------------------------------------------------------------- req
void req()
{
  doc["precio"] = doc_aux["precio"];     //Commands
  doc["STATE"] = STATE;     //Commands
  doc["litros"] = litros;
  serializeJson(doc, respx);
  Wire.write(respx, 199);
}



class PrintString : public Print, public String
{
  public:
    size_t write(uint8_t data) override
    {
      return concat(char(data));
    };
};




void drawBitmaps()
{
  //display.setFullWindow();
  //drawBitmaps200x200();
  showBigPic();

}
void showBigPic() {
  //bool mirror_y = (display.epd2.panel != GxEPD2::GDE0213B1);
  //display.drawImage(Bitmap800x480_1, 0, 0, 800, 480, false, false, true);
  //display.drawImage(nopaper, 30, 285, 64, 64, false, false, true);
  //display.drawImage(printer_on, 95, 285, 64, 64, false, false, true);
  //display.drawImage(wifi_on, 160, 285, 64, 64, false, false, true);
}


struct bitmap_pair
{
  const unsigned char* black;
  const unsigned char* red;
};



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


  Wire.setSDA(SDA_MAIN);
  Wire.setSCL(SCL_MAIN);
  Wire.begin(I2C_SLAVE_ADDRESS);
  Wire.onReceive(recv);
  Wire.onRequest(req);

  //delay(2000);
  Serial.println("I2C Ready");

  //Serial.println("setup done");

  //STATE = 1;
  //error_status = true;

  //doc["name"] = "David";
  //doc["client"] = 30;
  //doc["city"] = "Puebla";

  // Serializar el objeto JSON en la variable resp
  //serializeJson(doc, respx);

  //multicore_launch_core1(core1_blink);
  //Serial.begin(115200);
  //pinMode(27, OUTPUT);




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

    Serial.print("aux: ");
    serializeJson(doc_aux, Serial);
    Serial.println();
    Serial.print("resp: ");
    Serial.println(respx);



    // Ahora resp contiene el objeto JSON como una cadena
    // Salida: {"name":"John","age":30,"city":"New York"}
  }


  //delay(1000);


}


// ----------------------------------------------------------------- SETUP1
void setup1()
{
  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);

  display.begin();
  display.clearDisplay();

  digitalWrite(25, LOW);
}


// ----------------------------------------------------------------- LOOP1
void loop1()
{


  switch (STATE)
  {
    // -------------------------------------------------------- display icons
    case 0:
      digitalWrite(25, HIGH);
      //if (flag_print == true)
      //{
      Serial.println("Display Main Screen");
      //for (int i = 0; i < 4; i++)
      {
        display.setRotation(1);
        display.clearDisplay();
        // text display tests
        display.setTextSize(1);
        display.setTextColor(BLACK);
        display.setCursor(0, 0);
        display.println("Hello, world!");
        display.setTextColor(WHITE, BLACK); // inverted text
        display.println(3.141592);
        display.setTextSize(2);
        display.setTextColor(BLACK);
        display.print("0x");
        display.print(0xDEADBEEF, HEX);
        // Screen must be refreshed at least once per second
        //for (int j = 0; j < 4; j++)
        {
          display.refresh();
          delay(500); // 1/2 sec delay
        } // x4 = 2 second pause between rotations
      }

      //display.init(0);

      //display.setFullWindow();
      //display.firstPage();
      //do
      //{
      //display.drawImage(Bitmap800x480_1, 0, 0, 800, 480, false, false, true);
      //}
      //while (display.nextPage());

      //print_icons();


      //unixtime = ((uint32_t)time_num[0] << 24) | ((uint32_t)time_num[1] << 16) | ((uint32_t)time_num[2] << 8) | time_num[3];
      unixtime = doc_aux["time"].as<uint32_t>();
      //serializeJson(doc_aux["time"],Serial);
      stamp.getDateTime(unixtime);

      /*Serial.print("TIME");
        Serial.println(unixtime);
        Serial.println(stamp.day);
        Serial.println(stamp.month);
        Serial.println(stamp.year);
        Serial.println(stamp.hour);
        Serial.println(stamp.minute);

        display.setTextColor(GxEPD_BLACK);
        display.fillRect(237, 10, 490, 45, GxEPD_WHITE);
        display.setCursor(237, 49);
        display.setFont(&FreeMonoBold9pt7b);
        display.print(stamp.day);
        display.print("/");
        display.print(stamp.month);
        display.print("/");
        display.print(stamp.year);
        display.print("  ");
        display.print(stamp.hour);
        display.print(":");
        display.print(stamp.minute);
        display.displayWindow(237, 10, 490, 45);
        display.powerOff();*/


      Serial.println("goto STATE 1");

      //delay(10000);
      //flag_print = false;
      //}
      //touch_data=0;
      STATE = 1;
      display.clearDisplay();
      break;

    // -------------------------------------------------------- display litros
    case 1:

      //display.clearDisplay();

      digitalWrite(27, LOW);
      digitalWrite(28, LOW);



      Serial.print("Litros: ");
      Serial.print(litros);

      Serial.print("\t");

      print_litros = ceil(litros);
      print_pesos = pesos;

      Serial.print("Print_Litros: ");
      Serial.print(print_litros);

      Serial.print("\t");

      Serial.print("precio: ");
      Serial.print(pesos);

      Serial.print("\t");

      Serial.print("Print_Precio: ");
      Serial.println(pesos);

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
        display.setTextColor(BLACK);
        //display.setFont(&CodenameCoderFree4F_Bold40pt7b);
        //display.getTextBounds(litStr, 0, 0, &tbx, &tby, &tbw, &tbh);


        w = tbw + 10; // Un poco de margen
        h = tbh + 10;

        //display.setPartialWindow(x_lit, y_lit, w, h);
        //display.firstPage();


        //do {
        //display.setCursor(x_lit - tbx, y_lit - tby); // Ajustar la posición del cursor
        display.setTextSize(5);
        display.setTextColor(WHITE);
        display.setCursor(10, 20);
        // Dibuja un cuadro en blanco
        int x = 10; // Posición X inicial del cuadro
        int y = 10; // Posición Y inicial del cuadro
        int width = 50; // Ancho del cuadro
        int height = 30; // Altura del cuadro

        display.drawRect(x, y, width, height, BLACK); // Dibuja un rectángulo (cuadro) en blanco

        display.refresh();

        delay(100);

        display.setTextColor(BLACK);
        display.setCursor(10, 20);
        //if (print_litros < 100)
        //  display.print(" ");
        //if (print_litros < 10)
        //  display.print(" ");


        display.print(litros);
        display.refresh();
        delay(100);
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
      delay(10);
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
      delay(10);
      break;
    default:
      break;
  }

  // ------------------------------------------- take STATE from master
  if (!doc_aux["STATE"].isNull())
  {
    STATE = doc_aux["STATE"];
  }

}


void print_icons()
{
  //if ((error_byte >> 7) & 0x01)
  if (doc_aux["wifi"].as<bool>() == true) // ------------------ Wifi
  {
    Serial.println("Wifi On");
    //display.drawImage(wifi_on, 30, 285, 64, 64, false, false, true);
  }
  else
  {
    Serial.println("Wifi OFF");
    //display.drawImage(wifi_off, 30, 285, 64, 64, false, false, true);
  }

  //if ((error_byte >> 6) & 0x01)       // ------------------ valve
  if (doc_aux["valve"].as<bool>() == true)
  {
    Serial.println("Valve On");
    //display.drawImage(valve_on, 100, 285, 64, 64, false, false, true);
  }
  else
  {
    Serial.println("Valve OFF");
    //display.drawImage(valve_off, 100, 285, 64, 64, false, false, true);
  }


  if (doc_aux["gps"].as<bool>() == true)    // ------------------ gps
    //if ((error_byte >> 3) & 0x01)
  {
    Serial.println("GPS On");
    //display.drawImage(gps_on, 170, 285, 64, 64, false, false, true);
  }
  else
  {
    Serial.println("GPS OFF");
    //display.drawImage(gps_off, 170, 285, 64, 64, false, false, true);
  }

  if (doc_aux["clock"].as<bool>() == true)    // ------------------ clock
    // if ((error_byte >> 2) & 0x01)
  {
    Serial.println("Clock On");
    //display.drawImage(acc_on, 240, 285, 64, 64, false, false, true);
  }
  else
  {
    Serial.println("Clock OFF");
    //display.drawImage(acc_off, 240, 285, 64, 64, false, false, true);
  }

  if (doc_aux["printer"].as<bool>() == true)    // ------------------ printer
    //if ((error_byte >> 5) & 0x01)
  {
    Serial.println("Printer OK");
    //display.drawImage(printer_on, 320, 285, 64, 64, false, false, true);
  }
  else
  {
    Serial.println("Printer Offline");
    //display.drawImage(printer_off, 320, 285, 64, 64, false, false, true);
  }

  if (doc_aux["paper"].as<bool>() == false)    // ------------------ paper
    //if ((error_byte >> 4) & 0x01)
  {
    Serial.println("NO PAPER");
    //display.drawImage(nopaper, 390, 285, 64, 64, false, false, true);
  }
  else
  {
    Serial.println("Paper READY");
    //display.drawImage(nopaper, 390, 285, 64, 64, false, false, true);
  }
}
