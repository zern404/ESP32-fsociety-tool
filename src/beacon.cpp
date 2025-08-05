#include <WiFi.h>
#include <esp_wifi.h>

#include "beacon.h"

struct FakeNetwork {
  String ssid;
  uint8_t bssid[6];
  uint8_t channel;
};

FakeNetwork networks[MAX_SSIDS];

void generateFakeNetworks(String* network_name) {
  for (int i = 0; i < MAX_SSIDS; i++) {
    networks[i].ssid = *network_name + String(i);
    for (int j = 0; j < 6; j++)
      networks[i].bssid[j] = random(256);
    networks[i].channel = random(1, 14);
  }
}

void sendBeacon(FakeNetwork &net) {
  uint8_t packet[256] = {0};
  int ssid_len = net.ssid.length();
  int packet_len = 0;

  packet[0] = 0x80;
  packet[1] = 0x00;
  packet_len = 24;

  memcpy(&packet[4], "\xff\xff\xff\xff\xff\xff", 6);
  memcpy(&packet[10], net.bssid, 6);
  memcpy(&packet[16], net.bssid, 6);
  packet[22] = 0x00; packet[23] = 0x00;

  memset(&packet[24], 0x00, 8);
  packet[32] = 0x64; packet[33] = 0x00;
  packet[34] = 0x31; packet[35] = 0x04;

  int pos = 36;

  packet[pos++] = 0x00;
  packet[pos++] = ssid_len;
  memcpy(&packet[pos], net.ssid.c_str(), ssid_len);
  pos += ssid_len;

  uint8_t supportedRates[] = {0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c};
  memcpy(&packet[pos], supportedRates, sizeof(supportedRates));
  pos += sizeof(supportedRates);

  packet[pos++] = 0x03;
  packet[pos++] = 0x01;
  packet[pos++] = net.channel;

  const uint8_t rsn[] = {
    0x30, 0x18,
    0x01, 0x00,
    0x00, 0x0f, 0xac, 0x04,
    0x01, 0x00,
    0x00, 0x0f, 0xac, 0x04,
    0x01, 0x00,
    0x00, 0x0f, 0xac, 0x02,
    0x00, 0x00
  };
  memcpy(&packet[pos], rsn, sizeof(rsn));
  pos += sizeof(rsn);

  esp_wifi_set_channel(net.channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_80211_tx(WIFI_IF_AP, packet, pos, true);
}

void initBeaconSpam(String* network_name)
{
    WiFi.mode(WIFI_AP);
    esp_wifi_set_promiscuous(true);
    generateFakeNetworks(network_name);
}

void BeaconSpam() 
{
    for (int i = 0; i < MAX_SSIDS; i++) {
        sendBeacon(networks[i]);
        delay(1);
    }
    delay(HOP_INTERVAL_MS);
}