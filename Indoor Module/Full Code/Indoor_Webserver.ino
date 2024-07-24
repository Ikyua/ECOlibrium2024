#include <Wire.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SensirionI2CSen5x.h>
#include <SensirionI2CScd4x.h>

const char* ssid = "Syed Rayyan’s iPhone";
const char* password = "rayyan123";

const char* soft_ap_ssid = "ESP32_AP";
const char* soft_ap_password = "your_AP_PASSWORD";

AsyncWebServer server(80);
SensirionI2CSen5x sen5x;
SensirionI2CScd4x scd4x;

String serialBuffer = "";

float massConcentrationPm1p0;
float massConcentrationPm2p5;
float massConcentrationPm4p0;
float massConcentrationPm10p0;
float ambientHumidity;
float ambientTemperature;
float vocIndex;
float noxIndex;
uint16_t co2;

void printAndBuffer(String message, bool newline=true) {
  if (newline) {
    Serial.println(message);
    serialBuffer += message + "\n";
  } else {
    Serial.print(message);
    serialBuffer += message;
  }

while (serialBuffer.length() > 2000) {  // Adjust the size as necessary
    int nextLineBreak = serialBuffer.indexOf('\n') + 1;
    if (nextLineBreak > 0) {
      serialBuffer = serialBuffer.substring(nextLineBreak);
    } else {
      serialBuffer = serialBuffer.substring(100);  // Default trimming
    }
  }
}

void appendToBuffer(char c) {
    serialBuffer += c;
    if (serialBuffer.length() > 2000) {
        serialBuffer = "";  // Clear buffer when size limit is reached
    }
}

String readScdCo2() {
  
    uint16_t c;
    float t;
    float h;
    
    scd4x.readMeasurement(c, t, h);
    
    if (c != 0) {
      co2 = c;
    }
    
    return String(co2);
}

String readPm1p0() {
    
    sen5x.readMeasuredValues(
        massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
        massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
        noxIndex);
    
    return String(massConcentrationPm1p0);
}

String readPm2p5() {
    
    sen5x.readMeasuredValues(
        massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
        massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
        noxIndex);
    
    return String(massConcentrationPm2p5);
}

String readPm4p0() {
    
    sen5x.readMeasuredValues(
        massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
        massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
        noxIndex);
    
    return String(massConcentrationPm4p0);
}

String readPm10p0() {
    
    sen5x.readMeasuredValues(
        massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
        massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
        noxIndex);
    
    return String(massConcentrationPm10p0);
}

String readAmbientHumidity() {
    
    sen5x.readMeasuredValues(
        massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
        massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
        noxIndex);
    
    return String(ambientHumidity);
}

String readAmbientTemperature() {
    
    sen5x.readMeasuredValues(
        massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
        massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
        noxIndex);
    
    return String(ambientTemperature);
}

String readVocIndex() {
    
    sen5x.readMeasuredValues(
        massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
        massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
        noxIndex);
    
    return String(vocIndex);
}

String readNoxIndex() {
    
    sen5x.readMeasuredValues(
        massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
        massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
        noxIndex);
    
    return String(noxIndex);
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
    <title>ESP32 Air Quality Monitor</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        html {
            font-family: Arial;
            display: inline-block;
            text-align: center;
        }
        h1 {
            font-size: 4.0rem;
        }
        p {
            font-size: 3.0rem;
        }
        .units {
            font-size: 1.2rem;
        }
        .dht-labels {
            font-size: 1.5rem;
            vertical-align:middle;
            padding-bottom: 15px;
        }
        button {
            background-color: white;
            border: none;
            color: forestgreen;
            padding: 15px 32px;
            text-align: center;
            text-decoration: none;
            display: inline-block;
            font-size: 100px;
            margin: 4px 2px;
            cursor: pointer;
        }

        button:hover {
            background-color: white;
        }
    </style>
</head>
<body>
<h1>ESP32 Air Quality Monitor</h1>
<p>
    <span class="dht-labels">PM1.0:</span>
    <span id="pm1p0">%PM1P0%</span>
</p>
<p>
    <span class="dht-labels">PM2.5:</span>
    <span id="pm2p5">%PM2P5%</span>
</p>
<p>
    <span class="dht-labels">PM4.0:</span>
    <span id="pm4p0">%PM4P0%</span>
</p>
<p>
    <span class="dht-labels">PM10.0:</span>
    <span id="pm10p0">%PM10P0%</span>
</p>
<p>
    <span class="dht-labels">CO2:</span>
    <span id="co2">%CO2%</span>
</p>
<p>
    <span class="dht-labels">Humidity:</span>
    <span id="ambienthumidity">%AMBIENTHUMIDITY%</span>
</p>
<p>
    <span class="dht-labels">Temperature:</span>
    <span id="ambienttemperature">%AMBIENTTEMPERATURE%</span>
</p>
<p>
    <span class="dht-labels">VOC Index:</span>
    <span id="vocindex">%VOCINDEX%</span>
</p>
<p>
    <span class="dht-labels">NOx Index:</span>
    <span id="noxindex">%NOXINDEX%</span>
</p>
<p style="text-align: center;">
    <span id="serial-output">%SERIAL% </span>
</p>

<button id = "online-button"> Go Online</button>

</body>
<script>
    setInterval(function () {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
                document.getElementById("pm1p0").innerHTML = this.responseText;
            }
        };
        xhttp.open("GET", "/pm1p0", true);
        xhttp.send();
    }, 5000);

    setInterval(function () {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
                document.getElementById("pm2p5").innerHTML = this.responseText;
            }
        };
        xhttp.open("GET", "/pm2p5", true);
        xhttp.send();
    }, 5000);

    setInterval(function () {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
                document.getElementById("pm4p0").innerHTML = this.responseText;
            }
        };
        xhttp.open("GET", "/pm4p0", true);
        xhttp.send();
    }, 5000);

    setInterval(function () {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
                document.getElementById("pm10p0").innerHTML = this.responseText;
            }
        };
        xhttp.open("GET", "/pm10p0", true);
        xhttp.send();
    }, 5000);

    setInterval(function () {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
                document.getElementById("co2").innerHTML = this.responseText;
            }
        };
        xhttp.open("GET", "/co2", true);
        xhttp.send();
    }, 5000);

    setInterval(function () {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
                document.getElementById("ambienthumidity").innerHTML = this.responseText;
            }
        };
        xhttp.open("GET", "/ambienthumidity", true);
        xhttp.send();
    }, 5000);

    setInterval(function () {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
                document.getElementById("ambienttemperature").innerHTML = this.responseText;
            }
        };
        xhttp.open("GET", "/ambienttemperature", true);
        xhttp.send();
    }, 5000);

    setInterval(function () {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
                document.getElementById("vocindex").innerHTML = this.responseText;
            }
        };
        xhttp.open("GET", "/vocindex", true);
        xhttp.send();
    }, 5000);

    setInterval(function () {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
                document.getElementById("noxindex").innerHTML = this.responseText;
            }
        };
        xhttp.open("GET", "/noxindex", true);
        xhttp.send();
    }, 5000);

    setInterval(function () {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
                document.getElementById("serial-output").innerHTML = this.responseText;
            }
        };
        xhttp.open("GET", "/serial", true);
        xhttp.send();
    }, 5000);
</script>
</html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String& var){
  //Serial.println(var);
//  if(var == "TEMPERATURE"){
//    return readScdTemp();
//  }
//  else if(var == "HUMIDITY"){
//    return readScdHum();
//  }
//  else if(var == "CO2"){
//    return readScdCo2();
//  }
  if(var == "PM1P0"){
    return readPm1p0();
  }
  else if(var == "PM2P5"){
    return readPm2p5();
  }
  else if(var == "PM4P0"){
    return readPm4p0();
  }
  else if(var == "PM10P0"){
    return readPm10p0();
  }
  else if(var == "AMBIENTHUMIDITY"){
    return readAmbientHumidity();
  }
  else if(var == "AMBIENTTEMPERATURE"){
    return readAmbientTemperature();
  }
  else if(var == "VOCINDEX"){
    return readVocIndex();
  }
  else if(var == "CO2") {
    return readScdCo2();
  }
  else if(var == "NOXINDEX") {
    return readNoxIndex();
  }
  else if(var == "SERIALOUTPUT"){
  return serialBuffer;
}
  
  
  return String();
}

void setup() {
  Serial.begin(115200);

  // Try to connect to WiFi
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts > 10) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to WiFi");
  } else {
    Serial.println("Failed to connect to WiFi, starting AP mode");
    WiFi.softAP(soft_ap_ssid, soft_ap_password);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
  }

  // Initialize sensors
  // Your sensor initialization code here
  //begin sensors

Wire.begin();

sen5x.begin(Wire);

sen5x.deviceReset();

sen5x.setTemperatureOffsetSimple(0.00);

sen5x.startMeasurement();

delay(1000);

scd4x.begin(Wire);

scd4x.startPeriodicMeasurement();
  
  
 
  // Route for web page
server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
        request->send_P(200, "text/html", index_html, processor);
        return;
    }
  });
server.on("/pm1p0", HTTP_GET, [](AsyncWebServerRequest *request){
    if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
        request->send_P(200, "text/plain", readPm1p0().c_str());
        return;
    }
  });
server.on("/pm2p5", HTTP_GET, [](AsyncWebServerRequest *request){
      if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
        request->send_P(200, "text/plain", readPm2p5().c_str());
        return;
  }
});

server.on("/pm4p0", HTTP_GET, [](AsyncWebServerRequest *request){
      if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
        request->send_P(200, "text/plain", readPm4p0().c_str());
        return;
  }
});

server.on("/pm10p0", HTTP_GET, [](AsyncWebServerRequest *request){
      if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
        request->send_P(200, "text/plain", readPm10p0().c_str());
        return;
  }
});

server.on("/co2", HTTP_GET, [](AsyncWebServerRequest *request){
      if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
        request->send_P(200, "text/plain", readScdCo2().c_str());
        return;
  }
});

server.on("/ambienthumidity", HTTP_GET, [](AsyncWebServerRequest *request){
      if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
        request->send_P(200, "text/plain", readAmbientHumidity().c_str());
        return;
  }
});

server.on("/ambienttemperature", HTTP_GET, [](AsyncWebServerRequest *request){
      if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
        request->send_P(200, "text/plain", readAmbientTemperature().c_str());
        return;
  }
});

  server.on("/vocindex", HTTP_GET, [](AsyncWebServerRequest *request){
        if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
        request->send_P(200, "text/plain", readVocIndex().c_str());
        return;
  }
});

  server.on("/noxindex", HTTP_GET, [](AsyncWebServerRequest *request){
        if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
        request->send_P(200, "text/plain", readNoxIndex().c_str());
        return;
  }  
});
  server.on("/serial", HTTP_GET, [](AsyncWebServerRequest *request){
        if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
        request->send_P(200, "text/plain", serialBuffer.c_str());
        return;
  }
});


  // Serve web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  server.begin();
}

void loop() {
  // Your main code here

  delay(1000); // Example delay
}
