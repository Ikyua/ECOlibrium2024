//Arav Sharma
//Ecolibrium Remote Sensor
//Air Quality Monitor
String serialBuffer = "";


#include <WiFiManager.h> 
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include <SensirionI2CSen5x.h>
#include <SensirionI2CScd4x.h>
#include <Wire.h>
#include <time.h>
SensirionI2CSen5x sen5x;
SensirionI2CScd4x scd4x;

const char *soft_ap_ssid = "IndoorModule1";
const char *soft_ap_password = "testpassword";

AsyncWebServer server(80);

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

  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script src="https://kit.fontawesome.com/d91e44f906.js" crossorigin="anonymous"></script>
  <style>
    html {
    font-family: Arial;
    margin: 0px auto;
    text-align: center;
}
    h2 { font-size: 3.0rem; }
    td, th {
      padding: 15px;
      text-align: middle;
    }
    .units { font-size: 1.2rem; }
    .dht-labels {
      font-size: 1.5rem;
    }
    table {
  width: 60%; /* You can adjust this value based on your preference */
  border-collapse: collapse;
  margin-left: auto;
  margin-right: auto;
}
    th {
      background-color: #f2f2f2;
    }
  </style>
</head>
<body>
  <h2>ESP32 Air Quality Server</h2>
  <table>
    <tr>
      <th>Measurement</th>
      <th>Value</th>
    </tr>
    <tr>
  <td>PM 1.0</td>
  <td id="pm1p0">%PM1P0%<sup style="font-size:8.0pt">ug/m3</sup></td>
</tr>
<tr>
  <td>PM 2.5</td>
  <td id="pm2p5">%PM2P5%<sup style="font-size:8.0pt">ug/m3</sup></td>
</tr>
<tr>
  <td>PM 4.0</td>
  <td id="pm4p0">%PM4P0%<sup style="font-size:8.0pt">ug/m3</sup></td>
</tr>
<tr>
  <td>PM 10</td>
  <td id="pm10p0">%PM10P0%<sup style="font-size:8.0pt">ug/m3</sup></td>
</tr>
<tr>
  <td>Carbon Dioxide</td>
  <td id="co2">%CO2%<sup style="font-size:8.0pt">ppm</sup></td>
</tr>
<tr>
  <td>Ambient Humidity</td>
  <td id="ambienthumidity">%AMBIENTHUMIDITY%<sup style="font-size:8.0pt">&percnt;</sup></td>
</tr>
<tr>
  <td>Ambient Temperature</td>
  <td id="ambienttemperature">%AMBIENTTEMPERATURE%<sup style="font-size:8.0pt">&deg;C</sup></td>
</tr>
<tr>
  <td>Voc Index</td>
  <td id="vocindex">%VOCINDEX%<sup style="font-size:8.0pt">-</sup></td>
</tr>
<tr>
  <td>Nox Index</td>
  <td id="noxindex">%NOXINDEX%<sup style="font-size:8.0pt">-</sup></td>
</tr>
  </table>
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
      document.getElementById("pm1p0").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/pm1p0", true);
  xhttp.send();
}, 5000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("pm2p5").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/pm2p5", true);
  xhttp.send();
}, 5000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("pm4p0").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/pm4p0", true);
  xhttp.send();
}, 5000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("pm10p0").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/pm10p0", true);
  xhttp.send();
}, 5000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("co2").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/co2", true);
  xhttp.send();
}, 5000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ambienthumidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/ambienthumidity", true);
  xhttp.send();
}, 5000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ambienttemperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/ambienttemperature", true);
  xhttp.send();
}, 5000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("vocindex").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/vocindex", true);
  xhttp.send();
}, 5000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("noxindex").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/noxindex", true);
  xhttp.send();
}, 5000 ) ;

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


void setup(){
// Connect to Wi-Fi
  WiFi.mode(WIFI_AP_STA);
  Serial.begin(115200);
printAndBuffer("To connect to the WiFi network, follow these steps:");
printAndBuffer("1. Open your device's WiFi settings.");
printAndBuffer("2. Look for a network named 'AutoConnectAP'.");
printAndBuffer("3. Connect to 'AutoConnectAP'. If it's password-protected, use 'password' as the password.");
printAndBuffer("4. Wait until your device indicates that it has connected to the network.");
printAndBuffer("5. Once connected, enter the following address in a web browser: http://192.168.1.218/");

  // WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;
  
  bool res;

  res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

  if(!res) {
    printAndBuffer("Failed to connect");
    // ESP.restart();
  } 
  else {
    //if you get here you have connected to the WiFi    
    printAndBuffer("connected...yeey :)");
  }
  WiFi.softAP(soft_ap_ssid, soft_ap_password);
  //start sensors

  printAndBuffer("ESP32 IP as soft AP: ");
  printAndBuffer(WiFi.softAPIP().toString());

printAndBuffer("Local network server:");
printAndBuffer("http://",false);
printAndBuffer(WiFi.localIP().toString());

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

  // Start server
  server.begin();
printAndBuffer("Paste into web browser:");
printAndBuffer("http://", false); // false indicates that we're not adding a newline
printAndBuffer(WiFi.localIP().toString());
}
 
void loop(){
  
  delay(1000);
}
