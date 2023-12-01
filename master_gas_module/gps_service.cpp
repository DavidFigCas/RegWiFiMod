#include "gps_service.h"

TinyGPSPlus gps;


// ------------------------------------------------------ save gps_log
void save_gps_log()
{
  //check SD

  if (sd_ready)
  {
    // Guarda cada minuto la posicion
    // ----------------------------------------------- 1 minute refresh
    //if (millis() - previousMillisGPS >= intervalGPS)
    //if (millis() - previousMillisGPS >= 5000)
    {
      // Guarda la última vez que actualizaste el evento
      previousMillisGPS = millis();

      //StaticJsonDocument<100> gps_doc;
      //read_clock();
      //gps_doc["time"] = now.unixtime();
      //gps_doc["lat"] = status_doc["lat"];
      //gps_doc["lon"] = status_doc["lon"];

      //gps_name_file = "/gps/" + String(anio) + "_" + String(mes) + "_" + String(dia_hoy) + ".csv";
      gps_name_file = "/gps/" + String(anio) + "_" + String(mes) + "_" + String(dia_hoy) + ".csv";

      //String csvLine = String((int)status_doc["time"] + "," + String((double)status_doc["lat"], 6) + "," + String((double)status_doc["lon"], 6) );
      String csvLine = String(anio) + "-" + String(mes) + "-" + String(dia_hoy) + " " + String(hora) + ":" + String(minuto) + ":" + String(segundo) + ",";
      csvLine += String((double)status_doc["lat"], 6) + "," + String((double)status_doc["lon"], 6);
      csvLine += '\n'; // O puedes usar gps_str.concat('\n');

      // ------------------------------------------- log de GPS existe?
      if (SD.exists(gps_name_file))
      {
        //appendFile(SD, gps_name_file.c_str(), gps_str.c_str());
        appendFile(SD, gps_name_file.c_str(), csvLine.c_str());
      }
      else
      {
        //Serial.println("File not found, init SD");
        //sd_ready = false;
        //if (!SD.exists(gps_name_file))
        //{
        Serial.print("File not found, create?: ");
        Serial.println(gps_name_file);
        writeFile(SD, gps_name_file.c_str(), csvLine.c_str());
        //}
      }

    }
  }
}

// ---------------------------------------------------- gps_init
void gps_init()
{
  
  uint8_t rxPin = 34;
  uint8_t txPin = 33;
  //Serial2.begin(9600, SERIAL_8N1);  // Inicializa UART1 con 9600 baudios
  Serial2.begin(9600, SERIAL_8N1, rxPin, txPin);
  Serial.println(F("{\"gps_init\":true}")); //Serial.println(TinyGPSPlus::libraryVersion());
}


// ----------------------------------------------------- gps_update
void gps_update()
{

  strcpy(buffer_union_publish, obj["id"].as<const char*>());
  strcat(buffer_union_publish, publish_topic);
  strcat(buffer_union_publish, gps_topic);

  // -------------------------get GPS until a time
  smartDelay(2000);

  /*printInt(gps.satellites.value(), gps.satellites.isValid(), 5);
    printFloat(gps.hdop.hdop(), gps.hdop.isValid(), 6, 1);
    printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
    printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
    printInt(gps.location.age(), gps.location.isValid(), 5);
    printDateTime(gps.date, gps.time);
    printFloat(gps.altitude.meters(), gps.altitude.isValid(), 7, 2);
    printFloat(gps.speed.kmph(), gps.speed.isValid(), 6, 2);
    Serial.println();*/

  if ((millis() > 1000 && gps.charsProcessed() < 10))
  {
    STATE &= ~(1 << 5);                 // GPS error
    STATE &= ~(1 << 1);                 // GPS error
    Serial.println(F("{\"gps_status\": \"error_last_seen\"}"));
    obj["gps_status"] = "error_last_seen";

  }
  else if ((gps.hdop.isValid()) && (gps.location.isValid()))
  {
    //printInt(gps.satellites.value(), gps.satellites.isValid(), 5);
    //printFloat(gps.hdop.hdop(), gps.hdop.isValid(), 6, 1);
    //printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
    //printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
    //printInt(gps.location.age(), gps.location.isValid(), 5);
    //printDateTime(gps.date, gps.time);
    //printFloat(gps.altitude.meters(), gps.altitude.isValid(), 7, 2);
    //printFloat(gps.speed.kmph(), gps.speed.isValid(), 6, 2);
    //Serial.println();

    int hdopValue = int(gps.hdop.hdop());
    if (hdopValue >= 10)
    {
      // Send Previous GPS
      STATE |= (1 << 5);                  // GPS connected
      STATE &= ~(1 << 1);                 // GPS not ready
      obj["gps_status"] = "heating up";
      Serial.println(F("{\"gps_status\": \"heating up\"}"));

    }
    else
    {
      STATE |= (1 << 5);                  // GPS connected
      STATE |= (1 << 1);                  // GPS state OK

      obj["gps_status"] = "ready";
      obj["lat"] = gps.location.lat();
      obj["lon"] = gps.location.lng();

    }
  }
  else
  {
    // Send Previous GPS
    obj["gps_status"] = "calculating";
    Serial.println(F("{\"gps_status\": \"calculating\"}"));



    STATE |= (1 << 5);                  // GPS connected
    STATE &= ~(1 << 1);                 // GPS not ready
  }

  Serial.print("{\"lat\":");
  serializeJson(obj["lat"], Serial);
  Serial.print(",\"lon\":");
  serializeJson(obj["lon"], Serial);
  Serial.println("}");


  //status_doc["gps"].clear();
  //status_doc["gps"] = obj["gps"];
  status_doc["gps_status"] = obj["gps_status"];
  status_doc["lat"] = obj["lat"];
  status_doc["lon"] = obj["lon"];


}

// This custom version of delay() ensures that the gps object
// is being "fed".
// ----------------------------------------------------- smartDelay
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  //byte gps_char;
  //int co;
  //Serial.println("Get GPS");
  //do
  //{
  while ((Serial2.available() && (millis() - start < ms)))
  {
    //gps_char = Serial2.read();
    gps.encode(Serial2.read());
    //Serial.write(gps_char);
    //co++;
  }
  //} while (millis() - start < ms);
  //Serial.println(millis() - start);
  //Serial.println(co);
}

// ----------------------------------------------------- printFloat
static void printFloat(float val, bool valid, int len, int prec)
{
  if (!valid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i = flen; i < len; ++i)
      Serial.print(' ');
  }
  smartDelay(0);
}


// ----------------------------------------------------- printInt
static void printInt(unsigned long val, bool valid, int len)
{
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i = strlen(sz); i < len; ++i)
    sz[i] = ' ';
  if (len > 0)
    sz[len - 1] = ' ';
  Serial.print(sz);
  smartDelay(0);
}

// ----------------------------------------------------- printDateTime
static void printDateTime(TinyGPSDate & d, TinyGPSTime & t)
{
  if (!d.isValid())
  {
    Serial.print(F("********** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    Serial.print(sz);
  }

  if (!t.isValid())
  {
    Serial.print(F("******** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    Serial.print(sz);
  }

  printInt(d.age(), d.isValid(), 5);
  smartDelay(0);
}


// ----------------------------------------------------- printStr
static void printStr(const char *str, int len)
{
  int slen = strlen(str);
  for (int i = 0; i < len; ++i)
    Serial.print(i < slen ? str[i] : ' ');
  smartDelay(0);
}
