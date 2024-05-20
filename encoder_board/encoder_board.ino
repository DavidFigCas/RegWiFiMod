

#define LED_1      25
#define LED_2     27
#define LED_3     28
#define BTN_START   12
#define BTN_STOP    11
#define SOLENOID    10
#define FILE_SIZE   512

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
#include <LittleFS.h>
//#include "RP2040_TimerInterrupt.h"

static char buff[200];
static char resp[200];
String jsonStr;

StaticJsonDocument<200> doc;  // Asegúrate de que el tamaño sea suficiente para tu objeto JSON
StaticJsonDocument<200> doc_aux;  // Crea un documento JSON con espacio para 200 bytes
JsonObject obj_conf;
StaticJsonDocument<FILE_SIZE> doc_conf;
const char* filename = "/config.json";
volatile bool saveConfig = false;
File file;

// ------------------------------ Configs
unsigned long interval = 500;              // Print and send
unsigned long MAX_DELTA = 1;                //10 Pulsos detectados en Intervalo2  (DELTA)
unsigned long intervalo = 100;    //ENCODER Intervalo de tiempo (milisegundos)
unsigned long intervalo2 = 2000;  //DELTA Intervalo de tiempo (500 milisegundos)
unsigned long noDelta_timeSTOP = 60;// Maximo tiempo desde que se detecto STOP_FLOW 60seg


uint8_t STATE = 0;
volatile uint32_t total_encoder;
volatile int32_t current_value;
volatile int32_t target_value;
volatile int32_t new_value, delta, old_value = 0;
bool flow, buttonx, act_button = false;
unsigned long previousMillis = 0;  // Almacena la última vez que el LED cambió

unsigned long currentMillis;

PioEncoder encoder(2); // encoder is connected to GPIO2 and GPIO3
uint64_t alarm_callback(alarm_id_t id, void *user_data);


unsigned long tiempoAnterior = 0;
unsigned long tiempoActual;


unsigned long tiempoAnterior2 = 0;
unsigned long tiempoActual2;


unsigned long noDelta_timeCounter = 0;// Contador tiempo desde que se detecto STOP_FLOW
unsigned long noDelta_timeCountePrev;



bool newcommand = false;
bool dir = true;

//---------------------------------------------------- setup
void setup()
{
  //if (obj["test"].as<bool>() == true)
  {
    delay(2000);
    //delay(5000);
  }
  Serial.begin(115200);
  //while (!Serial);
  Serial.println("Encoder Init");
  Wire.setClock(400000);
  Wire.setSDA(SDA_MAIN);
  Wire.setSCL(SCL_MAIN);
  Wire.begin(I2C_SLAVE_ADDRESS);
  Wire.onReceive(recv);
  Wire.onRequest(req);
  pinMode(LED_1, OUTPUT);
  digitalWrite(LED_1, 0);
  pinMode(28, OUTPUT);
  digitalWrite(28, 0);
  pinMode(27, OUTPUT);
  digitalWrite(27, 0);

  

  Serial.println("I2C Ready");
  //gpio_set_irq_enabled_with_callback(BTN_START, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
  pinMode(SOLENOID, OUTPUT);
  pinMode(BTN_START, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN_START), open_valve, FALLING);  // configura la interrupción
  pinMode(2, INPUT);
  pinMode(3, INPUT);

  //pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);

  encoder.begin();
  // add_repeating_alarm_us(1e6, alarm_callback, NULL, NULL);
  // doc["valve_open"] = false;
  close_valve();

  if (spiffs_init())
  {
    loadConfig();       // Load and update behaivor of system
  }
}


// ------------------------------------------------------ loop
void loop()
{


  // ------------------------------------- process
  if (newcommand == true )
  {
    deserializeJson(doc_aux, jsonStr);
    //Serial.print("Master: ");
    //serializeJson(doc_aux, Serial);
    //Serial.println();
    
    newcommand = false;

    if (!doc_aux["valve"].isNull())
    {
      if (doc_aux["valve"].as<bool>() == true)
        open_valve();
      else
        close_valve();
    }

    if ((!doc_aux["litros_target"].isNull()) && (!doc_aux["pulsos_litro"].isNull()))
    {
      target_value = (doc_aux["litros_target"].as<int32_t>()) * (doc_aux["pulsos_litro"].as<int32_t>());
    }

    if (!doc_aux["t_update"].isNull())
    {
      doc_conf["t_update"] = doc_aux["t_update"];              // Print and send
      saveConfig = true;
    }


    if (!doc_aux["delta"].isNull())
    {
      doc_conf["delta"] = doc_aux["delta"];                //10 Pulsos detectados en Intervalo2  (DELTA)
      saveConfig = true;
    }

    if (!doc_aux["t_encoder"].isNull())
    {
      doc_conf["t_encoder"] = doc_aux["t_encoder"];    //ENCODER Intervalo de tiempo (milisegundos)
      saveConfig = true;
    }

    if (!doc_aux["t_delta"].isNull())
    {
      doc_conf["t_delta"] = doc_aux["t_delta"];  //DELTA Intervalo de tiempo (500 milisegundos)
      saveConfig = true;
    }


    if (!doc_aux["t_stop"].isNull())
    {
      doc_conf["t_stop"] = doc_aux["t_stop"];// Maximo tiempo desde que se detecto STOP_FLOW 60=30seg
      saveConfig = true;
    }
  }

  // ---------------------------------------------- read encoder
  tiempoActual = millis();
  if (tiempoActual - tiempoAnterior >= intervalo)
  {
    // Ha pasado 1 minuto
    tiempoAnterior = tiempoActual;
    new_value = abs(encoder.getCount()); // Con el absoluto, no importa la dir
    //Serial.print("new_value: ");
    //Serial.println(new_value);

    // ------------------------------------------- check direction
    if (new_value < 0)
    {
      encoder.reset();
      dir = false;
      Serial.println("no ok");
      //gpio_put(LED_1, 1);
      old_value = 0;
      new_value = 0;
      delta = 0;
    }
    else
    {
      dir = true;
      //total_encoder = total_encoder + new_value;
    }

  }

  // ---------------------------------------------- check_delta
  tiempoActual2 = millis();
  if (tiempoActual2 - tiempoAnterior2 >= intervalo2)
  {
    tiempoAnterior2 = tiempoActual2;

    // ------------------------------------- delta is noise?
    delta = new_value - old_value;
    
    if (delta > 0)
    {
      doc_conf["total_encoder"] = total_encoder + delta;
      saveConfig = true;
    }

    if (delta < MAX_DELTA)
    {
      noDelta_timeCounter = millis();
      if (noDelta_timeCounter - noDelta_timeCountePrev >= (noDelta_timeSTOP * 1000))
      {
        noDelta_timeCountePrev = noDelta_timeCounter;
        flow = false;
        //saveConfig = true;
        digitalWrite(LED_1, LOW);
        encoder.reset();
        if (STATE == 1)
        {
          STATE = 2;
        }
      }

    }
    else
    {
      if ((flow == false) || (STATE == 0))
      {
        Serial.println("Flow detected");
        saveConfig = true;
      }
      flow = true;
      noDelta_timeCountePrev = millis();
      digitalWrite(LED_1, HIGH);
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
      /*if (buttonx == true)
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
        }*/
      break;

    case 1: // ------------------------------------------------------process started
      current_value = new_value;
      if (target_value > 0)
      {
        if (current_value >= target_value)
        {
          if (digitalRead(SOLENOID) == LOW)
          {
            Serial.println("TARGET LITROS");
            Serial.println("AUTO STOP");
          }
          close_valve();
          target_value = 0;
          //STATE = 2;
        }

      }
      // ---------------------- open valve button
      /*if (buttonx == 1)
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
        }*/
      break;

    case 2: //------------------------------------------------- stop process, close valve
      Serial.println("STATE 2");
      Serial.println("FLOW STOP");
      close_valve();

      if (new_value < MAX_DELTA)
      {
        STATE = 0;
        noDelta_timeCounter = 0;
      }
      else
      {
        STATE = 3;
        current_value = new_value;
        saveConfig = true;
      }
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
    //doc["pulses"] = new_value;   //Commands
   /* doc["STATE"] = STATE;
    doc["delta"] = delta;
    doc["flow"] = flow;
    doc["current"] = current_value;
    doc["valve"] = !(bool (digitalRead(SOLENOID)));
    doc["total_encoder"] = doc_conf["total_encoder"];
    //memset(resp, 0, sizeof(resp));
    Serial.print("Encoder State: ");
    serializeJson(doc, resp);
    Serial.println(resp);*/
    previousMillis = currentMillis;
  }

  // ----------------------------------------- save new data
  if (saveConfig == true)  // Data change
  {
    saveConfig = false;

   // Serial.println("{\"upload_config\":true}");
    saveConfigData();
    loadConfig();
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
  doc.clear();
  //doc["pulses"] = new_value;   //Commands
  doc["STATE"] = STATE;
  //doc["delta"] = delta;
  //doc["flow"] = flow;
  doc["current"] = current_value;
  doc["valve"] = !((digitalRead(SOLENOID)));
  //doc["dir"] = dir;

  if(STATE == 3)
  {
    doc["total_encoder"] = total_encoder;
  }

  //memset(resp, 0, sizeof(resp));

  char temp[200];
  size_t len = serializeJson(doc, temp);
   memset(resp, 0, sizeof(resp));

  // Copiar solo los bytes útiles al buffer 'resp'
  memcpy(resp, temp, len);

  
  serializeJson(doc, resp);

  Wire.write(resp, len);
}

// ----------------------------------------------------------- open
void open_valve()
{
  STATE = 1;
  //buttonx = true;
  if (digitalRead(SOLENOID) == HIGH)
  {
    //gpio_put(SOLENOID, 0);
    //gpio_put(LED_1, 0);
    Serial.println("Valve OPEN");
    digitalWrite(SOLENOID, LOW);
    //doc["valve"] = true;
  }
}

// ----------------------------------------------------------- close
void close_valve()
{
  //buttonx = false;
  if (digitalRead(SOLENOID) == LOW)
  {
    Serial.println("Valve CLOSED");
    digitalWrite(SOLENOID, HIGH);
    //doc["valve"] = false;
    //delay(100);
    //gpio_put(SOLENOID, 1);
    //gpio_put(LED_1, 1);
  }
}


uint64_t alarm_callback(alarm_id_t id, void *user_data)
{
  // Cambiar el estado del LED cada vez que se llama a la interrupción
  //digitalWrite(LED_PIN, !digitalRead(LED_PIN));

  // Devolver 0 para que el timer siga llamando a esta función
  return 0;
}


// ------------------------------------------------------------------------------------- spiffs_init
bool spiffs_init()
{
  if (!LittleFS.begin())
  {
    Serial.println("{\"spiffs\":false}");
    return false;
  }
  else
  {
    Serial.println("{\"spiffs\":true}");
    listFiles();
    Cfg_get(/*NULL*/);  // Load File from spiffs
    loadConfig();
    return true;
  }
}

// --------------------------------------------------------------------------------------------------- Cfg_get
void Cfg_get()
//  {"method":"Config.Get"}
{
  // open file to load config

  obj_conf = getJSonFromFile(&doc_conf, filename);




  if (obj_conf.size() == 0)
  {
    Serial.println("{\"config_file\":\"empty\"}");
    //doc_conf = getJSonFromFile(&doc, filedefault);
    //Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_default_restore\":true}" : "{\"file_default_restore\":false}");
  }

  //if (obj["test"].as<bool>() == true)
  {
    // Comment for production
    serializeJson(doc_conf, Serial);
    Serial.println();
  }



}


// ------------------------------------------------------------------------------------------------ getJsonFromFile

JsonObject getJSonFromFile(StaticJsonDocument<FILE_SIZE> *doc_conf, String filename)
{


  file = LittleFS.open(filename, "r");
  if (file)
  {
    Serial.println("Opening File");

    size_t size = file.size();
    Serial.println(size);

    if (size > FILE_SIZE)
    {
      Serial.println("Too large file");

    }

    DeserializationError error = deserializeJson(*doc_conf, file);
    if (error)
    {
      // if the file didn't open, print an error:
      //Serial.print(F("Error parsing JSON "));
      //Serial.println(error.c_str());

      // if (forceCleanONJsonError)
      {
        Serial.println("{\"force_empty_json\": true}");
        return doc_conf->to<JsonObject>();
      }
    }

    // close the file:
    file.close();
    Serial.println("{\"json_loaded\": true}");

    return doc_conf->as<JsonObject>();
  } else {
    // if the file didn't open, print an error:
    //Serial.print(F("Error opening (or file not exists) "));
    //Serial.println(filename);

    //Serial.println(F("Empty json created"));
    Serial.println("{\"empty_json\": true}");
    return doc_conf->to<JsonObject>();
  }

}

// ------------------------------------------------------------------------ListFiles
void listFiles() {
  // Abre el directorio raíz

  // open the file for reading:
  /*char buff[32];
    int cnt = 1;
    File f = LittleFS.open("config.json", "r");
    if (f) {
    bzero(buff, 32);
    if (f.read((uint8_t *)buff, 31)) {
      sscanf(buff, "%d", &cnt);
      Serial.printf("I have been run %d times\n", cnt);
    }
    f.close();
    }*/

  Serial.println("Listing Files");

  Dir dir = LittleFS.openDir("/");

  // Recorre los archivos en el directorio raíz
  while (dir.next()) {
    Serial.print("File: ");
    Serial.print(dir.fileName());
    Serial.print("\tSize: ");
    Serial.println(dir.fileSize());
  }
}


// ---------------------------------------------------------------------------------------------------- loadConfig
// Update a new config in the file an change behivor
void loadConfig()
{
  // ----------- Load Counters
  //Serial.println("{\"loadConfig\":true}");

  // ------------------------------ Configs
  if (!doc_conf["total_encoder"].isNull())
    total_encoder = doc_conf["total_encoder"];// Maximo tiempo desde que se detecto STOP_FLOW 60=30seg

  if (!doc_conf["t_update"].isNull())
    interval = doc_conf["t_update"];              // Print and send

  if (!doc_conf["delta"].isNull())
    MAX_DELTA = doc_conf["delta"];                //10 Pulsos detectados en Intervalo2  (DELTA)

  if (!doc_conf["t_encoder"].isNull())
    intervalo = doc_conf["t_encoder"];    //ENCODER Intervalo de tiempo (milisegundos)

  if (!doc_conf["t_delta"].isNull())
    intervalo2 = doc_conf["t_delta"];  //DELTA Intervalo de tiempo (500 milisegundos)

  if (!doc_conf["t_stop"].isNull())
    noDelta_timeSTOP = doc_conf["t_stop"];// Maximo tiempo desde que se detecto STOP_FLOW 60=30seg
}


// --------------------------------------------------------------------------------------------- saveConfigData
void saveConfigData()
{
  saveJSonToAFile(&obj_conf, filename);
  
  //Serial.println(saveJSonToAFile(&obj_conf, filename) ? "{\"config_update_spiffs\":true}" : "{\"conifg_update_spiffs\":false}");
  //if (obj_conf["test"].as<bool>())
  //serializeJson(obj_conf, Serial);

  
}

// ----------------------------------------------------------------------------------------- saveJSonToAFile
bool saveJSonToAFile(JsonObject * doc_conf, String filename) {
  //SD.remove(filename);

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  //Serial.println(F("Open file in write mode"));
  file = LittleFS.open(filename, "w");
  if (file) {
    //Serial.print(F("Filename --> "));
    //Serial.println(filename);

    //Serial.print(F("Start write..."));

    serializeJson(*doc_conf, file);

    //Serial.print(F("..."));
    // close the file:
    file.close();
    //Serial.println(F("done."));

    return true;
  } else {
    // if the file didn't open, print an error:
    Serial.print(F("Error opening "));
    //Serial.println(filename);

    return false;
  }
}
