
#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/CodenameCoderFree4F_Bold40pt7b.h>
#include "i2c_fifo.h"
#include "i2c_slave.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
// select the display constructor line in one of the following files (old style):
#include "GxEPD2_display_selection.h"
#include "GxEPD2_display_selection_added.h"

// or select the display class and display driver class in the following file (new style):
#include "GxEPD2_display_selection_new_style.h"

#include "bitmaps/Bitmaps800x480.h" // 7.5"  b/w


#if defined(ARDUINO_ARCH_RP2040) && defined(ARDUINO_RASPBERRY_PI_PICO)
// SPI pins used by GoodDisplay DESPI-PICO. note: steals standard I2C pins PIN_WIRE_SDA (6), PIN_WIRE_SCL (7)
// uncomment next line for use with GoodDisplay DESPI-PICO. // MbedSPI(int miso, int mosi, int sck);
//arduino::MbedSPI SPI0(4, 7, 6); // need be valid pins for same SPI channel, else fails blinking 4 long 4 short
#endif

#if defined(ESP32) && defined(USE_HSPI_FOR_EPD)
SPIClass hspi(HSPI);
#endif

#define P_MOSI      3
#define P_MISO      4
#define P_SCK       2
//#define CS        1
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

float uprice = 9.8; //price of 1 litr

float number=0, l=0, p=0;
boolean new_num=0, printer=0, valve=0,pesos=0,litros=0, OK=0, DEL=0, stopCommand=0, mem_address_written=0, ask_litro = 0, ask_peso=0, ask_data=0, ask_state = 0, newcommand = 0;
uint8_t mem_address = 0;
boolean dec=0;
uint16_t tx, ty, tz1, tz2;
uint8_t STATE=0;
volatile uint8_t todo_byte = 0, state_byte = 0, j=0;
volatile uint8_t ltr_data[4], pes_data[4], uprice_data[2], litro_data[4];
volatile boolean newData=0;

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

static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    switch (event) {
    case I2C_SLAVE_RECEIVE: // master has written some data
        if (!mem_address_written) {
            // writes always start with the memory address
            mem_address = i2c_read_byte(i2c);
            if (mem_address == 0x01){
              ask_state = true;
            }
            else if (mem_address == 0x05){
              ask_litro = true;
            }
            else if (mem_address == 0x06){
              ask_peso = true;
            }
            mem_address_written = true;
        } else {
            // save into memory
            if (mem_address == 0x02){
              todo_byte = i2c_read_byte(i2c);
              newcommand = true;
            }
            if (mem_address == 0x03){
              uprice_data[j] = i2c_read_byte(i2c);
              j++;
              if (j>2){
                j=0;
              }
            }
            if (mem_address == 0x04){ 
              litro_data[j] = i2c_read_byte(i2c);
              j++;
              if (j>4){
                j=0;
              }     
            }
        }
        break;
    case I2C_SLAVE_REQUEST: // master is requesting data
        // load from memory  
      if (ask_state == true){
        i2c_write_byte(i2c, STATE);
        ask_state = false;
      }
      else if (ask_litro == true){
        i2c_write_byte(i2c, ltr_data[j]);
        j++;
        if (j>4){
          ask_data = false;
          j=0;
        }
      }
      else if (ask_peso == true){
        i2c_write_byte(i2c, pes_data[j]);
        j++;
        if (j>4){
          ask_data = false;
          j=0;
        }
      }
      else i2c_write_byte(i2c, 0);
      break;
    case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
        mem_address_written = false;
        //ask_data = false;
        //ask_state = false;
        j=0;
        //gpio_put(LED_2, 0);
       // gpio_put(LED_3, 0);
       // gpio_put(LED_1, 1);
       newData=1;
        break;
    default:
        break;
    }
}

void readData(){
  int ch=-1;
  //boolean dec=0;
   read_touch(&tx, &ty, &tz1, &tz2);
   if (tx>2620){
    if (tx<2970){
      if (ty<1916){
        if (ty>1425) ch=5;
        else if (ty>940) ch=100;
        else if (ty<760) DEL=1;
      }
      else if (ty<2400) ch=4;
      else if (ty<2880) ch=3;
      else if (ty<3330) ch=2;
      else ch=1;
    }
    else{
      if (ty<1916){
        if (ty>1425) ch=0;
        else if (ty<760) OK=1;
        else if (ty>940) ch=10;
      }
      else if (ty<2400) ch=9;
      else if (ty<2880) ch=8;
      else if (ty<3330) ch=7;
      else ch=6;
    }
   }
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
   if (ch>=0){
    new_num = 1;
    if (ch<10){
      if (dec==0){
        number=(number*10)+ch;
      }
      else{
        number = number+(ch*0.1);
      }
    }
    else if (ch==10){
      dec = 1;
    }
    else if (ch==100){
      if (dec==0){
        number = 1.0*((int)number / 10); // Убираем последнюю цифру
      }
      else{
        //number = (int)(number * 10) / 10.0;
        number = round(number);
        dec=0;
      }
      //decimalPlace *= 10.0; // Восстанавливаем десятичное место
    }
   }
   
}


void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  
  
  i2c_init(i2c0, 100 * 1000);
  // configure I2C0 for slave mode
  i2c_slave_init(i2c0, I2C_SLAVE_ADDRESS, &i2c_slave_handler);
  gpio_init(SDA_MAIN);
  gpio_init(SCL_MAIN);
  gpio_set_function(SDA_MAIN, GPIO_FUNC_I2C);
  gpio_set_function(SCL_MAIN, GPIO_FUNC_I2C);
  gpio_pull_up(SDA_MAIN);
  gpio_pull_up(SCL_MAIN);

  i2c_init(i2c1, 100 * 1000);
  gpio_set_function(10, GPIO_FUNC_I2C);
  gpio_set_function(11, GPIO_FUNC_I2C);
  gpio_pull_up(10);
  gpio_pull_up(11);
  touchcommand(MEASURE_TEMP0, POWERDOWN_IRQON, ADC_12BIT);
  
  pinMode(25, OUTPUT);
  digitalWrite(25, 1);
  
  delay(100);
#if defined(ARDUINO_ARCH_RP2040) && defined(ARDUINO_RASPBERRY_PI_PICO)
  // uncomment next line for use with GoodDisplay DESPI-PICO, or use the extended init method
  display.epd2.selectSPI(SPI, SPISettings(4000000, MSBFIRST, SPI_MODE0));
  gpio_set_function(P_MISO, GPIO_FUNC_SPI);
  gpio_set_function(P_SCK, GPIO_FUNC_SPI);
  gpio_set_function(P_MOSI, GPIO_FUNC_SPI);
#endif
#if defined(ESP32) && defined(USE_HSPI_FOR_EPD)
  hspi.begin(13, 12, 14, 15); // remap hspi for EPD (swap pins)
  display.epd2.selectSPI(hspi, SPISettings(4000000, MSBFIRST, SPI_MODE0));
#endif
  display.init(115200, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
  //drawBitmaps();
  drawBitmaps800x480(1);
  display.hibernate();
  //showPartialUpdate();

 // display.powerOff();
  //deepSleepTest();

  Serial.println("setup done");
    
  pinMode(27, OUTPUT);
  digitalWrite(27, 0);
  pinMode(9, INPUT);
  while(digitalRead(9)==1){
    //do nothing
  }
  drawBitmaps800x480(2);
  //display.epd2.writeScreenBufferAgain(); // use default for white
  //display.epd2.writeImageAgain(bitmaps[i], x, y, 200, 200, false, mirror_y, true);
}

void loop(){
  
  switch(STATE){
    case 0:               // wait litros or pesos
      if (digitalRead(9)==0){
        readData();
        if (litros==1){
          number=0;
          showLitrosUpdate();
          STATE = 1;
        }
        if (pesos==1){
          number=0;
          showPesosUpdate();
          STATE = 3;
        }
      }
      break;
    case 1:               //wait numbers of litros
      if (digitalRead(9)==0){
        readData();
        if (number<9999.99){
          if (new_num==1){
            showLitrosUpdate();
            new_num=0;
          } 
        }
        if (OK ==1){
          litros = 0;
          OK=0;
          dec=0;
          uint16_t myVar = (uprice_data[1] << 8) | uprice_data[0];
          uprice = (float)myVar/100;
          l = number;
          p = l*uprice;
          number = p;
          showPesosUpdate();
          uint32_t tmpVar = (uint32_t)(l*100);
          ltr_data[0] = (tmpVar >> 24) & 0xFF;  // extract the most significant byte
          ltr_data[1] = (tmpVar >> 16) & 0xFF;  // extract the second byte
          ltr_data[2] = (tmpVar >> 8) & 0xFF;   // extract the third byte
          ltr_data[3] = tmpVar & 0xFF;          // extract the least significant byte
          tmpVar = (uint32_t)(p*100);
          pes_data[0] = (tmpVar >> 24) & 0xFF;  // extract the most significant byte
          pes_data[1] = (tmpVar >> 16) & 0xFF;  // extract the second byte
          pes_data[2] = (tmpVar >> 8) & 0xFF;   // extract the third byte
          pes_data[3] = tmpVar & 0xFF;          // extract the least significant byte
          STATE = 5;
        }
        if (DEL==1){
          STATE=100;
        }
      }
      break;
    case 3:             //wait number of pesos
       if (digitalRead(9)==0){
        readData();
        if (number<9999.99){
          if (new_num==1){
            showPesosUpdate();
            new_num=0;
          } 
        }
        if (OK ==1){
          pesos = 0;
          OK=0;
          dec=0;
          uint16_t myVar = (uprice_data[1] << 8) | uprice_data[0];
          uprice = (float)myVar/100;
          p = number;
          l = p/uprice;
          number = l;
          showLitrosUpdate();
          uint32_t tmpVar = (uint32_t)(l*100);
          ltr_data[0] = (tmpVar >> 24) & 0xFF;  // extract the most significant byte
          ltr_data[1] = (tmpVar >> 16) & 0xFF;  // extract the second byte
          ltr_data[2] = (tmpVar >> 8) & 0xFF;   // extract the third byte
          ltr_data[3] = tmpVar & 0xFF;          // extract the least significant byte
          tmpVar = (uint32_t)(p*100);
          pes_data[0] = (tmpVar >> 24) & 0xFF;  // extract the most significant byte
          pes_data[1] = (tmpVar >> 16) & 0xFF;  // extract the second byte
          pes_data[2] = (tmpVar >> 8) & 0xFF;   // extract the third byte
          pes_data[3] = tmpVar & 0xFF;          // extract the least significant byte
          STATE = 5;
        }
        if (DEL==1){
          STATE=100;
        }
      }
      break;
    case 5:           //wait start of process
      if (digitalRead(9)==0){
        readData();
        if (valve == 1){
          drawBitmaps800x480(2);
          valve=0;
          STATE = 10;
        }
        if (DEL==1){
          STATE=100;
        }
      }
      /*
      uint8_t bite1 = (todo_byte >> 7) & 0x01;
      if (bite1 == 1){
        drawBitmaps800x480(2);
        valve=0;
        STATE = 10;
      }
      */
      break;
    case 10:          //GO!
      if(newData == 1){
        //readRegisterData();
        uint32_t myVar = ((uint32_t)litro_data[0] << 24) | ((uint32_t)litro_data[1] << 16) | ((uint32_t)litro_data[2] << 8) | litro_data[3];
        number = (float)myVar/100.0;
        showLitrosUpdate();
        newData=0;
      }
      
      if (digitalRead(9)==0){
        readData();
        if ((valve == 1)||(stopCommand==1)){
          valve=0;
          l = number;
          p = l*uprice;
          number = p;
          showPesosUpdate();
          STATE = 15;
        }
      }
      /*
      uint8_t bite = (todo_byte >> 7) & 0x01;
      if (bite == 0){
        valve=0;
        l = number;
        p = l*uprice;
        number = p;
        showPesosUpdate();
        STATE = 15;
      }
      */
      break;
    case 15:                    //wait printer
      if (digitalRead(9)==0){
        readData();
        if (printer==1){
          printer=0;
          gotoPrint();
          STATE = 101;
        }
      }
      break;
    case 101:
      printer=0;
      valve = 0;
      pesos=0;
      litros=0;
      dec=0;
      OK=0;
      DEL=0;
      newData=0;
      stopCommand=0;
      l=0;
      p=0;
      drawBitmaps800x480(1);
      display.hibernate();
      delay(1000);
      STATE=110;
      break;
    case 100:
      printer=0;
      valve = 0;
      pesos=0;
      litros=0;
      OK=0;
      DEL=0;
      dec=0;
      newData=0;
      stopCommand=0;
      l=0;
      p=0;
      drawBitmaps800x480(2);
      STATE=0;
      break;
    case 110:
       if (digitalRead(9)==0){
          drawBitmaps800x480(2);
          STATE=0;
       }
       break;
      
  }
}

#if defined(ESP8266) || defined(ESP32)
#include <StreamString.h>
#define PrintString StreamString
#else
class PrintString : public Print, public String
{
  public:
    size_t write(uint8_t data) override
    {
      return concat(char(data));
    };
};
#endif



void deepSleepTest()
{
  //Serial.println("deepSleepTest");
  const char hibernating[] = "hibernating ...";
  const char wokeup[] = "woke up";
  const char from[] = "from deep sleep";
  const char again[] = "again";
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  if (display.epd2.WIDTH < 104) display.setFont(0);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  // center text
  display.getTextBounds(hibernating, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(hibernating);
  }
  while (display.nextPage());
  display.hibernate();
  delay(5000);
  display.getTextBounds(wokeup, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t wx = (display.width() - tbw) / 2;
  uint16_t wy = ((display.height() / 3) - tbh / 2) - tby; // y is base line!
  display.getTextBounds(from, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t fx = (display.width() - tbw) / 2;
  uint16_t fy = ((display.height() * 2 / 3) - tbh / 2) - tby; // y is base line!
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(wx, wy);
    display.print(wokeup);
    display.setCursor(fx, fy);
    display.print(from);
  }
  while (display.nextPage());
  delay(5000);
  display.getTextBounds(hibernating, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t hx = (display.width() - tbw) / 2;
  uint16_t hy = ((display.height() / 3) - tbh / 2) - tby; // y is base line!
  display.getTextBounds(again, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t ax = (display.width() - tbw) / 2;
  uint16_t ay = ((display.height() * 2 / 3) - tbh / 2) - tby; // y is base line!
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(hx, hy);
    display.print(hibernating);
    display.setCursor(ax, ay);
    display.print(again);
  }
  while (display.nextPage());
  display.hibernate();
  //Serial.println("deepSleepTest done");
}

void showBox(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool partial)
{
  //Serial.println("showBox");
  display.setRotation(1);
  if (partial)
  {
    display.setPartialWindow(x, y, w, h);
  }
  else
  {
    display.setFullWindow();
  }
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.fillRect(x, y, w, h, GxEPD_BLACK);
  }
  while (display.nextPage());
  //Serial.println("showBox done");
}

void showLitrosUpdate()
{
  uint16_t box_x = 450;
  uint16_t box_y = 125;
  uint16_t box_w = 190;//226
  uint16_t box_h = 50;
  uint16_t cursor_y = box_y + box_h - 6;
  digitalWrite(27,1);
  if (display.epd2.WIDTH < 104) cursor_y = box_y + 6;
  //uint16_t incr = display.epd2.hasFastPartialUpdate ? 1 : 3;
  display.setFont(&CodenameCoderFree4F_Bold40pt7b);
  //display.setTextSize(2);
  display.setTextColor(GxEPD_BLACK);

  // show updates in the update box

    display.setRotation(0);
    display.setPartialWindow(box_x, box_y, box_w, box_h);
    display.firstPage();
      do
      {
        display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
        display.setCursor(box_x, cursor_y);
        display.print(number, 1);
      }
      while (display.nextPage());
      digitalWrite(27,0);
     // delay(200);

    //delay(300);
   // display.firstPage();
   // do
    //{
     // display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
   // }
   // while (display.nextPage());
   // delay(300);

}

void showPesosUpdate()
{
  uint16_t box_x = 450;
  uint16_t box_y = 212;
  uint16_t box_w = 290;//226
  uint16_t box_h = 50;
  uint16_t cursor_y = box_y + box_h - 6;
  digitalWrite(27,1);
  if (display.epd2.WIDTH < 104) cursor_y = box_y + 6;
  //uint16_t incr = display.epd2.hasFastPartialUpdate ? 1 : 3;
  display.setFont(&CodenameCoderFree4F_Bold40pt7b);
  //display.setTextSize(2);
  display.setTextColor(GxEPD_BLACK);

  // show updates in the update box

    display.setRotation(0);
    display.setPartialWindow(box_x, box_y, box_w, box_h);
    display.firstPage();
      do
      {
        display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
        display.setCursor(box_x, cursor_y);
        display.print(number, 1);
      }
      while (display.nextPage());
      digitalWrite(27,0);
     // delay(200);

    //delay(300);
   // display.firstPage();
   // do
    //{
     // display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
   // }
   // while (display.nextPage());
   // delay(300);

}

void showPartialUpdate()
{
  // some useful background
  //helloWorld();
  // use asymmetric values for test
  uint16_t box_x = 550;
  uint16_t box_y = 110;
  uint16_t box_w = 226;
  uint16_t box_h = 76;
  uint16_t cursor_y = box_y + box_h - 6;
  if (display.epd2.WIDTH < 104) cursor_y = box_y + 6;
  float value = 13.95;
  uint16_t incr = display.epd2.hasFastPartialUpdate ? 1 : 3;
  display.setFont(&CodenameCoderFree4F_Bold40pt7b);
  //display.setTextSize(4);
  display.setTextColor(GxEPD_BLACK);

  // show updates in the update box

    display.setRotation(0);
    display.setPartialWindow(box_x, box_y, box_w, box_h);
    for (uint16_t i = 1; i <= 10; i += incr)
    {
      display.firstPage();
      do
      {
        display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
        display.setCursor(box_x, cursor_y);
        display.print(value * i, 2);
      }
      while (display.nextPage());
      delay(200);
    }
    delay(300);
    display.firstPage();
    do
    {
      display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    }
    while (display.nextPage());
    delay(300);

}

void readRegisterData(){
  
}

void gotoPrint(){
  
}

void drawBitmaps800x480(byte numero)
{
  display.setFullWindow();
  if ((display.epd2.WIDTH == 800) && (display.epd2.HEIGHT == 480))
  {
      display.firstPage();
      do
      {
        display.fillScreen(GxEPD_WHITE);
        if (numero ==1){
          display.drawBitmap(0, 0, Bitmap800x480_2, 800, 480, GxEPD_BLACK);
        }
        else if (numero==2){
          display.drawBitmap(0, 0, Bitmap800x480_1, 800, 480, GxEPD_BLACK);
        }
      }
      while (display.nextPage());
      delay(500);

    //if ((display.epd2.panel == GxEPD2::GDEW075T7) || (display.epd2.panel == GxEPD2::GDEY075T7))
   // {
      // avoid ghosting caused by OTP waveform
      //display.clearScreen();
      //display.refresh(false); // full update
   // }
  }
}


struct bitmap_pair
{
  const unsigned char* black;
  const unsigned char* red;
};

#ifdef _GxBitmaps3c200x200_H_
void drawBitmaps3c200x200()
{
  bitmap_pair bitmap_pairs[] =
  {
    //{Bitmap3c200x200_black, Bitmap3c200x200_red},
    {WS_Bitmap3c200x200_black, WS_Bitmap3c200x200_red}
  };
  if (display.epd2.panel == GxEPD2::GDEW0154Z04)
  {
    display.firstPage();
    do
    {
      display.fillScreen(GxEPD_WHITE);
      // Bitmap3c200x200_black has 2 bits per pixel
      // taken from Adafruit_GFX.cpp, modified
      int16_t byteWidth = (display.epd2.WIDTH + 7) / 8; // Bitmap scanline pad = whole byte
      uint8_t byte = 0;
      for (int16_t j = 0; j < display.epd2.HEIGHT; j++)
      {
        for (int16_t i = 0; i < display.epd2.WIDTH; i++)
        {
          if (i & 3) byte <<= 2;
          else
          {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
            byte = pgm_read_byte(&Bitmap3c200x200_black[j * byteWidth * 2 + i / 4]);
#else
            byte = Bitmap3c200x200_black[j * byteWidth * 2 + i / 4];
#endif
          }
          if (!(byte & 0x80))
          {
            display.drawPixel(i, j, GxEPD_BLACK);
          }
        }
      }
      display.drawInvertedBitmap(0, 0, Bitmap3c200x200_red, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_RED);
    }
    while (display.nextPage());
    delay(2000);
    for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
    {
      display.firstPage();
      do
      {
        display.fillScreen(GxEPD_WHITE);
        display.drawInvertedBitmap(0, 0, bitmap_pairs[i].black, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_BLACK);
        display.drawInvertedBitmap(0, 0, bitmap_pairs[i].red, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_RED);
      }
      while (display.nextPage());
      delay(2000);
    }
  }
  if (display.epd2.hasColor)
  {
    display.clearScreen(); // use default for white
    int16_t x = (int16_t(display.epd2.WIDTH) - 200) / 2;
    int16_t y = (int16_t(display.epd2.HEIGHT) - 200) / 2;
    for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
    {
      display.drawImage(bitmap_pairs[i].black, bitmap_pairs[i].red, x, y, 200, 200, false, false, true);
      delay(2000);
    }
    for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
    {
      int16_t x = -60;
      int16_t y = -60;
      for (uint16_t j = 0; j < 10; j++)
      {
        display.writeScreenBuffer(); // use default for white
        display.writeImage(bitmap_pairs[i].black, bitmap_pairs[i].red, x, y, 200, 200, false, false, true);
        display.refresh();
        delay(1000);
        x += display.epd2.WIDTH / 4;
        y += display.epd2.HEIGHT / 4;
        if ((x >= int16_t(display.epd2.WIDTH)) || (y >= int16_t(display.epd2.HEIGHT))) break;
      }
    }
    display.writeScreenBuffer(); // use default for white
    display.writeImage(bitmap_pairs[0].black, bitmap_pairs[0].red, 0, 0, 200, 200, false, false, true);
    display.writeImage(bitmap_pairs[0].black, bitmap_pairs[0].red, int16_t(display.epd2.WIDTH) - 200, int16_t(display.epd2.HEIGHT) - 200, 200, 200, false, false, true);
    display.refresh();
    delay(2000);
  }
}
#endif

#ifdef _GxBitmaps3c104x212_H_
void drawBitmaps3c104x212()
{
#if !defined(__AVR)
  bitmap_pair bitmap_pairs[] =
  {
    {Bitmap3c104x212_1_black, Bitmap3c104x212_1_red},
    {Bitmap3c104x212_2_black, Bitmap3c104x212_2_red},
    {WS_Bitmap3c104x212_black, WS_Bitmap3c104x212_red}
  };
#else
  bitmap_pair bitmap_pairs[] =
  {
    {Bitmap3c104x212_1_black, Bitmap3c104x212_1_red},
    //{Bitmap3c104x212_2_black, Bitmap3c104x212_2_red},
    {WS_Bitmap3c104x212_black, WS_Bitmap3c104x212_red}
  };
#endif
  if (display.epd2.panel == GxEPD2::GDEW0213Z16)
  {
    for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
    {
      display.firstPage();
      do
      {
        display.fillScreen(GxEPD_WHITE);
        display.drawInvertedBitmap(0, 0, bitmap_pairs[i].black, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_BLACK);
        if (bitmap_pairs[i].red == WS_Bitmap3c104x212_red)
        {
          display.drawInvertedBitmap(0, 0, bitmap_pairs[i].red, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_RED);
        }
        else display.drawBitmap(0, 0, bitmap_pairs[i].red, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_RED);
      }
      while (display.nextPage());
      delay(2000);
    }
  }
}
#endif

#ifdef _GxBitmaps3c128x250_H_
void drawBitmaps3c128x250()
{
  if ((display.epd2.WIDTH == 128) && (display.epd2.HEIGHT == 250) && display.epd2.hasColor)
  {
    bool mirrored = display.mirror(true);
    display.firstPage();
    do
    {
      display.fillScreen(GxEPD_WHITE);
      display.drawInvertedBitmap(0, 0, Bitmap3c128x250_1_black, 128, 250, GxEPD_BLACK);
      display.drawInvertedBitmap(0, 0, Bitmap3c128x250_1_red, 128, 250, GxEPD_RED);
    }
    while (display.nextPage());
    delay(2000);
#if !defined(__AVR)
    display.firstPage();
    do
    {
      display.fillScreen(GxEPD_WHITE);
      display.drawInvertedBitmap(0, 0, Bitmap3c128x250_2_black, 128, 250, GxEPD_BLACK);
      display.drawBitmap(0, 0, Bitmap3c128x250_2_red, 128, 250, GxEPD_RED);
    }
    while (display.nextPage());
    delay(2000);
#endif
    display.mirror(mirrored);
  }
}
#endif

#ifdef _GxBitmaps3c128x296_H_
void drawBitmaps3c128x296()
{
#if !defined(__AVR)
  bitmap_pair bitmap_pairs[] =
  {
    {Bitmap3c128x296_1_black, Bitmap3c128x296_1_red},
    {Bitmap3c128x296_2_black, Bitmap3c128x296_2_red},
    {WS_Bitmap3c128x296_black, WS_Bitmap3c128x296_red}
  };
#else
  bitmap_pair bitmap_pairs[] =
  {
    //{Bitmap3c128x296_1_black, Bitmap3c128x296_1_red},
    //{Bitmap3c128x296_2_black, Bitmap3c128x296_2_red},
    {WS_Bitmap3c128x296_black, WS_Bitmap3c128x296_red}
  };
#endif
  if ((display.epd2.WIDTH == 128) && (display.epd2.HEIGHT == 296) && display.epd2.hasColor)
  {
    for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
    {
      display.firstPage();
      do
      {
        display.fillScreen(GxEPD_WHITE);
        display.drawInvertedBitmap(0, 0, bitmap_pairs[i].black, 128, 296, GxEPD_BLACK);
        if (bitmap_pairs[i].red == WS_Bitmap3c128x296_red)
        {
          display.drawInvertedBitmap(0, 0, bitmap_pairs[i].red, 128, 296, GxEPD_RED);
        }
        else display.drawBitmap(0, 0, bitmap_pairs[i].red, 128, 296, GxEPD_RED);
      }
      while (display.nextPage());
      delay(2000);
    }
  }
}
#endif

#ifdef _GxBitmaps3c152x296_H_
void drawBitmaps3c152x296()
{
  bitmap_pair bitmap_pairs[] =
  {
    {Bitmap3c152x296_black, Bitmap3c152x296_red}
  };
  if (display.epd2.panel == GxEPD2::GDEY0266Z90)
  {
    bool mirrored = display.mirror(true);
    for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
    {
      display.firstPage();
      do
      {
        display.fillScreen(GxEPD_WHITE);
        display.drawBitmap(0, 0, bitmap_pairs[i].black, 152, 296, GxEPD_BLACK);
        display.drawInvertedBitmap(0, 0, bitmap_pairs[i].red, 152, 296, GxEPD_RED);
      }
      while (display.nextPage());
      delay(2000);
    }
    display.mirror(mirrored);
  }
}
#endif

#ifdef _GxBitmaps3c176x264_H_
void drawBitmaps3c176x264()
{
  bitmap_pair bitmap_pairs[] =
  {
    {Bitmap3c176x264_black, Bitmap3c176x264_red}
  };
  if (display.epd2.panel == GxEPD2::GDEW027C44)
  {
    for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
    {
      display.firstPage();
      do
      {
        display.fillScreen(GxEPD_WHITE);
        display.drawBitmap(0, 0, bitmap_pairs[i].black, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_BLACK);
        display.drawBitmap(0, 0, bitmap_pairs[i].red, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_RED);
      }
      while (display.nextPage());
      delay(2000);
    }
  }
}
#endif

#ifdef _GxBitmaps3c400x300_H_
void drawBitmaps3c400x300()
{
#if !defined(__AVR)
  bitmap_pair bitmap_pairs[] =
  {
    {Bitmap3c400x300_1_black, Bitmap3c400x300_1_red},
    {Bitmap3c400x300_2_black, Bitmap3c400x300_2_red},
    {WS_Bitmap3c400x300_black, WS_Bitmap3c400x300_red}
  };
#else
  bitmap_pair bitmap_pairs[] = {}; // not enough code space
#endif
  if (display.epd2.panel == GxEPD2::GDEW042Z15)
  {
    for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
    {
      display.firstPage();
      do
      {
        display.fillScreen(GxEPD_WHITE);
        display.drawInvertedBitmap(0, 0, bitmap_pairs[i].black, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_BLACK);
        display.drawInvertedBitmap(0, 0, bitmap_pairs[i].red, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_RED);
      }
      while (display.nextPage());
      delay(2000);
    }
  }
}
#endif

#ifdef _GxBitmaps3c648x480_H_
void drawBitmaps3c648x480()
{
#if !defined(__AVR)
  bitmap_pair bitmap_pairs[] =
  {
    {Bitmap3c648x480_black, Bitmap3c648x480_red}
  };
#else
  bitmap_pair bitmap_pairs[] = {}; // not enough code space
#endif
  if (display.epd2.panel == GxEPD2::GDEW0583Z83)
  {
    for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
    {
      display.firstPage();
      do
      {
        display.fillScreen(GxEPD_WHITE);
        display.drawBitmap(0, 0, bitmap_pairs[i].black, 648, 480, GxEPD_BLACK);
        display.drawBitmap(0, 0, bitmap_pairs[i].red, 648, 480, GxEPD_RED);
      }
      while (display.nextPage());
      delay(2000);
    }
  }
}
#endif

#ifdef _GxBitmaps3c800x480_H_
void drawBitmaps3c800x480()
{
#if !defined(__AVR)
  bitmap_pair bitmap_pairs[] =
  {
    {Bitmap3c800x480_1_black, Bitmap3c800x480_1_red}
  };
#else
  bitmap_pair bitmap_pairs[] = {}; // not enough code space
#endif
  if (display.epd2.panel == GxEPD2::GDEW075Z08)
  {
    for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
    {
      display.firstPage();
      do
      {
        display.fillScreen(GxEPD_WHITE);
        display.drawBitmap(0, 0, bitmap_pairs[i].black, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_BLACK);
        display.drawBitmap(0, 0, bitmap_pairs[i].red, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_RED);
      }
      while (display.nextPage());
      delay(2000);
    }
  }
}
#endif

#ifdef _GxBitmaps3c880x528_H_
void drawBitmaps3c880x528()
{
#if !defined(__AVR)
  bitmap_pair bitmap_pairs[] =
  {
    {Bitmap3c880x528_black, Bitmap3c880x528_red}
  };
#else
  bitmap_pair bitmap_pairs[] = {}; // not enough code space
#endif
  if (display.epd2.panel == GxEPD2::GDEH075Z90)
  {
    bool m = display.mirror(true);
    for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
    {
      display.firstPage();
      do
      {
        display.fillScreen(GxEPD_WHITE);
        display.drawInvertedBitmap(0, 0, bitmap_pairs[i].black, 880, 528, GxEPD_BLACK);
        display.drawInvertedBitmap(0, 0, bitmap_pairs[i].red, 880, 528, GxEPD_RED);
      }
      while (display.nextPage());
      delay(2000);
    }
    display.mirror(m);
  }
}
#endif

#if defined(ESP32) && defined(_GxBitmaps3c1304x984_H_)
void drawBitmaps3c1304x984()
{
  if (display.epd2.panel == GxEPD2::GDEY1248Z51)
  {
    //display.drawImage(Bitmap3c1304x984_black, Bitmap3c1304x984_red, 0, 0, 1304, 984, false, false, true);
    display.writeImage(0, Bitmap3c1304x984_red, 0, 0, 1304, 984, true, false, true); // red bitmap is inverted
    display.drawImage(Bitmap3c1304x984_black, 0, 0, 0, 1304, 984, true, false, true); // black bitmap is normal
  }
}
#endif

#if defined(_WS_Bitmaps4c168x168_H_)
void drawBitmaps4c168x168()
{
  if (display.epd2.panel == GxEPD2::Waveshare437inch4color)
  {
    display.drawNative(WS_Bitmap4c168x168, 0, (display.epd2.WIDTH - 168) / 2, (display.epd2.HEIGHT - 168) / 2, 168, 168, false, false, true);
    delay(5000);
  }
}
#endif

#if defined(_WS_Bitmaps7c192x143_H_)
void drawBitmaps7c192x143()
{
  if (display.epd2.panel == GxEPD2::ACeP565)
  {
    display.drawNative(WS_Bitmap7c192x143, 0, (display.epd2.WIDTH - 192) / 2, (display.epd2.HEIGHT - 143) / 2, 192, 143, false, false, true);
    delay(5000);
  }
}
#endif

#if defined(_GxBitmaps7c800x480_H_)
void drawBitmaps7c800x480()
{
  if (display.epd2.panel == GxEPD2::GDEY073D46)
  {
    display.epd2.drawDemoBitmap(Bitmap7c800x480, 0, 0, 0, 800, 480, 0, false, true); // special format
    delay(5000);
  }
}
#endif

#if defined(_WS_Bitmaps7c300x180_H_)
void drawBitmaps7c300x180()
{
  if (display.epd2.panel == GxEPD2::GDEY073D46)
  {
    display.drawNative(WS_Bitmap7c300x180, 0, (display.epd2.WIDTH - 300) / 2, (display.epd2.HEIGHT - 180) / 2, 300, 180, false, false, true);
    delay(5000);
  }
}
#endif
