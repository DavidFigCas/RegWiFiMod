/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink
*/

//#include <SoftwareSerial.h>
//#include "Wire.h"
//#include <AS5600.h>
#include <avr/sleep.h>
#include <avr/power.h>

#define RX_PIN   PIN_PB4
#define TX_PIN   PIN_PA5
//#define TX_PIN   PIN_PA0

int contador;
uint16_t p;

//SoftwareSerial mySerial(RX_PIN, TX_PIN); // Reemplaza RX_PIN y TX_PIN con los números de pin reales
//AS5600 encoder;


// the setup function runs once when you press reset or power the board
void setup() {
  //F_CPU;
  //CCP = 0xD8;
  //CLKCTRL.MCLKCTRLA = 1;
  //CLKCTRL.OSC32KCTRLA = 1;

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(PIN_PA3, OUTPUT);
  digitalWrite(PIN_PA3, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(10);                       // wait for a second
  digitalWrite(PIN_PA3, LOW);    // turn the LED off by making the voltage LOW
  delay(10);
  //mySerial.begin(9600);
  //Wire.begin();
  //mySerial.write('A'); // Enviar el carácter 'A'

  pinMode(PIN_PA0, INPUT);
  pinMode(PIN_PA1, INPUT);
  pinMode(PIN_PA2, INPUT);
  pinMode(PIN_PA3, INPUT);
  pinMode(PIN_PA4, INPUT);
  pinMode(PIN_PA5, INPUT);
  pinMode(PIN_PA6, INPUT);
  pinMode(PIN_PA7, INPUT);

  pinMode(PIN_PB0, INPUT);
  pinMode(PIN_PB1, INPUT);
  pinMode(PIN_PB2, INPUT);
  pinMode(PIN_PB3, INPUT);
  pinMode(PIN_PB4, INPUT);
  pinMode(PIN_PB5, INPUT);
  //pinMode(PIN_PB6, INPUT);
  //pinMode(PIN_PB7, INPUT);

  pinMode(PIN_PC0, INPUT);
  pinMode(PIN_PC1, INPUT);
  pinMode(PIN_PC2, INPUT);
  pinMode(PIN_PC3, INPUT);
  //pinMode(PIN_PC4, INPUT);
  //pinMode(PIN_PC5, INPUT);
  //pinMode(PIN_PC6, INPUT);
  //pinMode(PIN_PC7, INPUT);

  for (int i = 0; i < NUM_DIGITAL_PINS; i++) {
    pinMode(i, INPUT);
    digitalWrite(i, LOW);
  }
  
  ADC0.CTRLA &= ~ADC_ENABLE_bm;
  power_all_disable(); 

  //power_adc_disable(); // Deshabilita el ADC
  //power_spi_disable(); // Deshabilita el SPI
  //power_timer0_disable(); // Deshabilita el Timer 0
  //power_timer1_disable(); // Deshabilita el Timer 1
  //power_twi_disable(); // Deshabilita el I2C

  

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_cpu();

}

// the loop function runs over and over again forever
void loop() {
  sleep_cpu();
  //digitalWrite(PIN_PA3, HIGH);   // turn the LED on (HIGH is the voltage level)
  //delay(100);                       // wait for a second
  //digitalWrite(PIN_PA3, LOW);    // turn the LED off by making the voltage LOW
  //delay(100);
  // mySerial.print("Sensor: "); // Enviar el carácter 'A'// wait for a second
  // p = encoder.getAngle();
  // mySerial.println(p); // Enviar el carácter 'A'// wait for a second

  //byte error, address;
  //int nDevices = 0;

  //delay(500);

  /* mySerial.println("Scanning for I2C devices ...");
    for(address = 0x01; address < 0x7f; address++){
     Wire.beginTransmission(address);
     error = Wire.endTransmission();
     if (error == 0){
       mySerial.printf("I2C device found at address 0x%02X\n", address);
       nDevices++;
     } else if(error != 2){
       mySerial.printf("Error %d at address 0x%02X\n", error, address);
     }
    }
    if (nDevices == 0){
     mySerial.println("No I2C devices found");
    }*/
}
