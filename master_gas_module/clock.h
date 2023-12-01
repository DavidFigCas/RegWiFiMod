#ifndef CLOCK_H
#define CLOCK_H

#include "system.h"

//#define SDA1_PIN 10  // Cambia esto al pin que desees usar para SDA
//#define SCL1_PIN 11  // Cambia esto al pin que desees usar para SCL

extern char daysOfTheWeek[7][12];
extern DateTime now;
extern DateTime last_ac;
extern RTC_DS3231 rtc;

extern int dias;
extern int mes;
extern int anio;
extern int dia_hoy;
extern int hora;
extern int minuto;
extern int segundo;

extern const char* ntpServer;
extern int32_t  gmtOffset_sec;
extern int32_t   daylightOffset_sec;
extern bool ntpConnected;
extern bool rtcUpdated;
extern bool rtc_ready;


extern WiFiUDP ntpUDP;
extern NTPClient timeClient;

void update_clock();
void read_clock();
void init_clock();
String DateTimeToString(const DateTime& now);

#endif
