#include <ESP8266WiFi.h>
#include "wifi.h"
#include "secrets.h"

WiFiClient ConnectWifi() {
  WiFiClient wifiClient;
  delay(10);
  WiFi.begin(GetWifiSSID(), GetWifiPassword());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");    
  }
  
  Serial.println("");
  Serial.println("WiFi verbunden");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());
  
  return wifiClient;
}
