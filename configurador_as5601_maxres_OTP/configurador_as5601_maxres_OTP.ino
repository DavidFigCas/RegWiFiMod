// Do not use this sketch unless You want program AS5601 to 2048 steps
// Very initial version 
// Wersja testowa wstępna, nie do użytku. Służy wyłącznie do jednorazowego zaprogramowania AS5601 na 2049 kroków.

#include <Wire.h>
void setup()
{
  Wire.begin();
  Serial.begin(115200);  
//Set Quadrature (AB) resolution to 2048 - (default is 8)
  /*
0000 : 8  - 0x00
0001 : 16 - 0x01
0010 : 32 - 0x02
0011 : 64 - 0x03
0100 : 128 - 0x04
0101 : 256 - 0x05
0110 : 512 - 0x06
0111 : 1024 - 0x07
others : 2048 - 0x08
      */
Serial.print("settings");
Wire.beginTransmission(0x36);       // start transmission to AS5601
Wire.write(byte(0x09));             // sets adress to ABN 
Wire.write(byte(0x08));             // set resolution 2048 -  
Wire.endTransmission();             // stop transmitting


Serial.print("PROGRAMOWANIE ");
Wire.beginTransmission(0x36);       // start transmission to AS5601
Wire.write(byte(0xFF));             // sets adress to OTP
Wire.write(byte(0x40));             // write 0x40 to adress from previous line - killer line!!!!
Wire.endTransmission();            // stop transmission
 }

void loop()
{
 }
