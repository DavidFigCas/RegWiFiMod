#include "wireservice.h"

bool ask_state = false;
bool ask_price = false;
bool ask_factor = false;
bool ask_name = false;
bool ask_nclient = false;
bool ask_litros = false;
bool mem_address_written = false;
boolean newcommand = false;
uint8_t mem_address = 0;
uint8_t STATE = 0b10101010; //uint8_t wstatus = 0b1000000;
volatile uint8_t todo_byte = 0, state_byte = 0, j = 0, error_data;

volatile uint8_t price_data[2], litro_data[4], factor_data[2], name_data[42];
volatile uint32_t nclient_data; // nclient_data[4]

// ------------------------------------- Init
void I2C_Init()
{
  //Wire.setClock(50000);
  Serial.println("Start i2c");
  Wire.begin(ADDRESS);
  //Wire.setClock(10000);
  Wire.onReceive(I2C_RxHandler);
  Wire.onRequest(I2C_TxHandler);
  //mem_address_written =  false;

  //nclient_data[0] = 0b10000001;
  //nclient_data[1] = 0b11100111;
  //nclient_data[2] = 0b10111101;
  //nclient_data[3] = 0b10001111;

}

// -------------------------------------- RX
void I2C_RxHandler(int numBytes)
{

  // Read Any Received Data
  Serial.print("---> I2C_RxHandler: ");
  /*if (Wire.available())
    {
    Serial.print("Reading: ");
    mem_address = Wire.read();
    Serial.println(mem_address);
    }*/


  if (!mem_address_written)
  {
    // writes always start with the memory address
    Serial.print("Reading: ");
    mem_address = Wire.read();
    Serial.println(mem_address);

    //mem_address = Wire.read();
    //Serial.print("Get addres: ");
    //Serial.println(mem_address);
    //mem_address_written = true;

    if (mem_address == 0x01)          // --------------------- STATE
    {
      ask_state = true;
      Serial.println("ask state");

    }
    else if (mem_address == 0x05)     // --------------------- price
    {
      ask_price = true;
      Serial.println("ask price");
    }
    else if (mem_address == 0x06)     // --------------------- factor
    {
      ask_factor = true;
      Serial.println("ask factor");
    }

    else if (mem_address == 0x07)     // --------------------- name
    {
      ask_name = true;
      Serial.println("ask name");
      mem_address_written = true;
      return;
    }

    else if (mem_address == 0x03)     // ---------------------- n client
    {
      ask_nclient = true;
      Serial.println("ask number of client");
      //return;

    }

    else if (mem_address == 0x04)     // --------------------- litros
    {
      ask_litros = true;
      Serial.println("ask litros");
    }

    mem_address_written = true;
    Serial.println("Go to response");
    //return;

  }
  else //if ((!ask_state) && (!ask_price) && (!ask_factor) && (!ask_name)  && (!ask_nclient) && (!ask_litros))
  {
    // save into memory
    Serial.print("Saving Regs: ");
    Serial.println(mem_address);
    if (mem_address == 0x02)
    {
      todo_byte = Wire.read();
      newcommand = true;
      Serial.print("New command ToDo: ");
      Serial.println(todo_byte, BIN);
    }
    // ------------------------------- Write Client
    if (mem_address == 0x03)
    {
      for (int i = 0; i < 4; i++) {
        nclient_data = (nclient_data << 8) | Wire.read(); // Lee un byte del bus I2C y lo agrega a nclient_data
      }
      Serial.print("writing client");
      Serial.println(nclient_data, BIN);
      mem_address_written = false;
      return;
    }

    if (mem_address == 0x08)
    {
      error_data = Wire.read();
    }
    //return;
  }
  return;
}



// -------------------------------------- TX
void I2C_TxHandler(void)
{
  Serial.println("I2C_TxHandler --->");
  // --------------------- STATE
  if (ask_state == true)
  {
    Wire.write(STATE);//i2c_write_byte(i2c, STATE);
    //Wire.write(0);//else i2c_write_byte(i2c, 0);
    //Wire.endTransmission(true);
    mem_address_written = false;
    ask_state = false;
    //Wire.endTransmission();
    Serial.print("state: \t");
    Serial.println(STATE, BIN);
    //delay(10);
    //return;
  }

  // --------------------- price
  else if (ask_price == true) {
    while (ask_price)
    {
      Wire.write(price_data[j]);
      j++;
      if (j >= 2) {
        ask_price = false;
        j = 0;
      }
    }
  }

  // --------------------- factor
  else if (ask_factor == true) {
    while (ask_factor)
    {
      Wire.write(factor_data[j]);//i2c_write_byte(i2c, ltr_data[j]);
      j++;
      if (j >= 2) {
        ask_factor = false;
        j = 0;
      }
    }
  }

  // --------------------- name
  else if (ask_name == true) {
    j = 0;
    Serial.print("name: \t");
    while (ask_name)
    {
      Wire.write(char (name_data[j]));
      Serial.print(name_data[j]);
      j++;
      if (j >= 42) {
        ask_name = false;
        j = 0;
      }
    }
    mem_address_written = false;
    Serial.println("\t name OK");
    return;
  }

  // --------------------- n client
  else if (ask_nclient == true) {
    while (ask_nclient)
    {
      Serial.print("client: \t");
      Serial.print(j);
      Serial.print("\t");
      Serial.println(nclient_data, BIN);

      Wire.write((byte)(nclient_data >> (8 * j))); // Envia el byte correspondiente

      j++;
      if (j >= 4) {
        ask_nclient = false;
        j = 0;
      }
    }
    mem_address_written = false;
  }

  // --------------------- litros
  else if (ask_litros == true) {
    while (ask_litros)
    {
      Wire.write(litro_data[j]);//i2c_write_byte(i2c, ltr_data[j]);
      j++;
      if (j >= 4) {
        ask_litros = false;
        j = 0;
      }
    }
  }

  // --------------------- error
  else
  {
    ask_state = false;
    ask_price = false;
    ask_factor = false;
    ask_name = false;
    ask_nclient = false;
    ask_litros = false;
    j = 0;

    Wire.write(0);//else i2c_write_byte(i2c, 0);
    mem_address_written = false;
    Serial.println("default error");

  }
  //mem_address_written = false;
  //return;
}
