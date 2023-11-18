#define ENABLE_GxEPD2_GFX 0
#include "icons.h"
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/CodenameCoderFree4F_Bold40pt7b.h>
//#include "i2c_fifo.h"
//#include "i2c_slave.h"
#include "pico/stdlib.h"
//#include "hardware/i2c.h"
#include "GxEPD2_display_selection_new_style.h"
//#include <pico/multicore.h>
#include <UnixTime.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <cmath> // 

#define P_MOSI      3
#define P_MISO      4
#define P_SCK       2
#define MEASURE_TEMP0  0
#define  MEASURE_AUX  2
#define  MEASURE_TEMP1  4

#define  SETUP_COMMAND  11
#define   MEASURE_X  12
#define  MEASURE_Y  13
#define  MEASURE_Z1  14
#define  MEASURE_Z2  15

#define  POWERDOWN_IRQON  0
#define  ADON_IRQOFF  1
#define  ADOFF_IRQON  2

#define ADC_12BIT  0
#define  ADC_8BIT  1

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

const uint TSC2007_ADDR = 0x48; // Адрес TSC2007 на шине I2C

GxEPD2_BW<GxEPD2_750_YT7, 480> display(GxEPD2_750_YT7(/*CS=*/ 1, /*DC=*/ 5, /*RST=*/ 6, /*BUSY=*/ 7)); // GDEY075T7 800x480, UC8179 (GD7965)
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

//float uprice = 9.8; //price of 1 litre
volatile uint32_t print_litros, print_pesos, pesos;
volatile float litros;

boolean new_num = 0, printer = 0, valve = 0, OK = 0, DEL = 0, stopCommand = 0, mem_address_written = 0, ask_litro = 0, ask_peso = 0, ask_data = 0, ask_state = 0, ask_todo = 0, error_status = 0, newcommand = 0, new_litros = 0;
uint8_t mem_address = 0;
boolean dec = 0;
uint16_t tx, ty, tz1, tz2;
uint8_t STATE = 0;
volatile uint8_t todo_byte = 0, state_byte = 0, error_byte = 0, j = 0;
volatile uint8_t ltr_data[4], pes_data[4], uprice_data[2], litros_num[4], pesos_num[4], client_num[4], time_num[4];
volatile boolean newData = 0, touch_data = 0, shown = 0, endprocess = 0;
volatile uint32_t number = 0;
uint32_t unixtime, client;

uint16_t touchcommand(uint8_t func,
                      uint8_t pwr,
                      uint8_t res) {
  uint8_t cmd = (uint8_t)func << 4;
  cmd |= (uint8_t)pwr << 2;
  cmd |= (uint8_t)res << 1;

  uint8_t reply[2];

  i2c_write_blocking(i2c1, TSC2007_ADDR, &cmd, 1, true); // Запись регистра X
  i2c_read_blocking(i2c1, TSC2007_ADDR, reply, 2, false); // Чтение данных X0
  return ((uint16_t)reply[0] << 4) | (reply[1] >> 4); // 12 bits
}

bool read_touch(uint16_t *x, uint16_t *y, uint16_t *z1,
                uint16_t *z2) {
  *x = touchcommand(MEASURE_X, ADON_IRQOFF, ADC_12BIT);
  *y = touchcommand(MEASURE_Y, ADON_IRQOFF, ADC_12BIT);
  *z1 = touchcommand(MEASURE_Z1, ADON_IRQOFF, ADC_12BIT);
  *z2 = touchcommand(MEASURE_Z2, ADON_IRQOFF, ADC_12BIT);
  touchcommand(MEASURE_TEMP0, POWERDOWN_IRQON, ADC_12BIT);

  return (*x != 4095) && (*y != 4095);
}

// These are called in an **INTERRUPT CONTEXT** which means NO serial port
// access (i.e. Serial.print is illegal) and no memory allocations, etc.

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
  newcommand = true;
  jsonStr = buffx;
  DeserializationError error = deserializeJson(doc_aux, jsonStr);
  if (error) {
    //Serial.print(F("deserializeJson() failed: "));
    //Serial.println(error.f_str());
  }

  litros = doc_aux["litros"];
  print_litros = ceil(litros);
  pesos = doc["precio"].as<uint32_t>();
  print_pesos = pesos;
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

// ------------------------------------------------------------------------- readData
void readData() {
  int ch = -1;
  //boolean dec=0;
  read_touch(&tx, &ty, &tz1, &tz2);
  if (tx > 2620) {
    if (tx < 2970) {
      if (ty < 1916) {
        if (ty > 1425) ch = 5;
        else if (ty > 940) ch = 100;
        else if (ty < 760) DEL = 1;
      }
      else if (ty < 2400) ch = 4;
      else if (ty < 2880) ch = 3;
      else if (ty < 3330) ch = 2;
      else ch = 1;
    }
    else {
      if (ty < 1916) {
        if (ty > 1425) ch = 0;
        else if (ty < 760) OK = 1;
        else if (ty > 940) ch = 10;
      }
      else if (ty < 2400) ch = 9;
      else if (ty < 2880) ch = 8;
      else if (ty < 3330) ch = 7;
      else ch = 6;
    }
  }
  /*
    else if(tx>2120){
    if(ty>3400) printer=1;
    else if (ty>3000) valve=1;
    }
    else if(tx>1620){
    pesos = 1;
    litros = 0;
    }
    else if (tx >940){
    pesos = 0;
    litros = 1;
    }
  */
  if (ch >= 0) {
    new_num = 1;
    if (ch < 10) {
      number = (number * 10) + ch;
    }
    else if (ch == 100) {

      number = number / 10; // Убираем последнюю цифру
      //decimalPlace *= 10.0; // Восстанавливаем десятичное место
    }
    else if (DEL == 1) {
      number = 0;
      DEL = 0;
    }
  }
}

class PrintString : public Print, public String
{
  public:
    size_t write(uint8_t data) override
    {
      return concat(char(data));
    };
};

void showPartialUpdate()
{
  // some useful background
  //helloWorld();
  // use asymmetric values for test
  uint16_t box_x = 450;
  uint16_t box_y = 125;
  uint16_t box_w = 190;
  uint16_t box_h = 50;
  uint16_t cursor_y = box_y + box_h - 6;
  float value = 13.95;
  uint16_t incr = display.epd2.hasFastPartialUpdate ? 1 : 3;
  display.setFont(&CodenameCoderFree4F_Bold40pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setFullWindow();
  // show where the update box is
  display.setRotation(0);
  for (uint16_t i = 1; i <= 10; i += incr) {
    //digitalWrite(27,1);
    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    display.setCursor(box_x, cursor_y);
    display.print(value * i, 2);
    display.displayWindow(box_x, box_y, box_w, box_h);
    //digitalWrite(27,0);
    delay(500);
  }
  delay(1000);
  display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
  display.displayWindow(box_x, box_y, box_w, box_h);
  delay(1000);
}


void drawBitmaps()
{
  display.setFullWindow();
  //drawBitmaps200x200();
  showBigPic();

}
void showBigPic() {
  //bool mirror_y = (display.epd2.panel != GxEPD2::GDE0213B1);
  display.drawImage(Bitmap800x480_1, 0, 0, 800, 480, false, false, true);
  display.drawImage(nopaper, 30, 285, 64, 64, false, false, true);
  display.drawImage(printer_on, 95, 285, 64, 64, false, false, true);
  display.drawImage(wifi_on, 160, 285, 64, 64, false, false, true);
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
  error_status = true;

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
  display.epd2.selectSPI(SPI, SPISettings(4000000, MSBFIRST, SPI_MODE0));
  gpio_set_function(P_MISO, GPIO_FUNC_SPI);
  gpio_set_function(P_SCK, GPIO_FUNC_SPI);
  gpio_set_function(P_MOSI, GPIO_FUNC_SPI);

  display.init(0); // default 10ms reset pulse, e.g. for bare panels with DESPI-C02
  //delay(500);
  display.setFullWindow();


  display.firstPage();
  do
  {
    //display.fillScreen(GxEPD_WHITE);
    display.drawImage(Bitmap800x480_2, 0, 0, 800, 480, false, false, true);
  }
  while (display.nextPage());


  //display.powerOff();
  display.hibernate();
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
      //display.init(0);
      display.setFullWindow();
      //display.firstPage();
      //do
      {
        display.drawImage(Bitmap800x480_1, 0, 0, 800, 480, false, false, true);
      }
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
      break;

    // -------------------------------------------------------- display litros
    case 1:

      digitalWrite(27, LOW);
      digitalWrite(28, LOW);



      Serial.print("Litros: ");
      Serial.print(litros);

      Serial.print("\t");



      Serial.print("precio: ");
      Serial.println(pesos);

      new_litros = false;
      newcommand = false;


      //if (shown == 1)
      {
        digitalWrite(25, !digitalRead(25));
        if (litros > 0)
          digitalWrite(27, !digitalRead(27));

        //Show litros
        String litStr = String(print_litros);  // Convierte el número a String
        int16_t tbx, tby; uint16_t tbw, tbh;
        // Obtener las dimensiones del texto
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&CodenameCoderFree4F_Bold40pt7b);
        display.getTextBounds(litStr, 0, 0, &tbx, &tby, &tbw, &tbh);


        w = tbw + 10; // Un poco de margen
        h = tbh + 10;

        display.setPartialWindow(x_lit, y_lit, w, h);
        display.firstPage();


        do {
          display.setCursor(x_lit - tbx, y_lit - tby); // Ajustar la posición del cursor
          display.print(litStr);
        } while (display.nextPage());


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


        String litStr = String(print_litros);  // Convierte el número a String
        int16_t tbx, tby; uint16_t tbw, tbh;
        // Obtener las dimensiones del texto
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&CodenameCoderFree4F_Bold40pt7b);
        display.getTextBounds(litStr, 0, 0, &tbx, &tby, &tbw, &tbh);


        w = tbw + 10; // Un poco de margen
        h = tbh + 10;

        display.setPartialWindow(x_lit, y_lit, w, h);
        display.firstPage();


        do {
          display.setCursor(x_lit - tbx, y_lit - tby); // Ajustar la posición del cursor
          display.print(litStr);
        } while (display.nextPage());


        String pesosStr = String(pesos);  // Convierte el número a String
        display.getTextBounds(pesosStr, 0, 0, &tbx, &tby, &tbw, &tbh);
        w = tbw + 10; // Un poco de margen
        h = tbh + 10;

        display.setPartialWindow(x_pes, y_pes, w, h);
        display.firstPage();
        do {
          display.setCursor(x_pes - tbx, y_pes - tby); // Ajustar la posición del cursor
          display.print(pesosStr);
        } while (display.nextPage());


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
        display.firstPage();
        do {
          display.setFullWindow();
          display.drawImage(BitmapPrinter, 300, 140, 200, 200, false, false, true);
        } while (display.nextPage());

        display.powerOff();
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
    display.drawImage(wifi_on, 30, 285, 64, 64, false, false, true);
  }
  else
  {
    Serial.println("Wifi OFF");
    display.drawImage(wifi_off, 30, 285, 64, 64, false, false, true);
  }

  //if ((error_byte >> 6) & 0x01)       // ------------------ valve
  if (doc_aux["valve"].as<bool>() == true)
  {
    Serial.println("Valve On");
    display.drawImage(valve_on, 100, 285, 64, 64, false, false, true);
  }
  else
  {
    Serial.println("Valve OFF");
    display.drawImage(valve_off, 100, 285, 64, 64, false, false, true);
  }


  if (doc_aux["gps"].as<bool>() == true)    // ------------------ gps
    //if ((error_byte >> 3) & 0x01)
  {
    Serial.println("GPS On");
    display.drawImage(gps_on, 170, 285, 64, 64, false, false, true);
  }
  else
  {
    Serial.println("GPS OFF");
    display.drawImage(gps_off, 170, 285, 64, 64, false, false, true);
  }

  if (doc_aux["clock"].as<bool>() == true)    // ------------------ clock
    // if ((error_byte >> 2) & 0x01)
  {
    Serial.println("Clock On");
    display.drawImage(acc_on, 240, 285, 64, 64, false, false, true);
  }
  else
  {
    Serial.println("Clock OFF");
    display.drawImage(acc_off, 240, 285, 64, 64, false, false, true);
  }

  if (doc_aux["printer"].as<bool>() == true)    // ------------------ printer
    //if ((error_byte >> 5) & 0x01)
  {
    Serial.println("Printer OK");
    display.drawImage(printer_on, 320, 285, 64, 64, false, false, true);
  }
  else
  {
    Serial.println("Printer Offline");
    display.drawImage(printer_off, 320, 285, 64, 64, false, false, true);
  }

  if (doc_aux["paper"].as<bool>() == false)    // ------------------ paper
    //if ((error_byte >> 4) & 0x01)
  {
    Serial.println("NO PAPER");
    display.drawImage(nopaper, 390, 285, 64, 64, false, false, true);
  }
  else
  {
    Serial.println("Paper READY");
    //display.drawImage(nopaper, 390, 285, 64, 64, false, false, true);
  }
}
