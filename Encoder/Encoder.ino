#include <ESP32Encoder.h>
#include <Arduino.h>
#include <AS5601.h>

ESP32Encoder encoder;
AS5601 Sensor;

void setup()
{

  Serial.begin(115200);
  // Enable the weak pull down resistors

  Sensor.setResolution(2048);

  //ESP32Encoder::useInternalWeakPullResistors = puType::down;
  // Enable the weak pull up resistors
  ESP32Encoder::useInternalWeakPullResistors = puType::up;

  // use pin 19 and 18 for the first encoder
  encoder.attachFullQuad(27, 26);
}

void loop()
{
  // Loop and read the count

  delay(100);

  Serial.print( F("Magnitude: ") );
  Serial.print( Sensor.getMagnitude() );

  Serial.print( F(" | Magnet: ") );
  Serial.print( Sensor.magnetDetected() );

  Serial.print( F(" | Raw angle: ") );
  Serial.print( Sensor.getRawAngle() );


  Serial.print( F(" | Encoder: ") );
  Serial.print(String((int32_t)encoder.getCount()));

  Serial.println();

}
