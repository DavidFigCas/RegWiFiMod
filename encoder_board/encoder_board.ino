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
volatile int32_t current_value, new_value, delta, old_value = 0, old_value_m = 0;
bool flow, buttonx, act_button = false;
static volatile bool alarm_fired;
unsigned long previousMillis = 0;  // Almacena la última vez que el LED cambió
const long interval = 500;
unsigned long currentMillis;

PioEncoder encoder(2); // encoder is connected to GPIO2 and GPIO3
uint64_t alarm_callback(alarm_id_t id, void *user_data);



//---------------------------------------------------- setup
void setup()
{
  encoder.begin();
  Serial.begin(115200);
  while (!Serial);
  Wire.setSDA(SDA_MAIN);
  Wire.setSCL(SCL_MAIN);
  Wire.begin(I2C_SLAVE_ADDRESS);
  Wire.onReceive(recv);
  Wire.onRequest(req);
  delay(1000);

  //gpio_set_irq_enabled_with_callback(BTN_START, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
  pinMode(SOLENOID, OUTPUT);
  pinMode(BTN_START, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN_START), open_valve, FALLING);  // configura la interrupción
  // add_repeating_alarm_us(1e6, alarm_callback, NULL, NULL);
  doc["valve_open"] = false;


}


// ------------------------------------------------------ loop
void loop()
{

  //if (alarm_fired == true)
  //{

  //}

  new_value = encoder.getCount();
  delay(50);
  
  currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {

    //alarm_fired = true;
    

    memset(resp, 0, sizeof(resp));
    //Serial.printf("Slave: '%s'\r\n", buff);

    jsonStr =  buff;
    deserializeJson(doc_aux, jsonStr);
    serializeJson(doc_aux, Serial);
    Serial.println();
    //Serial.println(buff);  // Salida: {"name":"John","age":30,"city":"New York"}

    doc["pulses"] = new_value;   //Commands
    serializeJson(doc, resp);
    Serial.println(resp);  // Salida: {"name":"John","age":30,"city":"New York"}
    //delay(500);
    doc["STATE"] = STATE;
    doc["delta"] = delta;
    doc["flow"] = flow;
    previousMillis = currentMillis;

    // ------------------------------------- delta is noise?
    delta = new_value - old_value_m;
    alarm_fired = false;
    Serial.print("Delta: ");
    Serial.println(delta);
    if (delta < 15)
    {
      //gpio_put(LED_2, 0);
      flow = false;
      //delta = 0;
      old_value_m = new_value;
      //encoder.reset();
    }
    else
    {
      Serial.println("Flow detected");
      flow = true;
      old_value_m = new_value;;
      delta = 0;
    }

    if (new_value < 0)
    {
      Serial.println("no ok");
      //gpio_put(LED_1, 1);
      old_value_m = 0;
      new_value = 0;
      delta = 0;
      encoder.reset();

    }
  }



  switch (STATE)
  {
    case 0:// ---------------------------------------------------------- wait start of process
      //Serial.println("Check flow");
      // ------------------------------------- start to moving


      if (flow == true)
      {
        //old_value_m = new_value;
        //delta = 0;
        STATE = 1;
        //doc["STATE"] = STATE;
        //state_byte |= (1 << 5); //new data
        //state_byte |= (1 << 4);//start process
        //gpio_put(LED_3, 1);
      }

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
      current_value = new_value - old_value;
      doc["current"] = current_value;         // Младший байт

      if (flow == false)
      {
        STATE = 2;
        //doc["STATE"] = STATE;
      }
      //else
      //{
        //old_value_m = current_value;
        //delta = 0;
      //}

      //doc["delta"] = delta;

      if (buttonx == 1) {
        sleep_ms(150);
        while (gpio_get(BTN_START) == 0) {}
        act_button = !act_button;

        if (act_button == 1)
        {
          Serial.println("Valve CLOSED");
          digitalWrite(SOLENOID, HIGH);
          doc["valve_open"] = false;
        }
        else
        {
          Serial.println("Valve OPEN");
          digitalWrite(SOLENOID, LOW);
          doc["valve_open"] = true;
          //gpio_put(LED_1, 0);
        }
        buttonx = 0;
      }
      break;

    case 2: //------------------------------------------------- stop process, close valve
      Serial.println("Valve CLOSED");
      digitalWrite(SOLENOID, HIGH);
      doc["valve_open"] = false;
      //state_byte &= ~((1 << 4) | (1 << 5)); // set state in stop
      STATE = 3;
      doc["STATE"] = STATE;
      flow = false;
      doc["flow"] = flow;
      break;

    case 3: //wait reset command from MASTER
      //if (bit == 1)
      if ((doc_aux["reset"].as<bool>() == true) && (!(doc_aux["reset"].isNull())))
      {
        delta = 0;
        old_value_m = current_value;
        old_value = old_value_m;
        //enc_data[0] = 0;
        //enc_data[1] = 0;
        //enc_data[2] = 0;
        //enc_data[3] = 0;
        STATE = 0;
        flow = 0;
        current_value = 0;
        doc["STATE"] = STATE;
        doc["flow"] = flow;
        doc["current"] = current_value;
        encoder.reset();
      }
      break;

    default:
      break;
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
  //buff[i] = 0;
}

// Called when the I2C slave is read from
// --------------------------------------------------------------- Req
void req()
{
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
