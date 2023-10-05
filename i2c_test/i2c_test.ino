#include "wireservice.h"





// ------------------------------------------------------ (CONFIG) setup
void setup()
{
  Serial.begin(115200);
  delay(5000);
  Serial.println("Test i2c");
  I2C_Init();
}


// ------------------------------------------------------ loop
void loop()
{
 delay(1000);
Serial.println("Testing");

}
