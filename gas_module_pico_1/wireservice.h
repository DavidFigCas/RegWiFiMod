#ifndef  WIRESERVICE_H
#define WIRESERVICE_H

#include "system.h"


#define ADDRESS 0x5E
#define SDA_MAIN    4
#define SCL_MAIN    5


extern byte RxByte;
extern uint8_t wstatus;
extern bool ask_state;
extern bool ask_price;
extern bool ask_factor;
extern bool ask_name;
extern bool ask_nclient;
extern bool ask_litros;
extern bool ask_peso;
extern bool ask_data;

extern boolean newcommand;
extern volatile boolean newData;
//uint8_t mem_address;
extern uint8_t STATE; //uint8_t wstatus = 0b1000000;
extern volatile uint8_t todo_byte, state_byte, j, error_data;
extern volatile uint8_t price_data[2], litro_data[4], factor_data[2], name_data[42],uprice_data[4],ltr_data[4],pes_data[4], nclient_data[4];
//extern volatile uint32_t nclient_data;
//volatile uint8_t ltr_data[4], pes_data[4], uprice_data[4], litro_data[4];

void I2C_Init();
static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event);


#endif  // 
