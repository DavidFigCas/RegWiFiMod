#ifndef FILESPIFFS_H
#define FILESPIFFS_H

#include "system.h"


#define FILE_SIZE   512
#define LIST_SIZE   6048
#define LOG_SIZE   6048

extern JsonObject obj;
extern StaticJsonDocument<FILE_SIZE> doc;

extern JsonArray obj_list;
extern JsonObject obj_in;
extern StaticJsonDocument<LIST_SIZE> doc_list;

extern JsonArray obj_log;
extern StaticJsonDocument<LOG_SIZE> doc_log;

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
void saveNewlog();


#endif  // FILESPIFFS_H
