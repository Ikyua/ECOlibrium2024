//Must use esp32 v2.0.3rc-1
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
  String newFilename = filename;
  int fileNumber = 1;

  while (SD_MMC.exists(newFilename)) {
    newFilename = filename.substring(0, filename.lastIndexOf('.')) + "_" + String(fileNumber) + ".jpg";
    fileNumber++;
  }

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

  File file = SD_MMC.open(newFilename.c_str(), "w");
  if (file) {
    printAndBuffer("Saving " + newFilename);
    file.write(fb->buf, fb->len);
    file.close();
  } else {
    printAndBuffer("Unable to write " + newFilename);
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
        <i class="fas fa-location-arrow" style="color:#0099ff;"></i>
        <span class="dht-labels">GPS Location:</span>
        <span id="gpsOutput">%GPSDATA%</span>
    </p>
    <p>
        <i class="fas fa-terminal" style="color:#ff6600;"></i>
        <span class="dht-labels">Serial Monitor Output:</span>
    </p>
    <div style="height:300px; overflow:auto; border:1px solid #ccc; padding:5px;">
        <pre id="serialOutput">%SERIALOUTPUT%</pre>
    </div>
    <p>
    <i class="fas fa-images" style="color:#0099ff;"></i>
    <span class="dht-labels"><a href="/photos">View Photos</a></span>
    </p>
</body>
<script>
    setInterval(function ( ) {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
                document.getElementById("gpsOutput").innerHTML = this.responseText;
            }
        };
        xhttp.open("GET", "/gpsdata", true);
        xhttp.send();
    }, 5000 );
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

String readGPS() {
    if (gps.location.isValid()) {
        return String(gps.location.lat(), 6) + ", " + String(gps.location.lng(), 6);
    } else {
        return "------";
    }
}

void handleListImages(AsyncWebServerRequest *request) {
    String html = "<html><body><ul>";

    File root = SD_MMC.open("/");
    if (root) {
        File file = root.openNextFile();
        while (file) {
            String fileName = file.name();
            if (fileName.endsWith(".jpg") || fileName.endsWith(".jpeg")) {
                html += "<li><a href='/view?name=" + fileName + "'>" + fileName + "</a></li>";
            }
            file = root.openNextFile();
        }
    }
    html += "</ul><br/><a href='/'>Back to main page</a></body></html>";

    request->send(200, "text/html", html);
}

void handleGetImage(AsyncWebServerRequest *request) {
    if (request->hasArg("name")) {
        String filename = "/" + request->arg("name");
        if (SD_MMC.exists(filename)) {
            AsyncWebServerResponse *response = request->beginResponse(SD_MMC, filename, "image/jpeg");
            request->send(response);
            return;
        }
    }
    request->send(404, "text/plain", "Image not found");
}

String processor(const String& var){
    if (var == "GPSDATA") {
        return readGPS();
    }
    else if (var == "SERIALOUTPUT") {
        return getBufferContents();
    }
    return String();
}

void setup() {
  Serial.begin(115200);
  GPSSerial.begin(9600, SERIAL_8N1, RXD0, TXD0);
  startMicroSD();
  pinMode(FLASH_PIN, OUTPUT);
  digitalWrite(FLASH_PIN, LOW);
  WiFi.mode(WIFI_AP_STA);
  Serial.setDebugOutput(true);
  Serial.println();
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    printAndBuffer("Camera init failed with error 0x" + String(err, HEX));
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  if(psramFound()){
    s->set_framesize(s, FRAMESIZE_UXGA);
  } else {
    s->set_framesize(s, FRAMESIZE_VGA);
  }

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

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/gpsdata", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readGPS().c_str());
    });
  server.on("/serial", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getBufferContents().c_str());
  });
  server.on("/photos", HTTP_GET, handleListImages);
  server.on("/view", HTTP_GET, handleGetImage); // This route will be used to view the actual images
  server.begin();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    static int number = 0;
    number++;
    String filename = "/photo_" + String(number) + ".jpg";  // Generate filename
    takePhoto(filename);
    printAndBuffer(String(gps.location.lat(), 6) + ", " + String(gps.location.lng(), 6));
  }
  while (GPSSerial.available()) {
    gps.encode(GPSSerial.read());
  }
}
