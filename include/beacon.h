#ifndef BEACON_H
#define BEACON_H

void initBeaconSpam(String* network_name);
void BeaconSpam();

#define MAX_SSIDS 100
#define BEACON_INTERVAL_MS 100
#define HOP_INTERVAL_MS 300

#endif