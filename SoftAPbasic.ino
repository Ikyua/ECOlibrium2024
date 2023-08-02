#include <WiFi.h>
#include <ESPAsyncWebServer.h>
 
const char* wifi_network_ssid = "yourNetworkName";
const char* wifi_network_password =  "yourNetworkPass";
 
const char *soft_ap_ssid = "MyESP32AP";
const char *soft_ap_password = "testpassword";
 
AsyncWebServer server(80);
 
 
void setup() {
 
  Serial.begin(115200);
  WiFi.mode(WIFI_MODE_APSTA);
 
  WiFi.softAP(soft_ap_ssid, soft_ap_password);
 
  WiFi.begin(wifi_network_ssid, wifi_network_password);
 
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.print("ESP32 IP as soft AP: ");
  Serial.println(WiFi.softAPIP());
 
  Serial.print("ESP32 IP on the WiFi network: ");
  Serial.println(WiFi.localIP());
 
 
  server.on("/hello", HTTP_GET, [](AsyncWebServerRequest * request) {
 
    if (ON_STA_FILTER(request)) {
      request->send(200, "text/plain", "Hello from STA");
      return;
 
    } else if (ON_AP_FILTER(request)) {
      request->send(200, "text/plain", "Hello from AP");
      return;
    }
 
    request->send(200, "text/plain", "Hello from undefined");
  });
 
  server.begin();
 
}
 
void loop() {}
