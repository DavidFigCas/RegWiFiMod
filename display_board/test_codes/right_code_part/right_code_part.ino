/**
 *  @filename   :   epd7in5-demo.ino
 *  @brief      :   7.5inch e-paper display demo
 *  @author     :   Yehui from Waveshare
 *
 *  Copyright (C) Waveshare     July 10 2017
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <SPI.h>
#include "epd7in5_V2.h"
#include "imagedata.h"

#define T1 30 // charge balance pre-phase
#define T2  5 // optional extension
#define T3 30 // color change phase (b/w)
#define T4  5 // optional extension for one color

Epd epd;

const unsigned char lut_20_LUTC_partial[] PROGMEM =
{
  0x00, T1, T2, T3, T4, 1, // 00 00 00 00
};

const unsigned char lut_21_LUTWW_partial[] PROGMEM =
{ // 10 w
  0x00, T1, T2, T3, T4, 1, // 00 00 00 00
};

const unsigned char lut_22_LUTKW_partial[] PROGMEM =
{ // 10 w
  //0x48, T1, T2, T3, T4, 1, // 01 00 10 00
  0x5A, T1, T2, T3, T4, 1, // 01 01 10 10 more white
};

const unsigned char lut_23_LUTWK_partial[] PROGMEM =
{ // 01 b
  0x84, T1, T2, T3, T4, 1, // 10 00 01 00
  //0xA5, T1, T2, T3, T4, 1, // 10 10 01 01 more black
};

const unsigned char lut_24_LUTKK_partial[] PROGMEM =
{ // 01 b
  0x00, T1, T2, T3, T4, 1, // 00 00 00 00
};

const unsigned char lut_25_LUTBD_partial[] PROGMEM =
{
  0x00, T1, T2, T3, T4, 1, // 00 00 00 00
};

void setPartialRamArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  uint16_t xe = (x + w - 1) | 0x0007; // byte boundary inclusive (last byte)
  uint16_t ye = y + h - 1;
  x &= 0xFFF8; // byte boundary
  epd.SendCommand(0x90); // partial window
  epd.SendData(x / 256);
  epd.SendData(x % 256);
  epd.SendData(xe / 256);
  epd.SendData(xe % 256);
  epd.SendData(y / 256);
  epd.SendData(y % 256);
  epd.SendData(ye / 256);
  epd.SendData(ye % 256);
  epd.SendData(0x01); // don't see any difference
  //epd.SendData(0x00); // don't see any difference
}

void _writeDataPGM(const uint8_t* data, uint16_t n, int16_t fill_with_zeroes)
{
  SPI.beginTransaction(SPISettings(7000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_PIN, LOW);
  for (uint16_t i = 0; i < n; i++)
  {
    SPI.transfer(pgm_read_byte(&*data++));
  }
  while (fill_with_zeroes > 0)
  {
    SPI.transfer(0x00);
    fill_with_zeroes--;
  }
  digitalWrite(CS_PIN, 1);
  SPI.endTransaction();
}

//void SendCommand(unsigned char command);
void setup() {
  // put your setup code here, to run once:
    Serial.begin(115200);
    
    Serial.print("e-Paper init \r\n ");
    if (epd.Init() != 0) {
        Serial.print("e-Paper init failed\r\n ");
        return;
    }
    
    Serial.print("e-Paper Display\r\n ");
    epd.Displaypart(IMAGE_DATA,0, 0,240,103);
    epd.SendCommand(0x12);
    delay(1000);
    
    epd.Init();
    epd.SendCommand(0x00);
    epd.SendData(0x3f);
    epd.SendCommand(0x82);
    epd.SendData(0x30);
    epd.SendCommand(0x50);
    epd.SendData(0x39);
    epd.SendData(0x07);
    epd.SendCommand(0x20);
    _writeDataPGM(lut_20_LUTC_partial, sizeof(lut_20_LUTC_partial), 42 - sizeof(lut_20_LUTC_partial));
  epd.SendCommand(0x21);
  _writeDataPGM(lut_21_LUTWW_partial, sizeof(lut_21_LUTWW_partial), 42 - sizeof(lut_21_LUTWW_partial));
  epd.SendCommand(0x22);
  _writeDataPGM(lut_22_LUTKW_partial, sizeof(lut_22_LUTKW_partial), 42 - sizeof(lut_22_LUTKW_partial));
  epd.SendCommand(0x23);
  _writeDataPGM(lut_23_LUTWK_partial, sizeof(lut_23_LUTWK_partial), 42 - sizeof(lut_23_LUTWK_partial));
  epd.SendCommand(0x24);
  _writeDataPGM(lut_24_LUTKK_partial, sizeof(lut_24_LUTKK_partial), 42 - sizeof(lut_24_LUTKK_partial));
  epd.SendCommand(0x25);
  _writeDataPGM(lut_25_LUTBD_partial, sizeof(lut_25_LUTBD_partial), 42 - sizeof(lut_25_LUTBD_partial));

    
    epd.SendCommand(0X91);
    setPartialRamArea(250, 0, 240, 103);
    epd.Displaypart(IMAGE_DATA,250, 0,240,103);
    epd.SendCommand(0X92);
 
    Serial.print("e-Paper Clear\r\n ");
    epd.Clear();

    epd.Sleep();
}

void loop() {
  // put your main code here, to run repeatedly:

}
