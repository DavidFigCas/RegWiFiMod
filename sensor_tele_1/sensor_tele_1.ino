#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

const char* ssid = "Inventoteca_2G";
const char* password = "science_7425";

#define SDerecha      D5
#define SIzquierda    D6
#define   DERECHA     0
#define   IZQUIERDA   1

int DA = D3;               // Salida 1 para motor
int DB = D4;               // Salida 2 para motor
int ENA = D1;              // Pin ENA del puente H
int ENB = D2;              // Pin ENA del puente H
int tiempoEspera = 5000;  // Tiempo de espera en milisegundos
int tiempoMovimiento = 60000;  // Tiempo de espera en milisegundos
int vel = 255;            // Valor de velocidad inicial
volatile int state = 0;
volatile unsigned long previousMillis = 0;
volatile unsigned long currentMillis = 0;


void setup()
{
  WiFi.mode(WIFI_STA); // Tipo de conexion
  WiFi.begin(ssid, password);

  //while (WiFi.waitForConnectResult() != WL_CONNECTED)
  //{
    //Serial.println("Connection Failed! Rebooting...");
    //delay(5000);
    //ESP.restart();
  //}
  ArduinoOTA.begin();

  Serial.begin(115200);
  Serial.println("Iniciando");
  pinMode(SDerecha, INPUT);
  pinMode(SIzquierda, INPUT);
  pinMode(DA, OUTPUT);
  pinMode(DB, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(SDerecha), botonDerPresionado, RISING);
  attachInterrupt(digitalPinToInterrupt(SIzquierda), botonIzqPresionado, RISING);

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

}

void loop()
{
  Serial.println(state);
  currentMillis = millis();
  switch (state)
  {
    case 0:
      direccion(DERECHA);
      if (currentMillis - previousMillis >= tiempoMovimiento)
      {
        state = 1;
        previousMillis = currentMillis; // Save the start time
      }

      break;

    // ----------------------------- sensor de derecha presionado
    case 1:
      detenerMotor();
      if (currentMillis - previousMillis >= tiempoEspera)
      {
        previousMillis = currentMillis; // Save the start time
        state = 2;
      }
      break;


    case 2:
      direccion(IZQUIERDA); // Cambiar a una dirección
      if (currentMillis - previousMillis >= tiempoMovimiento)
      {
        state = 3;
        previousMillis = currentMillis; // Save the start time
      }

      break;

    case 3:
      detenerMotor();
      if (currentMillis - previousMillis >= tiempoEspera)
      {
        previousMillis = currentMillis; // Save the start time
        state = 0;
      }
      break;

  }
  ArduinoOTA.handle();

}

ICACHE_RAM_ATTR void botonDerPresionado()
{
  if (state == 0)
  {
    state = 1;
    previousMillis = currentMillis; // Save the start time
  }

}

ICACHE_RAM_ATTR void botonIzqPresionado()
{
  if (state == 2)
  {
    state = 3;
    previousMillis = currentMillis; // Save the start time
  }
}

void detenerMotor()
{
  Serial.println("STOP");
  digitalWrite(DA, LOW);
  digitalWrite(DB, LOW);
  analogWrite(ENA, 0); // Detener el motor
  analogWrite(ENB, 0); // Detener el motor
}

void direccion(int dir)
{
  analogWrite(ENA, vel); //
  analogWrite(ENB, vel); //
  Serial.print("VEL: ");
  Serial.println(vel);
  if (dir == 1)
  {
    digitalWrite(DA, HIGH);
    digitalWrite(DB, HIGH);
  }
  else
  {
    digitalWrite(DA, LOW);
    digitalWrite(DB, LOW);
  }
}
