#include "oled_display.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ----------------------------------------------- text
void oled_display_text(String text)
{
  display.clearDisplay();
  display.setTextSize(2);
  //display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0, 0);            // Start at top-left corner
  display.println(text);

  //display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
  //display.println(3.141592);

  //display.setTextSize(2);             // Draw 2X-scale text
  //display.setTextColor(SSD1306_WHITE);
  //display.print(F("0x")); display.println(0xDEADBEEF, HEX);

  display.display();
  //delay(2000);
}


// -------------------------------------------- init
void oled_display_init(void)
{
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    //for(;;); // Don't proceed, loop forever
  }

  display.display();
  //delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  // Draw a single pixel in white
  display.drawPixel(10, 10, SSD1306_WHITE);

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
}


// --------------------------------------- numbers
void oled_display_number(float num)
{
  //uint32_t num
  
  display.clearDisplay();
  display.setTextSize(4);
  //display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text

  display.setCursor(0, 0); 

  /*if (num < 10)
    display.setCursor(102, 0);            //
  else if (num < 100)
    display.setCursor(77, 0);            //
  else if (num < 1000)
    display.setCursor(51, 0);            //
  else if (num < 10000)
    display.setCursor(26, 0);            //
  else
    display.setCursor(0, 0);            // Start at top-left corner*/

  //display.println(uint32_t(num));
   display.println(num);

  //display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
  //display.println(3.141592);

  //display.setTextSize(2);             // Draw 2X-scale text
  //display.setTextColor(SSD1306_WHITE);
  //display.print(F("0x")); display.println(0xDEADBEEF, HEX);

  display.display();
  //delay(2000);
}
