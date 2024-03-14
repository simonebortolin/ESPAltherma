#include "wireless.hpp"

WifiDetails **lastWifiScanResults = nullptr;
int16_t lastWifiScanResultAmount;

void start_standalone_wifi()
{
  IPAddress local_ip(192, 168, 1, 1);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAP("ESPAltherma-Config-WiFi");
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.setHostname("ESPAltherma");
}

void checkWifi()
{
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i++ == 120) {
      debugSerial.println("Tried for 60 sec, rebooting now.");
      restart_board();
    }
  }
}

void setup_wifi()
{
  delay(10);
  // we start by connecting to a WiFi network
  debugSerial.printf("Connecting to %s\n", config->SSID.c_str());

  if(config->SSID_STATIC_IP) {
    IPAddress local_IP;
    IPAddress subnet;
    IPAddress gateway;
    IPAddress primaryDNS;
    IPAddress secondaryDNS;

    local_IP.fromString(config->SSID_IP);
    subnet.fromString(config->SSID_SUBNET);
    gateway.fromString(config->SSID_GATEWAY);

    if(config->SSID_PRIMARY_DNS != "") {
      primaryDNS.fromString(config->SSID_PRIMARY_DNS);
    }

    if(config->SSID_SECONDARY_DNS != "") {
      secondaryDNS.fromString(config->SSID_SECONDARY_DNS);
    }

    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
        debugSerial.println("Failed to set static ip!");
    }
  }

  uint8_t *bssid = nullptr;
  uint32_t wifi_channel = 0;
#ifdef ARDUINO_ARCH_ESP8266
  get_wifi_bssid(WIFI_SSID, bssid, &wifi_channel);
#else //assume ESP32
  WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
  WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
#endif

  if (bssid != nullptr) {
    WiFi.begin(config->SSID.c_str(), config->SSID_PASSWORD.c_str(), wifi_channel, bssid);
  } else {
    WiFi.begin(config->SSID.c_str(), config->SSID_PASSWORD.c_str(), 0, 0, true);
  }

  checkWifi();
  debugSerial.printf("Connected. IP Address: %s\n", WiFi.localIP().toString().c_str());
}

void scan_wifi_delete_result()
{
  for (int16_t i = 0; i < lastWifiScanResultAmount; i++) {
    delete lastWifiScanResults[i];
  }

  delete[] lastWifiScanResults;
}

void scan_wifi()
{
  lastWifiScanResultAmount = WiFi.scanNetworks();
  lastWifiScanResults = new WifiDetails*[lastWifiScanResultAmount];

  for (int16_t i = 0; i < lastWifiScanResultAmount; i++) {
    lastWifiScanResults[i] = new WifiDetails {
      .SSID = WiFi.SSID(i),
      .RSSI = WiFi.RSSI(i),
      .EncryptionType = WiFi.encryptionType(i)
    };

    debugSerial.print(lastWifiScanResults[i]->SSID);
    String serialLog = " (" + String(lastWifiScanResults[i]->RSSI) + ") " + lastWifiScanResults[i]->EncryptionType + "\n";
    debugSerial.print(serialLog);
    delay(10);
  }
}

#ifdef ARDUINO_ARCH_ESP8266
void get_wifi_bssid(const char *ssid, uint8_t *bssid, uint32_t *wifi_channel)
{
  bssid = nullptr;
  int n = WiFi.scanNetworks(false, true);

  if (n < 1) { // no networks found
    return;
  }

  // sort networks on RSSI value
  int indices[n];
  for (int i = 0; i < n; i++) {
    indices[i] = i;
  }

  for (int i = 0; i < n; i++) {
    for (int j = i + 1; j < n; j++) {
      if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
        std::swap(indices[i], indices[j]);
      }
    }
  }

  // loop through result and match highest RSSI SSID
  for (int i = 0; i < n; i++) {
    char scan_ssid[33]; // ssid can be up to 32chars, => plus null term
    strlcpy(scan_ssid, WiFi.SSID(indices[i]).c_str(), sizeof(scan_ssid));

    if (strcmp(ssid, scan_ssid) == 0) {
      if (WiFi.BSSID(indices[i]) != 0) {
        bssid = WiFi.BSSID(indices[i]);
        *wifi_channel = WiFi.channel();
      }
      return;
    }
  }
}
#endif