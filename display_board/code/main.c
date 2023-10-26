#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "i2c_fifo.h"
#include "i2c_slave.h"
#include "pics.h"
#include "pico/time.h"

#define MOSI			3
#define MISO			4
#define SCK				2
#define CS				1

#define DC				5
#define RST				6
#define BUSY			7

#define SDA_TOUCH		10
#define SCL_TOUCH		11
#define IRQ_TOUCH		9

#define SDA_MAIN		16
#define SCL_MAIN		17

#define LED_1			25
#define LED_2			27
#define LED_3			28

#define BUF_LEN         0x100
#define I2C_SLAVE_ADDRESS	0x91
#define ADDRESS			0x48

unsigned int width = 800;
unsigned int height = 480;

//const int main_address = 0x91;//slave address
//const int ADDRESS = 0x48; // Touch address
// The slave implements a 256 byte memory. To write a series of bytes, the master first
// writes the memory address, followed by the data. The address is automatically incremented
// for each byte transferred, looping back to 0 upon reaching the end. Reading is done
// sequentially from the current memory address.
static struct
{
    uint8_t mem[256];
    uint8_t mem_address;
    bool mem_address_written;
} context;

static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    switch (event) {
    case I2C_SLAVE_RECEIVE: // master has written some data
        if (!context.mem_address_written) {
            // writes always start with the memory address
            context.mem_address = i2c_read_byte(i2c);
            context.mem_address_written = true;
        } else {
            // save into memory
            context.mem[context.mem_address] = i2c_read_byte(i2c);
            context.mem_address++;
        }
        break;
    case I2C_SLAVE_REQUEST: // master is requesting data
        // load from memory
        i2c_write_byte(i2c, context.mem[context.mem_address]);
        context.mem_address++;
        break;
    case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
        context.mem_address_written = false;
        break;
    default:
        break;
    }
}

void SendCommand(unsigned char command) {
    gpio_put(DC, 1);
    spi_write_blocking(spi_default, &command, 1); 
}
void SendData(unsigned char data) {
    gpio_put(DC, 1);
    spi_write_blocking(spi_default, &data, 1);
}
void WaitUntilIdle(void) {
    unsigned char busy;
    do{
        SendCommand(0x71);
        busy = gpio_get(BUSY);
    }while(busy == 0);
    sleep_ms(20);
}

void Reset(void) {
    
    gpio_put(RST, 1);
    sleep_ms(20);
    gpio_put(RST, 0);
    sleep_ms(4);
    gpio_put(RST, 1);
    sleep_ms(20);
}

void SetLut_by_host(unsigned char* lut_vcom,  unsigned char* lut_ww, unsigned char* lut_bw, unsigned char* lut_wb, unsigned char* lut_bb)
{
	unsigned char count;

	SendCommand(0x20); //VCOM	
	for(count=0; count<42; count++)
		SendData(lut_vcom[count]);

	SendCommand(0x21); //LUTBW
	for(count=0; count<42; count++)
		SendData(lut_ww[count]);

	SendCommand(0x22); //LUTBW
	for(count=0; count<42; count++)
		SendData(lut_bw[count]);

	SendCommand(0x23); //LUTWB
	for(count=0; count<42; count++)
		SendData(lut_wb[count]);

	SendCommand(0x24); //LUTBB
	for(count=0; count<42; count++)
		SendData(lut_bb[count]);
}

void display_init(void){
	Reset();
	SendCommand(0x01);  // power setting
	SendData(0x17);  // 1-0=11: internal power
	SendData(*(Voltage_Frame_7IN5_V2+6));  // VGH&VGL
	SendData(*(Voltage_Frame_7IN5_V2+1));  // VSH
	SendData(*(Voltage_Frame_7IN5_V2+2));  //  VSL
	SendData(*(Voltage_Frame_7IN5_V2+3));  //  VSHR
	
	SendCommand(0x82);  // VCOM DC Setting
	SendData(*(Voltage_Frame_7IN5_V2+4));  // VCOM

	SendCommand(0x06);  // Booster Setting
	SendData(0x27);
	SendData(0x27);
	SendData(0x2F);
	SendData(0x17);
	
	SendCommand(0x30);   // OSC Setting
	SendData(*(Voltage_Frame_7IN5_V2+0));  // 2-0=100: N=4  ; 5-3=111: M=7  ;  3C=50Hz     3A=100HZ

    SendCommand(0x04); //POWER ON
    sleep_ms(100);
    WaitUntilIdle();

    SendCommand(0X00);			//PANNEL SETTING
    SendData(0x3F);   //KW-3f   KWR-2F	BWROTP 0f	BWOTP 1f

    SendCommand(0x61);        	//tres
    SendData(0x03);		//source 800
    SendData(0x20);
    SendData(0x01);		//gate 480
    SendData(0xE0);

    SendCommand(0X15);
    SendData(0x00);

    SendCommand(0X50);			//VCOM AND DATA INTERVAL SETTING
    SendData(0x10);
    SendData(0x00);

    SendCommand(0X60);			//TCON SETTING
    SendData(0x22);

    SendCommand(0x65);  // Resolution setting
    SendData(0x00);
    SendData(0x00);//800*480
    SendData(0x00);
    SendData(0x00);

    SetLut_by_host(LUT_VCOM_7IN5_V2, LUT_WW_7IN5_V2, LUT_BW_7IN5_V2, LUT_WB_7IN5_V2, LUT_BB_7IN5_V2);

    //return 0;
}
/*
void GxEPD2_750_YT7::_InitDisplay()
{
  if (_hibernating) _reset();
  _writeCommand(0x01); // POWER SETTING
  _writeData (0x07);
  _writeData (0x07); // VGH=20V,VGL=-20V
  _writeData (0x3f); // VDH=15V
  _writeData (0x3f); // VDL=-15V
  _writeCommand(0x00); //PANEL SETTING
  _writeData(0x1f); //KW: 3f, KWR: 2F, BWROTP: 0f, BWOTP: 1f
  _writeCommand(0x61); //tres
  _writeData (WIDTH / 256); //source 800
  _writeData (WIDTH % 256);
  _writeData (HEIGHT / 256); //gate 480
  _writeData (HEIGHT % 256);
  _writeCommand(0x15);
  _writeData(0x00);
  _writeCommand(0x50); //VCOM AND DATA INTERVAL SETTING
  _writeData(0x29);    // LUTKW, N2OCP: copy new to old
  _writeData(0x07);
  _writeCommand(0x60); //TCON SETTING
  _writeData(0x22);
}

void GxEPD2_750_YT7::_Init_Full()
{
  _InitDisplay();
  _writeCommand(0x00); // panel setting
  _writeData(0x1f);    // full update LUT from OTP
  _PowerOn();
  _using_partial_mode = false;
}

void GxEPD2_750_YT7::_Init_Part()
{
  _InitDisplay();
  _writeCommand(0x00); //panel setting
  _writeData(hasFastPartialUpdate ? 0x3f : 0x1f); // partial update LUT from registers
  _writeCommand(0x82); // vcom_DC setting
  _writeData (0x30); // -2.5V same value as in OTP
  _writeCommand(0x50); // VCOM AND DATA INTERVAL SETTING
  _writeData(0x39);    // LUTBD, N2OCP: copy new to old
  _writeData(0x07);
  _writeCommand(0x20);
  _writeDataPGM(lut_20_LUTC_partial, sizeof(lut_20_LUTC_partial), 42 - sizeof(lut_20_LUTC_partial));
  _writeCommand(0x21);
  _writeDataPGM(lut_21_LUTWW_partial, sizeof(lut_21_LUTWW_partial), 42 - sizeof(lut_21_LUTWW_partial));
  _writeCommand(0x22);
  _writeDataPGM(lut_22_LUTKW_partial, sizeof(lut_22_LUTKW_partial), 42 - sizeof(lut_22_LUTKW_partial));
  _writeCommand(0x23);
  _writeDataPGM(lut_23_LUTWK_partial, sizeof(lut_23_LUTWK_partial), 42 - sizeof(lut_23_LUTWK_partial));
  _writeCommand(0x24);
  _writeDataPGM(lut_24_LUTKK_partial, sizeof(lut_24_LUTKK_partial), 42 - sizeof(lut_24_LUTKK_partial));
  _writeCommand(0x25);
  _writeDataPGM(lut_25_LUTBD_partial, sizeof(lut_25_LUTBD_partial), 42 - sizeof(lut_25_LUTBD_partial));
  _PowerOn();
  _using_partial_mode = true;
}
*/
void DisplayFrame(const unsigned char* frame_buffer) {
    
    SendCommand(0x13);
    for (unsigned long j = 0; j < height; j++) {
        for (unsigned long i = 0; i < width/8; i++) {
            SendData(~frame_buffer[i + j * width/8]);
        }
    }
    SendCommand(0x12);
    sleep_ms(100);
    WaitUntilIdle();
}

void Displaypart(const unsigned char* pbuffer, unsigned long xStart, unsigned long yStart,unsigned long Picture_Width,unsigned long Picture_Height) {
    SendCommand(0x13);
    for (unsigned long j = 0; j < height; j++) {
        for (unsigned long i = 0; i < width/8; i++) {
            if( (j>=yStart) && (j<yStart+Picture_Height) && (i*8>=xStart) && (i*8<xStart+Picture_Width)){
                //SendData(~(pgm_read_byte(&(pbuffer[i-xStart/8 + (Picture_Width)/8*(j-yStart)]))) );
                SendData(~(pbuffer[i - xStart / 8 + (Picture_Width / 8) * (j - yStart)]));
                // SendData(0xff);
            }else {
                SendData(0x00);
            }
        }
    }
    SendCommand(0x12);
    sleep_ms(100);
    WaitUntilIdle();
}

void Sleep(void) {
    SendCommand(0X02);
    WaitUntilIdle();
    SendCommand(0X07);
    SendData(0xA5);
}

void Clear(void) {
    SendCommand(0x13);
    for(unsigned long i=0; i<height*width; i++)	{
        SendData(0x00);
    }
    SendCommand(0x12);
    sleep_ms(100);
    WaitUntilIdle();
}

int main() {
    stdio_init_all();
    
    // Enable SPI 0 at 1 MHz and connect to GPIOs
    spi_init(spi_default, 1000 * 1000);
    gpio_set_function(MISO, GPIO_FUNC_SPI);
    gpio_set_function(SCK, GPIO_FUNC_SPI);
    gpio_set_function(MOSI, GPIO_FUNC_SPI);
    gpio_set_function(CS, GPIO_FUNC_SPI);
    
    //Init I2C for touch
    i2c_init(i2c1, 100 * 1000);
    gpio_set_function(SDA_TOUCH, GPIO_FUNC_I2C);
    gpio_set_function(SCL_TOUCH, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_TOUCH);
    gpio_pull_up(SCL_TOUCH);
    // Make the I2C pins available to picotool
    //bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));
    
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
	
	gpio_init(DC);
	gpio_set_dir(DC, GPIO_OUT);
	gpio_put(DC, 0);
	
	gpio_init(RST);
	gpio_set_dir(RST, GPIO_OUT);
	gpio_put(RST, 0);
	
	gpio_init(BUSY);
	gpio_set_dir(BUSY, GPIO_IN);
	gpio_init(IRQ_TOUCH);
	gpio_set_dir(IRQ_TOUCH, GPIO_IN);
	
	display_init();
	DisplayFrame(background);
	sleep_ms(5000);
	Clear();
	Sleep();
    //spi_write_blocking(	spi_inst_t * 	spi, const uint8_t * 	src, size_t 	len); 
   // uint8_t buf[2];

    // Turn normal mode and 1.344kHz data rate on
    //buf[0] = CTRL_REG_1;
    //buf[1] = 0x97;
    //i2c_write_blocking(i2c_default, ADDRESS, buf, 2, false);
}

