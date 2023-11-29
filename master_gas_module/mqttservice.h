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
extern const char* get_topic;
extern const char* print_topic;
extern const char* config_topic;
extern const char* wild_topic;
extern const char* gps_topic;
extern const char* status_topic;
extern const char* log_topic;

extern char buffer_union_publish[LOG_SIZE]; 
extern char buffer_union_subcribe[LOG_SIZE];
extern char buffer_msg[LOG_SIZE];
extern char buffer_msg_status[STATUS_SIZE];
extern char buffer_msg_list[LIST_SIZE];
extern const char* client_id;

extern volatile boolean send_log;
extern volatile boolean send_list;
extern volatile boolean clear_log;
extern volatile boolean new_log;
extern volatile boolean print_log;
extern volatile boolean flag_new_list;

extern byte STATE, todo_byte;
extern bool newcommand;
extern uint32_t nclient;

void callback(char* topic, byte* payload, unsigned int length);
bool reconnect();
void mqtt_init();
bool mqtt_check();
void mqtt_send();
void mqtt_send_list();
void mqtt_send_file(String file_to_send);



#endif  //
