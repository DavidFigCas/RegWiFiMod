#ifndef GPSSERVICE_H
#define GPSSERVICE_H

#include "system.h"
extern TinyGPSPlus gps;


void save_gps_log();
void gps_init();
void gps_update();
static void printInt(unsigned long val, bool valid, int len);
static void printFloat(float val, bool valid, int len, int prec);
static void smartDelay(unsigned long ms);
static void printDateTime(TinyGPSDate &d, TinyGPSTime &t);
static void printStr(const char *str, int len);


#endif  //
