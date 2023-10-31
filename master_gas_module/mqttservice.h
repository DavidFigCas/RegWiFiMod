#ifndef MQTTSERVICE_H
#define MQTTSERVICE_H

#include "system.h"

extern WiFiClient espClient;
extern PubSubClient Mclient;
extern WiFiClient client_http;

extern const char* publish_topic; 
extern const char* subcribe_topic; 

extern const char* list_topic;
extern const char* add_topic;
extern const char* config_topic;
extern const char* wild_topic;
extern const char* gps_topic;
extern const char* log_topic;

extern char buffer_union_publish[30]; 
extern char buffer_union_subcribe[30];
extern char buffer_msg[1024];
extern const char* client_id;

extern volatile boolean send_log;
extern volatile boolean clear_log;
extern volatile boolean new_log;
extern byte STATE, todo_byte;
extern bool newcommand;
extern uint32_t nclient;

void callback(char* topic, byte* payload, unsigned int length);
bool reconnect();
void mqtt_init();
bool mqtt_check();
void mqtt_send();



#endif  //
