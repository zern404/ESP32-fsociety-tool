#ifndef CONFIG_H
#define CONFIG_H

#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 display;
extern const uint8_t intro[];

extern bool display_power_state;
extern bool wifi_connect_state;
extern bool connect_menu;
extern bool scan_menu;
extern bool deauth_menu;
extern bool handshake_menu;
extern bool all_deauth_state;
extern bool beacon_spam_state;
extern String wifi;

void animateBitmapAppearFade(const uint8_t *bitmap, uint8_t bmp_width, uint8_t bmp_height, uint16_t holdTime = 500);
void start_input(String* password);
void wait_for_stop();
void checkSleep();
void btnHandler();
void drawMenu();
void drawStatusBar();
void get_wifi();

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define IR_PIN 4
#define BTN_UP_PIN 18
#define BTN_SELECT_PIN 23
#define BTN_DOWN_PIN 19

#endif
