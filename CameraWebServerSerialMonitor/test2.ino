// ... [Starting code as before]

void setup() {
  Serial.begin(115200);
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

  server.on("/server", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/server/serial", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getBufferContents().c_str());
  });
  server.begin();
}

// ... [Rest of the code as before]

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
