#ifndef  WIRESERVICE_H
#define WIRESERVICE_H

#include "system.h"


#define ADDRESS 0x5E

#define SDA_MAIN    16
#define SCL_MAIN    17

extern volatile uint32_t nclient;
extern volatile uint8_t mem_address, STATE;
extern volatile uint8_t todo_byte, state_byte, error_byte, j;
extern volatile boolean new_num, printer, valve, OK, DEL, stopCommand, mem_address_written;
extern volatile boolean ask_name, ask_factor, ask_nclien, ask_litro, ask_peso, ask_data, ask_state, ask_todo, error_status;
extern volatile boolean newcommand, new_litros, new_nclient;
extern volatile uint8_t name_data[42], factor_data[2], nclient_data[4], uprice_data[2], litros_num[4], pesos_num[4], client_num[4], time_num[4];



void I2C_Init();
//static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event);


#endif  // 
