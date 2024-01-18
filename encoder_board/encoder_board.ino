#define MAX_DELTA   10      // Pulsos detectados en 500ms (Intervalo2)
#define LED_1      25
#define LED_2     27
#define LED_3     28
#define BTN_START   12
#define BTN_STOP    11
#define SOLENOID    10

#define BUF_LEN         0x100
#define I2C_SLAVE_ADDRESS 0x5C

#define SDA_MAIN    16
#define SCL_MAIN    17

#define ALARM_NUM 0
#define ALARM_IRQ TIMER_IRQ_0

#include "pio_encoder.h"
#include "pico/time.h"
#include "hardware/timer.h"
#include <ArduinoJson.h>
#include <Wire.h>
//#include "RP2040_TimerInterrupt.h"

static char buff[200];
static char resp[200];
String jsonStr;

StaticJsonDocument<200> doc;  // Asegúrate de que el tamaño sea suficiente para tu objeto JSON
StaticJsonDocument<200> doc_aux;  // Crea un documento JSON con espacio para 200 bytes

uint8_t STATE = 0;
volatile int32_t current_value;
volatile int32_t new_value, delta, old_value = 0;
bool flow, buttonx, act_button = false;
unsigned long previousMillis = 0;  // Almacena la última vez que el LED cambió
const long interval = 500;
unsigned long currentMillis;

PioEncoder encoder(2); // encoder is connected to GPIO2 and GPIO3
uint64_t alarm_callback(alarm_id_t id, void *user_data);

const unsigned long intervalo = 100;  // Intervalo de tiempo (1 minuto en milisegundos)
unsigned long tiempoAnterior = 0;
unsigned long tiempoActual;

const unsigned long intervalo2 = 500;  // Intervalo de tiempo (1 minuto en milisegundos)
unsigned long tiempoAnterior2 = 0;
unsigned long tiempoActual2;

unsigned long noDelta_timeSTOP = 60;// Maximo tiempo desde que se detecto STOP_FLOW 
unsigned long noDelta_timeCounter = 0;// Maximo tiempo desde que se detecto STOP_FLOW



bool newcommand = false;

//---------------------------------------------------- setup
void setup()
{

  Serial.begin(115200);
  //while (!Serial);
  Serial.println("Encoder Init");
  //delay(2000);
  Wire.setSDA(SDA_MAIN);
  Wire.setSCL(SCL_MAIN);
  Wire.begin(I2C_SLAVE_ADDRESS);
  Wire.onReceive(recv);
  Wire.onRequest(req);
  pinMode(25, OUTPUT);
  digitalWrite(25, 0);
  pinMode(28, OUTPUT);
  digitalWrite(28, 0);
  pinMode(27, OUTPUT);
  digitalWrite(27, 0);
  //delay(2000);
  Serial.println("I2C Ready");
  //gpio_set_irq_enabled_with_callback(BTN_START, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
  pinMode(SOLENOID, OUTPUT);
  pinMode(BTN_START, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN_START), open_valve, FALLING);  // configura la interrupción
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  encoder.begin();
  // add_repeating_alarm_us(1e6, alarm_callback, NULL, NULL);
  doc["valve_open"] = false;


}


// ------------------------------------------------------ loop
void loop()
{
  if (newcommand == true )
  {
    deserializeJson(doc_aux, jsonStr);
    Serial.print("Master: ");
    serializeJson(doc_aux, Serial);
    Serial.println();
    newcommand = false;
  }

  // ---------------------------------------------- read encoder
  tiempoActual = millis();
  if (tiempoActual - tiempoAnterior >= intervalo)
  {
    // Ha pasado 1 minuto
    tiempoAnterior = tiempoActual;
    new_value = encoder.getCount();
    //Serial.print("new_value: ");
    //Serial.println(new_value);

    // ------------------------------------------- check direction
    if (new_value < 0)
    {
      encoder.reset();
      Serial.println("no ok");
      //gpio_put(LED_1, 1);
      old_value = 0;
      new_value = 0;
      delta = 0;
    }
  }

  // ---------------------------------------------- check_delta
  tiempoActual2 = millis();
  if (tiempoActual2 - tiempoAnterior2 >= intervalo2)
  {
    // Ha pasado 1 minuto
    tiempoAnterior2 = tiempoActual2;

    // ------------------------------------- delta is noise?
    delta = new_value - old_value;
    if (delta < MAX_DELTA)
    {
      noDelta_timeCounter++;
      if (noDelta_timeCounter >= noDelta_timeSTOP)
      {
        flow = false;
        digitalWrite(25, LOW);
        encoder.reset();
        if (STATE == 1)
        {
          STATE = 2;
        }
      }

    }
    else
    {
      if (flow == false)
      {
        Serial.println("Flow detected");
      }
      flow = true;
      noDelta_timeCounter = 0;
      digitalWrite(25, HIGH);
      if (STATE == 0)
      {
        STATE = 1;
      }

    }
    old_value = new_value;
    //Serial.print("Delta: ");
    //Serial.println(delta);
  }



  switch (STATE)
  {
    case 0:// ---------------------------------------------------------- wait start of process
      //Serial.println("Check flow");
      // ------------------------------------- start to moving

      // ---------------------- open valve button
      if (buttonx == true)
      {
        sleep_ms(150);
        while (digitalRead(BTN_START) == 0) {}
        Serial.println("Button_preset");
        act_button = !act_button;
        if (act_button == 1) {
          Serial.println("Valve CLOSED");
          digitalWrite(SOLENOID, HIGH);
          doc["valve_open"] = false;
          //delay(100);
          //gpio_put(SOLENOID, 1);
          //gpio_put(LED_1, 1);
        }
        else {
          //gpio_put(SOLENOID, 0);
          //gpio_put(LED_1, 0);
          Serial.println("Valve OPEN");
          digitalWrite(SOLENOID, LOW);
          doc["valve_open"] = true;
        }
        buttonx = false;
      }
      break;

    case 1: // ------------------------------------------------------process started
      current_value = new_value;
      // ---------------------- open valve button
      if (buttonx == 1)
      {
        sleep_ms(150);
        while (gpio_get(BTN_START) == 0) {}
        act_button = !act_button;

        if (act_button == 1)
        {
          Serial.println("Valve CLOSED");
          digitalWrite(SOLENOID, HIGH);
          //doc["valve_open"] = false;
        }
        else
        {
          Serial.println("Valve OPEN");
          digitalWrite(SOLENOID, LOW);
          //doc["valve_open"] = true;
          //gpio_put(LED_1, 0);
        }
        buttonx = 0;
      }
      break;

    case 2: //------------------------------------------------- stop process, close valve
      Serial.println("STATE 2");
      Serial.println("FLOW STOP");
      digitalWrite(SOLENOID, LOW);
      STATE = 3;
      current_value = new_value;
      break;

    case 3: //wait reset command from MASTER
      if ((doc_aux["reset"].as<bool>() == true) && (!(doc_aux["reset"].isNull())))
      {
        old_value = 0;
        new_value = 0;
        STATE = 0;
        current_value = 0;
        encoder.reset();
      }
      break;

    default:
      break;
  }

  // --------------------------------------------------- print_and_send
  currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {

    // ---------------------------------------- Blink LED
    digitalWrite(28, !(digitalRead(28)));
    //Serial.printf("Slave: '%s'\r\n", buff);


    // ------------------------------------- print states
    doc["pulses"] = new_value;   //Commands
    doc["STATE"] = STATE;
    doc["delta"] = delta;
    doc["flow"] = flow;
    doc["current"] = current_value;
    doc["valve_open"] = bool (digitalRead(SOLENOID));
    //memset(resp, 0, sizeof(resp));
    Serial.print("Encoder State: ");
    serializeJson(doc, resp);
    Serial.println(resp);

    previousMillis = currentMillis;
  }
}



// Called when the I2C slave gets written to
// ------------------------------------------------------------ Recv
void recv(int len)
{
  int i;
  memset(buff, 0, sizeof(buff));
  // Just stuff the sent bytes into a global the main routine can pick up and use
  for (i = 0; i < len; i++)
  {
    buff[i] = Wire.read();
  }
  jsonStr =  buff;
  newcommand = true;
}

// Called when the I2C slave is read from
// --------------------------------------------------------------- Req
void req()
{
  doc["pulses"] = new_value;   //Commands
  doc["STATE"] = STATE;
  doc["delta"] = delta;
  doc["flow"] = flow;
  doc["current"] = current_value;
  doc["valve_open"] = bool (digitalRead(SOLENOID));
  //memset(resp, 0, sizeof(resp));
  serializeJson(doc, resp);

  Wire.write(resp, 199);
}


void open_valve()
{
  buttonx = true;
}


uint64_t alarm_callback(alarm_id_t id, void *user_data) {
  // Cambiar el estado del LED cada vez que se llama a la interrupción
  //digitalWrite(LED_PIN, !digitalRead(LED_PIN));

  // Devolver 0 para que el timer siga llamando a esta función
  return 0;
}
