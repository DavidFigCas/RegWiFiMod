#include "wireservice.h"

volatile uint32_t nclient;

uint8_t mem_address = 0, STATE = 0;
volatile uint8_t todo_byte = 0b10001001, state_byte = 0b10010101, error_byte = 0, j = 0;
boolean new_num = 0, printer = 0, valve = 0, OK = 0, DEL = 0, stopCommand = 0, mem_address_written = 0;
boolean ask_name = 0, ask_factor = 0, ask_nclient = 0, ask_litro = 0, ask_peso = 0, ask_data = 0, ask_state = 0, ask_todo = 0, error_status = 0;
boolean newcommand = 0, new_litros = 0, new_nclient = 0;
volatile uint8_t name_data[42], factor_data[2], nclient_data[4], uprice_data[2], litros_num[4], pesos_num[4], client_num[4], time_num[4];


// ------------------------------------- Init
void I2C_Init()
{


  // configure I2C0 for slave mode
  STATE |= (1 << 7);                  // Module is alaive
  gpio_init(SDA_MAIN);
  gpio_init(SCL_MAIN);
  gpio_set_function(SDA_MAIN, GPIO_FUNC_I2C);
  gpio_set_function(SCL_MAIN, GPIO_FUNC_I2C);
  gpio_pull_up(SDA_MAIN);
  gpio_pull_up(SCL_MAIN);
  i2c_init(i2c0, 100 * 1000);
  // configure I2C0 for slave mode
  i2c_slave_init(i2c0, ADDRESS, &i2c_slave_handler);
  delay(3000);

}



static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
  switch (event) {
    case I2C_SLAVE_RECEIVE: // master has written some data
      if (!mem_address_written) {
        // ------------------------------------------ writes always start with the memory address
        mem_address = i2c_read_byte(i2c);         // ---------state
        if (mem_address == 0x01) {
          ask_state = true;
        }
        else if (mem_address == 0x03) {           // --------- nclient
          ask_nclient = true;
        }
        else if (mem_address == 0x02) {           // ---------- ToDo
          ask_todo = true;
        }
        else if (mem_address == 0x04) {           // ---------- Litros
          ask_litro = true;
        }
        else if (mem_address == 0x05) {           // ---------- Pesos
          ask_peso = true;
        }
        else if (mem_address == 0x06) {           // ---------- Factor
          ask_factor = true;
        }
        else if (mem_address == 0x07) {           // ---------- Nombre
          ask_name = true;
        }
        mem_address_written = true;
      } else {
        // ---------------------------------------------- save into memory
        if (mem_address == 0x08) {
          error_status = true;
          error_byte = i2c_read_byte(i2c);
        }
        else if (mem_address == 0x02) {
          todo_byte = i2c_read_byte(i2c);
          newcommand = true;
        }
        else if (mem_address == 0x03) {
          nclient_data[j] = i2c_read_byte(i2c);
          j++;
          if (j > 4) {
            j = 0;
          }
        }
        /*if (mem_address == 0x04) {
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
          }*/
      }
      break;
    case I2C_SLAVE_REQUEST: // ------------------------ master is requesting data
      // load from memory
      if (ask_state == true) {
        i2c_write_byte(i2c, STATE);            // --------- state
        ask_state = false;
      }
      else if (ask_todo == true) {
        i2c_write_byte(i2c, todo_byte);       // --------- todo
        ask_todo = false;
      }
      else if (ask_litro == true) {
        i2c_write_byte(i2c, litros_num[j]);      // --------- litro
        j++;
        if (j > 4) {
          ask_litro = false;
          j = 0;
        }
      }
      else if (ask_peso == true) {              // --------- pesos
        i2c_write_byte(i2c, pesos_num[j]);
        j++;
        if (j > 2) {
          ask_peso = false;
          j = 0;
        }
      }
      else if (ask_factor == true) {            // --------- factor
        i2c_write_byte(i2c, factor_data[j]);
        j++;
        if (j > 2) {
          ask_factor = false;
          j = 0;
        }
      }
      else if (ask_name == true) {            // --------- name
        i2c_write_byte(i2c, name_data[j]);
        j++;
        if (j > 42) {
          ask_name = false;
          j = 0;
        }
      }
      else i2c_write_byte(i2c, 0);
      break;
    case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
      mem_address_written = false;
      //ask_data = false;
      //ask_state = false;
      j = 0;
      //gpio_put(LED_2, 0);
      // gpio_put(LED_3, 0);
      // gpio_put(LED_1, 1);
      //newData=1;
      break;
    default:
      break;
  }
}
