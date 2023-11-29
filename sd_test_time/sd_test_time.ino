#include <SPI.h>
#include <SD.h>

// Definiciones de pines para la tarjeta SD y SPI
#define SD_MISO 2
#define SD_MOSI 15
#define SD_SCLK 14
#define SD_CS   13

void setup() {
  Serial.begin(115200);

  // Inicializa la interfaz SPI con los pines específicos
  SPI.begin(SD_SCLK, SD_MISO, SD_MOSI);

  // Inicializa la tarjeta SD con el pin CS especificado
  if (!SD.begin(SD_CS)) 
  {
    Serial.println("La inicialización de la tarjeta SD falló!");
    return;
  }
  Serial.println("La tarjeta SD está lista para usarse.");
}

void loop() {
  String data = "Datos a guardar: " + String(millis());
  
  // Abre el archivo en modo de escritura
  File dataFile = SD.open("datos.txt", FILE_WRITE);

  // Si el archivo está disponible, escribe en él
  if (dataFile) {
    dataFile.println(data);
    dataFile.close(); // Cierra el archivo después de escribir los datos
    Serial.println("Datos guardados: " + data);
  } else {
    // Si el archivo no se abre, imprime un error
    Serial.println("Error al abrir 'datos.txt'");
  }

  // Espera 5 segundos antes de volver a escribir
  delay(5000);
}
