#include "wireservice.h"

volatile uint32_t nclient;

volatile uint8_t mem_address = 0, STATE = 0;
volatile uint8_t todo_byte = 0b10001001, state_byte = 0b10010101, error_byte = 0, j = 0;
volatile boolean new_num = 0, printer = 0, valve = 0, OK = 0, DEL = 0, stopCommand = 0, mem_address_written = 0;
volatile boolean ask_name = 0, ask_factor = 0, ask_nclient = 0, ask_litro = 0, ask_peso = 0, ask_data = 0, ask_state = 0, ask_todo = 0, error_status = 0;
volatile boolean newcommand = 0, new_litros = 0, new_nclient = 0;
volatile uint8_t name_data[42], factor_data[2], nclient_data[4], uprice_data[2], litros_num[4], pesos_num[4], client_num[4], time_num[4];

#define LED_1      25
#define LED_2     27
#define LED_3     28


// ------------------------------------- Init
void I2C_Init()
{


  // configure I2C0 for slave mode
  STATE |= (1 << 7);                  // Module is alaive
  /*gpio_init(SDA_MAIN);
    gpio_init(SCL_MAIN);
    gpio_set_function(SDA_MAIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_MAIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_MAIN);
    gpio_pull_up(SCL_MAIN);
    i2c_init(i2c0, 100 * 1000);
    // configure I2C0 for slave mode
    i2c_slave_init(i2c0, ADDRESS, &i2c_slave_handler);
    delay(3000);*/

  /*i2c_init(i2c0, 100 * 1000);
    // configure I2C0 for slave mode
    gpio_init(SDA_MAIN);
    gpio_init(SCL_MAIN);
    gpio_set_function(SDA_MAIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_MAIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_MAIN);
    gpio_pull_up(SCL_MAIN);*/

  //Wire.setClock(100000);
  Wire.setSDA(SDA_MAIN);
  Wire.setSCL(SCL_MAIN);
  Wire.begin();
  //gpio_put(LED_1, 1);

  nclient_data[0] = 2;
}
