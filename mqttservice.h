#ifndef MQTTSERVICE_H
#define MQTTSERVICE_H

#include "system.h"

extern WiFiClient espClient;
extern PubSubClient Mclient;
extern WiFiClient client_http;

extern const char* publish_topic; 
extern const char* subcribe_topic; 
extern char buffer_union_publish[30]; 
extern char buffer_union_subcribe[30];
extern char buffer_msg[30];
extern const char* client_id;

inline void callback(char* topic, byte* payload, unsigned int length);
inline bool reconnect();
inline void mqtt_init();
inline bool mqtt_check();
inline void mqtt_send();



#endif  //
