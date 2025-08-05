#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <Wire.h>
#include <vector>
#include <esp_wifi.h>

#include "types.h"
#include "esp_wifi.h"
#include "config.h"
#include "web_interface.h"
#include "deauth.h"
#include "definitions.h"

int curr_channel = 1;

const char* tabsMenu[] = { "Settings", "WiFi", "NFC", "IR"};
const short tabsLength = sizeof(tabsMenu) / sizeof(tabsMenu[0]);
short selectedItemTab = 0;

const char* wifiMenu[] = { "Scan", "Connect", "Deauth", "Handshake", "Back"};
const short wifiLength = sizeof(wifiMenu) / sizeof(wifiMenu[0]);
short selectedItemWifi = 0;

const char* nfcMenu[] = { "Test", "Back" };
const short nfcLength = sizeof(nfcMenu) / sizeof(nfcMenu[0]);
short selectedItemNfc = 0;

const char* irMenu[] = { "Test", "Back"};
const short irLength = sizeof(irMenu) / sizeof(irMenu[0]);
short selectedItemIr = 0;

const char* settingsMenu[] = { "Theme1", "Back"};
const short settingsLength = sizeof(settingsMenu) / sizeof(settingsMenu[0]);
short selectedItemSettings = 0;

short currentTab = 0;

int last_push_btn_time = 0;
int time_to_sleep = 60000;

bool display_power_state = true;
bool wifi_connect_state = false;
bool connect_menu = false;
bool scan_menu = false;
bool deauth_menu = false;
bool handshake_menu = false;

String wifi;
short wifiScroll = 0;


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


void checkSleep();
void btnHandler();
void drawMenu();
void drawStatusBar();
void get_wifi();


void setup() {
  pinMode(BTN_UP_PIN, INPUT_PULLUP);
  pinMode(BTN_DOWN_PIN, INPUT_PULLUP);
  pinMode(BTN_SELECT_PIN, INPUT_PULLUP);
  pinMode(IR_PIN, OUTPUT);
  WiFi.disconnect();
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Display error"));
    while(true);
  }

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
  display.display();

  delay(2000);
  animateBitmapAppearFade(intro, SCREEN_WIDTH, SCREEN_HEIGHT, 1000);
  last_push_btn_time = millis(); 
  
#ifdef SERIAL_DEBUG
  Serial.begin(115200);
#endif
#ifdef LED
  pinMode(LED, OUTPUT);
#endif

  WiFi.mode(WIFI_MODE_AP);
  WiFi.softAP(AP_SSID, AP_PASS);

  start_web_interface();
}

void loop() {
  if (deauth_type == DEAUTH_TYPE_ALL) {
    if (curr_channel > CHANNEL_MAX) curr_channel = 1;
    esp_wifi_set_channel(curr_channel, WIFI_SECOND_CHAN_NONE);
    curr_channel++;
    delay(10);
  } else {
    web_interface_handle_client();
  }
  
  checkSleep();
  if (!display_power_state) return;

  btnHandler();
  drawMenu();
}


void connectToWiFi() {
  String password = "";
  short charIndex = 0;
  const char* chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  const short charsCount = strlen(chars);
  bool entering = true;

  while (entering) {
    display.clearDisplay();
    drawStatusBar();

    display.setCursor(0, 10);
    display.print("SSID: ");
    display.println(wifi);

    display.setCursor(0, 25);
    display.print("Pass: ");
    display.println(password + "_");

    display.setCursor(0, 40);
    if (charIndex < charsCount) {
      display.print("Char: ");
      display.print(chars[charIndex]);
    } else if (charIndex == charsCount) {
      display.print("<< [DELETE] >>");
    } else {
      display.print("<< [READY] >>");
    }

    display.display();

    if (digitalRead(BTN_UP_PIN) == LOW) {
      charIndex = (charIndex + 1) % (charsCount + 2);
      delay(150);
    }

    if (digitalRead(BTN_DOWN_PIN) == LOW) {
      charIndex = (charIndex - 1 + charsCount + 2) % (charsCount + 2);
      delay(150);
    }

    if (digitalRead(BTN_SELECT_PIN) == LOW) {
      if (charIndex < charsCount) {
        password += chars[charIndex];
      } else if (charIndex == charsCount) {
        if (password.length() > 0) {
          password.remove(password.length() - 1);
        }
      } else {
        entering = false; 
      }
      delay(150);
    }
  }

  WiFi.begin(wifi.c_str(), password.c_str());
  unsigned long startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
    display.clearDisplay();
    drawStatusBar();
    display.setCursor(0, 20);
    display.println("Connecting...");
    display.display();
    delay(500);
  }

  display.clearDisplay();
  drawStatusBar();
  display.setCursor(0, 20);
  if (WiFi.status() == WL_CONNECTED) {
    display.println("Connected!");
    wifi_connect_state = true;
  } else {
    display.println("Failed.");
    wifi_connect_state = false;
  }

  display.display();
  delay(2000);
}

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

  for (short i = 0; i < currentMenuLength; i++) {
    String lineText;
    if (i == selectedItem) {
      lineText = "> " + String(currentMenu[i]) + " <";
    } else {
      lineText = "  " + String(currentMenu[i]) + "  ";
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
      }
      break;

    case 1:
      if (selectedItemWifi == wifiLength - 1) {
        currentTab = 0;
        selectedItemWifi = 0;
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
            display.println("Scanning connect...");
            display.display();

            connect_menu = true;
            get_wifi();
            break;
          case 2:
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("Scanning deauth...");
            display.display();

            deauth_menu = true;
            get_wifi();
            break;
          case 3:
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("Scanning handshake...");
            display.display();

            handshake_menu = true;
            get_wifi();
            break;
        }
      }
      break;

    case 2:
      if (selectedItemNfc == nfcLength - 1) {
        currentTab = 0;
        selectedItemNfc = 0;
      } else {
        Serial.println("Checking NFC...");
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
        Serial.print("Theme selected: ");
        Serial.println(settingsMenu[selectedItemSettings]);
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

void checkSleep()
{
  bool btn_pressed = !digitalRead(BTN_UP_PIN) || !digitalRead(BTN_SELECT_PIN) || !digitalRead(BTN_DOWN_PIN);

  if (btn_pressed)
  {
    if (!display_power_state)
    {
      display.ssd1306_command(SSD1306_DISPLAYON);
      display_power_state = true;
    }
    last_push_btn_time = millis(); 
    delay(20);
  }
  
  if (display_power_state)
  {
    if (millis() - last_push_btn_time >= time_to_sleep)
    {
      display_power_state = false;
      display.ssd1306_command(SSD1306_DISPLAYOFF);
      delay(20);
    } 
  }
}

std::vector<std::pair<String, int>> scanWiFiNetworks() {
  std::vector<std::pair<String, int>> networks;

  int n = WiFi.scanNetworks();
  if (n == 0) {
    networks.push_back({"No networks found", 0});
  } else {
    for (int i = 0; i < n; ++i) {
      String ssid = WiFi.SSID(i);
      int rssi = WiFi.RSSI(i);
      networks.push_back({ssid, rssi});
    }
  }

  return networks;
}

void wait_for_stop()
{
  while (true)
  {
    if (digitalRead(BTN_SELECT_PIN) == LOW) {
      stop_deauth();
      delay(200);
      return;
    }

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("ATTACK...");
    display.setCursor(0, 32);
    display.println("-> SELECT to stop");
    display.display();
  }
}

void get_wifi() 
{
  display.clearDisplay();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(150);

  auto networks = scanWiFiNetworks();
  int totalNetworks = networks.size();
  wifiScroll = 0;

  while (true)
  {
    checkSleep();

    if (digitalRead(BTN_SELECT_PIN) == LOW) {
      if (scan_menu){
        scan_menu = false;
       return; 
      }
      if (connect_menu){
        wifi = networks[wifiScroll].first;
        delay(200);
        connectToWiFi();
        connect_menu = false;
        return;
      }
      if (deauth_menu){
        wifi = networks[wifiScroll].first;
        delay(200);
        start_deauth(wifiScroll, DEAUTH_TYPE_SINGLE, 2);
        deauth_menu = false;
        wait_for_stop();
        return;
      }
      if (handshake_menu){
        wifi = networks[wifiScroll].first;
        delay(200);
        handshake_menu = false;
        return;
      }
      delay(150);
    }

    if (digitalRead(BTN_DOWN_PIN) == LOW) {
      if (wifiScroll < totalNetworks - 1) {
        wifiScroll++;
        delay(150);
      }
    }

    if (digitalRead(BTN_UP_PIN) == LOW) {
      delay(150);
      return;
    }

    display.clearDisplay();
    drawStatusBar();

    short selectedWifiIndex = wifiScroll; 

    for (int i = 0; i < 6; ++i) {
      int idx = wifiScroll + i;
      if (idx >= totalNetworks) break;
    
      display.setCursor(0, 10 + i * 10);
      if (idx == selectedWifiIndex) {
        display.print("-> ");   
      } else {
        display.print("   ");  
      }
    
      display.print(networks[idx].first);
      display.print(" ");
      display.print(networks[idx].second);
      display.println("dBm");
    }

    display.display();
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