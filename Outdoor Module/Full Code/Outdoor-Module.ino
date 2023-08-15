
//Ecolibrium Remote Sensor
//Air Quality Monitor
String lastPM1P0SValue = "0"; 
String lastPM2P5SValue = "0";
String lastPM10P0SValue = "0";

String serialBuffer = "";

#define RXD2 16 // To sensor TXD
#define TXD2 17 // To sensor RXD
#define PMS_SET_PIN 26
#define PMS_RST_PIN 25
#define PMS_READ_INTERVAL 9U
#define PMS_READ_DELAY 1U

#include <WiFiManager.h> 
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <NOxGasIndexAlgorithm.h>
#include <SensirionI2CSgp41.h>
#include <VOCGasIndexAlgorithm.h>
#include <Wire.h>
#include <time.h>
#include "PMS.h"
SensirionI2CSgp41 sgp41;

const char *soft_ap_ssid = "OutdoorModule1";
const char *soft_ap_password = "testpassword";

AsyncWebServer server(80);


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

uint8_t pms_tick_count = PMS_READ_INTERVAL;

PMS pms(Serial2);
PMS::DATA data;

static void setup_pins(void) {
  pinMode(PMS_RST_PIN, OUTPUT);
  digitalWrite(PMS_RST_PIN, HIGH);
  pinMode(PMS_SET_PIN, OUTPUT);
  digitalWrite(PMS_SET_PIN, HIGH);
}

static void toggle_set(bool sleep) {
  if (sleep) {
    digitalWrite(PMS_SET_PIN, LOW);
  } else {
    digitalWrite(PMS_SET_PIN, HIGH);
  }
  delay(500);
}

static void toggle_reset(void) {
  digitalWrite(PMS_RST_PIN, LOW);
  delay(500);
  digitalWrite(PMS_RST_PIN, HIGH);
  delay(500);
}

bool pms5003_init(void) {
  setup_pins();
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  delay(1000);
  pms_tick_count = PMS_READ_INTERVAL;
  return true;
}

bool pms5003_read(uint16_t *pmSp1_0, uint16_t *pmSp2_5, uint16_t *pmSp10_0, uint16_t *pmAe1_0, uint16_t *pmAe2_5, uint16_t *pmAe10_0) {
  bool result = false;
  if ((NULL == pmSp1_0) || (NULL == pmSp2_5) || (NULL == pmSp10_0) || (NULL == pmAe1_0) || (NULL == pmAe2_5) || (NULL == pmAe10_0)) {
    result = false;
  } else {
    pms_tick_count++;
    if (pms_tick_count == PMS_READ_DELAY) {
      while (Serial2.available()){      
        Serial2.read();
      }
      if (pms.readUntil(data, 2*PMS::SINGLE_RESPONSE_TIME)){
        *pmSp1_0 = data.PM_AE_UG_1_0;
        *pmSp2_5 = data.PM_AE_UG_2_5;
        *pmSp10_0 = data.PM_AE_UG_10_0;
        *pmAe1_0 = data.PM_SP_UG_1_0;
        *pmAe2_5 = data.PM_SP_UG_2_5;
        *pmAe1_0 = data.PM_SP_UG_10_0;
        result = true;
      } else {
        toggle_reset();
      }
      toggle_set(true);
      } else if (pms_tick_count >= PMS_READ_INTERVAL) {
      toggle_set(false);
      pms_tick_count = 0;
    }
  }
  return result;
}

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme; // I2C
VOCGasIndexAlgorithm voc_algorithm;
NOxGasIndexAlgorithm nox_algorithm;


String val1;
String val2;
String val3;

uint16_t conditioning_s = 10;
bool status;
char errorMessage[64];
// time in seconds needed for NOx conditioning


//pms5003


//SGP41
    uint16_t error;
    float humidity = 0;     // %RH
    float temperature = 0;  // degreeC
    float pressure = 0;     //hPa
    float appaltitude = 0;  //m
    uint16_t srawVoc = 0;
    uint16_t srawNox = 0;
    uint16_t defaultCompenstaionRh = 0x8000;  // in ticks as defined by SGP41
    uint16_t defaultCompenstaionT = 0x6666;   // in ticks as defined by SGP41
    uint16_t compensationRh = 0;              // in ticks as defined by SGP41
    uint16_t compensationT = 0;               // in ticks as defined by SGP41
    uint16_t pmSp1_0 = 0;
    uint16_t pmSp2_5 = 0;
    uint16_t pmSp10_0 = 0;
    uint16_t pmAe1_0 = 0;
    uint16_t pmAe2_5 = 0;
    uint16_t pmAe10_0 = 0;

   
 
String readPMSPM1P0S() {
      if (pms5003_read(&pmSp1_0, &pmSp2_5, &pmSp10_0, &pmAe1_0, &pmAe2_5, &pmAe10_0)) {
    return String(pmSp1_0);
  }
}

String readPMSPM2P5S() {
    if (pms5003_read(&pmSp1_0, &pmSp2_5, &pmSp10_0, &pmAe1_0, &pmAe2_5, &pmAe10_0)) {
    return String(pmSp2_5);
  }
}


String readPMSPM10P0S() {
if (pms5003_read(&pmSp1_0, &pmSp2_5, &pmSp10_0, &pmAe1_0, &pmAe2_5, &pmAe10_0)) {
    return String(pmSp10_0);
  }
}

 
    
   
//BME280 sensor data

String readTempBME() {
  return String(bme.readTemperature());


}
String readAmbientHumidity() {
   return String(bme.readHumidity());


}
String readPressureBME() {
  
    return String(bme.readPressure() / 100.0F);
}

String readAltitudeBME() {

    return String(bme.readAltitude(SEALEVELPRESSURE_HPA));
}


String readVocIndex() {
  compensationT = static_cast<uint16_t>((bme.readTemperature() + 45) * 65535 / 175);
  compensationRh = static_cast<uint16_t>(bme.readHumidity() * 65535 / 100);
    if (conditioning_s > 0) {
        // During NOx conditioning (10s) SRAW NOx will remain 0
        error =
            sgp41.executeConditioning(compensationRh, compensationT, srawVoc);
        conditioning_s--;
    } else {
        error = sgp41.measureRawSignals(compensationRh, compensationT, srawVoc,
                                        srawNox);
    }
   
        int32_t voc_index = voc_algorithm.process(srawVoc);
        return String(voc_index);
    
}
String readNoxIndex() {
   compensationT = static_cast<uint16_t>((bme.readTemperature() + 45) * 65535 / 175);
  compensationRh = static_cast<uint16_t>(bme.readHumidity() * 65535 / 100);
     if (conditioning_s > 0) {
        // During NOx conditioning (10s) SRAW NOx will remain 0
        error =
            sgp41.executeConditioning(compensationRh, compensationT, srawVoc);
        conditioning_s--;
    } else {
        error = sgp41.measureRawSignals(compensationRh, compensationT, srawVoc,
                                        srawNox);
    }
   
        int32_t nox_index = nox_algorithm.process(srawNox);
        return String(nox_index);
    
  
}


//PMS5003


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
  <h2>ESP32 Outdoor Air Quality Server</h2>
  <p>
    <span class="dht-labels">Ambient Temperature (BME280) </span>
    <span id="ambienttemperature">%AMBIENTTEMPERATUREBME%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <span class="dht-labels">Pressure</span>
    <span id="pressure">%PRESSUREBME%</span>
    <sup class="units">hPa</sup>
  </p>
  <p>
    <span class="dht-labels">Ambient Humidity</span>
    <span id="ambienthumidity">%AMBIENTHUMIDITY%</span>
    <sup class="units">&percnt;</sup>
  </p>
  <p>
    <span class="dht-labels">Approx. Altitude</span>
    <span id="altitude">%ALTITUDEBME%</span>
    <sup class="units">m</sup>
  </p>
  <p>
    <span class="dht-labels">PM 1.0</span>
    <span id="PMSpm1p0S">%PMSPM1P0S%</span>
    <sup class="units">ug/m3</sup>
  </p>
   <p>
    <span class="dht-labels">PM 2.5</span>
    <span id="PMSpm2p5S">%PMSPM2P5S%</span>
    <sup class="units">ug/m3</sup>
  </p>

  <p>
    <span class="dht-labels">PM 10.0</span>
    <span id="PMSpm10p0S">%PMSPM10P0S%</span>
    <sup class="units">ug/m3</sup>
  </p>

  <p>
    <span class="dht-labels">Voc Index</span>
    <span id="vocindex">%VOCINDEX%</span>
    <sup class="units">-</sup>
  </p>
  <p>
    <span class="dht-labels">Nox Index</span>
    <span id="noxindex">%NOXINDEX%</span>
    <sup class="units">-</sup>
  </p>
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
      document.getElementById("pressure").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/pressure", true);
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
      document.getElementById("altitude").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/altitude", true);
  xhttp.send();
}, 5000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("PMSpm1p0S").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/PMSpm1p0S", true);
  xhttp.send();
}, 5000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("PMSpm2p5S").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/PMSpm2p5S", true);
  xhttp.send();
}, 5000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("PMSpm10p0S").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/PMSpm10p0S", true);
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
}, 1000 ) ;

setInterval(function ( ) {i
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("noxindex").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/noxindex", true);
  xhttp.send();
}, 1000 ) ;
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
  if(var == "AMBIENTTEMPERATUREBME"){
    return readTempBME();
  }
  else if(var == "PRESSUREBME") {
    return readPressureBME();
  }
  else if(var == "AMBIENTHUMIDITY"){
    return readAmbientHumidity();
  }
  else if(var == "ALTITUDEBME") {
    return readAltitudeBME();
  }
  else if(var == "PMSPM1P0S"){
    return readPMSPM1P0S();
  }
  else if(var == "PMSPM2P5S"){
    return readPMSPM2P5S();
  }
  else if(var == "PMSPM10P0S"){
    return readPMSPM10P0S();
  }
else if(var == "VOCINDEX"){
    return readVocIndex();
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
 printAndBuffer("If not connected WiFi network, follow these steps:");
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
  printAndBuffer("ESP32 IP as soft AP: ");
  printAndBuffer(WiFi.softAPIP().toString());

  printAndBuffer("Local network server:");
printAndBuffer("http://",false);
printAndBuffer(WiFi.localIP().toString());
  //start sensors
Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

 //bme280
  status = bme.begin(0x76);  
  if (!status) {
    printAndBuffer("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }


  Wire.begin();

  sgp41.begin(Wire);

    int32_t index_offset;
    int32_t learning_time_offset_hours;
    int32_t learning_time_gain_hours;
    int32_t gating_max_duration_minutes;
    int32_t std_initial;
    int32_t gain_factor;

voc_algorithm.get_tuning_parameters(
        index_offset, learning_time_offset_hours, learning_time_gain_hours,
        gating_max_duration_minutes, std_initial, gain_factor);
nox_algorithm.get_tuning_parameters(
        index_offset, learning_time_offset_hours, learning_time_gain_hours,
        gating_max_duration_minutes, std_initial, gain_factor);

/*
  sen5x.begin(Wire);

  sen5x.deviceReset();

  sen5x.setTemperatureOffsetSimple(0.00);

  sen5x.startMeasurement();

  delay(1000);

  scd4x.begin(Wire);

  scd4x.startPeriodicMeasurement();
  */
 

 
  // Route for web page
server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
        request->send_P(200, "text/html", index_html, processor);
        return;
    }
});

server.on("/ambienttemperature", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
        request->send_P(200, "text/plain", readTempBME().c_str());
        return;
    }
});
server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
        request->send_P(200, "text/plain", readPressureBME().c_str());
        return;
    }
});
server.on("/ambienthumidity", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
        request->send_P(200, "text/plain", readAmbientHumidity().c_str());
        return;
    }
});
server.on("/altitude", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
        request->send_P(200, "text/plain", readAltitudeBME().c_str());
        return;
    }
});

server.on("/PMSpm1p0S", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
        request->send_P(200, "text/plain", readPMSPM1P0S().c_str());
        return;
    }
});

server.on("/PMSpm2p5S", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
        request->send_P(200, "text/plain", readPMSPM2P5S().c_str());
        return;
    }
});

server.on("/PMSpm10p0S", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
        request->send_P(200, "text/plain", readPMSPM10P0S().c_str());
        return;
    }
});

server.on("/vocindex", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
        request->send_P(200, "text/plain", readVocIndex().c_str());
        return;
    }
});

server.on("/noxindex", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
        request->send_P(200, "text/plain", readNoxIndex().c_str());
        return;
    }
});

server.on("/serial", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
        request->send_P(200, "text/plain", serialBuffer.c_str());
        return;
    }
});

  
  // Start server
  server.begin();

}

void loop(){
 delay(1000);

String noxIndex = readNoxIndex();
//Serial.println(noxIndex);

}
