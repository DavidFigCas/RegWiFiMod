#include "mqttservice.h"

WiFiClient espClient;
PubSubClient Mclient(espClient);
WiFiClient client_http;

const char* publish_topic = "/out";
const char* subcribe_topic = "/in";
const char* list_topic = "/list";
const char* add_topic = "/add";
const char* config_topic = "/config";
const char* log_topic = "/log";
const char* gps_topic = "/gps";
const char* wild_topic = "/#";
char buffer_union_publish[30];
char buffer_union_subcribe[30];
char buffer_msg[1024];
//const char* client_id = "maquina00018";
volatile boolean send_log = false;
volatile boolean clear_log = false;
volatile boolean new_log = false;


// -------------------------------------------------- mqtt_init
void mqtt_init()
{
  //if (obj["enable_mqtt"].as<bool>())
  {

    Serial.println("{\"mqtt\":\"init\"}"); Mclient.setBufferSize(LIST_SIZE);
    Mclient.setServer(obj["mqtt_server"].as<const char*>(), obj["mqtt_port"].as<unsigned int>());
    //client.setServer(obj["mqtt"]["broker"].as<const char*>(),1883);
    //client.setServer("inventoteca.com", 1883);
    Mclient.setCallback(callback);
    // Serial.println(obj["mqtt"]["port"].as<unsigned int>());
    // client_id = obj["id"].as<String>();

  }


}

// ------------------------------------------------- mqtt_check
bool mqtt_check()
{
  // MQTT Enable
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

//--------------------------------------------------- callback
void callback(char* topic, byte* payload, unsigned int length)
{
  char jsonPayload[length + 1]; // +1 para el carácter nulo
  memcpy(jsonPayload, payload, length);
  jsonPayload[length] = '\0'; // Agrega el carácter nulo al final
  Serial.print("Message arrived: ");

  if (obj["test"].as<bool>())
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
    obj_list = doc_m.as<JsonArray>();
    for (JsonObject jsonObject : obj_list) {
      const char* nombre = jsonObject["nombre"];
      int cliente = jsonObject["cliente"];
      float lat = jsonObject["lat"];
      float lon = jsonObject["lon"];
      int litros = jsonObject["litros"];
      float precio = jsonObject["precio"];
      float factor = jsonObject["factor"];

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

    saveListData();
    STATE |= (1 << 4);                  // NEW LIST

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
    obj_list.add(doc_m.as<JsonObject>());
    saveListData();
  }
  else  if (strcmp(topic, strcat(strcat(strcpy(buffer_union_subcribe, obj["id"].as<const char*>()), subcribe_topic), config_topic)) == 0)
  {
    Serial.println("Config");
  }
  else  if (strcmp(topic, strcat(strcat(strcpy(buffer_union_subcribe, obj["id"].as<const char*>()), subcribe_topic), log_topic)) == 0)
  {
    if (strcmp(jsonPayload, "delete") == 0) {
      clear_log = true;
    } else if (strcmp(jsonPayload, "get") == 0) {
      send_log = true;
      Serial.println("prepare send");
    }
    else if (strcmp(jsonPayload, "add") == 0) {
      new_log = true;
      Serial.println("Add new Log");
    }
    
  }
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
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    if (Mclient.connect(clientId.c_str()))
      //if (Mclient.connect(obj["id"].as<const char*>()/*, mqttUser, mqttPassword*/))
      // if (Mclient.connect(macAddress))
    {
      Serial.println("connected");
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
  return recsta;
}
