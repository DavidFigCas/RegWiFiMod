#include "encoder_service.h"

ESP32Encoder encoder;
AS5601 Sensor;

// ------------------------------ Configs
//unsigned long interval = 500;              // Print and send
unsigned long MAX_DELTA = 3;                //10 Pulsos detectados en Intervalo2  (DELTA)
//unsigned long intervalo = 100;    //ENCODER Intervalo de tiempo (milisegundos)
unsigned long t_delta = 100;  //DELTA Intervalo de tiempo (500 milisegundos)
unsigned long noDelta_timeSTOP = 15;// Maximo tiempo desde que se detecto STOP_FLOW 60seg
int32_t current;
int32_t previous_pulses;
int32_t total_encoder;
volatile uint32_t angle_encoder;
bool valve_state = false;



//----------------------------------- endoer_init
void encoder_init()
{
  Sensor.setResolution(2048); // coerce angle steps to supported values (8, 16, 32, …, 2048)
  angle_encoder = Sensor.getRawAngle();

  //ESP32Encoder::useInternalWeakPullResistors = puType::down;
  // Enable the weak pull up resistors
  ESP32Encoder::useInternalWeakPullResistors = puType::up;

  // use pin 19 and 18 for the first encoder
  encoder.attachFullQuad(ENCODER_A, ENCODER_B);
  pinMode(SOLENOID, OUTPUT);
  close_valve();
}


// ----------------------------------------------------------- open
void open_valve()
{
  if (valve_state == false)
  {
    Serial.println("{\"valve_open\":true}");
  }
  status_doc["valve"] = true;
  pinMode(SOLENOID, OUTPUT);
  digitalWrite(SOLENOID, LOW);
  valve_state = true;
}

// ----------------------------------------------------------- close
void close_valve()
{
  if (valve_state == true)
  {
    Serial.println("{\"valve_open\":false}");
  }
  status_doc["valve"] = false;
  digitalWrite(SOLENOID, HIGH);
  pinMode(SOLENOID, INPUT);
  valve_state = false;

  /*if (on_service)
  {
    readyToPrint = true;
    if (litros >= 1)
    {
      STATE_DISPLAY = 2;
      startCounting = true;
      litros_check = ceil(litros);
      precio = litros_check * uprice;
      precio_check = precio;
      encoder_reset = true;
      angle_encoder = Sensor.getRawAngle();
      read_clock();
      saveNewlog();
      send_event = true;        // Send event to mqtt

    }
    else
    {
      startCounting = false;
      STATE_DISPLAY = 0;
      litros = 0;
      litros_check = 0;
      precio = 0;
      precio_check = 0;
      encoder_reset = true;
    }
  }*/

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



  //obj["angle_encoder"] = Sensor.getRawAngle();
  //status_doc["angle_encoder"] = obj["angle_encoder"];

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


//---------------------------------------------------------- checkEncoderPulses
void checkEncoderPulses(void * parameter) 
{
  unsigned long lastFlowCheck = 0;
  unsigned long debounceStart = 0;
  bool debounceActive = false;
  const unsigned long debounceDelay = 20; // Ajusta esto según sea necesario

  for (;;)
  {
    // Leer el valor del encoder
    read_encoder();

    // Verificar si el cambio supera el umbral
    if (abs((int32_t)(current - previous_pulses)) >= MAX_DELTA)
    {
      if (!debounceActive)
      {
        debounceStart = millis();
        debounceActive = true;
      }

      if (millis() - debounceStart >= debounceDelay)
      {
        startFlowing = true;
        saveConfig = true;
        lastFlowCheck = millis();

        if (stopFlowing && !on_service)
        {
          Serial.println("--------------------START FLOWING-----------------");
          start_process_time = now.unixtime();
          angle_encoder = Sensor.getRawAngle();
          stopFlowing = false;
          on_service = true;
          STATE_DISPLAY = 1;
        }

        debounceActive = false; // Reset debounce
      }
    }
    else
    {
      debounceActive = false;
      startFlowing = false;

      // Si el flujo ha comenzado, monitorear si se detiene
      if (on_service)
      {
        if (millis() - lastFlowCheck >= noDelta_timeSTOP * 1000)
        { 
          lastFlowCheck = millis();
          if (abs((int32_t)(current - previous_pulses)) < MAX_DELTA)
          {
            saveConfig = true;
            stopFlowing = true;
            on_service = false;
            Serial.println("--------------------STOP FLOWING-----------------");
            close_valve();

            if (litros >= 1)
            {
              STATE_DISPLAY = 2;
              startCounting = true;
              litros_check = ceil(litros);
              precio = litros_check * uprice;
              precio_check = precio;
              encoder_reset = true;
              angle_encoder = Sensor.getRawAngle();
              read_clock();
              saveNewlog();
              send_event = true;
            }
            else
            {
              startCounting = false;
              STATE_DISPLAY = 1;
              litros = 0;
              litros_check = 0;
              precio = 0;
              precio_check = 0;
              encoder_reset = true;
            }
            encoder.setCount(0);
          }
        }
      }
    }

    // Actualizar el valor anterior de los pulsos
    total_encoder += current;
    obj["total_encoder"] = total_encoder;
    previous_pulses = current;

    // Calcular litros y precio
    litros = int(current / pulsos_litro);
    precio = ceil(litros) * uprice;

    status_doc["l"] = litros;
    status_doc["$"] = precio;
    status_doc["ang"] = angle_encoder;

    if (!doc_display["open"].isNull())
    {
      open_valve();
    }

    if (!doc_display["close"].isNull())
    {
      close_valve();
    }

    // Esperar t_delta ms antes de la siguiente verificación
    vTaskDelay(t_delta / portTICK_PERIOD_MS);
  }
}
