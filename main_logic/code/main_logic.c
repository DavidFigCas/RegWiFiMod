/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

#define SDA_MAIN    16
#define SCL_MAIN    17
#define OPEN_BOX	2
#define LED_1			25
#define LED_2			27
#define LED_3			28
const uint8_t REG_STATUS = 0x01;
//const uint8_t REG_UPRICE = 0x03;
const uint8_t REG_SET_PESOS = 0x03;
const uint8_t REG_SET_LITROS = 0x04;
const uint8_t REG_ACT_LITROS = 0x04;
//const uint8_t REG_SET_PESOS = 0x06;
const uint8_t REG_SET_STATUS = 0x08;
const uint8_t REG_DISPALY_NUMBER = 0x09;

const uint8_t REG_TODO = 0x02;
const uint8_t REG_TARGET=0x04;
const uint8_t REG_COUNTER=0x03;

const uint8_t REG_CLIENT_NUMBER=0x03;
const uint8_t REG_LITROS=0x04;
const uint8_t REG_PRICE=0x05;
const uint8_t REG_FACTOR=0x06;
const uint8_t REG_NOMBRE=0x07;

uint8_t STATUS=0, CONTROL=0, stopper=0, STATE=0;
uint8_t readBuffer[1];
uint8_t temp_data;

uint32_t unitprice = 950, factor=1;
uint32_t converter = 368;
uint32_t litros_set=0, pesos=0, litros_act=0, target =0;
uint32_t client_number = 0;

uint32_t litros_print, pesos_print, litros;

uint8_t buffer[4];
uint8_t folio = 3, err_data=0;
// Глобальная переменная для хранения времени в миллисекундах
volatile uint32_t milliseconds = 0;
uint32_t currentMillis;

bool display_board=0, power_board=0, control_board=0, printer_board=0, internet_board=0, no_service=0;
bool bit_value=0;
char nombre[42];
const char* unidades[] = {"", "uno", "dos", "tres", "cuatro", "cinco", "seis", "siete", "ocho", "nueve"};
const char* decenas[] = {"", "diez", "veinte", "treinta", "cuarenta", "cincuenta", "sesenta", "setenta", "ochenta", "noventa"};
const char* especiales[] = {"diez", "once", "doce", "trece", "catorce", "quince"};

void printCheck (uint32_t num, uint32_t ltr){
	//char* resultado = "";
	char resultado[150];
	const char* Total = "TOTAL   $";
	const char  end1 = '\r';
	const char  end2 = '\n';
	uint8_t resultadoBytes[100];
	uint8_t tempVar=0;
	char tempChar;
	uint32_t tempnum=0;
	//Set double size
	tempVar = 0x1B;
	i2c_write_blocking(i2c0, 0x5D, &tempVar, 1, false);
	tempVar = 0x21;
	i2c_write_blocking(i2c0, 0x5D, &tempVar, 1, false);
	tempVar = 48;
	i2c_write_blocking(i2c0, 0x5D, &tempVar, 1, false);
	//Send  "Total"
	
	strncpy((char*)resultadoBytes, Total, 9);
	//resultadoBytes[BUFFER_SIZE - 1] = '\0';
	i2c_write_blocking(i2c0, 0x5D, resultadoBytes, 9, false);
	sprintf(resultado, "%u", num);
	size_t size = strlen(resultado);
	strncpy((char*)resultadoBytes, resultado, size);
	i2c_write_blocking(i2c0, 0x5D, resultadoBytes, size, false);
	/*
	tempVar = (uint8_t)(num);
	if (tempVar>0){
		tempChar = (char)tempVar;
		i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);	
	}
	tempnum = num%1000;
	tempVar = (uint8_t)(tempnum/100);
	tempChar = (char)tempVar;
	i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);
	tempnum = num%100;
	tempVar = (uint8_t)(tempnum/10);
	tempChar = (char)tempVar;
	i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);
	tempVar = (uint8_t)(tempnum%10);
	tempChar = (char)tempVar;
	i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);
	*/
	
	//Send end of string
	//tempVar = (uint8_t)(end1);
	tempChar = end1;
	i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);
	//tempVar = (uint8_t)(end2);
	tempChar = end2;
	i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);
	
	//Reset double size
	tempVar = 0x1B;
	i2c_write_blocking(i2c0, 0x5D, &tempVar, 1, false);
	tempVar = 0x21;
	i2c_write_blocking(i2c0, 0x5D, &tempVar, 1, false);
	tempVar = 0;
	i2c_write_blocking(i2c0, 0x5D, &tempVar, 1, false);
	
	if (num == 0) {
		strcpy(resultado, "cero");
		//resultado = "cero";
	}
	if (num >= 1000) {
		int miles = num / 1000;
		strcat(resultado, unidades[miles]);
		strcat(resultado, " mil ");
		//resultado += unidades[miles];
		//resultado += " mil ";
		num %= 1000;
	}
	 // Обработка сотен
	if (num >= 200) {
		int centenas = num / 100;
		strcat(resultado, unidades[centenas]);
		strcat(resultado, " cientos ");
		//resultado += unidades[centenas];
		//	resultado += " cientos ";
		num %= 100;
	}
	else if (num > 100){
		strcat(resultado, "ciento ");
		//resultado += "ciento ";
		num %= 100;
	}
	else if (num==100){
		strcat(resultado, "cien");
		//resultado += "cien";
		num %= 100;
	}

  // Обработка десятков
	if (num > 29) {
		int decena = num / 10;
		strcat(resultado, decenas[decena]);
		strcat(resultado, " ");
		//resultado += decenas[decena];
		//resultado += " ";
		num %= 10;
		if (num > 0){
			strcat(resultado, "y ");
			//resultado += "y ";
		}
	}
	else if (num > 20){
		strcat(resultado, "veinti");
		//resultado += "veinti";
		num %= 10;
	}
	else if (num==20){
		strcat(resultado, "veinte");
		//resultado += "veinte";
		num %= 10;
	}
	else if (num > 15){
		strcat(resultado, "dieci");
		strcat(resultado, unidades[num - 10]);
		//resultado += "dieci";
		//resultado +=  unidades[num - 10];
		num %= 10;
	}
	else if (num >=11){
		strcat(resultado, especiales[num - 10]);
		//strcat(resultado, " ");
		//resultado += especiales[num - 10];
		num %= 10;
	}
	else if (num==10){
		strcat(resultado, "diez");
		//resultado += "diez";
		num %= 10;
	}

  // Обработка единиц
	if (num > 0) {
		strcat(resultado, unidades[num]);
		//resultado += unidades[num];
	}
	size = strlen(resultado);
	strncpy((char*)resultadoBytes, resultado, size);
	i2c_write_blocking(i2c0, 0x5D, resultadoBytes, size, false);
	//Send end of string
	//tempVar = (uint8_t)(end1);
	//i2c_write_blocking(i2c0, 0x5D, &tempVar, 1, false);
	//tempVar = (uint8_t)(end2);
	//i2c_write_blocking(i2c0, 0x5D, &tempVar, 1, false);
	
	//Send end of string
	tempChar = end1;
	i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);
	tempChar = end2;
	i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);
	
	strcpy(resultado, "Precio U.   $");
	size = strlen(resultado);
	strncpy((char*)resultadoBytes, resultado, size);
	i2c_write_blocking(i2c0, 0x5D, resultadoBytes, size, false);
	//resultado = "Precio U.   $";
	
	tempVar = (uint8_t)(unitprice);
	if (tempVar>0){
		tempChar = (char)tempVar;
		i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);	
	}
	tempnum = unitprice%1000;
	tempVar = (uint8_t)(tempnum/100);
	tempChar = (char)tempVar;
	i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);
	tempnum = num%100;
	tempVar = (uint8_t)(tempnum/10);
	tempChar = (char)tempVar;
	i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);
	tempVar = (uint8_t)(tempnum%10);
	tempChar = (char)tempVar;
	i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);
	//Send end of string
	//Send end of string
	tempChar = end1;
	i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);
	tempChar = end2;
	i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);
	// need to add converter from ubnixtime to string
	strcpy(resultado, "21/09/23  10:23");
	//resultado = "21/09/23  10:23";
	size = strlen(resultado);
	strncpy((char*)resultadoBytes, resultado, size);
	i2c_write_blocking(i2c0, 0x5D, resultadoBytes, size, false);
	//Send end of string
	tempChar = end1;
	i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);
	tempChar = end2;
	i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);
	strcpy(resultado, "Unit 002");
	//resultado = "Unit 002";
	size = strlen(resultado);
	strncpy((char*)resultadoBytes, resultado, size);
	i2c_write_blocking(i2c0, 0x5D, resultadoBytes, size, false);
	//Send end of string
	tempChar = end1;
	i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);
	tempChar = end2;
	i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);
	
	strcpy(resultado, "Folio  ");
	//resultado = "Folio    1";
	size = strlen(resultado);
	strncpy((char*)resultadoBytes, resultado, size);
	i2c_write_blocking(i2c0, 0x5D, resultadoBytes, size, false);
	sprintf(resultado, "%u", folio);
	size = strlen(resultado);
	strncpy((char*)resultadoBytes, resultado, size);
	i2c_write_blocking(i2c0, 0x5D, resultadoBytes, size, false);
	
	//tempChar = (char)folio;
	//i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);
	//Send end of string
	tempChar = end1;
	i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);
	tempChar = end2;
	i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);

//Set double size
	tempVar = 0x1B;
	i2c_write_blocking(i2c0, 0x5D, &tempVar, 1, false);
	tempVar = 0x21;
	i2c_write_blocking(i2c0, 0x5D, &tempVar, 1, false);
	tempVar = 48;
	i2c_write_blocking(i2c0, 0x5D, &tempVar, 1, false);

	strcpy(resultado, "LITROS   ");
	//resultado = "LITROS   ";
	size = strlen(resultado);
	strncpy((char*)resultadoBytes, resultado, size);
	i2c_write_blocking(i2c0, 0x5D, resultadoBytes, size, false);
	
	sprintf(resultado, "%u", ltr);
	size = strlen(resultado);
	strncpy((char*)resultadoBytes, resultado, size);
	i2c_write_blocking(i2c0, 0x5D, resultadoBytes, size, false);
	
	/*
	tempVar = (uint8_t)(ltr);
	if (tempVar>0){
		tempChar = (char)tempVar;
		i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);	
	}
	tempnum = num%1000;
	tempVar = (uint8_t)(tempnum/100);
	tempChar = (char)tempVar;
	i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);
	tempnum = num%100;
	tempVar = (uint8_t)(tempnum/10);
	tempChar = (char)tempVar;
	i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);
	tempVar = (uint8_t)(tempnum%10);
	tempChar = (char)tempVar;
	i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);
	*/ 
	//Send end of string
	//Send end of string
	tempChar = end1;
	i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);
	tempChar = end2;
	i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);
	
	//Reset double size
	tempVar = 0x1B;
	i2c_write_blocking(i2c0, 0x5D, &tempVar, 1, false);
	tempVar = 0x21;
	i2c_write_blocking(i2c0, 0x5D, &tempVar, 1, false);
	tempVar = 0;
	i2c_write_blocking(i2c0, 0x5D, &tempVar, 1, false);
	
	strcpy(resultado, "Gracias por su preferencia");
	//resultado = "Gracias por su preferencia";
	size = strlen(resultado);
	strncpy((char*)resultadoBytes, resultado, size);
	i2c_write_blocking(i2c0, 0x5D, resultadoBytes, size, false);
	//Send end of string
	tempChar = end1;
	i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);
	tempChar = end2;
	i2c_write_blocking(i2c0, 0x5D, &tempChar, 1, false);;
	
	//end print
	tempVar = 0x1D;
	i2c_write_blocking(i2c0, 0x5D, &tempVar, 1, false);
	tempVar = (uint8_t)('V');
	i2c_write_blocking(i2c0, 0x5D, &tempVar, 1, false);
	tempVar = 0x66;
	i2c_write_blocking(i2c0, 0x5D, &tempVar, 1, false);
	tempVar = 0xA;
	i2c_write_blocking(i2c0, 0x5D, &tempVar, 1, false);

}
bool timer_callback( repeating_timer_t *rt ){ 
    milliseconds++;
}


bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void check_status (void){
	//check status register of internet board	
	i2c_write_blocking(i2c0, 0x5E, &REG_STATUS, 1, true);//check STATUS of internet board
	//uint8_t readBuffer[1];
	i2c_read_blocking(i2c0, 0x5E, readBuffer, 1, false);
	temp_data = readBuffer[0];
	printf("ret \n");
    	printf("%u",temp_data);
	bit_value = (temp_data >> 6) & 0x01; //check internet_connection
	if (bit_value == 0) err_data &= ~(1 << 7);
	else err_data |= (1 << 7);
	bit_value = (temp_data >> 5) & 0x01; //check gps connection
	if (bit_value == 0) err_data &= ~(1 << 3);
	else err_data |= (1 << 3);
// check solenoid
	sleep_ms(100);
	i2c_write_blocking(i2c0, 0x5C, &REG_STATUS, 1, true);//check STATUS of control board
	i2c_read_blocking(i2c0, 0x5C, readBuffer, 1, false);
	temp_data = readBuffer[0];
	bit_value = (temp_data >> 2) & 0x01; //check solenoid connection
	if (bit_value == 0) err_data &= ~(1 << 6);
	else err_data |= (1 << 6);
// check printer
	sleep_ms(100);
	i2c_write_blocking(i2c0, 0x5D, &REG_STATUS, 1, true);//check STATUS of printer board
	i2c_read_blocking(i2c0, 0x5D, readBuffer, 1, false);
	temp_data = readBuffer[0];
	bit_value = (temp_data >> 6) & 0x01; //check printer connection
	if (bit_value == 0) err_data &= ~(1 << 5);
	else err_data |= (1 << 5);
	bit_value = (temp_data >> 5) & 0x01; //check paper
	if (bit_value == 0) err_data &= ~(1 << 4);
	else err_data |= (1 << 4);
// send data to display board for show statuses
	sleep_ms(100);
	buffer[0] = err_data;
	i2c_write_blocking(i2c0, 0x5A, &REG_SET_STATUS, 1, true);
	i2c_write_blocking(i2c0, 0x5A, buffer, 1, false);
	sleep_ms(100);
	i2c_write_blocking(i2c0, 0x5E, &REG_SET_STATUS, 1, true);
	i2c_write_blocking(i2c0, 0x5E, buffer, 1, false);
}

uint8_t test[4] = {10,20,30,40};
const uint8_t addr=0x04;
int main() {
	static repeating_timer_t timer;

	folio=0;
    stdio_init_all();
 
    gpio_init(OPEN_BOX);
	gpio_set_dir(OPEN_BOX, GPIO_IN);
	gpio_pull_up(OPEN_BOX);
    
     gpio_init(LED_1);
	gpio_set_dir(LED_1, GPIO_OUT);
	gpio_put(LED_1, 0);
	
	gpio_init(LED_2);
	gpio_set_dir(LED_2, GPIO_OUT);
	gpio_put(LED_2, 0);
	
	gpio_init(LED_3);
	gpio_set_dir(LED_3, GPIO_OUT);
	gpio_put(LED_3, 0);
    
	i2c_init(i2c0, 100 * 1000);
  // configure I2C0 for slave mode
	gpio_init(SDA_MAIN);
	gpio_init(SCL_MAIN);
	gpio_set_function(SDA_MAIN, GPIO_FUNC_I2C);
	gpio_set_function(SCL_MAIN, GPIO_FUNC_I2C);
	gpio_pull_up(SDA_MAIN);
	gpio_pull_up(SCL_MAIN);
	gpio_put(LED_1, 1);
	
	printf("Hello, world!\n");
	sleep_ms(10000);
	printf("Hello, world 2!\n");

	
	//check configuration of sistem
/*	
	for (int addr = 0x50; addr < (1 << 7); ++addr) {
        if (addr % 16 == 0) {
            printf("%02x ", addr);
        }

        // Perform a 1-byte dummy read from the probe address. If a slave
        // acknowledges this address, the function returns the number of bytes
        // transferred. If the address byte is ignored, the function returns
        // -1.

        // Skip over any reserved addresses.
        int ret;
        uint8_t rxdata;
        if (reserved_addr(addr))
            ret = PICO_ERROR_GENERIC;
        else
            ret = i2c_read_blocking(i2c0, addr, &rxdata, 1, false);
		if (ret>=0){
			switch(addr){
				case 0x5A:
					display_board=1;
					break;
				case 0x5B:
					power_board=1;
					break;
				case 0x5C:
					control_board=1;
					err_data |= (1 << 1);//place 1 if control board works
					break;
				case 0x5D:
					printer_board=1;
					err_data |= (1 << 5);//place 1 if printerboard connected
					break;
				case 0x5E:
					internet_board=1;
					gpio_put(LED_1, 1);
				default:
					break;
			}
		}
    }
    */
    gpio_put(LED_2, 1);
    int ret;
    uint8_t rxdata;
    i2c_write_blocking(i2c0, 0x5A, &REG_STATUS, 1, true);
    ret = i2c_read_blocking(i2c0, 0x5A, &rxdata, 1, false);
    printf("ret \n");
    printf("%u",ret);
    if (ret>=0) display_board=1;
    gpio_put(LED_2, 0);
    sleep_ms(100);
    gpio_put(LED_2, 1);
    i2c_write_blocking(i2c0, 0x5B, &REG_STATUS, 1, true);
    ret = i2c_read_blocking(i2c0, 0x5B, &rxdata, 1, false);
    if (ret>=0) power_board=1;
    gpio_put(LED_2, 0);
    sleep_ms(100);
    gpio_put(LED_2, 1);
    i2c_write_blocking(i2c0, 0x5C, &REG_STATUS, 1, true);
    ret = i2c_read_blocking(i2c0, 0x5C, &rxdata, 1, false);
    if (ret>=0) {
		control_board=1;
		err_data |= (1 << 1);//place 1 if control board works
	}
	gpio_put(LED_2, 0);
	sleep_ms(100);
	gpio_put(LED_2, 1);
	i2c_write_blocking(i2c0, 0x5D, &REG_STATUS, 1, true);
	ret = i2c_read_blocking(i2c0, 0x5D, &rxdata, 1, false);
    if (ret>=0) {
		printer_board=1;
		err_data |= (1 << 5);//place 1 if control board works
	}
	gpio_put(LED_2, 0);
	sleep_ms(100);
	gpio_put(LED_2, 1);
	i2c_write_blocking(i2c0, 0x5E, &REG_STATUS, 1, true);
	ret = i2c_read_blocking(i2c0, 0x5E, &rxdata, 1, false);
    if (ret>=0) internet_board=1;
    gpio_put(LED_2, 0);
    //sleep_ms(100);
    //gpio_put(LED_2, 1);
    //i2c_write_blocking(i2c0, 0x5A, &REG_STATUS, 1, true);
    //ret = i2c_read_blocking(i2c0, 0x5A, &rxdata, 1, false);
    //if (ret>=0) display_board=1;
	//gpio_put(LED_2, 0);
	
	sleep_ms(1000);

    //check other modules
    if ((power_board==0)||(control_board==0)||(internet_board==0)){
		no_service=1;
	}
	
	//check box status
	if(gpio_get(OPEN_BOX)==0){
		err_data &= ~(1 << 0);//place 0 to err_register
		no_service=1;
	}
	
	check_status();
	
	//reset counter
	buffer[0] = 160;
	i2c_write_blocking(i2c0, 0x5C, &REG_TODO, 1, true);
	i2c_write_blocking(i2c0, 0x5C, buffer, 1, false);
	/*
	if (display_board==1){
		buffer[1] = unitprice & 0xFF;         // extract least significant byte
		buffer[0] = (unitprice >> 8) & 0xFF;  // extract most significant byte
		i2c_write_blocking(i2c0, 0x5A, &REG_UPRICE, 1, true);//write new Unit Price
		i2c_write_blocking(i2c0, 0x5A, buffer, 2, true);
	}
	*/
	//printCheck(345, 38);
	//Sets a repeating timer to call a function every 1000 milliseconds
    //add_repeating_timer_ms( int interval_in_milliseconds, ?&function to call?, ???, ?timer identifier? );
    add_repeating_timer_ms( 1, &timer_callback, NULL, &timer );

	currentMillis = milliseconds;
	sleep_ms(10000);
    while (1) {
		// Check some errors every time
		//openbox
		
		printf("Hello, world 3!\n");
		
		if(gpio_get(OPEN_BOX)==0){
			err_data &= ~(1 << 0);//place 0 to err_register
			no_service=1;
		}
		printf("%i",STATE);
		switch (STATE){
				case 0:
					if ((milliseconds-currentMillis)>=10000){	//check error status every 5000ms
						check_status();
						currentMillis=milliseconds;
					}
					i2c_write_blocking(i2c0, 0x5C, &REG_STATUS, 1, true);//check STATUS of control board
					i2c_read_blocking(i2c0, 0x5C, readBuffer, 1, false);
					temp_data = readBuffer[0];
					bit_value = (temp_data >> 5) & 0x01; //check data from encoder
					if (bit_value == 1) {
						client_number=0;
						gpio_put(LED_3, 1);
						STATE=1;
					}
					sleep_ms(100);
					/*
					i2c_write_blocking(i2c0, 0x5E, &REG_STATUS, 1, true);//check STATUS of internet board
					i2c_read_blocking(i2c0, 0x5E, readBuffer, 1, false);
					temp_data = readBuffer[0];
					bit_value = (temp_data >> 3) & 0x01; //if GPS find client
					if (bit_value == 1) STATE=5;
					
					i2c_write_blocking(i2c0, 0x5A, &REG_STATUS, 1, true);//check STATUS of display board
					i2c_read_blocking(i2c0, 0x5A, readBuffer, 1, false);
					if (readBuffer[0] == 3) STATE=2;// display wait number of client
					* */
					break;
				case 1:
					//ask uprice
					//i2c_write_blocking(i2c0, 0x5E, &REG_PRICE, 1, true);
					//i2c_read_blocking(i2c0, 0x5E, buffer, 4, false);
					//unitprice = ((uint32_t)buffer[0] << 24) | ((uint32_t)buffer[1] << 16) | ((uint32_t)buffer[2] << 8) | buffer[3];
					//ask factor
					//i2c_write_blocking(i2c0, 0x5E, &REG_FACTOR, 1, true);
					//i2c_read_blocking(i2c0, 0x5E, buffer, 4, false);
					//factor = unitprice = ((uint32_t)buffer[0] << 24) | ((uint32_t)buffer[1] << 16) | ((uint32_t)buffer[2] << 8) | buffer[3];
					//read data from encoder
					i2c_write_blocking(i2c0, 0x5C, &REG_COUNTER, 1, true);
					i2c_read_blocking(i2c0, 0x5C, buffer, 4, false);
					litros = ((uint32_t)buffer[0] << 24) | ((uint32_t)buffer[1] << 16) | ((uint32_t)buffer[2] << 8) | buffer[3];
					//convert counters to litros and calculate price
					litros = (litros*100)/converter;
					litros = litros*factor;
					pesos = (litros*unitprice)/100;
					sleep_ms(10);
					//place dta to degisters and send todo byte to display board
					buffer[0] = (litros >> 24) & 0xFF;  // extract the most significant byte
					buffer[1] = (litros >> 16) & 0xFF;  // extract the second byte
					buffer[2] = (litros >> 8) & 0xFF;   // extract the third byte
					buffer[3] = litros & 0xFF;          // extract the least significant byte
					i2c_write_blocking(i2c0, 0x5A, &REG_SET_LITROS, 1, true);
					i2c_write_blocking(i2c0, 0x5A, buffer, 4, false);
					sleep_ms(10);
					buffer[0] = (pesos >> 24) & 0xFF;  // extract the most significant byte
					buffer[1] = (pesos >> 16) & 0xFF;  // extract the second byte
					buffer[2] = (pesos >> 8) & 0xFF;   // extract the third byte
					buffer[3] = pesos & 0xFF;          // extract the least significant byte
					i2c_write_blocking(i2c0, 0x5A, &REG_SET_PESOS, 1, true);
					i2c_write_blocking(i2c0, 0x5A, buffer, 4, false);
					//check todo byte
					sleep_ms(10);
					i2c_write_blocking(i2c0, 0x5A, &REG_TODO, 1, true);
					i2c_read_blocking(i2c0, 0x5A, buffer, 1, false);
					temp_data = readBuffer[0];
					temp_data |= (1 << 6); //start process
					buffer[0] = temp_data;
					sleep_ms(10);
					i2c_write_blocking(i2c0, 0x5A, &REG_TODO, 1, true);
					i2c_write_blocking(i2c0, 0x5A, buffer, 1, false);
					STATE = 2;
					currentMillis=milliseconds;
					break;
				case 2:
					/*					
					if ((milliseconds-currentMillis)>=10000){	//check error status every 500ms
						check_status();
						currentMillis=milliseconds;
					}
					* */
					if ((milliseconds-currentMillis)>=500){	//check actual number of liters
						i2c_write_blocking(i2c0, 0x5C, &REG_COUNTER, 1, true);
						i2c_read_blocking(i2c0, 0x5C, buffer, 4, false);
						litros = ((uint32_t)buffer[0] << 24) | ((uint32_t)buffer[1] << 16) | ((uint32_t)buffer[2] << 8) | buffer[3];
					//convert counters to litros and calculate price
						litros = (litros*100)/converter;
						litros = litros*factor;
						pesos = (litros*unitprice)/100;
						sleep_ms(10);
					//place dta to degisters and send todo byte to display board
						buffer[0] = (litros >> 24) & 0xFF;  // extract the most significant byte
						buffer[1] = (litros >> 16) & 0xFF;  // extract the second byte
						buffer[2] = (litros >> 8) & 0xFF;   // extract the third byte
						buffer[3] = litros & 0xFF;          // extract the least significant byte
						i2c_write_blocking(i2c0, 0x5A, &REG_SET_LITROS, 1, true);
						i2c_write_blocking(i2c0, 0x5A, buffer, 4, false);
						sleep_ms(10);
						buffer[0] = (pesos >> 24) & 0xFF;  // extract the most significant byte
						buffer[1] = (pesos >> 16) & 0xFF;  // extract the second byte
						buffer[2] = (pesos >> 8) & 0xFF;   // extract the third byte
						buffer[3] = pesos & 0xFF;          // extract the least significant byte
						i2c_write_blocking(i2c0, 0x5A, &REG_SET_PESOS, 1, true);
						i2c_write_blocking(i2c0, 0x5A, buffer, 4, false);
						sleep_ms(10);
					//check todo byte
						i2c_write_blocking(i2c0, 0x5A, &REG_TODO, 1, true);
						i2c_read_blocking(i2c0, 0x5A, buffer, 1, false);
						temp_data = readBuffer[0];
						temp_data |= (1 << 6); //start process
						buffer[0] = temp_data;
						sleep_ms(10);
						i2c_write_blocking(i2c0, 0x5A, &REG_TODO, 1, true);
						i2c_write_blocking(i2c0, 0x5A, buffer, 1, false);
						
						i2c_write_blocking(i2c0, 0x5C, &REG_STATUS, 1, true);//check STATUS of control board
						i2c_read_blocking(i2c0, 0x5C, readBuffer, 1, false);
						temp_data = readBuffer[0];
						bit_value = (temp_data >> 4) & 0x01; //if process stop
						if (bit_value == 0){
							i2c_write_blocking(i2c0, 0x5A, &REG_TODO, 1, true);
							i2c_read_blocking(i2c0, 0x5A, buffer, 1, false);
							temp_data = readBuffer[0];
							temp_data &= ~((1 << 6));//stop process
							buffer[0] = temp_data;
							i2c_write_blocking(i2c0, 0x5A, &REG_TODO, 1, true);
							i2c_write_blocking(i2c0, 0x5A, buffer, 1, false);
							buffer[0] = 160;
							i2c_write_blocking(i2c0, 0x5C, &REG_TODO, 1, true);
							i2c_write_blocking(i2c0, 0x5C, buffer, 1, false);
							STATE = 3;
						}
						currentMillis=milliseconds;
					}
					/*
					i2c_write_blocking(i2c0, 0x5C, &REG_STATUS, 1, true);//check STATUS of control board
					i2c_read_blocking(i2c0, 0x5C, readBuffer, 1, false);
					temp_data = readBuffer[0];
					bit_value = (temp_data >> 4) & 0x01; //if process stop
					if (bit_value == 0){
						i2c_write_blocking(i2c0, 0x5A, &REG_TODO, 1, true);
						i2c_read_blocking(i2c0, 0x5A, buffer, 1, false);
						temp_data = readBuffer[0];
						temp_data &= ~((1 << 6));//stop process
						buffer[0] = temp_data;
						i2c_write_blocking(i2c0, 0x5A, &REG_TODO, 1, true);
						i2c_write_blocking(i2c0, 0x5A, buffer, 1, false);
						buffer[0] = 160;
						i2c_write_blocking(i2c0, 0x5C, &REG_TODO, 1, true);
						i2c_write_blocking(i2c0, 0x5C, buffer, 1, false);
						STATE = 3;
					}
					sleep_ms(100);
					* */
					break;
				case 3:
					gpio_put(LED_3, 0);
					pesos_print = pesos/100;
					litros_print = litros/100;
					printCheck (pesos_print, litros_print);
					folio++;
					STATE = 0;
					break;
		}		
		
    }
  }

