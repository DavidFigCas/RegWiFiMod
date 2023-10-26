/**
 * Copyright (c) 2021 pmarques-dev @ github
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "hardware/i2c.h"
#include "i2c_fifo.h"
#include "i2c_slave.h"
#include "quadrature_encoder.pio.h"

//
// ---- quadrature encoder interface example
//
// the PIO program reads phase A/B of a quadrature encoder and increments or
// decrements an internal counter to keep the current absolute step count
// updated. At any point, the main code can query the current count by using
// the quadrature_encoder_*_count functions. The counter is kept in a full
// 32 bit register that just wraps around. Two's complement arithmetic means
// that it can be interpreted as a 32-bit signed or unsigned value, and it will
// work anyway.
//
// As an example, a two wheel robot being controlled at 100Hz, can use two
// state machines to read the two encoders and in the main control loop it can
// simply ask for the current encoder counts to get the absolute step count. It
// can also subtract the values from the last sample to check how many steps
// each wheel as done since the last sample period.
//
// One advantage of this approach is that it requires zero CPU time to keep the
// encoder count updated and because of that it supports very high step rates.
//
#define LED_1			25
#define LED_2			27
#define LED_3			28
#define BTN_START		12
#define BTN_STOP		11
#define SOLENOID		10

#define BUF_LEN         0x100
#define I2C_SLAVE_ADDRESS	0x5C

#define SDA_MAIN		16
#define SCL_MAIN		17
/*
static struct
{
    uint8_t mem[256];
    uint8_t mem_address;
    bool mem_address_written;
} context;
*/
bool mem_address_written=false, ask_data = false, ask_state = false, newcommand = false, START=false, STOP=false, bit=0, act_button=0;
volatile bool timerr=0, flow=0, button=0;
uint8_t mem_address = 0;
volatile uint8_t todo_byte = 0, state_byte = 0, j=0;
volatile uint8_t enc_data[4], targ_data[4];
uint32_t target=0;
volatile int32_t current_value, new_value, delta, old_value = 0, old_value_m = 0;
uint8_t STATE = 0;

static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    switch (event) {
    case I2C_SLAVE_RECEIVE: // master has written some data
        if (!mem_address_written) {
            // writes always start with the memory address
            mem_address = i2c_read_byte(i2c);
            if (mem_address == 0x01){
				ask_state = true;
			}
			else if (mem_address == 0x03){
				ask_data = true;
			}
            mem_address_written = true;
        } else {
            mem_address_written = false;
            // save into memory
            if (mem_address == 0x02){
				todo_byte = i2c_read_byte(i2c);
				newcommand = true;
            }
            if (mem_address == 0x04){
				targ_data[j] = i2c_read_byte(i2c);
				j++;
				if (j>4){
				j=0;
				}
				
            }				
        }
        break;
    case I2C_SLAVE_REQUEST: // master is requesting data
        // load from memory
        mem_address_written = false;
        if (ask_state == true){
			i2c_write_byte(i2c, state_byte);
			ask_state = false;
		}
		else if (ask_data == true){
			i2c_write_byte(i2c, enc_data[j]);
			j++;
			if (j>4){
				ask_data = false;
				j=0;
			}
		}
		else i2c_write_byte(i2c, 0);
        break;
    case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
        //mem_address_written = false;
        //ask_data = false;
        //ask_state = false;
		j=0;
        //gpio_put(LED_2, 0);
       // gpio_put(LED_3, 0);
       // gpio_put(LED_1, 1);
        break;
    default:
        break;
    }
}

bool timer_callback( repeating_timer_t *rt ){ 
	//gpio_put(LED_1, 1);
	timerr = 1;    
}

void gpio_callback(uint gpio, uint32_t events) {
    // Put the GPIO event(s) that just happened into event_str
    // so we can print it
    button=1;
}

int main() {
    
    // Base pin to connect the A phase of the encoder.
    // The B phase must be connected to the next pin
    const uint PIN_AB = 2;
	static repeating_timer_t timer;
    stdio_init_all();
    
         //Init Main I2C
    //i2c_slave_init	(i2c0, main_address, handler);
    
    gpio_init(SDA_MAIN);
    gpio_init(SCL_MAIN);
    gpio_set_function(SDA_MAIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_MAIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_MAIN);
    gpio_pull_up(SCL_MAIN);

    i2c_init(i2c0, 100 * 1000);
    // configure I2C0 for slave mode
    i2c_slave_init(i2c0, I2C_SLAVE_ADDRESS, &i2c_slave_handler);
	
    
    gpio_init(LED_1);
	gpio_set_dir(LED_1, GPIO_OUT);
	gpio_put(LED_1, 0);
	
	gpio_init(LED_2);
	gpio_set_dir(LED_2, GPIO_OUT);
	gpio_put(LED_2, 0);
	
	gpio_init(LED_3);
	gpio_set_dir(LED_3, GPIO_OUT);
	gpio_put(LED_3, 0);
	
	gpio_init(SOLENOID);
	gpio_set_dir(SOLENOID, GPIO_OUT);
	gpio_put(SOLENOID, 0);
	
	gpio_init(BTN_START);
	gpio_set_dir(BTN_START, GPIO_IN);
	gpio_pull_up(BTN_START);
	
	gpio_init(BTN_STOP);
	gpio_set_dir(BTN_STOP, GPIO_IN);
	gpio_pull_up(BTN_STOP);
	
	//gpio_put(LED_2, 1);
	
    PIO pio = pio0;
    const int sm = 0;

    int offset = pio_add_program(pio, &quadrature_encoder_program);
    quadrature_encoder_program_init(pio, sm, offset, PIN_AB, 0);
    
    state_byte |= (1 << 7);
    state_byte &= ~((1 << 0) | (1 << 1) | (1 << 2));
    
    todo_byte |= (1 << 7);
    todo_byte &= ~((1 << 0) | (1 << 1) | (1 << 2));
    //gpio_put(LED_3, 1);
    sleep_ms(500);
    gpio_set_irq_enabled_with_callback(BTN_START, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    add_repeating_timer_ms( 500, &timer_callback, NULL, &timer ); //timer for checking rotation every 3s

    while (1) {
		switch (STATE){
				case 0://wait start of process
					if (timerr == 1){
						new_value = quadrature_encoder_get_count(pio, sm);
						if(new_value < 0) {
							gpio_put(LED_1, 1);
							old_value_m = new_value;
							delta = 0;
						}
						delta = new_value - old_value_m;
						if (delta<150){
							gpio_put(LED_2, 0);
							flow = false;
						}
						else{
							gpio_put(LED_2, 1);
							flow = true;
							old_value = old_value_m;
						}
						old_value_m = new_value;
						delta = 0;
						timerr = 0;
					}
					if (flow==true) {
						STATE = 1;
						state_byte |= (1 << 5); //new data
						state_byte |= (1 << 4);//start process
						gpio_put(LED_3, 1);
					}
					if (button==1){
						sleep_ms(150);
						while (gpio_get(BTN_START) == 0){}
						act_button = !act_button;
						if (act_button == 1){
							gpio_put(SOLENOID, 1);
							gpio_put(LED_1, 1);
						}
						else{
							gpio_put(SOLENOID, 0);
							gpio_put(LED_1, 0);
						}
						button = 0;
					}
						
					break;
				case 1://process started
					sleep_ms (100); 
					current_value = quadrature_encoder_get_count(pio, sm) - old_value;
					enc_data[0] = (current_value >> 24) & 0xFF;  // Старший байт
					enc_data[1] = (current_value >> 16) & 0xFF;  // Второй байт
					enc_data[2] = (current_value >> 8) & 0xFF;   // Третий байт
					enc_data[3] = current_value & 0xFF;          // Младший байт
					delta = current_value - old_value_m;
					if (delta == 0) STATE = 2;
					else {
						old_value_m = current_value;
						delta = 0;
					}
					if (button==1){
						sleep_ms(150);
						while (gpio_get(BTN_START) == 0){}
						act_button = !act_button;
						if (act_button == 1){
							gpio_put(SOLENOID, 1);
							gpio_put(LED_1, 1);
						}
						else{
							gpio_put(SOLENOID, 0);
							gpio_put(LED_1, 0);
						}
						button = 0;
					}
					break;				
				case 2: //stop process, close valve 
					gpio_put(LED_3, 0);
					gpio_put(SOLENOID, 0);
					state_byte &= ~((1 << 4) | (1 << 5)); // set state in stop
					STATE = 3;
					break;
				case 3: //wait reset command from MASTER
					bit = (todo_byte >> 5) & 0x01;
					if (bit == 1){
						delta = 0;
						old_value_m = current_value;
						old_value = old_value_m;
						enc_data[0] = 0;
						enc_data[1] = 0;
						enc_data[2] = 0;
						enc_data[3] = 0;
						STATE = 0;
						flow = 0;
					}
					break;
				default:
					break;
			}
		}
	}


/*	
		if(gpio_get(BTN_START)==0){
			sleep_ms(150);
			while (gpio_get(BTN_START) == 0){}
			button = !button;
		}
		if (button == 1){
			gpio_put(SOLENOID, 1);
			gpio_put(LED_1, 1);
		}
		else{
			gpio_put(SOLENOID, 0);
			gpio_put(LED_1, 0);
		}
		if (timerr == 1){
			timerr=0;
			new_value = quadrature_encoder_get_count(pio, sm);
			delta = new_value - old_value_m;
			if (delta < 150){
				gpio_put(LED_2, 0);
				flow = false;
			}
			else {
				gpio_put(LED_2, 1);
				flow = true;
			}
			old_value_m = new_value;
			delta = 0;
		}
		if (flow==true){
			gpio_put(LED_3, 1);
			state_byte |= (1 << 5); //new data
			state_byte |= (1 << 4);//start process
			while(flow==true){
				sleep_ms(100);
				if (timerr == 1){
					timerr=0;
					new_value = quadrature_encoder_get_count(pio, sm);
					delta = new_value - old_value_m;
					if (delta < 150){
						gpio_put(LED_2, 0);
						flow = false;
					}
					else {
						gpio_put(LED_2, 1);
						flow = true;
					}
					old_value_m = new_value;
					delta = 0;
				}
				current_value = quadrature_encoder_get_count(pio, sm) - old_value;
				enc_data[0] = (current_value >> 24) & 0xFF;  // Старший байт
				enc_data[1] = (current_value >> 16) & 0xFF;  // Второй байт
				enc_data[2] = (current_value >> 8) & 0xFF;   // Третий байт
				enc_data[3] = current_value & 0xFF;          // Младший байт
				if(gpio_get(BTN_START)==0){
					sleep_ms(150);
					while (gpio_get(BTN_START) == 0){}
					button = !button;
				}
				if (button == 1){
					gpio_put(SOLENOID, 1);
					gpio_put(LED_1, 1);
				}
				else{
					gpio_put(SOLENOID, 0);
					gpio_put(LED_1, 0);
				}
			}
			gpio_put(LED_3, 0);
			state_byte &= ~((1 << 4) | (1 << 5)); // set state in stop
			while(bit==0){
				bit = (todo_byte >> 5) & 0x01; //wait reset comand
				if(gpio_get(BTN_START)==0){
					sleep_ms(150);
					while (gpio_get(BTN_START) == 0){}
					button = !button;
				}
				if (button == 1){
					gpio_put(SOLENOID, 1);
					gpio_put(LED_1, 1);
				}
				else{
					gpio_put(SOLENOID, 0);
					gpio_put(LED_1, 0);
				}
			}
			current_value=0;
			enc_data[0] = 0;
			enc_data[1] = 0;
			enc_data[2] = 0;
			enc_data[3] = 0;
			old_value = new_value;
			delta = 0;
		}
	}
}
*/
