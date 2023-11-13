#define SDerecha      D5
#define SIzquierda    D6
#define   DERECHA     0
#define   IZQUIERDA   1

int DA = D3;               // Salida 1 para motor
int DB = D4;               // Salida 2 para motor
int ENA = D1;              // Pin ENA del puente H
int ENB = D2;              // Pin ENA del puente H
int tiempoEspera = 3000;  // Tiempo de espera en milisegundos
int vel = 64;            // Valor de velocidad inicial
int state = 0;


void setup()
{
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

}

void loop()
{
  Serial.println(state);
  switch (state)
  {
    case 0:
      direccion(DERECHA);
      break;

    // ----------------------------- sensor de derecha presionado
    case 1:
      detenerMotor();
      delay(tiempoEspera);
      state = 2;
      break;


    case 2:
      direccion(IZQUIERDA); // Cambiar a una dirección
      break;

    case 3:
      detenerMotor();
      delay(tiempoEspera);
      state = 0;
      break;

  }

}

ICACHE_RAM_ATTR void botonDerPresionado()
{
  if ((state == 0))
    state = 1;
}

ICACHE_RAM_ATTR void botonIzqPresionado()
{
  if ((state == 2))
    state = 3;
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
