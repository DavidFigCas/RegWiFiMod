#include "clock.h"


RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
DateTime now;
DateTime last_ac;

int dias;
int mes;
int anio;
int hora;
int minuto;
int dia_hoy;
int segundo;

const char* ntpServer = "pool.ntp.org";
//long  gmtOffset_sec = obj["gmtOff"].as<long>();               // Central Mexico (-5 UTC, -18000): Pacifico (-7 UTC, -25200) :  Noroeste (-8 UTC, -28800)
//int   daylightOffset_sec = obj["dayOff"].as<int>();               // Horario de verano, disabled
int32_t  gmtOffset_sec;
int32_t   daylightOffset_sec;
bool ntpConnected = false;
bool rtcUpdated = false;
bool rtc_ready = false;


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);

// ---------------------------------- init_clock
void init_clock()
{
  //  Wire1.setSDA(2);
  //Wire1.setSCL(3);
  //Wire.begin();
  delay(100);
  if (!rtc.begin(&Wire))
  {
    Serial.println("{\"rtc_init\":false}");
    rtc_ready = false;
    delay(10);
  }
  else
  {
    Serial.println("{\"rtc_init\":true}");
    delay(10);
    gmtOffset_sec = obj["gmtOff"].as<int32_t>();
    daylightOffset_sec = obj["dayOff"].as<int32_t>();
    rtc_ready = true;

    // For New devices
    //if (! rtc.isrunning())
    if (rtc.lostPower())
    {

      // When time needs to be set on a new device, or after a power loss, the
      // following line sets the RTC to the date & time this sketch was compiled

      // Uncomment for new
      //Serial.println("RTC is NOT running, let's set factory the time!");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

      Serial.println("RTC is NOT running, rebooting ...");
      delay(1000);
      //ESP.restart();  // Reiniciar el ESP32

    }


    now = rtc.now();
    status_doc["time"] = now.unixtime();
    doc["time"] = now.unixtime();

    // Tiempo Unix para el 1 de enero de 2050 a las 00:00:00 UTC
    const uint32_t unixTime2050 = 2524608000;
    if (now.unixtime() >= unixTime2050)
    {
      Serial.println("RTC ERROR Reboot...");
      delay(1000);
      //ESP.restart();  // Reiniciar el ESP32
    }

    Serial.print("{\"time\":\"");
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println("\"}");

    gmtOffset_sec = obj["gmtOff"].as<int32_t>();
    daylightOffset_sec = obj["dayOff"].as<int32_t>();

    Serial.print("{\"gmtOff\":");
    Serial.print(gmtOffset_sec);
    Serial.println("}");

    Serial.print("{\"dayOff\":");
    Serial.print(daylightOffset_sec);
    Serial.println("}");

  }
}


// -------------------------------- update_clock
void update_clock()
{
  if (rtc_ready == true)
  {
    // Sincroniza el tiempo del cliente NTP
    if (rtcUpdated == false)
    {
      // New connection to NTP server
      // ---------------------------- Time NTP
      WiFiUDP ntpUDP;
      NTPClient timeClient(ntpUDP, "pool.ntp.org");  // Puedes cambiar "pool.ntp.org" por cualquier servidor NTP de tu elección.

      if (ntpConnected == false)
      {
        //Serial.println("{\"ntp\":\"connecting...\"}");
        //timeClient.begin();
        timeClient.end();
        gmtOffset_sec = obj["gmtOff"].as<int32_t>();               // Central Mexico (-5 UTC, -18000): Pacifico (-7 UTC, -25200) :  Noroeste (-8 UTC, -28800)
        daylightOffset_sec = obj["dayOff"].as<int32_t>();               // Horario de verano, disabled
        timeClient = NTPClient(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);
        timeClient.begin();
        Serial.println("{\"ntp\":\"connected\"}");
        ntpConnected = true;
      }

      // Update time only when connected
      if (timeClient.update())
      {
        Serial.print("{\"time_ntp\":\"");
        Serial.print(timeClient.getFormattedTime());
        Serial.println("\"}");

        //Serial.println(timeClient.getEpochTime(), DEC);

        if (timeClient.getSeconds() != 0)
        {
          Serial.println("{\"rtc\":\"updated from NTP\"}");
          Serial.println(timeClient.getFormattedTime());
          // Establece el tiempo del DS1307 utilizando el tiempo del cliente NTP
          rtc.adjust(DateTime(timeClient.getEpochTime()));
          rtcUpdated = true;
        }
        else
        {
          Serial.println("{\"ntp\":\"fail\"}");
          rtcUpdated = false;
        }
      }
      else
        Serial.println("{\"rtc\":\"NOT updated, Battery mode\"}");

      //if (rtc.isrunning())
      {
        now = rtc.now();
        status_doc["time"] = now.unixtime();
        doc["time"] = now.unixtime();

        Serial.print("{\"time_rtc\":\"");
        Serial.print(now.year(), DEC);
        Serial.print('/');
        Serial.print(now.month(), DEC);
        Serial.print('/');
        Serial.print(now.day(), DEC);
        Serial.print(' ');
        Serial.print(now.hour(), DEC);
        Serial.print(':');
        Serial.print(now.minute(), DEC);
        Serial.print(':');
        Serial.print(now.second(), DEC);
        Serial.println("\"}");
      }
      //else
      //{
      //Serial.print("{\"time_rtc\":\"fail\"");
      //delay(1000);
      //ESP.restart();  // Reiniciar el ESP32
      //}
    }
  }
  else
  {
    init_clock();
  }

}

// -------------------------------- read_clock
void read_clock()
{
  if (rtc_ready)
  {

    //if (rtc.isrunning())
    {
      now = rtc.now();
      status_doc["time"] = now.unixtime();
      doc["time"] = now.unixtime();

      // Tiempo Unix para el 1 de enero de 2050 a las 00:00:00 UTC
      const uint32_t unixTime2050 = 2524608000;
      if (now.unixtime() >= unixTime2050)
      {
        Serial.println("RTC ERROR Reboot...");
        delay(1000);
        //ESP.restart();  // Reiniciar el ESP32
      }

      //dias = int(round(round(now.unixtime() - last_ac.unixtime()) / 86400L));
      //dias = (now.unixtime() - last_ac.unixtime()) / 86400;
      mes = now.month();
      anio = now.year();
      dia_hoy = now.day();
      hora = now.hour();
      minuto = now.minute();
      segundo = now.second();

      Serial.print("{\"time\":\"");
      Serial.print(now.year(), DEC);
      Serial.print('/');
      Serial.print(now.month(), DEC);
      Serial.print('/');
      Serial.print(now.day(), DEC);
      Serial.print(' ');
      Serial.print(now.hour(), DEC);
      Serial.print(':');
      Serial.print(now.minute(), DEC);
      Serial.print(':');
      Serial.print(now.second(), DEC);
      Serial.println("\"}");


      lcd.setCursor(0, 2); //
      lcd.print(dia_hoy);
      lcd.print("/");
      lcd.print(mes);
      lcd.print("/");
      lcd.print(anio);
      lcd.print("  ");
      lcd.print(hora);
      lcd.print(":");
      lcd.print(minuto);
      lcd.print(":");
      lcd.print(segundo);

    }
  }
  else
  {
    init_clock();
  }
}

String DateTimeToString(const DateTime& now) {
  char buffer[20];
  snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
           now.year(), now.month(), now.day(),
           now.hour(), now.minute(), now.second());
  return String(buffer);
}


//
//// Local time
//void printLocalTime()
//{
//  struct tm timeinfo;
//  if (!getLocalTime(&timeinfo)) {
//    Serial.println("Failed to obtain time");
//    return;
//  }
//  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");   // Comment for ESP8266
//}
