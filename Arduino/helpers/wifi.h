#pragma once

#include <WiFi.h>

String mac2String(byte ar[]) {
  String s;
  for (byte i = 0; i < 6; ++i) {
    char buf[3];
    sprintf(buf, "%02X", ar[i]); // J-M-L: slight modification, added the 0 in
                                 // the format for padding
    s += buf;
    if (i < 5)
      s += ':';
  }
  return s;
}

// WiFi connection status tracking
unsigned long wifiStartTime = 0;    // Timestamp when WiFi connection attempt started
bool wifiConnected = false;         // Current WiFi connection status

void setupWiFi() {

    Serial.println(F("[WiFi] Initializing..."));

  uint64_t chipid = ESP.getEfuseMac();  //The chip ID is essentially its MAC address(length: 6 bytes).
  Serial.print("[Eth ] MAC address: ");
  Serial.println(mac2String((byte *)&chipid));

  // Non-blocking WiFi connection
  wifiStartTime = millis();
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  Serial.print("[Wifi] Attempting to connect to SSID: ");
  Serial.println(SECRET_SSID);
}

void loopWifi() {
  // Check WiFi connection status
  if (!wifiConnected && WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("[WiFi] Connected!");
    Serial.print("[WiFi] IP address: ");
    Serial.println(WiFi.localIP());
  } else if (wifiConnected && WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
    Serial.println("[WiFi] Connection lost!");
  }

  // Reconnect if connection is lost for too long
  if (!wifiConnected && (millis() - wifiStartTime > 60000)) { // 1 minute
    Serial.println("[WiFi] Attempting reconnection...");
    WiFi.disconnect();
    WiFi.begin(SECRET_SSID, SECRET_PASS);
    wifiStartTime = millis();
  }
}
