#include <SD_MMC.h>
#include "esp_camera.h"
#include <WiFiManager.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <TinyGPS++.h>

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

#define RXD0 3
#define TXD0 1
TinyGPSPlus gps;
HardwareSerial GPSSerial(1); // Use Hardware Serial 1

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
    String timestamp = "[" + String(millis()) + " ms] ";
    String output = timestamp + message;

    if (newline) {
        Serial.println(output);
        addToBuffer(output + "\n");
    } else {
        Serial.print(output);
        addToBuffer(output);
    }
}

bool startMicroSD() {
    // ... your existing code ...
}

void takePhoto(const String& filename) {
    // ... your existing code ...
}

void setup() {
    // ... your existing setup code ...

    server.on("/photos", HTTP_GET, [](AsyncWebServerRequest *request){
        String html = "<h2>Photos on SD Card</h2>";
        File root = SD_MMC.open("/");
        File file = root.openNextFile();
        while(file){
            if(file.isDirectory()){
                html += "<p><a href='" + file.name() + "'>" + file.name() + "</a></p>";
            }
            file = root.openNextFile();
        }
        request->send(200, "text/html", html);
    });

    server.onNotFound([](AsyncWebServerRequest *request){
        String path = request->url();
        if (path.endsWith(".jpg")) {
            File photoFile = SD_MMC.open(path, FILE_READ);
            if(photoFile){
                request->send(SD_MMC, path, "image/jpeg");
                photoFile.close();
            }
        }
    });

    // ... your existing server setup ...
}

void loop() {
    // ... your existing loop code ...
}
