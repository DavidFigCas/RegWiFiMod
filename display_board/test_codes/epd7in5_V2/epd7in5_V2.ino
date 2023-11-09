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
Epd epd;
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


void setup() {
  // put your setup code here, to run once:
    Serial.begin(115200);
    //Epd epd;
    Serial.print("e-Paper init \r\n ");
    if (epd.Init() != 0) {
        Serial.print("e-Paper init failed\r\n ");
        return;
    }
    
    Serial.print("e-Paper Display\r\n ");
    epd.Displaypart(IMAGE_DATA,0, 0,240,103);
    epd.SendCommand(0x12);
    delay(3000);
    //epd.SendCommand(0x91);
    //setPartialRamArea(100, 200, 240, 103);
    //epd.Displaypart(IMAGE_DATA,100, 200,240,103);
    //epd.SendCommand(0x92);
    Serial.print("e-Paper Clear\r\n ");
    epd.Clear();

    epd.Sleep();
}

void loop() {
  // put your main code here, to run repeatedly:

}
