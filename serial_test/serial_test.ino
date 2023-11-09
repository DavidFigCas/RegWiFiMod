

// ----------------------------------------------------- setup
void setup()
{

  /*gpio_init(SDA_MAIN);
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
  digitalWrite(27, 0);*/


  Serial.begin(115200);
  Serial.println("Display Init");
  delay(3000);
  Serial.println("Display Init OK");

  //multicore_launch_core1(core1_blink);
  //Serial.begin(115200);
  //pinMode(27, OUTPUT);
  /*display.epd2.selectSPI(SPI, SPISettings(4000000, MSBFIRST, SPI_MODE0));
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
  //Serial.println("setup done");*/
}


// ----------------------------------------------------- loop
void loop() {
  Serial.print("STATE");//Serial.println(STATE);
  delay(1000);
  /*
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
  }*/

}
