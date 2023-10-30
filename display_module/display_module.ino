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

const uint TSC2007_ADDR = 0x48; // Адрес TSC2007 на шине I2C

GxEPD2_BW<GxEPD2_750_YT7, 480> display(GxEPD2_750_YT7(/*CS=*/ 1, /*DC=*/ 5, /*RST=*/ 6, /*BUSY=*/ 7)); // GDEY075T7 800x480, UC8179 (GD7965)
UnixTime stamp(-6);

float uprice = 9.8; //price of 1 litr

static char buffx[200];
static char respx[200];
StaticJsonDocument<200> doc;  // Asegúrate de que el tamaño sea suficiente para tu objeto JSON
StaticJsonDocument<200> doc_aux;  // Crea un documento JSON con espacio para 200
String jsonStr;
const char* aux_char;

//float number=0, l=0, p=0;
volatile uint32_t litros, pesos, print_litros, print_pesos;

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
  
}

// Called when the I2C slave is read from
// ---------------------------------------------------------------------------- req
void req()
{
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
  delay(5000);
  Serial.println("Init Display");
  pinMode(25, OUTPUT);
  digitalWrite(25, 0);
  pinMode(28, OUTPUT);
  digitalWrite(28, 0);
  pinMode(27, OUTPUT);
  digitalWrite(27, 0);


  Wire.setSDA(SDA_MAIN);
  Wire.setSCL(SCL_MAIN);
  Wire.begin(I2C_SLAVE_ADDRESS);
  Wire.onReceive(recv);
  Wire.onRequest(req);

  delay(1000);


  //Serial.println("setup done");

  //STATE = 1;
  error_status = true;

  doc["name"] = "John";
  doc["age"] = 30;
  doc["city"] = "New York";

  // Serializar el objeto JSON en la variable resp
  serializeJson(doc, respx);

  //multicore_launch_core1(core1_blink);
  //Serial.begin(115200);
  //pinMode(27, OUTPUT);




}


// --------------------------------------------------------------------- LOOP
void loop() {

  //Serial.println(STATE);
  //memset(respx, 0, sizeof(respx));


  Serial.printf("Slave Buffer: '%s'\r\n", buffx);

  //aux_char = jsonStr.c_str();  // Obtén una representación const char* de la cadena
  //Serial.println(aux_char);  // Imprime la cadena JSON

  //deserializeJson(doc_aux, jsonStr);  // (FUNCIONA)Serializa el documento JSON a una cadena
  //jsonStr = String(buffx);

  jsonStr = buffx;
  Serial.println(jsonStr);
  //deserializeJson(doc_aux, jsonStr);  // Serializa el documento JSON a una cadena

  doc["precio"] = doc_aux["precio"];     //Commands
  doc["STATE"] = STATE;     //Commands
  //doc["valve"] = doc_aux["valve"];     //Commands

  DeserializationError error = deserializeJson(doc_aux, jsonStr);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    //return;
  }

  Serial.print("aux: ");
  serializeJson(doc_aux, Serial); Serial.println();
  Serial.print("resp: ");
  serializeJson(doc, respx);Serial.println();
  Serial.println(respx);  // Salida: {"name":"John","age":30,"city":"New York"}


  // Ahora resp contiene el objeto JSON como una cadena
  // Salida: {"name":"John","age":30,"city":"New York"}

  delay(1000);


}


// ----------------------------------------------------------------- SETUP1
void setup1()
{

  display.epd2.selectSPI(SPI, SPISettings(4000000, MSBFIRST, SPI_MODE0));
  gpio_set_function(P_MISO, GPIO_FUNC_SPI);
  gpio_set_function(P_SCK, GPIO_FUNC_SPI);
  gpio_set_function(P_MOSI, GPIO_FUNC_SPI);

  display.init(0); // default 10ms reset pulse, e.g. for bare panels with DESPI-C02
  display.setFullWindow();
  display.drawImage(Bitmap800x480_2, 0, 0, 800, 480, false, false, true);

  display.powerOff();
}


// ----------------------------------------------------------------- LOOP1
void loop1()
{
  
  
  switch (STATE) {
    case 0:
      digitalWrite(25, 1);
      if (error_status == true)
      {
        Serial.println("recieve error status");
        //display.init(0);
        display.setFullWindow();
        display.drawImage(Bitmap800x480_1, 0, 0, 800, 480, false, false, true);

        print_icons();


        unixtime = ((uint32_t)time_num[0] << 24) | ((uint32_t)time_num[1] << 16) | ((uint32_t)time_num[2] << 8) | time_num[3];
        stamp.getDateTime(unixtime);
        //display.fillRect(237, 10, 490, 45, GxEPD_WHITE);
        //display.setCursor(237, 49);
        //display.setFont(&FreeMonoBold9pt7b);
        //display.print(stamp.day);
        //display.print("/");
        //display.print(stamp.month);
        //display.print("/");
        //display.print(stamp.year);
        //display.print("  ");
        //display.print(stamp.hour);
        //display.print(":");
        //display.print(stamp.minute);
        //display.displayWindow(237, 10, 490, 45);
        //display.powerOff();

        STATE = 1;
        Serial.println("goto STATE 1");

        delay(10000);

      }
      //touch_data=0;
      break;
    
    case 1:
      digitalWrite(27, 1);
      if (newcommand == 1)
      {
        Serial.println("***************************new command***********************************************");
        //litros = ((uint32_t)litros_num[0] << 24) | ((uint32_t)litros_num[1] << 16) | ((uint32_t)litros_num[2] << 8) | litros_num[3];
        litros = doc_aux["litros"].as<uint32_t>();
        Serial.println(litros);

        //pesos = ((uint32_t)pesos_num[0] << 24) | ((uint32_t)pesos_num[1] << 16) | ((uint32_t)pesos_num[2] << 8) | pesos_num[3];
        pesos = doc["precio"];
        Serial.println(pesos);
        print_litros = litros;
        Serial.println(print_litros);
        print_pesos = pesos / 100;
        Serial.println(print_pesos);
        
        new_litros = false;
        newcommand = 0;
        shown = 1;
        
        //if (!((todo_byte >> 6) & 0x01))
        //{
        //  endprocess = 1;
        //  Serial.println("end process");
        //  digitalWrite(27, 0);
        //}
      }
      if (shown == 1) 
      {
        digitalWrite(28, 1);
        shown = 0;
        //uprice = ((uint16_t)client_num[0] << 8) | client_num[1];
        //litros = ((uint32_t)litros_num[0] << 24) | ((uint32_t)litros_num[1] << 16) | ((uint32_t)litros_num[2] << 8) | litros_num[3];
        //pesos = ((uint32_t)pesos_num[0] << 24) | ((uint32_t)pesos_num[1] << 16) | ((uint32_t)pesos_num[2] << 8) | pesos_num[3];
        //Show litros

        display.setTextColor(GxEPD_BLACK);
        display.setFont(&CodenameCoderFree4F_Bold40pt7b);
        display.fillRect(450, 125, 250, 50, GxEPD_WHITE);
        display.setCursor(450, 169);
        display.print(print_litros);
        display.displayWindow(450, 125, 250, 50);

        //Show price
        //display.setTextColor(GxEPD_BLACK);
        //display.setFont(&CodenameCoderFree4F_Bold40pt7b);
        //display.fillRect(450, 212, 250, 50, GxEPD_WHITE);
        //display.setCursor(450, 256);
        //display.print(print_pesos/100);
        //display.displayWindow(450, 212, 250, 50);
        //shown = 0;
        digitalWrite(28, 0);
      }

      if (endprocess == 1)
      {
        STATE = 2;
        //digitalWrite(27, 0);
        Serial.println("goto STATE 2");
        endprocess = 0;
      }


      //digitalWrite(27, 0);
      break;
    case 2:

      //litros = ((uint32_t)litros_num[0] << 24) | ((uint32_t)litros_num[1] << 16) | ((uint32_t)litros_num[2] << 8) | litros_num[3];
      //pesos = ((uint32_t)pesos_num[0] << 24) | ((uint32_t)pesos_num[1] << 16) | ((uint32_t)pesos_num[2] << 8) | pesos_num[3];

      //display.setTextColor(GxEPD_BLACK);
      //display.setFont(&CodenameCoderFree4F_Bold40pt7b);
      //display.fillRect(450, 125, 250, 50, GxEPD_WHITE);
      //display.setCursor(450, 169);
      //display.print(print_litros);
      //display.displayWindow(450, 125, 250, 50);

      //Show price
      //display.setTextColor(GxEPD_BLACK);
      //display.setFont(&CodenameCoderFree4F_Bold40pt7b);
      //display.fillRect(450, 212, 250, 50, GxEPD_WHITE);
      //display.setCursor(450, 256);
      //display.print(print_pesos);
      //display.displayWindow(450, 212, 250, 50);
      Serial.println("Show price");
      delay(5000);
      Serial.println("goto STATE 3");
      STATE = 3;
      break;
    case 3:

      //display.init(0);
      //display.setFullWindow();
      //display.drawImage(BitmapPrinter, 300, 140, 200, 200, false, false, true);
      //display.powerOff();
      sleep_ms(20000);
      //display.init(0);
      //display.setFullWindow();
      //display.drawImage(Bitmap800x480_1, 0, 0, 800, 480, false, false, true);
      //display.powerOff();
      STATE = 0;
      Serial.println("goto STATE 0");
      break;
    default:
      Serial.println("fuck");
      break;
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
    display.drawImage(valve_on, 100, 285, 64, 64, false, false, true);
  }
  else
  {
    display.drawImage(valve_off, 100, 285, 64, 64, false, false, true);
  }


  if (doc_aux["gps"].as<bool>() == true)    // ------------------ gps
    //if ((error_byte >> 3) & 0x01)
  {
    display.drawImage(gps_on, 170, 285, 64, 64, false, false, true);
  }
  else
  {
    display.drawImage(gps_off, 170, 285, 64, 64, false, false, true);
  }

  if (doc_aux["clock"].as<bool>() == true)    // ------------------ clock
    // if ((error_byte >> 2) & 0x01)
  {
    display.drawImage(acc_on, 240, 285, 64, 64, false, false, true);
  }
  else
  {
    display.drawImage(acc_off, 240, 285, 64, 64, false, false, true);
  }

  if (doc_aux["printer"].as<bool>() == true)    // ------------------ printer
    //if ((error_byte >> 5) & 0x01)
  {
    display.drawImage(printer_on, 320, 285, 64, 64, false, false, true);
  }
  else
  {
    display.drawImage(printer_off, 320, 285, 64, 64, false, false, true);
  }

  if (doc_aux["paper"].as<bool>() == false)    // ------------------ paper
    //if ((error_byte >> 4) & 0x01)
  {
    display.drawImage(nopaper, 390, 285, 64, 64, false, false, true);
  }
  else
  {
    //display.drawImage(nopaper, 390, 285, 64, 64, false, false, true);
  }
}
