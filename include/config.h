#ifndef CONFIG_H
#define CONFIG_H

#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 display;
extern const uint8_t intro[];

void animateBitmapAppearFade(const uint8_t *bitmap, uint8_t bmp_width, uint8_t bmp_height, uint16_t holdTime = 500);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define IR_PIN 4
#define BTN_UP_PIN 18
#define BTN_SELECT_PIN 23
#define BTN_DOWN_PIN 19

#endif
