//Must use esp32 v2.0.3rc-1

#include <SD_MMC.h>
#include "esp_camera.h"
#include <WiFiManager.h>

const char *soft_ap_ssid = "ESP32-CAM";
const char *soft_ap_password = "ESP32-CAMpassword";

#define MINUTES_BETWEEN_PHOTOS 1

#define FLASH_PIN         4

bool startMicroSD() {
  Serial.print("Starting microSD... ");

  // Pin 13 needs to be pulled-up
  // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/sd_pullup_requirements.html#pull-up-conflicts-on-gpio13
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  if(SD_MMC.begin("/sdcard", true)) {
    Serial.println("OKAY");
    return true;
  } else {
    Serial.println("FAILED");
    return false;
  }
}

void takePhoto(String filename) { 
  // Take a photo and get the data

  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Unable to take a photo");
    return;
  }

  // Make sure it is a JPEG
  if (fb->format != PIXFORMAT_JPEG) {
     Serial.println("Capture format not JPEG");
     esp_camera_fb_return(fb); // Return the photo data
     return;
  }

  // Save the picture to the SD card

  File file = SD_MMC.open(filename.c_str(), "w");
  if(file) {
    Serial.println("Saving " + filename);
    file.write(fb->buf, fb->len);
    file.close();

    // Momentarily blink the flash
    digitalWrite(FLASH_PIN, HIGH);
    delay(100);
    digitalWrite(FLASH_PIN, LOW);
    delay(500);
  } else {
    Serial.println("Unable to write " + filename);
  }

  // Return the picture data
  esp_camera_fb_return(fb);
}

//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_UNITCAM // No PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM

#include "camera_pins.h"

void startCameraServer();

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
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
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

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  if(psramFound()){
    s->set_framesize(s, FRAMESIZE_UXGA);
  } else {
    s->set_framesize(s, FRAMESIZE_VGA);
  }
#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  WiFiManager wm;

  wm.resetSettings();
  wm.setConfigPortalBlocking(true);
  wm.setConfigPortalTimeout(120);
  if(wm.autoConnect("ESP32-CAMAP")){
        Serial.println("Sensor Connected :)");
  }
    else {
        Serial.println("AccessPoint Only Mode");
  }

  WiFi.softAP(soft_ap_ssid, soft_ap_password);

  startCameraServer();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Camera Ready! Use http://");
    Serial.println(WiFi.localIP());
  }
  Serial.print("SoftAP IP: http://");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  // Keep a count of the number of photos we have taken
  static int number = 0;
  number++;

  // Construct a filename that looks like "/photo_0001.jpg"
  
  String filename = "/photo_";
  if(number < 1000) filename += "0";
  if(number < 100)  filename += "0";
  if(number < 10)   filename += "0";
  filename += number;
  filename += ".jpg";
  
  takePhoto(filename);

  // Delay until the next photo

  delay(MINUTES_BETWEEN_PHOTOS * 60 * 1000);
}
