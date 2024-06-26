#ifndef  WIRESERVICE_H
#define WIRESERVICE_H

#include "system.h"




#define ENCODE_ADD  0x5C
#define DISPLAY_ADD  0x5A
#define TIME_SPACE  10
#define TIMEOUT_MS 100 // Timeout de 1000 ms

#define SDA_MAIN    16
#define SCL_MAIN    17


// Manejador de tarea para la tarea WiFi
extern TaskHandle_t wifiTaskHandle;

void I2C_Init();
void I2C_Get();
void I2C_Put();
bool i2cRequestWithTimeout(uint8_t address, uint8_t numBytes) ;
void I2C_GetTO();


#endif  // 
