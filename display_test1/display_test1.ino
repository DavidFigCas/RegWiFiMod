#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <GxEPD2_7C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>  // Ejemplo de fuente

#define P_MOSI      3 
#define P_MISO      4
#define P_SCK       2

// Tus configuraciones aquí...
GxEPD2_BW<GxEPD2_750_YT7, 480> display(GxEPD2_750_YT7(/*CS=*/ 1, /*DC=*/ 5, /*RST=*/ 6, /*BUSY=*/ 7));
const char HelloWorld[] = "Hello World!";

void setup()
{
  display.epd2.selectSPI(SPI, SPISettings(4000000, MSBFIRST, SPI_MODE0));
  gpio_set_function(P_MISO, GPIO_FUNC_SPI);
  gpio_set_function(P_SCK, GPIO_FUNC_SPI);
  gpio_set_function(P_MOSI, GPIO_FUNC_SPI);
  display.init(0); // Inicializa el display
  display.setRotation(1);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold9pt7b);

  display.setRotation(1);
  //display.setFont(&FreeMonoBold9pt7b);
  display.setFont(&FreeSans9pt7b);  // Establecer la fuente
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center the bounding box by transposition of the origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(HelloWorld);
  }
  while (display.nextPage());

  do
  {
    display.fillScreen(GxEPD_WHITE);
    //display.setCursor(x, y);
    //display.print(HelloWorld);
  }
  while (display.nextPage());
  display.setTextSize(5); // Hace que el texto sea el doble de grande
  display.fillScreen(GxEPD_WHITE);

  
}

void loop() {
  static int number = 0; // Número a mostrar

  String numStr = String(number); // Convierte el número a String
  int16_t tbx, tby; uint16_t tbw, tbh;
  // Obtener las dimensiones del texto
  display.getTextBounds(numStr, 0, 0, &tbx, &tby, &tbw, &tbh);

  // Ajustar la ventana parcial alrededor del texto
  int16_t x = (display.width() - tbw) / 2;
  int16_t y = (display.height() - tbh) / 2;
  uint16_t w = tbw + 10; // Un poco de margen
  uint16_t h = tbh + 10;

  display.setPartialWindow(x, y, w, h);

  display.firstPage();
  do {
    display.setCursor(x - tbx, y - tby); // Ajustar la posición del cursor
    display.print(numStr);
  } while (display.nextPage());

  delay(350); // Retraso para visualizar el número
  number++; // Incrementar el número
}
