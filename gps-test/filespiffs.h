#ifndef FILESPIFFS_H
#define FILESPIFFS_H

#include "system.h"

#define FILE_SIZE   1750

extern JsonObject obj;
extern StaticJsonDocument<FILE_SIZE> doc;
extern JsonObject obj_list;
extern StaticJsonDocument<FILE_SIZE> doc_list;

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

#endif  // FILESPIFFS_H
