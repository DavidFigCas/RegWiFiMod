#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include "system.h"

extern Adafruit_SSD1306 display;

void oled_display_text(String);
void oled_display_init(void);
void oled_display_number(float); //uint32_t


#endif
