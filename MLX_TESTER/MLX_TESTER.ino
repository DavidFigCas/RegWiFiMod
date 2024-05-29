#include "Inventoteca_MLX90393.h"

Adafruit_MLX90393 sensor = Adafruit_MLX90393();
#define MLX90393_CS 10

// Optimizar el almacenamiento de cadenas
/*const char startMsg[] PROGMEM = "Starting";
const char sensorNotFoundMsg[] PROGMEM = "No sensor?";
const char sensorFoundMsg[] PROGMEM = "Found";
const char gainSetMsg[] PROGMEM = "Gain";
const char unableToReadMsg[] PROGMEM = "Unable\n";
const char axisX[] PROGMEM = "X: ";
const char axisY[] PROGMEM = "Y: ";
const char axisZ[] PROGMEM = "Z: ";
const char angleMsg[] PROGMEM = "A: ";*/

void setup(void) {
  delay(5000);
  Serial1.begin(9600);
  
  while (!Serial1) {
    delay(10);
  }

  // Utiliza Serial.write para enviar cadenas desde PROGMEM
  //writeProgmemString(startMsg);

  if (!sensor.begin_I2C(0x0F)) {
    //writeProgmemString(sensorNotFoundMsg);
    while (1) {
      delay(10);
    }
  }
  //writeProgmemString(sensorFoundMsg);

  //sensor.setGain(MLX90393_GAIN_5X);
  //writeProgmemString(gainSetMsg);
  /*switch (sensor.getGain()) {
    case MLX90393_GAIN_1X: Serial.write("1 x\n"); break;
    case MLX90393_GAIN_1_33X: Serial.write("1.33 x\n"); break;
    case MLX90393_GAIN_1_67X: Serial.write("1.67 x\n"); break;
    case MLX90393_GAIN_2X: Serial.write("2 x\n"); break;
    case MLX90393_GAIN_2_5X: Serial.write("2.5 x\n"); break;
    case MLX90393_GAIN_3X: Serial.write("3 x\n"); break;
    case MLX90393_GAIN_4X: Serial.write("4 x\n"); break;
    case MLX90393_GAIN_5X: Serial.write("5 x\n"); break;
  }

  sensor.setResolution(MLX90393_X, MLX90393_RES_17);
  sensor.setResolution(MLX90393_Y, MLX90393_RES_17);
  sensor.setResolution(MLX90393_Z, MLX90393_RES_16);

  sensor.setOversampling(MLX90393_OSR_3);
  sensor.setFilter(MLX90393_FILTER_5);*/
}

void loop(void) {
  float x, y, z;

  if (sensor.readData(&x, &y, &z)) 
  {
    //writeProgmemString(axisX); writeFloatToSerial(x);
    //writeProgmemString(axisY); writeFloatToSerial(y);
    //writeProgmemString(axisZ); writeFloatToSerial(z);
    //Serial.write("\n");
  } else {
    //writeProgmemString(unableToReadMsg);
  }

  delay(500);

  float rad = atan2(y, x);
  float angulo = rad * (180.0 / M_PI);

  if (angulo < 0) {
    angulo += 360;
  } else if (angulo >= 360) {
    angulo -= 360;
  }
  int aux_an = angulo;

  //Serial1.println(aux_an);
  
  char buffer[10];

  
  //buffer[0] = 32 + byte (angulo);
  //buffer[1] = '2';
  //buffer[2] = '3';
  //buffer[3] = 0;
  
  //Serial.print(F("Unique ID:    "));
  sprintf(buffer, "%d", aux_an); 
  
  Serial.write(buffer);
  
  //Serial.write("\n");
}
