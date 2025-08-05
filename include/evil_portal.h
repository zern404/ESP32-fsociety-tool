#ifndef EVIL_PORTAL_H
#define EVIL_PORTAL_H

extern bool portalRunning;
extern bool isCaptured;
extern String client_password;
extern String client_ip;

void startCaptivePortal(String* ssid);
void updateCaptivePortal();
void stopCaptivePortal();

#endif