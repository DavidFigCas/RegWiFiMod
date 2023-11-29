#include "filespiffs.h"

JsonObject obj;
StaticJsonDocument<FILE_SIZE> doc;
StaticJsonDocument<FILE_SIZE> status_doc;
//DynamicJsonDocument doc(FILE_SIZE);

JsonArray obj_list;
JsonObject obj_in;
StaticJsonDocument<LIST_SIZE> doc_list;

JsonArray obj_log;
StaticJsonDocument<LOG_SIZE> doc_log;

//JsonObject newLogEntry;

StaticJsonDocument<200> doc_aux;
StaticJsonDocument<200> doc_display;  // Crea un documento JSON con espacio para 200
StaticJsonDocument<200> doc_encoder;  // Crea un documento JSON con espacio para 200


const char* fileconfig = "/config.json";
const char *filedefault = "/default.json";
const char *filelist = "/list.json";
String filelog;
String consult_filelog;
String file_to_send;
volatile bool saveConfig = false;

File file;




// ------------------------------------------------------------------------------------- spiffs_init
bool spiffs_init()
{
  // SPIFFS Init
  if (!SPIFFS.begin(true)) 
  {
    Serial.println("{\"spiffs\":false}");
    return false;
  } 
  else 
  {
    Serial.println("{\"spiffs\":true}");
    return true;
  }
}


// ----------------------------------------------------------------------------------------- saveJSonToAFile
bool saveJSonToAFile(fs::FS &fs, JsonObject * doc, const char * path)
{
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  Serial.println(F("Open file in write mode"));
  file = fs.open(path, FILE_WRITE);
  if (file) {
    Serial.print(F("Filename --> "));
    Serial.println(path);

    Serial.print(F("Start write..."));

    serializeJson(*doc, file);

    Serial.print(F("..."));
    //close the file:
    file.close();
    Serial.println(F("done."));

    return true;
  } else {
    // if the file didn't open, print an error:
    Serial.print(F("Error opening "));
    Serial.println(path);
    file.close();
    return false;
  }
}


// ------------------------------------------------------------------------------------------------ getJsonFromFile

JsonObject getJSonFromFile(fs::FS &fs, StaticJsonDocument<FILE_SIZE> *doc, const char * path, bool forceCleanONJsonError)
{
  // open the file for reading:
  //file = LittleFS.open(filename, "r");

  file = fs.open(path);
  if (file)
  {
    Serial.print("Opening File");
    Serial.println(path);

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
      Serial.print(F("Error parsing JSON "));
      Serial.println(error.c_str());

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
    Serial.print(F("Error opening (or file not exists) "));
    Serial.println(path);

    //Serial.println(F("Empty json created"));
    Serial.println("{\"empty_json\": true}");
    file.close();
    return doc->to<JsonObject>();
  }

}


// --------------------------------------------------------------------------------------------- saveConfigData
void saveConfigData()
{
  Serial.println(saveJSonToAFile(SPIFFS, &obj, fileconfig) ? "{\"config_update_spiffs\":true}" : "{\"conifg_update_spiffs\":false}");
  if (obj["test"].as<bool>())
    serializeJson(obj, Serial);
}

// ------------------------------------------------------------------------------------------- saveListData
void saveListData()
{
  //serializeJson(obj_list, Serial);
  Serial.println(saveJSonArrayToAFile(SPIFFS, &obj_list, filelist) ? "{\"list_update_spiffs\":true}" : "{\"list_update_spiffs\":false}");
  if (obj["test"].as<bool>())
    serializeJson(obj_list, Serial);
}


// ------------------------------------------------------------------------------------------------ getJsonArrayFromFile

JsonArray getJSonArrayFromFile(fs::FS &fs, StaticJsonDocument<LIST_SIZE> *doc_gen, const char * path)
{
  // open the file for reading:
  file = fs.open(path);
  if (file)
  {
    Serial.print("Opening File Array:");
    Serial.println(path);

    size_t size = file.size();
    //Serial.println(size);

    if (size > LIST_SIZE)
    {
      Serial.println("Too large LIST");
      //return false;
    }

    DeserializationError error = deserializeJson(*doc_gen, file);
    if (error)
    {
      // if the file didn't open, print an error:
      Serial.print(F("Error parsing JSON "));
      Serial.println(error.c_str());

      //if (forceCleanONJsonError)
      {
        return doc_gen->to<JsonArray>();
      }
    }

    // close the file:
    file.close();
    return doc_gen->as<JsonArray>();
  }
  else
  {
    // if the file didn't open, print an error:
    Serial.print(F("Error opening (or file not exists) "));
    Serial.println(path);

    Serial.println(F("Empty json created"));
    file.close();
    return doc_gen->to<JsonArray>();
  }


}


// --------------------------------------------------------------------------------------------------- saveJSonArrayToAFile
bool saveJSonArrayToAFile(fs::FS &fs, JsonArray * doc_gen2, const char * path)
{
  Serial.println(F("Open file Array in write mode"));
  file = fs.open(path, FILE_WRITE);
  if (file)
  {
    Serial.print(F("Filename --> "));
    Serial.println(path);

    Serial.print(F("Start write..."));

    serializeJson(*doc_gen2, file);

    Serial.print(F("..."));
    // close the file:
    file.close();
    Serial.println(F("done."));

    return true;
  }
  else
  {
    // if the file didn't open, print an error:
    Serial.print(F("Error opening "));
    Serial.println(path);
    file.close();
    return false;
  }
}
