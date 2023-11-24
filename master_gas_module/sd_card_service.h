#ifndef SD_CARD_SERVICE_H
#define SD_CARD_SERVICE_H

#include "system.h"

bool SD_Init(void);
JsonObject getJSonFromSD( StaticJsonDocument<FILE_SIZE> *doc, const char * path, bool forceCleanONJsonError);
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
void createDir(fs::FS &fs, const char * path);
void removeDir(fs::FS &fs, const char * path);
void readFile(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);
void renameFile(fs::FS &fs, const char * path1, const char * path2);
bool testFileIO(fs::FS &fs, const char * path);
void deleteFile(fs::FS &fs, const char * path);
void dirTest(fs::FS &fs, const char * dirname);

#endif
