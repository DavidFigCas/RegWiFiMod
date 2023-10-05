#ifndef PINES_H
#define PINES_H

#include "system.h"

// ------------- Pins Definitios
#define ONDDEMANDPIN  0
#define RXD2          16
#define TXD2          17
#define I2C_SDA       21                    
#define I2C_SCL       22

#define I1            36  
#define I2            39
#define I3            34
#define I4            35
#define I5            32
#define I_maq_onoff   33    /////si esta apagada manda 0, encendida 1
#define cont_premios  25  //CONTADOR PREMIOS  en la tarjeta el derecho
#define cont_monedas  26  //CONTADOR MONEDERO  lado izquiero
#define LED           2
#define P_ir          5  //Salida ir
#define relay_prog    15  //Salida ir
#define relay_onoff   4  //Salida ir

#define servo1Pin     18

void pins_init();


#endif  //
