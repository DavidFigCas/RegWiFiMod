#ifndef PRINERSERVICE_H
#define PRINERSERVICE_H

#include "system.h"

void printReport(uint32_t num, uint32_t ltr, uint32_t unitprice, uint8_t d, uint8_t m, uint8_t y, uint8_t h, uint8_t mn, uint8_t f);
//void printCheck(uint32_t num, uint32_t ltr, uint32_t unitprice, uint8_t d, uint8_t m, uint8_t y, uint8_t h, uint8_t mn, uint8_t f);
void printCheck(uint32_t num, uint32_t ltr, uint32_t unitprice, uint8_t f, uint32_t start_timestamp, uint32_t end_timestamp);
void printing_logs();
void setPrintMode(uint8_t mode) ;
void printString(const char* str);
void printNumber(uint32_t num);
void convertNumberToWords(uint32_t num, char* resultado);
void printDateTime(uint8_t d, uint8_t m, uint8_t y, uint8_t h, uint8_t mn);
void printPrice(uint32_t price);
void endPrint();

#endif