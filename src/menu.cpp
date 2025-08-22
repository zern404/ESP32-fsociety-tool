#include "config.h"
#include "definitions.h"
#include "beacon.h"
#include "web_interface.h"
#include "deauth.h"
#include "flappy_bird.h"
#include "ir_controll.h"
#include "evil_portal.h"
#include "rfid_controll.h"
#include "snake_game.h" 
#include "controll_1101.h" 
#include "ble_spam.h"

const float targetScaleSelected = 1.4;  
const float targetScaleOther = 1.0;      
const float scaleSpeed = 0.30;          

float itemScales[20];

const char* tabsMenu[] = { "Settings", "WiFi", "Games", "Modules", "BLE"};
const short tabsLength = sizeof(tabsMenu) / sizeof(tabsMenu[0]);
short selectedItemTab = 1;

const char* wifiMenu[] = { "Scan", "Connect", "Deauth selected", "Deauth all", "Fishing Wi-Fi", "Fishing Email", "BeaconSpam", "Back"};
const short wifiLength = sizeof(wifiMenu) / sizeof(wifiMenu[0]);
short selectedItemWifi = 0;
short wifiMenuScroll = 0;
const short wifiMenuVisible = 4; 

const char* nfcMenu[] = { "Flappy Bird", "Snake", "Back" };
const short nfcLength = sizeof(nfcMenu) / sizeof(nfcMenu[0]);
short nfcMenuScroll = 0;
short selectedItemNfc = 0;

const char* irMenu[] = { "IR Killer", "RFID Read/Write", "RFID Default", "RM Read/Send", "RM Default", "Back"};
const short irLength = sizeof(irMenu) / sizeof(irMenu[0]);
short irMenuScroll = 0;
short selectedItemIr = 0;

const char* settingsMenu[] = { "Reboot", "Back"};
const short settingsLength = sizeof(settingsMenu) / sizeof(settingsMenu[0]);
short settingsMenuScroll = 0;
short selectedItemSettings = 0;

short currentTab = 0;

String network_name = "";


void drawMenu() {
    display.clearDisplay();
    drawStatusBar();

    const char** currentMenu;
    short currentMenuLength;
    short selectedItem;
    short menuScroll = 0;
    const short menuVisible = 4;

    switch(currentTab) {
        case 0: currentMenu = tabsMenu; currentMenuLength = tabsLength; selectedItem = selectedItemTab; menuScroll = 0; break;
        case 1: currentMenu = wifiMenu; currentMenuLength = wifiLength; selectedItem = selectedItemWifi;
            if (selectedItemWifi < wifiMenuScroll) wifiMenuScroll = selectedItemWifi;
            if (selectedItemWifi >= wifiMenuScroll + menuVisible) wifiMenuScroll = selectedItemWifi - menuVisible + 1;
            menuScroll = wifiMenuScroll; break;
        case 2: currentMenu = nfcMenu; currentMenuLength = nfcLength; selectedItem = selectedItemNfc;
            if (selectedItemNfc < nfcMenuScroll) nfcMenuScroll = selectedItemNfc;
            if (selectedItemNfc >= nfcMenuScroll + menuVisible) nfcMenuScroll = selectedItemNfc - menuVisible + 1;
            menuScroll = nfcMenuScroll; break;
        case 3: currentMenu = irMenu; currentMenuLength = irLength; selectedItem = selectedItemIr;
            if (selectedItemIr < irMenuScroll) irMenuScroll = selectedItemIr;
            if (selectedItemIr >= irMenuScroll + menuVisible) irMenuScroll = selectedItemIr - menuVisible + 1;
            menuScroll = irMenuScroll; break;
        case 4: currentMenu = settingsMenu; currentMenuLength = settingsLength; selectedItem = selectedItemSettings;
            if (selectedItemSettings < settingsMenuScroll) settingsMenuScroll = selectedItemSettings;
            if (selectedItemSettings >= settingsMenuScroll + menuVisible) settingsMenuScroll = selectedItemSettings - menuVisible + 1;
            menuScroll = settingsMenuScroll; break;
    }

    for (short i = 0; i < menuVisible; i++) {
        short idx = menuScroll + i;
        if (idx >= currentMenuLength) break;
        float target = (idx == selectedItem) ? targetScaleSelected : targetScaleOther;
        if (itemScales[idx] == 0) itemScales[idx] = targetScaleOther;
        itemScales[idx] += (target - itemScales[idx]) * scaleSpeed;
    }

    float baseY = 10;
    for (short i = 0; i < menuVisible; i++) {
        short idx = menuScroll + i;
        if (idx >= currentMenuLength) break;

        float scale = itemScales[idx];
        int textWidth = strlen(currentMenu[idx]) * 6 * scale;
        int x_pos = (SCREEN_WIDTH - textWidth) / 2;

        float y_pos = baseY + i * 12 + (12 * (1.0 - scale) / 2.0);

        display.setTextSize(scale);
        if (idx == selectedItem) {
            display.fillRect(0, y_pos - 1, SCREEN_WIDTH, 12 * scale, SSD1306_WHITE);
            display.setTextColor(SSD1306_BLACK);
        } else {
            display.setTextColor(SSD1306_WHITE);
        }

        display.setCursor(x_pos, y_pos);
        display.print(currentMenu[idx]);
    }

    display.setTextSize(1);
    display.display();
}

void handleMenuSelect() {
  switch (currentTab) {
    case 0:
      if (selectedItemTab == 0) {
        currentTab = 4;
      } else if (selectedItemTab == 1) {
        currentTab = 1;
      } else if (selectedItemTab == 2) {
        currentTab = 2;
      } else if (selectedItemTab == 3) {
        currentTab = 3;
      }
      break;

    case 1:
      if (selectedItemWifi == wifiLength - 1) {
        currentTab = 0;
        selectedItemWifi = 0;
        wifiMenuScroll = 0;
      } else {
       switch (selectedItemWifi) {
        
          case 0:
            delay(150);
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("Scanning...");
            display.drawBitmap(0, 11, antenna_img, 64, 64, SSD1306_WHITE);
            display.display();

            scan_menu = true;
            get_wifi();
            break;
          case 1:
            delay(150);
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("Start | Connect...");
            display.drawBitmap(0, 11, antenna_img, 64, 64, SSD1306_WHITE);
            display.display();

            connect_menu = true;
            get_wifi();
            break;
          case 2:
            delay(150);
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("Start | Deauth...");
            display.drawBitmap(0, 11, antenna_img, 64, 64, SSD1306_WHITE);
            display.display();

            deauth_menu = true;
            get_wifi();
            break;
          case 3:
            delay(150);
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("Start | Deauth all...");
            display.drawBitmap(0, 11, antenna_img, 64, 64, SSD1306_WHITE);
            display.display();
            server.stop();
            all_deauth_state = true;
            start_deauth(0, DEAUTH_TYPE_ALL, 2);
            delay(200);
            wait_for_stop();
            break;
          case 4:
            delay(150);
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("Start | Fishing...");
            display.println("Mode: Wi-Fi passwords");
            display.drawBitmap(0, 16, antenna_img, 64, 64, SSD1306_WHITE);
            display.display();

            handshake_menu = true;
            get_wifi();
            break;
          case 5:
            delay(150);
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("Start | Fishing...");
            display.println("Mode: Email and passwords");
            display.drawBitmap(0, 16, antenna_img, 64, 64, SSD1306_WHITE);
            display.display();

            server.stop();
            delay(200);

            start_input(&network_name);
            startCaptivePortal(&network_name, true);
            wait_for_stop();
            break;
          case 6:
            delay(150);
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("Start | BeaconSpam...");
            display.drawBitmap(0, 11, antenna_img, 64, 64, SSD1306_WHITE);
            display.display();

            delay(200);            
            beacon_spam_state = true;
            start_input(&network_name);
            initBeaconSpam(&network_name);
            wait_for_stop();
            break;
        }
      }
      break;

    case 2:
  if (selectedItemNfc == nfcLength - 1) {
    currentTab = 0;
    selectedItemNfc = 0;
  } else {
    switch (selectedItemNfc) {
      case 0:
        delay(150);
        runFlappyBird();
        break;
      case 1:
        delay(150);
        startSnakeGame();
        break;
    }
  }
  break;

  case 3:
    if (selectedItemIr == irLength - 1) {
      currentTab = 0;
      selectedItemIr = 0;
    } else {
      switch (selectedItemIr) {
        case 0: 
          delay(150);
          display.clearDisplay();
          display.setCursor(0, 0);
          display.println("Start | IrSpam...");
          display.display();
          delay(200);
          irSpam = true;
          wait_for_stop();
          break;

        case 1:
          delay(150);
          display.clearDisplay();
          display.setCursor(0, 0);
          display.println("RFID Mode...");
          display.display();
          delay(200);
          waitForCardAndHandle();   
          break;

        case 2: 
          delay(500);
          display.clearDisplay();
          display.setCursor(0, 0);
          display.println("Default UIDs:");
          display.println("01:02:03:04");
          display.println("AA:BB:CC:DD");
          display.display();
          break;

        case 3: 
          delay(500);
          display.clearDisplay();
          cc1101_signal_capture();
          break;

        case 4: 
          delay(500);
          display.clearDisplay();
          cc1101_popular_attacks();
          break;
      }
    }
    break;
    
    case 4:
      if (selectedItemSettings == settingsLength - 1) {
        currentTab = 0;
        selectedItemSettings = 0;
      } else {
        ESP.restart();
      }
      break;

    default:
      currentTab = 0;
      break;
  }
}

void btnHandler() {
  short *selectedItemPtr = nullptr;
  short currentMenuLength = 0;

  switch (currentTab) {
    case 0: selectedItemPtr = &selectedItemTab; currentMenuLength = tabsLength; break;
    case 1: selectedItemPtr = &selectedItemWifi; currentMenuLength = wifiLength; break;
    case 2: selectedItemPtr = &selectedItemNfc; currentMenuLength = nfcLength; break;
    case 3: selectedItemPtr = &selectedItemIr; currentMenuLength = irLength; break;
    case 4: selectedItemPtr = &selectedItemSettings; currentMenuLength = settingsLength; break;
  }

  if (digitalRead(BTN_UP_PIN) == LOW) {
    if (*selectedItemPtr > 0) (*selectedItemPtr)--; else *selectedItemPtr = currentMenuLength - 1;
    delay(50); 
  }

  if (digitalRead(BTN_DOWN_PIN) == LOW) {
    if (*selectedItemPtr < currentMenuLength - 1) (*selectedItemPtr)++; else *selectedItemPtr = 0;
    delay(50);
  }

  if (digitalRead(BTN_SELECT_PIN) == LOW) {
    handleMenuSelect();
    delay(150);
  }
}

void drawStatusBar() {
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  display.setCursor(0, 0);

  if (wifi_connect_state) {
    display.print("->");
    display.print(wifi);
  } else {
    display.print("Not connected");
  }

  display.drawLine(0, 9, SCREEN_WIDTH - 1, 9, SSD1306_WHITE);
}