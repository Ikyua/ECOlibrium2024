    server.on("/gpsdata", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/plain", readGPS().c_str());
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
