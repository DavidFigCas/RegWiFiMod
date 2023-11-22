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

JsonObject newLogEntry;

StaticJsonDocument<200> doc_aux;
StaticJsonDocument<200> doc_display;  // Crea un documento JSON con espacio para 200
StaticJsonDocument<200> doc_encoder;  // Crea un documento JSON con espacio para 200


const char* filename = "/config.json";
const char *filedefault = "/default.json";
const char *filelist = "/list.json";
const char *filelog = "/log.json";
volatile bool saveConfig = false;

File file;




// ------------------------------------------------------------------------------------- spiffs_init
bool spiffs_init()
{
  /*if (!LittleFS.begin()) {
    Serial.println("{\"spiffs\":false}");
    return false;
    } else {
    Serial.println("{\"spiffs\":true}");
    Cfg_get();  // Load File from spiffs
    return true;
    }*/

  // SPIFFS Init
  if (!SPIFFS.begin(true)) {
    Serial.println("{\"spiffs\":false}");
    return false;
  } else {
    Serial.println("{\"spiffs\":true}");
    Cfg_get(/*NULL*/);  // Load File from spiffs
    loadConfig();       // Load and update behaivor of system
    return true;
  }
}



// --------------------------------------------------------------------------------------------------- Cfg_get
/*static*/ void Cfg_get(/*struct jsonrpc_request * r*/)
//  {"method":"Config.Get"}
{
  // open file to load config

  obj = getJSonFromFile(SPIFFS, &doc, filename);
  obj_list = getJSonArrayFromFile(SPIFFS, &doc_list, filelist);
  obj_log = getJSonArrayFromFile(SD, &doc_log, filelog);


  if (obj_list.isNull())
  {
    Serial.println("Rehaciendo null");
    obj_list = doc_list.to<JsonArray>();
  }


  if (obj.size() == 0)
  {
    Serial.println("{\"config_file\":\"empty\"}");
    obj = getJSonFromFile(SPIFFS, &doc, filedefault);
    Serial.println(saveJSonToAFile(SPIFFS, &obj, filename) ? "{\"file_default_restore\":true}" : "{\"file_default_restore\":false}");
  }

  //if (obj["test"].as<bool>() == true)
  {
    // Comment for production
    serializeJson(obj, Serial);
    Serial.println();
    serializeJsonPretty(obj_list, Serial);
    Serial.println();

    serializeJsonPretty(obj_log, Serial);
    Serial.println();

    Serial.println("SPIFFS");

    obj_log = getJSonArrayFromFile(SPIFFS, &doc_log, filelog);
    serializeJsonPretty(obj_log, Serial);
  }



}


// ----------------------------------------------------------------------------------------- saveJSonToAFile
bool saveJSonToAFile(fs::FS &fs, JsonObject * doc, String filename)
{
  //SD.remove(filename);

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  //Serial.println(F("Open file in write mode"));
  //file = LittleFS.open(filename, "w");
  file = fs.open(filename, FILE_WRITE);
  if (file) {
    Serial.print(F("Filename --> "));
    Serial.println(filename);

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
    Serial.println(filename);

    return false;
  }
}


// ------------------------------------------------------------------------------------------------ getJsonFromFile

JsonObject getJSonFromFile(fs::FS &fs, StaticJsonDocument<FILE_SIZE> *doc, String filename, bool forceCleanONJsonError)
{
  // open the file for reading:
  //file = LittleFS.open(filename, "r");
  file = fs.open(filename);
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
    Serial.println(filename);

    //Serial.println(F("Empty json created"));
    Serial.println("{\"empty_json\": true}");
    return doc->to<JsonObject>();
  }

}


// --------------------------------------------------------------------------------------------- saveConfigData
void saveConfigData()
{
  Serial.println(saveJSonToAFile(SPIFFS, &obj, filename) ? "{\"config_update_spiffs\":true}" : "{\"conifg_update_spiffs\":false}");
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

JsonArray getJSonArrayFromFile(fs::FS &fs, StaticJsonDocument<LIST_SIZE> *doc_list, String filename)
{
  // open the file for reading:
  //file = LittleFS.open(filename, "r");
  file = fs.open(filename);
  //if (file)
  {
    Serial.println("Opening File");

    size_t size = file.size();
    //Serial.println(size);

    if (size > LIST_SIZE)
    {
      Serial.println("Too large LIST");
      //return false;
    }

    DeserializationError error = deserializeJson(*doc_list, file);
    if (error)
    {
      // if the file didn't open, print an error:
      Serial.print(F("Error parsing JSON "));
      Serial.println(error.c_str());

      //if (forceCleanONJsonError)
      {
        return doc_list->to<JsonArray>();
      }
    }

    // close the file:
    file.close();

    return doc_list->as<JsonArray>();
  } 
  //else 
  {
    // if the file didn't open, print an error:
    //Serial.print(F("Error opening (or file not exists) "));
    //Serial.println(filename);

    //Serial.println(F("Empty json created"));
    //return doc_list->to<JsonArray>();
  }

}


// --------------------------------------------------------------------------------------------------- saveJSonArrayToAFile
bool saveJSonArrayToAFile(fs::FS &fs, JsonArray * doc_list, String filename)
{
  //SD.remove(filename);

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  Serial.println(F("Open file in write mode"));

  //file = LittleFS.open(filename, "w");
  file = fs.open(filename, FILE_WRITE);
  if (file) {
   Serial.print(F("Filename --> "));
   Serial.println(filename);

    Serial.print(F("Start write..."));

    serializeJson(*doc_list, file);

    //Serial.print(F("..."));
    // close the file:
    file.close();
    Serial.println(F("done."));

    return true;
  } else {
    // if the file didn't open, print an error:
    Serial.print(F("Error opening "));
    Serial.println(filename);

    return false;
  }
}
