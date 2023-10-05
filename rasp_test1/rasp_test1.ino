#include "i2c_fifo.h"
#include "i2c_slave.h"
#include "pico/stdlib.h"

#define I2C_SLAVE_ADDRESS  0x5E

#define SDA_MAIN    4
#define SCL_MAIN    5

//#define SDA_MAIN    16
//#define SCL_MAIN    17

float number = 0, l = 0, p = 0;
boolean new_num = 0, printer = 0, valve = 0, pesos = 0, litros = 0, OK = 0, DEL = 0, stopCommand = 0, mem_address_written = 0, ask_litro = 0, ask_peso = 0, ask_data = 0, ask_state = 0, newcommand = 0;
volatile uint8_t mem_address = 0;
boolean dec = 0;
uint16_t tx, ty, tz1, tz2;
volatile uint8_t STATE = 0b10101010;
volatile uint8_t todo_byte = 0, state_byte = 0, j = 0;
volatile uint8_t ltr_data[4], pes_data[4], uprice_data[4], litro_data[4];
volatile boolean newData = 0;
volatile uint8_t buffer[4];



static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
  switch (event) {

    case I2C_SLAVE_RECEIVE: // master has written some data
      Serial.println("i2c_REC");
      Serial.print("mem_address_written: ");
      Serial.println(mem_address_written);
      if (!mem_address_written)
      {
        // writes always start with the memory address

        mem_address = i2c_read_byte(i2c);
        mem_address_written = true;
        Serial.print("mem_address_written: ");
        Serial.println(mem_address_written);


        Serial.print("Get from addres: ");
        Serial.println(mem_address);

        if (mem_address == 0x01) {
          ask_state = true;
          Serial.println("ask state");
        }
        else if (mem_address == 0x05) {
          ask_litro = true;
        }
        else if (mem_address == 0x06) {
          ask_peso = true;
        }

        //mem_address_written = true;

      }

      else
      {
        // save into memory
        Serial.print("Save to addres: ");
        Serial.println(mem_address);

        if (mem_address == 0x02) {
          todo_byte = i2c_read_byte(i2c);
          Serial.println(todo_byte);
          newcommand = true;
        }
        if (mem_address == 0x03) {

          uprice_data[j] = i2c_read_byte(i2c);
          Serial.println(uprice_data[j]);
          j++;
          if (j > 4) {
            j = 0;
            Serial.println();
          }
        }
        if (mem_address == 0x04) {
          litro_data[j] = i2c_read_byte(i2c);
          j++;
          if (j > 4) {
            j = 0;
          }
        }
      }
      break;

    case I2C_SLAVE_REQUEST: // master is requesting data
      Serial.println("i2c_RQT");
      // load from memory
      if (ask_state == true) {
        i2c_write_byte(i2c, STATE);
        ask_state = false;
        Serial.println("send state");
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
      Serial.println("i2c_FIN");
      mem_address_written = false;
      Serial.print("mem_address_written: ");
      Serial.println(mem_address_written);
      j = 0;
      newData = 1;
      break;
    default:
      break;
  }
}

void setup() {

  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");

  //i2c_write_blocking(i2c0, 0x5A, &REG_TODO, 1, true);
  //i2c_read_blocking(i2c0, 0x5A, buffer, 1, false);

  i2c_init(i2c0, 100 * 1000);
  // configure I2C0 for slave mode
  i2c_slave_init(i2c0, I2C_SLAVE_ADDRESS, &i2c_slave_handler);
  gpio_init(SDA_MAIN);
  gpio_init(SCL_MAIN);
  gpio_set_function(SDA_MAIN, GPIO_FUNC_I2C);
  gpio_set_function(SCL_MAIN, GPIO_FUNC_I2C);
  gpio_pull_up(SDA_MAIN);
  gpio_pull_up(SCL_MAIN);

}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  Serial.println("Testing");

}
