#include <Wire.h>
#define FAULT   PIN_PC2
#define VIN     PIN_PC0
#define VRES    PIN_PB2
#define VNORM   PIN_PB3
#define LED_1   PIN_PB4
#define LED_2   PIN_PB4

#define ADDRESS 0x5B
bool ask_status=0;
byte RxByte = 0;
volatile byte TxByte = 0;
int volt = 0;
void I2C_RxHandler(int numBytes)
{
  while(Wire.available()) {  // Read Any Received Data
    RxByte = Wire.read();
  }
  if (RxByte == 0x01) ask_status=1;
}
 
void I2C_TxHandler(void)
{
  if (ask_status==1){
    Wire.write(TxByte);
    ask_status=0;
  }
  else{
   Wire.write(0);
  }
}

void setup() {
  pinMode(FAULT, INPUT_PULLUP);
  pinMode(VRES, OUTPUT);
  pinMode(VNORM, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  digitalWrite(VRES, 0);
  digitalWrite(VNORM, 1);
  analogReference(INTERNAL2V048);
  Wire.begin(ADDRESS);
  Wire.onReceive(I2C_RxHandler);
  Wire.onRequest(I2C_TxHandler);
  TxByte |= (1 << 7);
  TxByte &= ~((1 << 0) | (1 << 1) | (1 << 2));
}

void loop() {
  volt = analogRead(VIN);
  if (volt<410){//<9V  -  use battery
    digitalWrite(VRES, 1);
    digitalWrite(VNORM, 0);
    TxByte &= ~(1 << 3);
    digitalWrite(LED_2, 0);
  }
  if (volt>500){//>11V - normal power supply
   digitalWrite(VRES, 0);
   digitalWrite(VNORM, 1); 
   TxByte |= (1 << 3);
   digitalWrite(LED_2, 1);
  }
  if (volt<365){//undervoltage
    TxByte |= (1 << 4);
  }
  else{
    TxByte &= ~(1 << 4);
  }
  if (volt>680){ //overvoltage
    TxByte |= (1 << 5);
  }
  else{
    TxByte &= ~(1 << 5);
  }
  if(digitalRead(FAULT)==0){
    TxByte |= (1 << 6);
    digitalWrite(LED_1, 1);
  }
  else {
    TxByte &= ~(1 << 6);
    digitalWrite(LED_1, 0);
  }
}
