//Must use esp32 v2.0.3rc-1
#include <SD_MMC.h>
#include "esp_camera.h"
#include <WiFiManager.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"

AsyncWebServer server(82);

const char *soft_ap_ssid = "ESP32-CAM";
const char *soft_ap_password = "testpassword";

#define MINUTES_BETWEEN_PHOTOS 1
#define FLASH_PIN 4 

// Global variables for non-blocking timer
unsigned long previousMillis = 0;
const long interval = MINUTES_BETWEEN_PHOTOS * 60 * 1000;

// Circular buffer for Serial Output
#define BUFFER_SIZE 2000
char serialBuffer[BUFFER_SIZE];
uint16_t bufferStart = 0;
uint16_t bufferEnd = 0;

void addToBuffer(const String& data) {
    for (uint16_t i = 0; i < data.length(); i++) {
        serialBuffer[bufferEnd] = data[i];
        bufferEnd = (bufferEnd + 1) % BUFFER_SIZE;
        if (bufferEnd == bufferStart) {
            bufferStart = (bufferStart + 1) % BUFFER_SIZE;
        }
    }
}

String getBufferContents() {
    String result = "";
    uint16_t current = bufferStart;
    while (current != bufferEnd) {
        result += serialBuffer[current];
        current = (current + 1) % BUFFER_SIZE;
    }
    return result;
}

void printAndBuffer(const String& message, bool newline = true) {
    if (newline) {
        Serial.println(message);
        addToBuffer(message + "\n");
    } else {
        Serial.print(message);
        addToBuffer(message);
    }
}

bool startMicroSD() {
  printAndBuffer("Starting microSD... ");
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  if(SD_MMC.begin("/sdcard", true)) {
    printAndBuffer("OKAY");
    return true;
  } else {
    printAndBuffer("FAILED");
    return false;
  }
}

void takePhoto(const String& filename) { 
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    printAndBuffer("Unable to take a photo");
    return;
  }
  if (fb->format != PIXFORMAT_JPEG) {
     printAndBuffer("Capture format not JPEG");
     esp_camera_fb_return(fb);
     return;
  }
  File file = SD_MMC.open(filename.c_str(), "w");
  if(file) {
    printAndBuffer("Saving " + filename);
    file.write(fb->buf, fb->len);
    file.close();
  } else {
    printAndBuffer("Unable to write " + filename);
  }
  esp_camera_fb_return(fb);
}

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"
void startCameraServer();

const char index_html[] PROGMEM = R"rawliteral(
// ... [Rest of your HTML content]
)rawliteral";

String processor(const String& var){
  if(var == "SERIALOUTPUT"){
    return getBufferContents();
  }
  return String();
}

void setup() {
  Serial.begin(115200);
  startMicroSD();
  pinMode(FLASH_PIN, OUTPUT);
  digitalWrite(FLASH_PIN, LOW);
  WiFi.mode(WIFI_AP_STA);
  Serial.setDebugOutput(true);
  Serial.println();
  
  // ... [Camera initialization and setup]
  
  WiFiManager wm;
  wm.setConfigPortalBlocking(true);
  wm.setConfigPortalTimeout(120);
  if(wm.autoConnect("ESP32-CAMAP")){
        printAndBuffer("Sensor Connected :)");
  } else {
        printAndBuffer("AccessPoint Only Mode");
  }
  WiFi.softAP(soft_ap_ssid, soft_ap_password);

  startCameraServer();

  if (WiFi.status() == WL_CONNECTED) {
    printAndBuffer("Camera Ready! Use http://");
    printAndBuffer(WiFi.localIP().toString());
  }
  printAndBuffer("SoftAP IP: http://");
  printAndBuffer(WiFi.softAPIP().toString());

  server.on("/server", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/server/serial", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send_P(200, "text/plain", getBufferContents().c_str());
});
  server.begin();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    static int number = 0;
    number++;
    String filename = "/photo_";
    if(number < 1000) filename += "0";
    if(number < 100)  filename += "0";
    if(number < 10)   filename += "0";
    filename += number;
    filename += ".jpg";
    takePhoto(filename);
  }
}
