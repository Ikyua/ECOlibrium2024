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
