#include <Wire.h>
#include <Arduino.h>
#include <NOxGasIndexAlgorithm.h>
#include <SensirionI2CSgp41.h>
#include <VOCGasIndexAlgorithm.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <HardwareSerial.h>

#include "PMS.h"

#define SEALEVELPRESSURE_HPA (1013.25)
#define RXD2 16 // To sensor TXD
#define TXD2 17 // To sensor RXD

SensirionI2CSgp41 sgp41;
Adafruit_BME280 bme;

VOCGasIndexAlgorithm voc_algorithm;
NOxGasIndexAlgorithm nox_algorithm;

// time in seconds needed for NOx conditioning
uint16_t conditioning_s = 10;
bool status;

String val1;
String val2;
String val3;

#define PMS_SET_PIN 26
#define PMS_RST_PIN 25
#define PMS_READ_INTERVAL 9U
#define PMS_READ_DELAY 1U

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

void setup() {
    Serial.begin(9600);
    while (!Serial) {
        delay(100);
    }
    Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
    
    Wire.begin(); 
    sgp41.begin(Wire);
    status = bme.begin(0x76);
    
    delay(1000);  // needed on some Arduino boards in order to have Serial ready

    int32_t index_offset;
    int32_t learning_time_offset_hours;
    int32_t learning_time_gain_hours;
    int32_t gating_max_duration_minutes;
    int32_t std_initial;
    int32_t gain_factor;
    voc_algorithm.get_tuning_parameters(
        index_offset, learning_time_offset_hours, learning_time_gain_hours,
        gating_max_duration_minutes, std_initial, gain_factor);

    Serial.println("\nVOC Gas Index Algorithm parameters");
    Serial.print("Index offset:\t");
    Serial.println(index_offset);
    Serial.print("Learing time offset hours:\t");
    Serial.println(learning_time_offset_hours);
    Serial.print("Learing time gain hours:\t");
    Serial.println(learning_time_gain_hours);
    Serial.print("Gating max duration minutes:\t");
    Serial.println(gating_max_duration_minutes);
    Serial.print("Std inital:\t");
    Serial.println(std_initial);
    Serial.print("Gain factor:\t");
    Serial.println(gain_factor);

    nox_algorithm.get_tuning_parameters(
        index_offset, learning_time_offset_hours, learning_time_gain_hours,
        gating_max_duration_minutes, std_initial, gain_factor);

    Serial.println("\nNOx Gas Index Algorithm parameters");
    Serial.print("Index offset:\t");
    Serial.println(index_offset);
    Serial.print("Learing time offset hours:\t");
    Serial.println(learning_time_offset_hours);
    Serial.print("Gating max duration minutes:\t");
    Serial.println(gating_max_duration_minutes);
    Serial.print("Gain factor:\t");
    Serial.println(gain_factor);
    Serial.println("");
}

void loop() {
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

    delay(1000);

    if (!status){
        Serial.print("BME280 - Error trying to execute begin");
        Serial.println("Fallback to use default values for humidity and "
                       "temperature compensation for SGP41");
        compensationRh = defaultCompenstaionRh;
        compensationT = defaultCompenstaionT;
    } else {
        humidity = bme.readHumidity();
        temperature = bme.readTemperature();
        pressure = (bme.readPressure()/100.0F);
        appaltitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
        Serial.print("Temperature:");
        Serial.print(temperature);
        Serial.print("\t");
        Serial.print("Relative Humidity:");
        Serial.println(humidity);
        Serial.print("Pressure:");
        Serial.print(pressure);
        Serial.print("\t");
        Serial.print("Approx. Altitude:");
        Serial.println(appaltitude);
        compensationT = static_cast<uint16_t>((temperature + 45) * 65535 / 175);
        compensationRh = static_cast<uint16_t>(humidity * 65535 / 100);
    }

    if (conditioning_s > 0) {
        error =
            sgp41.executeConditioning(compensationRh, compensationT, srawVoc);
        conditioning_s--;
    } else {
        error = sgp41.measureRawSignals(compensationRh, compensationT, srawVoc, srawNox);
    }

    if (error) {
        Serial.print("SGP41 - Error trying to execute measureRawSignals(): ");
    } else {
        int32_t voc_index = voc_algorithm.process(srawVoc);
        int32_t nox_index = nox_algorithm.process(srawNox);
        Serial.print("VOC Index: ");
        Serial.print(voc_index);
        Serial.print("\t");
        Serial.print("NOx Index: ");
        Serial.println(nox_index);
    }

  if (pms5003_read(&pmSp1_0, &pmSp2_5, &pmSp10_0, &pmAe1_0, &pmAe2_5, &pmAe10_0)) {
    Serial.print("pmSp1_0:");
    Serial.println(String(pmSp1_0));
    Serial.print("pmSp2_5:");
    Serial.println(String(pmSp2_5));
    Serial.print("pmSp10_0:"); 
    Serial.println(String(pmSp10_0));
    Serial.print("pmAe1_0:");
    Serial.println(String(pmAe1_0));
    Serial.print("pmAe2_5:");
    Serial.println(String(pmAe2_5));
    Serial.print("pmAe10_0:");
    Serial.println(String(pmAe10_0));
  }

  delay(500);
}
