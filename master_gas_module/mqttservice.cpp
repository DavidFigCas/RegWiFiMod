#include "mqttservice.h"

WiFiClient espClient;
PubSubClient Mclient(espClient);
WiFiClient client_http;

const char* publish_topic = "/out";
const char* subcribe_topic = "/in";
const char* list_topic = "/list";
const char* add_topic = "/add";
const char* get_topic = "/get";
const char* print_topic = "/print";
const char* config_topic = "/config";
const char* log_topic = "/log";
const char* gps_topic = "/gps";
const char* status_topic = "/status";
const char* wild_topic = "/#";
char buffer_union_publish[LOG_SIZE];
char buffer_union_subcribe[LOG_SIZE];
char buffer_msg[LOG_SIZE];
char buffer_msg_status[STATUS_SIZE];
char buffer_msg_list[LIST_SIZE];
volatile boolean send_log = false;
volatile boolean send_list = false;
volatile boolean clear_log = false;
volatile boolean new_log = false;
volatile boolean print_log = false;
volatile boolean flag_new_list = false;
byte STATE, todo_byte;
bool newcommand;
uint32_t nclient;


// -------------------------------------------------- mqtt_init
void mqtt_init()
{
  //if (obj["enable_mqtt"].as<bool>())
  {

    Serial.println("{\"mqtt\":\"init\"}");
    Mclient.setBufferSize(LIST_SIZE);
    Mclient.setServer(obj["mqtt_server"].as<const char*>(), obj["mqtt_port"].as<unsigned int>());
    Mclient.setCallback(callback);
    Mclient.setKeepAlive(30);
  }


}

// ------------------------------------------------- mqtt_check
bool mqtt_check()
{
  // MQTT Enable
  esp_task_wdt_reset();
  if (!Mclient.connected())
  {
    if (reconnect())
      return true;
    else
      return false;

  }
  else
  {
    Mclient.loop();
    return true;
  }
}

//---------------------------------------------------- mqtt_send
void mqtt_send()
{
  Mclient.publish(buffer_union_publish, buffer_msg);
}


// --------------------------------------------------- mqtt_send_file
void mqtt_send_file(String file_to_send)
{
  send_log = false;
  File file = SD.open(file_to_send);
  if (!file) {
    Serial.print("Error al abrir el archivo: ");
    Serial.println(file_to_send);
    return;
  }

  strcpy(buffer_union_publish, obj["id"].as<const char*>());
  strcat(buffer_union_publish, publish_topic);
  strcat(buffer_union_publish, file_to_send.c_str());


  while (file.available())
  {
    String line = file.readStringUntil('\n');
    if (line.length() > 0) {
      // Verificar si la línea es un JSON válido
      //StaticJsonDocument<256> doc;
      //DeserializationError error = deserializeJson(doc, line);
      //if (error) {
      //Serial.print("Error al parsear JSON: ");
      //Serial.println(error.c_str());
      //continue;
      //}

      // Publicar la línea
      Mclient.publish(buffer_union_publish, line.c_str(), line.length());
      delay(100); // Pequeña pausa para evitar saturar el cliente MQTT
      esp_task_wdt_reset();
    }
  }

  file.close();
  Serial.println("Archivo enviado");
}


//---------------------------------------------------- mqtt_send_list
void mqtt_send_list()
{
  strcpy(buffer_union_publish, obj["id"].as<const char*>());
  strcat(buffer_union_publish, publish_topic);
  strcat(buffer_union_publish, list_topic);

  JsonArray logObject = obj_list;
  size_t serializedLength = measureJson(logObject) + 1;
  char tempBuffer[serializedLength];
  serializeJson(logObject, tempBuffer, serializedLength);
  strcpy(buffer_msg_list, tempBuffer);

  Mclient.publish(buffer_union_publish, buffer_msg_list);

  Serial.println("{\"mqtt_list\":\"sending\"}");


}

//--------------------------------------------------- callback
void callback(char* topic, byte* payload, unsigned int length)
{
  esp_task_wdt_reset();
  char jsonPayload[length + 1]; // +1 para el carácter nulo
  memcpy(jsonPayload, payload, length);
  jsonPayload[length] = '\0'; // Agrega el carácter nulo al final
  Serial.print("Message arrived: ");

  //if (obj["test"].as<bool>())
  {
    Serial.print(topic);
    Serial.print("<-- ");
    Serial.print(jsonPayload); // Imprime el payload como cadena
  }
  Serial.println();



  if (strcmp(topic, strcat(strcat(strcpy(buffer_union_subcribe, obj["id"].as<const char*>()), subcribe_topic), list_topic)) == 0)
  {
    // Parsear el payload a un array de objetos JSON
    //DynamicJsonDocument doc_m(LIST_SIZE); // Tamaño máximo del JSON, ajusta según tus necesidades
    StaticJsonDocument<LIST_SIZE> doc_m;

    DeserializationError error = deserializeJson(doc_m, jsonPayload);

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }

    // Verificar que el payload sea un array
    if (!doc_m.is<JsonArray>()) {
      Serial.println("El payload no es un array JSON.");
      return;
    }

    // Iterar sobre los elementos del array
    //obj_list.clear();
    obj_list = doc_m.as<JsonArray>();
    for (JsonObject jsonObject : obj_list)
    {
      const char* nombre = jsonObject["nombre"];
      int cliente = jsonObject["cliente"];
      float lat = jsonObject["lat"];
      float lon = jsonObject["lon"];
      int litros = jsonObject["litros"];
      float precio = jsonObject["precio"];
      float factor = jsonObject["factor"];
      //uint32_t pulsos_litro = jsonObject["pulsos_litro"];

      Serial.println();
      Serial.print("Nombre: ");
      Serial.println(nombre);
      Serial.print("Cliente: ");
      Serial.println(cliente);
      Serial.print("Latitud: ");
      Serial.println(lat, 6); // Imprimir con 6 decimales de precisión
      Serial.print("Longitud: ");
      Serial.println(lon, 6); // Imprimir con 6 decimales de precisión
      Serial.print("Litros: ");
      Serial.println(litros);
      Serial.print("Precio: ");
      Serial.println(precio);
      Serial.print("Factor: ");
      Serial.println(factor);
      Serial.println();
    }

    //
    // Flag save ListData

    //Serial.print("New List Data: ");
    //serializeJson(obj_list,Serial);
    //Serial.println();

    //esp_task_wdt_reset();
    saveListData();
    search_nclient(0);

    //send_list = true;
    //flag_new_list = true;
    mqtt_send_list();


    return;

  }
  else  if (strcmp(topic, strcat(strcat(strcpy(buffer_union_subcribe, obj["id"].as<const char*>()), subcribe_topic), add_topic)) == 0)
  {
    Serial.println("Adding");
    // const size_t len = (length+10);
    const size_t len = 128;
    StaticJsonDocument<len> doc_m; // Tamaño máximo del JSON, ajusta según tus necesidades
    //DynamicJsonDocument doc_m(len); // Tamaño máximo del JSON, ajusta según tus necesidades
    DeserializationError error = deserializeJson(doc_m, jsonPayload);

    // Verificar que el payload sea un object
    //if (!doc_m.is<JsonObject>()) {
    //Serial.println("El payload no es un JSON.");
    //return;
    //}

    // Iterar sobre los elementos del array
    //obj["new"] = "new new";
    //doc_list.add(obj["new"]);
    //obj_list.add(obj["new"].as<JsonObject>());
    //doc_list.add(doc_m_add.as<JsonObject>());
    //obj_list.add(doc_m.as<JsonObject>());
    //saveListData();
    return;
  }
  else  if (strcmp(topic, strcat(strcat(strcpy(buffer_union_subcribe, obj["id"].as<const char*>()), subcribe_topic), get_topic)) == 0)
  {
    send_list = true;
    Serial.println("Consult List");
  }
  else  if (strcmp(topic, strcat(strcat(strcpy(buffer_union_subcribe, obj["id"].as<const char*>()), subcribe_topic), config_topic)) == 0)
  {
    StaticJsonDocument<LIST_SIZE> conf_mqtt_doc;
    Serial.println("Config Update");

    DeserializationError error = deserializeJson(conf_mqtt_doc, jsonPayload);

    if (error) {
      Serial.print("deserialize conf_mqtt_doc failed: ");
      Serial.println(error.c_str());
      return;
    }

    // Recorrer cada par clave-valor en el JSON recibido
    for (JsonPair p : conf_mqtt_doc.as<JsonObject>())
    {
      const char* key = p.key().c_str(); // Obtener la clave
      JsonVariant value = p.value();     // Obtener el valor

      // Verificar si la clave y el valor no son null
      if (key != NULL && !value.isNull())
      {
        obj[key] = value;
      }
    }

    // Serializar y mostrar el JSON actualizado
    serializeJson(conf_mqtt_doc, Serial);
    Serial.println();

    saveConfig = true;
    return;
  }
  else  if (strcmp(topic, strcat(strcat(strcpy(buffer_union_subcribe, obj["id"].as<const char*>()), subcribe_topic), log_topic)) == 0)
    //else  if (strcmp(topic,strcat( strcat(strcat(strcpy(buffer_union_subcribe, obj["id"].as<const char*>()), subcribe_topic), log_topic , get_topic))) == 0)
  {
    if (strcmp(jsonPayload, "delete") == 0)
    {
      clear_log = true;
      return;
    }

    // else if (strcmp(jsonPayload, "get") == 0)
    //{
    //send_log = true;
    //Serial.println("prepare send");
    //return;
    //}
    //else if (strcmp(jsonPayload, "print") == 0)
    //{
    //print_log = true;
    //Serial.println("Print All Logs");
    //return;
    //}

  }
  else  if (strcmp(topic, (strcat( strcat(strcat(strcpy(buffer_union_subcribe, obj["id"].as<const char*>()), subcribe_topic), log_topic ), print_topic))) == 0)
  {
    print_log = true;
    Serial.print("Print Logs: ");
    consult_filelog = "/logs/";
    consult_filelog += jsonPayload;
    Serial.println(consult_filelog);
    return;
  }

  else  if (strcmp(topic, (strcat( strcat(strcat(strcpy(buffer_union_subcribe, obj["id"].as<const char*>()), subcribe_topic), log_topic ), get_topic))) == 0)
  {
    send_log = true;
    Serial.print("Prepare to SEND file: ");
    //consult_filelog = "/logs/";
    file_to_send = jsonPayload;
    Serial.println(file_to_send);
    return;
  }
  return;
}



//--------------------------------------------------- reconnect
bool reconnect()
{
  bool recsta = false;

  //strcat(strcpy(buffer_union_subcribe, client_id), subcribe_topic);
  strcat(strcpy(buffer_union_subcribe, obj["id"].as<const char*>()), subcribe_topic);
  const char* macAddress = "pico2w";
  //const char* macAddress = getMACAddress();

  if (!Mclient.connected())
  {
    Serial.print("Attempting MQTT connection...");
    String clientId = "GasCar-";
    clientId += String(random(0xffff), HEX);

    // Verificar si las claves 'mqtt_user' y 'mqtt_password' existen y no son null
    if (obj.containsKey("mqtt_user") && !obj["mqtt_user"].isNull() && obj.containsKey("mqtt_pass") && !obj["mqtt_pass"].isNull())
    {

      const char* mqtt_user = obj["mqtt_user"].as<const char*>();
      const char* mqtt_pass = obj["mqtt_pass"].as<const char*>();

      // Configurar usuario y contraseña
      if (Mclient.connect(obj["id"].as<const char*>(), mqtt_user, mqtt_pass))
      {
        Serial.println("connected whit user/pass");
        Mclient.subscribe(strcat(strcat(strcpy(buffer_union_subcribe, obj["id"].as<const char*>()), subcribe_topic), wild_topic));
        STATE |= (1 << 0);                  // MQTT state OK
        recsta =  true;
      }
      else
      {
        Serial.print("failed, rc=");
        Serial.print(Mclient.state());
        Serial.println(" try in the next");

        STATE &= ~(1 << 0);                 // MQTT error
        recsta =  false;
      }
    }
    else
    {
      if (Mclient.connect(clientId.c_str()))
      {
        Serial.println("connected, NO user/pass");
        Mclient.subscribe(strcat(strcat(strcpy(buffer_union_subcribe, obj["id"].as<const char*>()), subcribe_topic), wild_topic));
        STATE |= (1 << 0);                  // MQTT state OK
        recsta =  true;
      }
      else
      {
        Serial.print("failed, rc=");
        Serial.print(Mclient.state());
        Serial.println(" try in the next");

        STATE &= ~(1 << 0);                 // MQTT error
        recsta =  false;
      }
    }


  }
  return recsta;
}
