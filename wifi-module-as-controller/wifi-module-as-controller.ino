#include "ESP8266WiFi.h"

void setup()
{
  Serial.begin(115200);
  Serial.println();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
}

void loop()
{
  Serial.println(getRSSI());
  delay(500);
}

int32_t getRSSI() {
  byte available_networks = WiFi.scanNetworks();
  int strength = 0;
  for (int network = 0; network < available_networks; network++) {
    strength += abs(WiFi.RSSI(network));
  }
  return strength;
}
