#ifndef  WIRESERVICE_H
#define WIRESERVICE_H

#include "system.h"


#define ADDRESS 0x5E


extern byte RxByte;
extern uint8_t wstatus;

void I2C_RxHandler(int numBytes);
void I2C_TxHandler(void);
void I2C_Init();


#endif  // 
