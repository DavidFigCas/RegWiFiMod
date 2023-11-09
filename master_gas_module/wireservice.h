#ifndef  WIRESERVICE_H
#define WIRESERVICE_H

#include "system.h"


#define ENCODE_ADD  0x5C
#define DISPLAY_ADD  0x5A
#define TIME_SPACE  10

#define SDA_MAIN    16
#define SCL_MAIN    17

void I2C_Init();
void I2C_Get();
void I2C_Put();

#endif  // 
