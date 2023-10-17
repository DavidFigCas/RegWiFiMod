#include "system.h"





// ------------------------------------------------------ (CONFIG) setup
void setup()
{
  delay(5000);
  system_init();
  //search_nclient(0);
  //check_status();
  //saveNewlog();
}


// ------------------------------------------------------ loop
void loop()
{
  byte error, address;
  int nDevices = 0;

  if (millis() - mainRefresh > mainTime)
  {
    mainRefresh = millis();
    Serial.println("Scanning for I2C devices ...");
    for (address = 0x01; address < 0x7f; address++) {
      Wire.beginTransmission(address);
      error = Wire.endTransmission();
      delay(100);
      if (error == 0) {
        Serial.printf("I2C device found at address 0x%02X\n", address);
        nDevices++;
      } else if (error != 2) {
        Serial.printf("Error %d at address 0x%02X\n", error, address);
      }
    }
    if (nDevices == 0) {
      Serial.println("No I2C devices found");
    }

    buffer[0] = err_data;
    i2c_write_blocking(i2c0, 0x5A, &REG_SET_STATUS, 1, true);
    i2c_write_blocking(i2c0, 0x5A, buffer, 1, false);
    sleep_ms(100);
    i2c_write_blocking(i2c0, 0x5E, &REG_SET_STATUS, 1, true);
    i2c_write_blocking(i2c0, 0x5E, buffer, 1, false);
  }

  gps_update();


  // ----------------------------------------- check internet
  if (wifi_check())
  {
    //update_clock();
    //read_clock();
    if (mqtt_check())
    {
      // ------------------------------------------- Send Log
      if (send_log == true)
      {
        Serial.println("mqtt sending");

        //saveNewlog();

        strcpy(buffer_union_publish, obj["id"].as<const char*>());
        strcat(buffer_union_publish, publish_topic);
        strcat(buffer_union_publish, log_topic);

        JsonArray logObject = obj_log;
        size_t serializedLength = measureJson(logObject) + 1;
        char tempBuffer[serializedLength];
        serializeJson(logObject, tempBuffer, serializedLength);
        strcpy(buffer_msg, tempBuffer);

        Mclient.publish(buffer_union_publish, buffer_msg);
        send_log = false;
      }
    }

  }

  // ------------------------------------------- Clear Log
  if (clear_log == true)
  {
    obj_log.clear();
    Serial.println(saveJSonArrayToAFile(&obj_log, filelog) ? "{\"log_clear_spiffs\":true}" : "{\"log_clear_spiffs\":false}");
    clear_log = false;
  }


  // ------------------------------------------- MQTT new LOG
  if (new_log == true)
  {
    saveNewlog();
    new_log = false;
  }

  Serial.print("STATE: ");
  Serial.println(STATE, BIN);
  Serial.print("ToDO: ");
  Serial.println(todo_byte, BIN);
  Serial.print("Client: ");
  for (int i = 0; i < 4; i++)
    Serial.print(nclient_data[i]);
  Serial.println();

  //check_status();
}


// ---------------------------------------------- I2C new command
if (newcommand)
{
  Serial.print("New Command ToDo: "); Serial.println(todo_byte, BIN);


  if (todo_byte & (1 << 6)) {  // ---- Search Client
    //new_nclient = true;
    //if (new_nclient)
    //{
    nclient = 0;
    nclient |= (uint32_t)nclient_data[0] << 24; // Byte más significativo
    nclient |= (uint32_t)nclient_data[1] << 16;
    nclient |= (uint32_t)nclient_data[2] << 8;
    nclient |= (uint32_t)nclient_data[3];
    search_nclient(nclient);
    todo_byte &= ~(1 << 6);  // Reset ToDo bit
    //}
  }

  if (todo_byte & (1 << 5)) {  // New LOG
    //new_nclient = true;
    //if (new_nclient)
    //{
    saveNewlog();
    todo_byte &= ~(1 << 5);  // Reset ToDo bit
    //}
  }
  newcommand = 0;
  todo_byte = 0;
}

if (new_litros)
{
  Serial.println("NEW Litros: ");
  //Serial.println(nclient);
  for (int i = 0; i < 4; i++)
  {
    Serial.println(litros_num[i]);
  }
  new_litros = 0;
}

// ------------------------------- gps










// ----------------------------------------- save new data
if (saveConfig)  // Data change
{
  //saveConfig = false;
  //Serial.println("{\"upload_config_from_loop\":true}");
  //saveConfigData();

  //Serial.println("saving config");


  //DynamicJsonDocument json(1024);

  //for (WiFiManagerParameter* p : customParams) {
  //  doc[p->getID()] = p->getValue();
  //}

  //File configFile = SPIFFS.open("/config.json", "w");
  //if (!configFile) {
  //  Serial.println("failed to open config file for writing");
  //}

  //serializeJson(obj, Serial);
  //serializeJson(obj, configFile);
  //configFile.close();
  //end save
  //ESP.restart();

  //loadConfig();

  // ----------------------------------------- save new data
  //if (flag_newList)
  //{
  //Serial.println("{\"upload_list\":true}");
  //saveListData();
  //flag_newList = false;
  //loadConfig();
  //saveConfig = false;
  //ESP.restart();
  //return;
  //}
  //else
  //{
  Serial.println("{\"upload_config\":true}");
  saveConfigData();
  loadConfig();
  //ESP.restart();
  //}

  saveConfig = false;
}


}
