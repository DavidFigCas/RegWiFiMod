#define ENABLE_GxEPD2_GFX 0
#include "icons.h"
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/CodenameCoderFree4F_Bold40pt7b.h>
#include "i2c_fifo.h"
#include "i2c_slave.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "GxEPD2_display_selection_new_style.h"
//#include <pico/multicore.h>
#include <UnixTime.h>

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

//float number=0, l=0, p=0;
uint32_t litros, pesos;

boolean new_num = 0, printer = 0, valve = 0, OK = 0, DEL = 0, stopCommand = 0, mem_address_written = 0, ask_litro = 0, ask_peso = 0, ask_data = 0, ask_state = 0, ask_todo = 0, error_status = 0, newcommand = 0, new_litros = 0;
uint8_t mem_address = 0;
boolean dec = 0;
uint16_t tx, ty, tz1, tz2;
uint8_t STATE = 0;
volatile uint8_t todo_byte = 0, state_byte = 0, error_byte = 0, j = 0;
volatile uint8_t ltr_data[4], pes_data[4], uprice_data[2], litros_num[4], pesos_num[4], client_num[4], time_num[4];
volatile boolean newData = 0, touch_data = 0;
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

static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {

  switch (event) {
    case I2C_SLAVE_RECEIVE: // master has written some data

      if (!mem_address_written) {
        // writes always start with the memory address
        //digitalWrite(28,1);
        mem_address = i2c_read_byte(i2c);
        //digitalWrite(28,1);
        if (mem_address == 0x01) {
          ask_state = true;

        }
        else if (mem_address == 0x05) {
          ask_litro = true;
        }
        else if (mem_address == 0x06) {
          ask_peso = true;
        }
        else if (mem_address == 0x02) {
          ask_todo = true;
        }
        else {
          //digitalWrite(28,1);
        }
        mem_address_written = true;
      } else {
        //digitalWrite(28,1);
        mem_address_written = 0;
        // save into memory
        if (mem_address == 0x08) {
          //digitalWrite(27,1);
          error_status = true;
          error_byte = i2c_read_byte(i2c);
        }
        if (mem_address == 0x02) {
          todo_byte = i2c_read_byte(i2c);
          newcommand = true;
        }
        if (mem_address == 0x03) {
          pesos_num[j] = i2c_read_byte(i2c);
          j++;
          if (j > 4) {
            j = 0;
          }
        }
        if (mem_address == 0x04) {
          new_litros = true;
          litros_num[j] = i2c_read_byte(i2c);
          j++;
          if (j > 4) {
            j = 0;
          }
        }
        if (mem_address == 0x09) {
          client_num[j] = i2c_read_byte(i2c);
          j++;
          if (j > 4) {
            j = 0;
          }
        }
        if (mem_address == 0x10) {
          time_num[j] = i2c_read_byte(i2c);
          j++;
          if (j > 4) {
            j = 0;
          }
        }
        if (mem_address == 0x05) {
          litros_num[j] = i2c_read_byte(i2c);
          j++;
          if (j > 4) {
            j = 0;
          }
        }
      }
      break;
    case I2C_SLAVE_REQUEST: // master is requesting data
      // load from memory
      mem_address_written = 0;
      if (ask_state == true) {
        i2c_write_byte(i2c, STATE);
        ask_state = false;
      }
      else if (ask_todo == true) {
        i2c_write_byte(i2c, todo_byte);
        ask_todo = false;
      }
      else if (ask_litro == true) {
        i2c_write_byte(i2c, ltr_data[j]);
        j++;
        if (j > 4) {
          ask_data = false;
          j = 0;
        }
      }
      else if (ask_peso == true) {
        i2c_write_byte(i2c, pes_data[j]);
        j++;
        if (j > 4) {
          ask_data = false;
          j = 0;
        }
      }
      else i2c_write_byte(i2c, 0);
      break;
    case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
      //mem_address_written = false;
      //ask_data = false;
      //ask_state = false;
      j = 0;
      //gpio_put(LED_2, 0);
      // gpio_put(LED_3, 0);
      // gpio_put(LED_1, 1);
      newData = 1;
      break;
    default:
      break;
  }
}
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


// ----------------------------------------------------- setup
void setup()
{

  gpio_init(SDA_MAIN);
  gpio_init(SCL_MAIN);
  gpio_set_function(SDA_MAIN, GPIO_FUNC_I2C);
  gpio_set_function(SCL_MAIN, GPIO_FUNC_I2C);
  gpio_pull_up(SDA_MAIN);
  gpio_pull_up(SCL_MAIN);
  i2c_init(i2c0, 100 * 1000);
  // configure I2C0 for slave mode
  i2c_slave_init(i2c0, I2C_SLAVE_ADDRESS, &i2c_slave_handler);

  pinMode(25, OUTPUT);
  digitalWrite(25, 0);
  pinMode(28, OUTPUT);
  digitalWrite(28, 0);
  pinMode(27, OUTPUT);
  digitalWrite(27, 0);


  Serial.begin(115200);
  Serial.println("Display Init");
  delay(3000);
  Serial.println("Display Init OK");

  //multicore_launch_core1(core1_blink);
  //Serial.begin(115200);
  //pinMode(27, OUTPUT);
  display.epd2.selectSPI(SPI, SPISettings(4000000, MSBFIRST, SPI_MODE0));
  gpio_set_function(P_MISO, GPIO_FUNC_SPI);
  gpio_set_function(P_SCK, GPIO_FUNC_SPI);
  gpio_set_function(P_MOSI, GPIO_FUNC_SPI);

  display.init(0); // default 10ms reset pulse, e.g. for bare panels with DESPI-C02

  //display.init(115200, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
  display.setFullWindow();
  display.drawImage(Bitmap800x480_2, 0, 0, 800, 480, false, false, true);
  //digitalWrite(25, 0);
  //display.init(115200, true, 2, false);
  //showPartialUpdate();
  //deepSleepTest();
  display.powerOff();
  //Serial.println("setup done");
}


// ----------------------------------------------------- loop
void loop() {
  Serial.print("STATE");Serial.println(STATE);
  delay(1000);
  
  switch (STATE) {
    case 0:
      digitalWrite(25, 1);
      if (error_status == true) {
        display.init(0);
        display.setFullWindow();
        display.drawImage(Bitmap800x480_1, 0, 0, 800, 480, false, false, true);

        if ((error_byte >> 7) & 0x01) {
          display.drawImage(wifi_on, 30, 285, 64, 64, false, false, true);
        }
        else {
          display.drawImage(wifi_off, 30, 285, 64, 64, false, false, true);
        }
        if ((error_byte >> 6) & 0x01) {
          display.drawImage(valve_on, 100, 285, 64, 64, false, false, true);
        }
        else {
          display.drawImage(valve_off, 100, 285, 64, 64, false, false, true);
        }
        if ((error_byte >> 3) & 0x01) {
          display.drawImage(gps_on, 170, 285, 64, 64, false, false, true);
        }
        else {
          display.drawImage(gps_off, 170, 285, 64, 64, false, false, true);
        }
        if ((error_byte >> 2) & 0x01) {
          display.drawImage(acc_on, 240, 285, 64, 64, false, false, true);
        }
        else {
          display.drawImage(acc_off, 240, 285, 64, 64, false, false, true);
        }
        if ((error_byte >> 5) & 0x01) {
          display.drawImage(printer_on, 320, 285, 64, 64, false, false, true);
        }
        else {
          display.drawImage(printer_off, 320, 285, 64, 64, false, false, true);
        }
        if ((error_byte >> 4) & 0x01) {
          //display.drawImage(printer_on, 290, 285, 64, 64, false, false, true);
        }
        else {
          display.drawImage(nopaper, 390, 285, 64, 64, false, false, true);
        }
        unixtime = ((uint32_t)time_num[0] << 24) | ((uint32_t)time_num[1] << 16) | ((uint32_t)time_num[2] << 8) | time_num[3];
        stamp.getDateTime(unixtime);
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
        display.powerOff();
        STATE = 1;
      }
      //touch_data=0;
      break;
    case 1:
      digitalWrite(27, 1);
      if (newcommand == 1) {
        if ((todo_byte >> 6) & 0x01) {
          //uprice = ((uint16_t)client_num[0] << 8) | client_num[1];
          litros = ((uint32_t)litros_num[0] << 24) | ((uint32_t)litros_num[1] << 16) | ((uint32_t)litros_num[2] << 8) | litros_num[3];
          pesos = ((uint32_t)pesos_num[0] << 24) | ((uint32_t)pesos_num[1] << 16) | ((uint32_t)pesos_num[2] << 8) | pesos_num[3];
          //Show litros
          display.setTextColor(GxEPD_BLACK);
          display.setFont(&CodenameCoderFree4F_Bold40pt7b);
          display.fillRect(450, 125, 250, 50, GxEPD_WHITE);
          display.setCursor(450, 169);
          display.print(litros / 100);
          display.displayWindow(450, 125, 250, 50);
          //Show price
          display.setTextColor(GxEPD_BLACK);
          display.setFont(&CodenameCoderFree4F_Bold40pt7b);
          display.fillRect(450, 212, 250, 50, GxEPD_WHITE);
          display.setCursor(450, 256);
          display.print(pesos / 100);
          display.displayWindow(450, 212, 250, 50);
        }
        else STATE = 2;
        newcommand = 0;
      }
      if (error_status == true) {
        if ((error_byte >> 7) & 0x01) {
          display.drawImage(wifi_on, 30, 285, 64, 64, false, false, true);
        }
        else {
          display.drawImage(wifi_off, 30, 285, 64, 64, false, false, true);
        }
        if ((error_byte >> 6) & 0x01) {
          display.drawImage(valve_on, 100, 285, 64, 64, false, false, true);
        }
        else {
          display.drawImage(valve_off, 100, 285, 64, 64, false, false, true);
        }
        if ((error_byte >> 3) & 0x01) {
          display.drawImage(gps_on, 170, 285, 64, 64, false, false, true);
        }
        else {
          display.drawImage(gps_off, 170, 285, 64, 64, false, false, true);
        }
        if ((error_byte >> 2) & 0x01) {
          display.drawImage(acc_on, 240, 285, 64, 64, false, false, true);
        }
        else {
          display.drawImage(acc_off, 240, 285, 64, 64, false, false, true);
        }
        if ((error_byte >> 5) & 0x01) {
          display.drawImage(printer_on, 320, 285, 64, 64, false, false, true);
        }
        else {
          display.drawImage(printer_off, 320, 285, 64, 64, false, false, true);
        }
        if ((error_byte >> 4) & 0x01) {
          //display.drawImage(printer_on, 290, 285, 64, 64, false, false, true);
        }
        else {
          display.drawImage(nopaper, 390, 285, 64, 64, false, false, true);
        }
        unixtime = ((uint32_t)time_num[0] << 24) | ((uint32_t)time_num[1] << 16) | ((uint32_t)time_num[2] << 8) | time_num[3];
        stamp.getDateTime(unixtime);
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
        display.powerOff();
      }
      //digitalWrite(27, 0);
      break;
    case 2:
      litros = ((uint32_t)litros_num[0] << 24) | ((uint32_t)litros_num[1] << 16) | ((uint32_t)litros_num[2] << 8) | litros_num[3];
      pesos = ((uint32_t)pesos_num[0] << 24) | ((uint32_t)pesos_num[1] << 16) | ((uint32_t)pesos_num[2] << 8) | pesos_num[3];
      display.setTextColor(GxEPD_BLACK);
      display.setFont(&CodenameCoderFree4F_Bold40pt7b);
      display.fillRect(450, 125, 250, 50, GxEPD_WHITE);
      display.setCursor(450, 169);
      display.print(litros / 100);
      display.displayWindow(450, 125, 250, 50);
      //Show price
      display.setTextColor(GxEPD_BLACK);
      display.setFont(&CodenameCoderFree4F_Bold40pt7b);
      display.fillRect(450, 212, 250, 50, GxEPD_WHITE);
      display.setCursor(450, 256);
      display.print(pesos / 100);
      display.displayWindow(450, 212, 250, 50);
      delay(5000);
      STATE = 3;
      break;
    case 3:

      display.init(0);
      display.setFullWindow();
      display.drawImage(BitmapPrinter, 300, 140, 200, 200, false, false, true);
      display.powerOff();
      sleep_ms(20000);
      display.init(0);
      display.setFullWindow();
      display.drawImage(Bitmap800x480_1, 0, 0, 800, 480, false, false, true);
      display.powerOff();
      STATE = 0;
      break;
    default:
      break;
  }

}
