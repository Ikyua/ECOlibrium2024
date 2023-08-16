#include <SD_MMC.h>
#include <esp_camera.h>
#include <WiFiManager.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

const char *soft_ap_ssid = "ESP32-CAM";
const char *soft_ap_password = "testpassword";
#define MINUTES_BETWEEN_PHOTOS 1
#define FLASH_PIN 4

String serialBuffer = "";
AsyncWebServer server(82);

#define RXD0 3  // GPIO3, IO3
#define TXD0 1  // GPIO1, IO1
TinyGPSPlus gps;
HardwareSerial GPSSerial(1);

void printAndBuffer(String message, bool newline = true) {
    // ... [Same as before]
}

bool startMicroSD() {
    // ... [Same as before]
}

void takePhoto(String filename) {
    // ... [Same as before]
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
<p>
  <i class="fas fa-location-arrow" style="color:#0099ff;"></i>
  <span class="dht-labels">GPS Location:</span>
</p>
<div id="gpsOutput">%GPSDATA%</div>
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

setInterval(function() {
    location.reload();
}, 10000);  // Reload the page every 10 seconds

</script>
</html>)rawliteral";

String processor(const String &var) {
    if (var == "SERIALOUTPUT") {
        return serialBuffer;
    }
    if (var == "GPSDATA") {
        if (gps.location.isValid()) {
            return "Latitude: " + String(gps.location.lat(), 6) + ", Longitude: " + String(gps.location.lng(), 6);
        } else {
            return "Error: GPS not fixed yet.";
        }
    }
    return String();
}

void setup() {
    // ... [Your previous setup code]

    // Initialize the GPS
    GPSSerial.begin(9600, SERIAL_8N1, RXD0, TXD0);

    // ... [Rest of your setup code]
}

void loop() {
    // ... [Your previous loop code]

    // Read data from GPS and update the GPS object
    while (GPSSerial.available()) {
        gps.encode(GPSSerial.read());
    }

    // ... [Rest of your loop code]
}

// ... [Your existing server routes]

// ... [Your main function]
