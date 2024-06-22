#include "encoder_service.h"

ESP32Encoder encoder;
AS5601 Sensor;

// ------------------------------ Configs
//unsigned long interval = 500;              // Print and send
unsigned long MAX_DELTA = 3;                //10 Pulsos detectados en Intervalo2  (DELTA)
//unsigned long intervalo = 100;    //ENCODER Intervalo de tiempo (milisegundos)
unsigned long t_delta = 100;  //DELTA Intervalo de tiempo (500 milisegundos)
unsigned long noDelta_timeSTOP = 60;// Maximo tiempo desde que se detecto STOP_FLOW 60seg
uint32_t current;
uint32_t previous_pulses;

//----------------------------------- endoer_init
void encoder_init()
{
  Sensor.setResolution(1024);

  //ESP32Encoder::useInternalWeakPullResistors = puType::down;
  // Enable the weak pull up resistors
  ESP32Encoder::useInternalWeakPullResistors = puType::up;

  // use pin 19 and 18 for the first encoder
  encoder.attachFullQuad(27, 26);
}


// ----------------------------------- read_encoder
void read_encoder()
{
  //Serial.print( F("Magnitude: ") );
  //Serial.print( Sensor.getMagnitude() );

  //Serial.print( F(" | Magnet  : ") );
  //Serial.print( Sensor.magnetDetected() );

  //Serial.print( F(" | Raw angle: ") );
  //Serial.print( Sensor.getRawAngle() );


  //Serial.print( F(" | Encoder: ") );
  //Serial.print(String((int32_t)encoder.getCount()));

  //Serial.println();
  current = abs(encoder.getCount());
}

// ----------------------------------- print_encoder
void print_encoder()
{
  Serial.print( F("Magnitude: ") );
  Serial.print( Sensor.getMagnitude() );

  Serial.print( F(" | Magnet  : ") );
  Serial.print( Sensor.magnetDetected() );

  Serial.print( F(" | Raw angle: ") );
  Serial.print( Sensor.getRawAngle() );


  Serial.print( F(" | Encoder: ") );
  Serial.print(String((int32_t)encoder.getCount()));

  Serial.println();
}

// -------------------------------------------------- checkEncoderPulses
void checkEncoderPulses(void * parameter) {
   for (;;) {
        // Leer el valor del encoder
        read_encoder();
        
        // Verificar si el cambio supera el umbral
        if (abs((int32_t)(current - previous_pulses)) > MAX_DELTA)
        {
            Serial.println((current - previous_pulses));
        }
        
        // Actualizar el valor anterior de los pulsos
        previous_pulses = current;
        
        // Esperar 1 ms antes de la siguiente verificación
        vTaskDelay(t_delta / portTICK_PERIOD_MS);
    }
}
