#include "wireservice.h"

bool ask_state = false;
bool ask_price = false;
bool ask_factor = false;
bool ask_name = false;
bool ask_nclient = false;
bool ask_litros = false;
bool ask_peso = false;
bool ask_data = false;
bool mem_address_written = false;
volatile boolean newData = 0;
boolean newcommand = false;
uint8_t mem_address = 0;
uint8_t STATE = 0; //uint8_t wstatus = 0b1000000;
volatile uint8_t todo_byte = 0, state_byte = 0, j = 0, error_data;

volatile uint8_t price_data[2], litro_data[4], factor_data[2], name_data[42], uprice_data[4], ltr_data[4], pes_data[4], nclient_data[4];
//volatile uint32_t nclient_data; // nclient_data[4]
//volatile uint8_t ltr_data[4], pes_data[4], uprice_data[4], litro_data[4];

// ------------------------------------- Init
void I2C_Init()
{

  //Wire.begin(ADDRESS);
  //Wire.onReceive(I2C_RxHandler);
  //mem_address_written =  false;

  // configure I2C0 for slave mode
  //STATE = 1;
  STATE |= (1 << 7);
  i2c_init(i2c0, 100 * 1000);
  i2c_slave_init(i2c0, ADDRESS, &i2c_slave_handler);
  gpio_init(SDA_MAIN);
  gpio_init(SCL_MAIN);
  gpio_set_function(SDA_MAIN, GPIO_FUNC_I2C);
  gpio_set_function(SCL_MAIN, GPIO_FUNC_I2C);
  gpio_pull_up(SDA_MAIN);
  gpio_pull_up(SCL_MAIN);
  Serial.println("i2c_Init");
  

}


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
        //mem_address_written = true;
        Serial.print("mem_address_written: ");
        Serial.println(mem_address_written);


        Serial.print("Get from addres: ");
        Serial.println(mem_address);

        if (mem_address == 0x01) {
          ask_state = true;
          Serial.println("ask state");
        }
        else if (mem_address == 0x05) {
          ask_litros = true;
        }
        else if (mem_address == 0x06) {
          ask_peso = true;
        }

        mem_address_written = true;

      }

      else
      {
        // save into memory
        Serial.print("Save to addres: ");
        Serial.println(mem_address);

        if (mem_address == 0x02) {
          Serial.print("ToDo:");
          todo_byte = i2c_read_byte(i2c);
          Serial.println(todo_byte);
          newcommand = true;
        }
        if (mem_address == 0x03) {
          Serial.print("# client:");
          nclient_data[j] = i2c_read_byte(i2c);
          Serial.println(nclient_data[j]);
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
        Serial.print("send state: ");
        Serial.println(STATE, BIN);
      }
      else if (ask_litros == true) {
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
      //i2c_write_byte(i2c, 0);
      break;
    default:
      break;
  }
}

// -------------------------------------- RX
//void I2C_RxHandler(int numBytes)
//{
//
//  // Read Any Received Data
//  Serial.print("---> I2C_RxHandler: ");
//  /*if (Wire.available())
//    {
//    Serial.print("Reading: ");
//    mem_address = Wire.read();
//    Serial.println(mem_address);
//    }*/
//
//
//  if (!mem_address_written)
//  {
//    // writes always start with the memory address
//    Serial.print("Reading: ");
//    mem_address = Wire.read();
//    Serial.println(mem_address);
//
//    //mem_address = Wire.read();
//    //Serial.print("Get addres: ");
//    //Serial.println(mem_address);
//    mem_address_written = true;
//
//    if (mem_address == 0x01)          // --------------------- STATE
//    {
//      ask_state = true;
//      Serial.println("ask state");
//      //if(Wire.available())
//      {
//        Serial.println(Wire.read());
//      }
//      //Serial.println("ask state");
//
//      Wire.write(STATE);//i2c_write_byte(i2c, STATE);
//
//      //Wire.write(0);//else i2c_write_byte(i2c, 0);
//      //Wire.endTransmission(true);
//      mem_address_written = false;
//      //ask_state = false;
//      //Wire.endTransmission();
//      Serial.print("state: \t");
//      Serial.println(STATE, BIN);
//      delay(10);
//      //mem_address_written = true;
//      return;
//    }
//    else if (mem_address == 0x05)     // --------------------- price
//    {
//      ask_price = true;
//      Serial.println("ask price");
//    }
//    else if (mem_address == 0x06)     // --------------------- factor
//    {
//      ask_factor = true;
//      Serial.println("ask factor");
//    }
//
//    else if (mem_address == 0x07)     // --------------------- name
//    {
//      ask_name = true;
//      Serial.println("ask name");
//      mem_address_written = true;
//      return;
//    }
//
//    else if (mem_address == 0x03)     // ---------------------- n client
//    {
//      ask_nclient = true;
//      Serial.println("ask number of client");
//      mem_address_written = true;
//      return;
//
//    }
//
//    else if (mem_address == 0x04)     // --------------------- litros
//    {
//      ask_litros = true;
//      Serial.println("ask litros");
//    }
//
//    Serial.println("Go to response");
//    //return;
//
//  }
//  else// if ((!ask_state) && (!ask_price) && (!ask_factor) && (!ask_name)  && (!ask_nclient) && (!ask_litros))
//  {
//    // save into memory
//    Serial.print("Saving Regs: ");
//    Serial.println(mem_address);
//    if (mem_address == 0x02)
//    {
//      todo_byte = Wire.read();
//      newcommand = true;
//      Serial.print("New command ToDo: ");
//      Serial.println(todo_byte, BIN);
//    }
//    // ------------------------------- Write Client
//    if (mem_address == 0x03)
//    {
//      for (int i = 0; i < 4; i++) {
//        nclient_data = (nclient_data << 8) | Wire.read(); // Lee un byte del bus I2C y lo agrega a nclient_data
//      }
//      Serial.print("writing client");
//      Serial.println(nclient_data, BIN);
//      mem_address_written = false;
//      return;
//    }
//
//    if (mem_address == 0x08)
//    {
//      error_data = Wire.read();
//    }
//    //return;
//  }
//  return;
//}
//
//
//
//// -------------------------------------- TX
//void I2C_TxHandler(void)
//{
//  Serial.println("I2C_TxHandler --->");
//  // --------------------- STATE
//  if (ask_state == true)
//  {
//    Wire.write(STATE);//i2c_write_byte(i2c, STATE);
//    //Wire.write(0);//else i2c_write_byte(i2c, 0);
//    //Wire.endTransmission(true);
//    mem_address_written = false;
//    ask_state = false;
//    Wire.endTransmission();
//    Serial.print("state: \t");
//    Serial.println(STATE, BIN);
//    delay(10);
//    //return;
//  }
//
//  // --------------------- price
//  else if (ask_price == true) {
//    while (ask_price)
//    {
//      Wire.write(price_data[j]);
//      j++;
//      if (j >= 2) {
//        ask_price = false;
//        j = 0;
//      }
//    }
//  }
//
//  // --------------------- factor
//  else if (ask_factor == true) {
//    while (ask_factor)
//    {
//      Wire.write(factor_data[j]);//i2c_write_byte(i2c, ltr_data[j]);
//      j++;
//      if (j >= 2) {
//        ask_factor = false;
//        j = 0;
//      }
//    }
//  }
//
//  // --------------------- name
//  else if (ask_name == true) {
//    j = 0;
//    Serial.print("name: \t");
//    while (ask_name)
//    {
//      Wire.write(char (name_data[j]));
//      Serial.print(name_data[j]);
//      j++;
//      if (j >= 42) {
//        ask_name = false;
//        j = 0;
//      }
//    }
//    mem_address_written = false;
//    Serial.println("\t name OK");
//    return;
//  }
//
//  // --------------------- n client
//  else if (ask_nclient == true) {
//    while (ask_nclient)
//    {
//      Serial.print("client: \t");
//      Serial.print(j);
//      Serial.print("\t");
//      Serial.println(nclient_data, BIN);
//
//      Wire.write((byte)(nclient_data >> (8 * j))); // Envia el byte correspondiente
//
//      j++;
//      if (j >= 4) {
//        ask_nclient = false;
//        j = 0;
//      }
//    }
//    mem_address_written = false;
//  }
//
//  // --------------------- litros
//  else if (ask_litros == true) {
//    while (ask_litros)
//    {
//      Wire.write(litro_data[j]);//i2c_write_byte(i2c, ltr_data[j]);
//      j++;
//      if (j >= 4) {
//        ask_litros = false;
//        j = 0;
//      }
//    }
//  }
//
//  // --------------------- error
//  else
//  {
//    ask_state = false;
//    ask_price = false;
//    ask_factor = false;
//    ask_name = false;
//    ask_nclient = false;
//    ask_litros = false;
//    j = 0;
//
//    Wire.write(0);//else i2c_write_byte(i2c, 0);
//    mem_address_written = false;
//    Serial.println("default error");
//
//  }
//  //mem_address_written = false;
//  //return;
//}
