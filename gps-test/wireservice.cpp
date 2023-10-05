#include "wireservice.h"

uint8_t wstatus = 0b1000000;
byte RxByte;

// ------------------------------------- Init
void I2C_Init()
{
  Wire.begin(ADDRESS);
  Wire.onReceive(I2C_RxHandler);
  Wire.onRequest(I2C_TxHandler);
}

// -------------------------------------- RX
void I2C_RxHandler(int numBytes)
{
  while (Wire.available()) { // Read Any Received Data
    RxByte = Wire.read();

    if (!mem_address_written) {
      // writes always start with the memory address
      mem_address = RxByte;
      if (mem_address == 0x01) {
        ask_state = true;
      }
      else if (mem_address == 0x05) {
        ask_price = true;
      }
      else if (mem_address == 0x06) {
        ask_factor = true;
      }

      else if (mem_address == 0x07) {
        ask_name = true;
      }

      else if (mem_address == 0x03) {
        ask_nclient = true;
      }

      else if (mem_address == 0x04) {
        ask_litros = true;
      }

      
      
      mem_address_written = true;
    } else {
      // save into memory
      if (mem_address == 0x02) {
        todo_byte = i2c_read_byte(i2c);
        newcommand = true;
      }
      if (mem_address == 0x03) {
        nclient_data[j] = i2c_read_byte(i2c);
        j++;
        if (j > 4) {
          j = 0;
        }
      }
      if (mem_address == 0x08) {
        error_data[j] = i2c_read_byte(i2c);
        //j++;
        //if (j > 1) {
        //  j = 0;
        //}
      }
    }

    Serial.write(RxByte);
  }
}



// -------------------------------------- TX
void I2C_TxHandler(void)
{
  Wire.write(wstatus);
}
