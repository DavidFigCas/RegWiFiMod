#include "sd_card_service.h"


// --------------------------------------------------------- dir_exist
void dirTest(fs::FS &fs, const char * dirname)
{
  if (!fs.exists(dirname))
  {
    if (fs.mkdir(dirname))
    {
      Serial.print("Create Dir: ");
      Serial.println(dirname);
    }
    else
    {
      Serial.print("EROOR Dir: ");
      Serial.println(dirname);
    }
  }
}

// --------------------------------------------------------- SD_Init
bool SD_Init(void)
{
  

  //if (!sd_ready)
  uint64_t cardSize;
  uint8_t cardType;
  //sd_ready = SD.begin();

  SPI.begin(SD_SCLK, SD_MISO, SD_MOSI);
  //if (!SD.begin(SD_CS))
  sd_ready = SD.begin(SD_CS);


  if (!sd_ready)
  {
    Serial.println("> It looks like you haven't inserted the SD card..");
    //Serial.println("{\"retry_sd\":true}");
    if (obj["sd_try"].isNull())
      obj["sd_try"] = 0;

    //obj["sd_try"] = (obj["sd_try"].as< unsigned int>()) + 1;
    //saveConfigData();
    //if ((obj["sd_try"].as< unsigned int>()  < 3) || (obj["sd_try"].as< unsigned int>()  > 10))
    {
      //Serial.println("reboot");
      //ESP.restart();
    }
    return sd_ready;
  }
  else
  {
    cardSize = SD.cardSize() / (1024 * 1024);
    String str = "> SDCard Size: " + String(cardSize) + "MB";
    Serial.println(str);


    cardType = SD.cardType();

    if (cardType == CARD_NONE)
    {
      Serial.println("No SD card attached");
      sd_ready = false;
    }

    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC)
    {
      Serial.println("MMC");
    }
    else if (cardType == CARD_SD)
    {
      Serial.println("SDSC");
    }
    else if (cardType == CARD_SDHC)
    {
      Serial.println("SDHC");
    }
    else
    {
      Serial.println("UNKNOWN");
      sd_ready = false;
      ESP.restart();
    }

    // if ((SD.cardSize() != 0) && ((SD.totalBytes() == 0) || (SD.usedBytes() == 0)))
    //{
    //Serial.println("SD Error, reboot");
    //ESP.restart();
    //}


    dirTest(SD, "/gps");
    dirTest(SD, "/servicios");
    dirTest(SD, "/logs");
    dirTest(SD, "/reportes");

    consult_filelog = "/logs/" + String(anio) + "_" + String(mes) + "_" + String(dia_hoy) + ".json";




    listDir(SD, "/", 2);
    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));

    status_doc["sd"] = bool(sd_ready);
    obj["sd_try"] = 0;
    saveConfig = true;
    return sd_ready;
  }

}


// ------------------------------------------------------------------------------------------------ getJsonFromSD
JsonObject getJSonFromSD( StaticJsonDocument<FILE_SIZE> *doc, const char * path, bool forceCleanONJsonError)
{
  // open the file for reading:
  //file = LittleFS.open(filename, "r");
  /*file = SPIFFS.open(path);
    if (file)
    {
    //Serial.println("Opening File");

    size_t size = file.size();
    //Serial.println(size);

    if (size > FILE_SIZE)
    {
      Serial.println("Too large file");

    }

    DeserializationError error = deserializeJson(*doc, file);
    if (error)
    {
      // if the file didn't open, print an error:
      //Serial.print(F("Error parsing JSON "));
      //Serial.println(error.c_str());

      if (forceCleanONJsonError)
      {
        Serial.println("{\"force_empty_json\": true}");
        return doc->to<JsonObject>();
      }
    }

    // close the file:
    file.close();
    Serial.println("{\"json_loaded\": true}");

    return doc->as<JsonObject>();
    } else {
    // if the file didn't open, print an error:
    //Serial.print(F("Error opening (or file not exists) "));
    //Serial.println(filename);

    //Serial.println(F("Empty json created"));
    Serial.println("{\"empty_json\": true}");
    return doc->to<JsonObject>();
    }*/

}

// ---------------------------------------------------------listDir
void listDir(fs::FS &fs, const char * dirname, uint8_t levels)
{
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.path(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

// ---------------------------------------------------------createDir
void createDir(fs::FS &fs, const char * path)
{
  Serial.printf("Creating Dir: %s\n", path);
  if (fs.mkdir(path)) {
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

// ---------------------------------------------------------removeDir
void removeDir(fs::FS &fs, const char * path)
{
  Serial.printf("Removing Dir: %s\n", path);
  if (fs.rmdir(path)) {
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}

// ---------------------------------------------------------readFile
void readFile(fs::FS &fs, const char * path)
{
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    sd_ready = false;
    file.close();
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}


// ---------------------------------------------------------writeFile
void writeFile(fs::FS &fs, const char * path, const char * message)
{
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);

  if (!file)
  {
    Serial.println("Failed to open file for writing");
    file.close();
    sd_ready = false;
    //SD_Init();
    return;
  }
  else
  {
    if (file.print(message))
    {
      Serial.println("File written");
    }
    else
    {
      Serial.println("Write failed");
      sd_ready = false;
    }
  }

  file.close();
}


// ---------------------------------------------------------appendFile
void appendFile(fs::FS &fs, const char * path, const char * message)
{
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println("Failed to open file, not exist");
    file.close();
    
    Serial.println("Retry");
    delay(50);
    file = fs.open(path, FILE_APPEND);
    if (file.print(message))
    {
      Serial.print("Append File size: ");
      Serial.println(file.size());
      sd_ready = true;
    }
    else
    {
      Serial.println("Append failed again");
      sd_ready = false;
    }

    // Probar iniciar la SD aqui
    //return;
  }
  else
  {
    if (file.print(message))
    {
      Serial.print("Append File size: ");
      Serial.println(file.size());
      sd_ready = true;
    }
    else
    {
      Serial.println("Append failed, retry");
      delay(50);
      file = fs.open(path, FILE_APPEND);
      if (file.print(message))
      {
        Serial.print("Append File size: ");
        Serial.println(file.size());
        sd_ready = true;
      }
      else
      {
        Serial.println("Append failed again");
        sd_ready = false;
      }

    }
  }

  file.close();
}


// ---------------------------------------------------------renameFile
void renameFile(fs::FS &fs, const char * path1, const char * path2)
{
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}


// ---------------------------------------------------------deleteFile
void deleteFile(fs::FS &fs, const char * path)
{
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}


// ---------------------------------------------------------testFileIO
bool testFileIO(fs::FS &fs, const char * path)
{
  Serial.print("Testing: ");
  Serial.println(path);

  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;

  if (file)
  {
    len = file.size();
    if (len <= 0)
    {
      file.close();
      Serial.println("Failed, size error");
      return false;
    }
    else
    {
      size_t flen = len;
      start = millis();
      while (len)
      {
        size_t toRead = len;
        if (toRead > 512)
        {
          toRead = 512;
        }
        file.read(buf, toRead);
        len -= toRead;
      }
      end = millis() - start;
      Serial.printf("%u bytes read for %u ms\n", flen, end);
      file.close();
      return true;
    }

  }
  else
  {
    file.close();
    Serial.println("Failed to open file for reading");
    return false;
  }


  /*file = fs.open(path, FILE_WRITE);
    if (!file) {
    Serial.println("Failed to open file for writing");
    return;
    }

    size_t i;
    start = millis();
    for (i = 0; i < 2048; i++) {
    file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();*/
}
