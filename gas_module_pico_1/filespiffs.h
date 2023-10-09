#ifndef FILESPIFFS_H
#define FILESPIFFS_H

#include "system.h"
//#include <LittleFS.h>


#define FILE_SIZE   512
#define LIST_SIZE   2048

extern JsonObject obj;
extern StaticJsonDocument<FILE_SIZE> doc;
//extern DynamicJsonDocument doc(FILE_SIZE);
extern JsonArray obj_list;
extern StaticJsonDocument<LIST_SIZE> doc_list;
//extern DynamicJsonDocument doc_list;

extern const char *filename;
extern const char *filedefault;
extern const char *filelist;
extern volatile bool saveConfig;

extern File file;

JsonObject getJSonFromFile(/*DynamicJsonDocument *doc*/ StaticJsonDocument<FILE_SIZE> *doc, String filename,bool forceCleanONJsonError = true);
/*static*/ void Cfg_get(/*struct jsonrpc_request * r*/);
bool saveJSonToAFile(JsonObject * doc, String filename);
void saveConfigData();
void saveListData();
bool spiffs_init();
bool saveJSonArrayToAFile(JsonArray * doc_list, String filename);
//JsonArray getJSonArrayFromFile(StaticJsonDocument<LIST_SIZE> *doc_list, String filename,bool forceCleanONJsonError = true);
JsonArray getJSonArrayFromFile(StaticJsonDocument<LIST_SIZE> *doc_list, String filename);


#endif  // FILESPIFFS_H
