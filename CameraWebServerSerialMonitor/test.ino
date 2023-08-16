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
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script src="https://kit.fontawesome.com/d91e44f906.js" crossorigin="anonymous"></script>
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP32-CAM Webserver</h2>
<p>
  <i class="fas fa-terminal" style="color:#ff6600;"></i>
  <span class="dht-labels">Serial Monitor Output:</span>
</p>
<div style="height:300px; overflow:auto; border:1px solid #ccc; padding:5px;">
  <pre id="serialOutput">%SERIALOUTPUT%</pre>
</div>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("serialOutput").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/serial", true);
  xhttp.send();
}, 5000 );
</script>
</html>)rawliteral";

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
  
  // Initialize GPS UART
  GPSSerial.begin(9600, SERIAL_8N1, 1, 3); // UART0, TX on GPIO1, RX on GPIO3
  
  // ... Rest of the setup code ...

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/serial", HTTP_GET, [](AsyncWebServerRequest *request){
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

  while (GPSSerial.available()) {
    gps.encode(GPSSerial.read());
  }
}
