#ifndef FILESPIFFS_H
#define FILESPIFFS_H

#include "system.h"


extern JsonObject obj;
extern StaticJsonDocument<FILE_SIZE> doc;
extern StaticJsonDocument<FILE_SIZE> status_doc;

extern JsonArray obj_list;
extern JsonObject obj_in;
extern StaticJsonDocument<LIST_SIZE> doc_list;

extern JsonArray obj_log;
extern StaticJsonDocument<LOG_SIZE> doc_log;

extern JsonObject newLogEntry;

//extern StaticJsonDocument<200> doc;  // Asegúrate de que el tamaño sea suficiente para tu objeto JSON
extern StaticJsonDocument<200> doc_aux;  // Crea un documento JSON con espacio para 200
extern StaticJsonDocument<200> doc_display;  // Crea un documento JSON con espacio para 200
extern StaticJsonDocument<200> doc_encoder;  // Crea un documento JSON con espacio para 200

extern const char *fileconfig;
extern const char *filedefault;
extern const char *filelist;
extern String filelog;
extern String consult_filelog;
extern String file_to_send;
extern volatile bool saveConfig;

extern File file;

JsonObject getJSonFromFile(fs::FS &fs,StaticJsonDocument<FILE_SIZE> *doc, const char * path,bool forceCleanONJsonError = true);
bool saveJSonToAFile(fs::FS &fs,JsonObject * doc, const char * path);
void saveConfigData();
void saveListData();
bool spiffs_init();
bool saveJSonArrayToAFile(fs::FS &fs, JsonArray * doc_list, const char * path);
//JsonArray getJSonArrayFromFile(StaticJsonDocument<LIST_SIZE> *doc_list, String filename,bool forceCleanONJsonError = true);
JsonArray getJSonArrayFromFile(fs::FS &fs, StaticJsonDocument<LIST_SIZE> *doc_list, const char * path);
//void saveNewlog();


#endif  // FILESPIFFS_H
