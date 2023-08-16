// Must use esp32 v2.0.3rc-1
#include <SD_MMC.h>
#include "esp_camera.h"
#include <WiFiManager.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <TinyGPS++.h>
#include <exif.h>

AsyncWebServer server(82);

const char *soft_ap_ssid = "ESP32-CAM";
const char *soft_ap_password = "testpassword";

// ... (Your global variables and setup functions above)

String photo_list_html = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <!-- ... Your head content ... -->
</head>
<body>
    <h2>ESP32-CAM Webserver - Photo List</h2>
    <ul>
        %PHOTOLIST%
    </ul>
</body>
</html>
)rawliteral";

String generatePhotoList() {
    String photolist = "";
    File dir = SD_MMC.open("/");
    while (true) {
        File entry = dir.openNextFile();
        if (!entry) {
            break;
        }
        if (entry.isDirectory()) {
            // Skip directories
            continue;
        }
        String filename = entry.name();
        photolist += "<li><a href='/photos?filename=" + filename + "'>" + filename + "</a> ";
        photolist += "<a href='/download?filename=" + filename + "'>[Download]</a></li>";
        entry.close();
    }
    dir.close();
    return photolist;
}

void setup() {
    // ... (Your existing setup code above)

    server.on("/photolist", HTTP_GET, [](AsyncWebServerRequest *request){
        String photolist = generatePhotoList();
        photo_list_html.replace("%PHOTOLIST%", photolist);
        request->send_P(200, "text/html", photo_list_html.c_str());
    });

    server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request){
        String photoPath = "/" + request->getParam("filename")->value();
        File photoFile = SD_MMC.open(photoPath, FILE_READ);
        if (photoFile) {
            AsyncWebServerResponse *response = request->beginResponse(SD_MMC, photoPath, "image/jpeg");
            response->addHeader("Content-Disposition", "attachment; filename=" + request->getParam("filename")->value());
            request->send(response);
            photoFile.close();
        } else {
            request->send(404);
        }
    });

    // ... (Your existing server handlers above)
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
    if (file) {
        printAndBuffer("Saving " + filename);

        // Write the image data
        file.write(fb->buf, fb->len);
        file.close();

        // Add GPS information to EXIF metadata if GPS is valid
        if (gps.location.isValid()) {
            file = SD_MMC.open(filename.c_str(), "r+");
            if (file) {
                EXIFInfo exif;
                exif.setImageSize(fb->len);
                exif.setThumbnailSize(0);  // No thumbnail

                exif.addGpsMetadata(gps.location.lat(), gps.location.lng());

                exif.updateFile(file);
                file.close();
            } else {
                printAndBuffer("Unable to update EXIF metadata");
            }
        }
    } else {
        printAndBuffer("Unable to write " + filename);
    }

    esp_camera_fb_return(fb);
}

// ... (Your loop and other functions above)
