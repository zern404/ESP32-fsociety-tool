#include "config.h"
#include "definitions.h"
#include "beacon.h"
#include "web_interface.h"
#include "deauth.h"
#include "flappy_bird.h"

const char* tabsMenu[] = { "Settings", "WiFi", "Games", "IR", "BLE Spam"};
const short tabsLength = sizeof(tabsMenu) / sizeof(tabsMenu[0]);
short selectedItemTab = 0;

const char* wifiMenu[] = { "Scan", "Connect", "Deauth selected", "Deauth all", "Phishing", "BeaconSpam", "Back"};
const short wifiLength = sizeof(wifiMenu) / sizeof(wifiMenu[0]);
short selectedItemWifi = 0;
short wifiMenuScroll = 0;
const short wifiMenuVisible = 4; 

const char* nfcMenu[] = { "Flappy Bird", "Back" };
const short nfcLength = sizeof(nfcMenu) / sizeof(nfcMenu[0]);
short selectedItemNfc = 0;

const char* irMenu[] = { "Start", "Back"};
const short irLength = sizeof(irMenu) / sizeof(irMenu[0]);
short selectedItemIr = 0;

const char* settingsMenu[] = { "Reboot", "Back"};
const short settingsLength = sizeof(settingsMenu) / sizeof(settingsMenu[0]);
short selectedItemSettings = 0;

short currentTab = 0;

void drawMenu() {
  display.clearDisplay();
  drawStatusBar();

  const char** currentMenu;
  short currentMenuLength;
  short selectedItem;

  switch (currentTab) {
    case 0:
      currentMenu = tabsMenu;
      currentMenuLength = tabsLength;
      selectedItem = selectedItemTab;
      break;
    case 1:
      currentMenu = wifiMenu;
      currentMenuLength = wifiLength;
      selectedItem = selectedItemWifi;

      if (selectedItemWifi < wifiMenuScroll) wifiMenuScroll = selectedItemWifi;
      if (selectedItemWifi >= wifiMenuScroll + wifiMenuVisible) wifiMenuScroll = selectedItemWifi - wifiMenuVisible + 1;
      break;
    case 2:
      currentMenu = nfcMenu;
      currentMenuLength = nfcLength;
      selectedItem = selectedItemNfc;
      break;
    case 3:
      currentMenu = irMenu;
      currentMenuLength = irLength;
      selectedItem = selectedItemIr;
      break;
    case 4:
      currentMenu = settingsMenu;
      currentMenuLength = settingsLength;
      selectedItem = selectedItemSettings;
      break;
  }

  for (short i = 0; i < wifiMenuVisible; i++) {
    short idx = wifiMenuScroll + i;
    if (idx >= currentMenuLength) break;
    String lineText;
    if (idx == selectedItem) {
      lineText = "> " + String(currentMenu[idx]) + " <";
    } else {
      lineText = "  " + String(currentMenu[idx]) + "  ";
    }
    int x_pos = (SCREEN_WIDTH - (lineText.length() * 6)) / 2;
    int y_pos = 10 + i * 10;
    display.setCursor(x_pos, y_pos);
    display.print(lineText);
  }

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
      } else if (selectedItemTab == 4) {
    
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Start | BLE Spam...");
      display.display();
      wait_for_stop();
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
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("Scanning...");
            display.display();

            scan_menu = true;
            get_wifi();
            break;
          case 1:
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("Start | Connect...");
            display.display();

            connect_menu = true;
            get_wifi();
            break;
          case 2:
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("Start | Deauth...");
            display.display();

            deauth_menu = true;
            get_wifi();
            break;
          case 3:
          display.clearDisplay();
            display.setCursor(0, 0);
            display.println("Start | Deauth all...");
            display.display();
            server.stop();
            all_deauth_state = true;
            start_deauth(0, DEAUTH_TYPE_ALL, 2);
            delay(200);
            wait_for_stop();
            break;
          case 4:
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("Start | Phishing...");
            display.display();

            handshake_menu = true;
            get_wifi();
            break;
          case 5:
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("Start | BeaconSpam...");
            display.display();

            delay(200);            
            beacon_spam_state = true;
            String network_name = "";
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
        runFlappyBird();
      }
      break;

    case 3:
      if (selectedItemIr == irLength - 1) {
        currentTab = 0;
        selectedItemIr = 0;
      } else {
        Serial.println("IR Test started...");
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
    case 0:
      selectedItemPtr = &selectedItemTab;
      currentMenuLength = tabsLength;
      break;
    case 1:
      selectedItemPtr = &selectedItemWifi;
      currentMenuLength = wifiLength;
      break;
    case 2:
      selectedItemPtr = &selectedItemNfc;
      currentMenuLength = nfcLength;
      break;
    case 3:
      selectedItemPtr = &selectedItemIr;
      currentMenuLength = irLength;
      break;
    case 4:
      selectedItemPtr = &selectedItemSettings;
      currentMenuLength = settingsLength;
      break;
  }
    if (digitalRead(BTN_UP_PIN) == LOW) {
      if (*selectedItemPtr > 0) {
        (*selectedItemPtr)--;
      } else {
        *selectedItemPtr = currentMenuLength - 1;
      }
      delay(200);
    }
  
    if (digitalRead(BTN_DOWN_PIN) == LOW) {
      if (*selectedItemPtr < currentMenuLength - 1) {
        (*selectedItemPtr)++;
      } else {
        *selectedItemPtr = 0;
      }
      delay(200);
    }
  
    if (digitalRead(BTN_SELECT_PIN) == LOW) {
      handleMenuSelect();
      delay(200);
    }
}

void drawStatusBar() {
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  display.setCursor(0, 0);

  if (wifi_connect_state) {
    display.print("WI-FI: ");
    display.print(wifi);
  } else {
    display.print("WI-FI: Not connected");
  }

  display.drawLine(0, 9, SCREEN_WIDTH - 1, 9, SSD1306_WHITE);
}