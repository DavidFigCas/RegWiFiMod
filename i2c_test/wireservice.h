#ifndef  WIRESERVICE_H
#define WIRESERVICE_H

#include <Arduino.h>
#include <Wire.h>


#define ADDRESS 0x5E


extern byte RxByte;
extern uint8_t wstatus;
extern bool ask_state;
extern bool ask_price;
extern bool ask_factor;
extern bool ask_name;
extern bool ask_nclient;
extern bool ask_litros;

extern boolean newcommand;
//uint8_t mem_address;
extern uint8_t STATE; //uint8_t wstatus = 0b1000000;
extern volatile uint8_t todo_byte, state_byte, j, error_data;
extern volatile uint8_t price_data[2], litro_data[4], factor_data[2], name_data[42];
extern volatile uint32_t nclient_data;

void I2C_RxHandler(int numBytes);
void I2C_TxHandler(void);
void I2C_Init();


#endif  // 
