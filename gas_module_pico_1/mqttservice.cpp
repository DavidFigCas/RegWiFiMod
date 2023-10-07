#include "mqttservice.h"

WiFiClient espClient;
PubSubClient Mclient(espClient);
WiFiClient client_http;

const char* publish_topic = "/out";
const char* subcribe_topic = "/in";
char buffer_union_publish[30];
char buffer_union_subcribe[30];
char buffer_msg[30];
//const char* client_id = "maquina00018";

// -------------------------------------------------- mqtt_init
void mqtt_init()
{
  if (obj["enable_mqtt"].as<bool>())
  {

    Serial.println("{\"mqtt\":\"init\"}"); Mclient.setBufferSize(1024);
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
  strcat(strcpy(buffer_union_publish, obj["id"].as<const char*>()), publish_topic);
  snprintf(buffer_msg, sizeof(buffer_msg), "%ld", STATE);
  Mclient.publish(buffer_union_publish, buffer_msg);
}

//--------------------------------------------------- callback
void callback(char* topic, byte* payload, unsigned int length)
{
  //strcat(strcpy(buffer_union_publish, client_id), publish_topic);
  //strcat(strcpy(buffer_union_publish, obj["id"].as<const char*>()), publish_topic);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}


//--------------------------------------------------- reconnect
bool reconnect()
{
  bool recsta = false;

  //strcat(strcpy(buffer_union_subcribe, client_id), subcribe_topic);
  strcat(strcpy(buffer_union_subcribe, obj["id"].as<const char*>()), subcribe_topic);

  if (!Mclient.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (Mclient.connect(obj["id"].as<const char*>()/*, mqttUser, mqttPassword*/))
    {
      Serial.println("connected");
      Mclient.subscribe(buffer_union_subcribe);
      recsta =  true;
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(Mclient.state());
      Serial.println(" try in the next");
      recsta =  false;
    }
  }
  return recsta;
}
