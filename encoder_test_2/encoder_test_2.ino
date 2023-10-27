
#include "hardware/pio.h"
#include "quadrature_encoder.pio.h"
 int new_value, delta, old_value = 0;

    // Base pin to connect the A phase of the encoder.
    // The B phase must be connected to the next pin
const uint PIN_AB = 10;
const int sm = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  PIO pio = pio0;
  

  int offset = pio_add_program(pio, &quadrature_encoder_program);
  quadrature_encoder_program_init(pio, sm, offset, PIN_AB, 0);
  new_value = 0;
}

void loop() {
  // put your main code here, to run repeatedly:
 new_value = quadrature_encoder_get_count(pio, sm);
 Serial.print("counter = ");
 Serial.println(new_value);
 delta = new_value - old_value;
 Serial.print("delta = ");
 Serial.println(delta);
 old_value = new_value;
 delay(100);
}
