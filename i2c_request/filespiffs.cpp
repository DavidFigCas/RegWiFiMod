#include "filespiffs.h"

JsonObject obj;
StaticJsonDocument<FILE_SIZE> doc;

JsonObject obj_list;
StaticJsonDocument<FILE_SIZE> doc_list;

const char* filename = "/config.json";
const char *filedefault = "/default.json";
const char *filelist = "/list.json";
volatile bool saveConfig = false;

File file;


// ------------------------------------------------------------------------------------- spiffs_init
bool spiffs_init()
{
  if (!SPIFFS.begin(true)) {
    Serial.println("{\"spiffs\":false}");
    return false;
  } else {
    Serial.println("{\"spiffs\":true}");
    Cfg_get(/*NULL*/);  // Load File from spiffs
    return true;
  }
}



// --------------------------------------------------------------------------------------------------- Cfg_get
/*static*/ void Cfg_get(/*struct jsonrpc_request * r*/)
//  {"method":"Config.Get"}
{
  // open file to load config

  obj = getJSonFromFile(&doc, filename);
  obj_list = getJSonFromFile(&doc_list, filelist);


  if (obj.size() == 0)
  {
    Serial.println("{\"config_file\":\"empty\"}");
    obj = getJSonFromFile(&doc, filedefault);
    Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_default_restore\":true}" : "{\"file_default_restore\":false}");
  }

  //if (obj["test"].as<bool>() == true)
  {
    // Comment for production
    serializeJson(obj, Serial);
    Serial.println();
    serializeJson(obj_list, Serial);
    Serial.println();
  }



}


// ----------------------------------------------------------------------------------------- saveJSonToAFile
bool saveJSonToAFile(JsonObject * doc, String filename) {
  //SD.remove(filename);

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  //Serial.println(F("Open file in write mode"));
  file = SPIFFS.open(filename, FILE_WRITE);
  if (file) {
    //Serial.print(F("Filename --> "));
    //Serial.println(filename);

    //Serial.print(F("Start write..."));

    serializeJson(*doc, file);

    //Serial.print(F("..."));
    // close the file:
    file.close();
    //Serial.println(F("done."));

    return true;
  } else {
    // if the file didn't open, print an error:
    //Serial.print(F("Error opening "));
    //Serial.println(filename);

    return false;
  }
}


// ------------------------------------------------------------------------------------------------ getJsonFromFile

JsonObject getJSonFromFile(/*DynamicJsonDocument *doc*/ StaticJsonDocument<FILE_SIZE> *doc, String filename, bool forceCleanONJsonError)
{
  // open the file for reading:
  file = SPIFFS.open(filename);;
  if (file)
  {
    //Serial.println("Opening File");

    size_t size = file.size();
    //Serial.println(size);

    if (size > FILE_SIZE)
    {
      //Serial.println("Too large file");

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

    return doc->as<JsonObject>();
  } else {
    // if the file didn't open, print an error:
    //Serial.print(F("Error opening (or file not exists) "));
    //Serial.println(filename);

    //Serial.println(F("Empty json created"));
    Serial.println("{\"empty_json\": true}");
    return doc->to<JsonObject>();
  }

}


// --------------------------------------------------------------------------------------------- saveConfigData
void saveConfigData()
{
  Serial.println(saveJSonToAFile(&obj, filename) ? "{\"config_update_spiffs\":true}" : "{\"conifg_update_spiffs\":false}");
  if (obj["test"].as<bool>())
    serializeJson(obj, Serial);
}

// ------------------------------------------------------------------------------------------- saveListData
void saveListData()
{
  Serial.println(saveJSonToAFile(&obj_list, filelist) ? "{\"list_update_spiffs\":true}" : "{\"list_update_spiffs\":false}");
  if (obj["test"].as<bool>())
    serializeJson(doc_list, Serial);
}
