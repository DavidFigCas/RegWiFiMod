#ifndef ENCODER_SERVICE_H
#define ENDODER_SERVICE_H

#include "system.h"

#include <ESP32Encoder.h>
#include <AS5601.h>



extern ESP32Encoder encoder;
extern AS5601 Sensor;

// ------------------------------ Configs
//extern unsigned long interval;              // Print and send
extern unsigned long MAX_DELTA;                //10 Pulsos detectados en Intervalo2  (DELTA)
//extern unsigned long intervalo;    //ENCODER Intervalo de tiempo (milisegundos)
extern unsigned long t_delta;    //DELTA Intervalo de tiempo (500 milisegundos)
extern unsigned long noDelta_timeSTOP;// Maximo tiempo desde que se detecto STOP_FLOW 60seg
extern uint32_t previous_pulses;
extern uint32_t current;
extern uint32_t total_encoder;
extern volatile uint32_t angle_encoder;
extern bool valve_state;

void open_valve();
void close_valve();
void encoder_init();
void read_encoder();
void print_encoder();
void checkEncoderPulses(void * parameter);

#endif
