#include <Arduino.h>

#include "PMS.h"

 

#define PMS_SET_PIN 26
#define PMS_RST_PIN 25
#define PMS_READ_INTERVAL 9
#define PMS_READ_DELAY 1

uint8_t pms_tick_count = PMS_READ_INTERVAL;

PMS pms(Serial2);
PMS::DATA data;

 

/***************************************************/

static void setup_pins(void) {
  pinMode(PMS_RST_PIN, OUTPUT);
  digitalWrite(PMS_RST_PIN, HIGH);
  pinMode(PMS_SET_PIN, OUTPUT);
  digitalWrite(PMS_SET_PIN, HIGH);
}

 

/***************************************************/

static void toggle_set(bool sleep) {

  if (sleep) {

    digitalWrite(PMS_SET_PIN, LOW);

  } else {

    digitalWrite(PMS_SET_PIN, HIGH);

  }

  delay(500U);

}

 

/***************************************************/

static void toggle_reset(void) {

  digitalWrite(PMS_RST_PIN, LOW);

  delay(500U);

  digitalWrite(PMS_RST_PIN, HIGH);

  delay(500U);

}

 

/***************************************************/

bool pms5003_init(void) {

  setup_pins();

  Serial2.begin(9600, SERIAL_8N1, 16U, 17U);

  delay(1000U);

  pms_tick_count = PMS_READ_INTERVAL;

  return true;

}

 

/***************************************************/

bool pms5003_read(uint16_t *pmSp1_0, uint16_t *pmSp2_5, uint16_t *pmSp10_0,

                  uint16_t *pmAe1_0, uint16_t *pmAe2_5, uint16_t *pmAe10_0) {

  bool result = false;

 

  if ((NULL == pmSp1_0) || (NULL == pmSp2_5) || (NULL == pmSp10_0)

  || (NULL == pmAe1_0) || (NULL == pmAe2_5) || (NULL == pmAe10_0)) {

    result = false;

  }

  else {

    pms_tick_count++;

    if (pms_tick_count == PMS_READ_DELAY) {

      PMS::DATA data;

      while (Serial2.available())

      {

        Serial2.read();

      }

 

      if (pms.readUntil(data, 2U*PMS::SINGLE_RESPONSE_TIME))

      {

        *pmSp1_0 = data.PM_AE_UG_1_0;

        *pmSp2_5 = data.PM_AE_UG_2_5;

        *pmSp10_0 = data.PM_AE_UG_10_0;

        *pmAe1_0 = data.PM_SP_UG_1_0;

        *pmAe2_5 = data.PM_SP_UG_2_5;

        *pmAe1_0 = data.PM_SP_UG_10_0;

        result = true;

      }

      else

      {

        toggle_reset();

      }

      toggle_set(true);

    } else if (pms_tick_count >= PMS_READ_INTERVAL) {

      toggle_set(false);

      pms_tick_count = 0U;

    }

  }

 

  return result;

}

void setup() {
  Serial.begin(115200);
  Serial.print("HELLO");
  delay(1000);
}

void loop() {
  uint16_t pmSp1_0 = 0;
  uint16_t pmSp2_5 = 0;
  uint16_t pmSp10_0 = 0;
  uint16_t pmAe1_0 = 0;
  uint16_t pmAe2_5 = 0;
  uint16_t pmAe10_0 = 0;

  if (pms5003_read(&pmSp1_0, &pmSp2_5, &pmSp10_0, &pmAe1_0, &pmAe2_5, &pmAe10_0)) {
    Serial.print(String(pmSp1_0));
    Serial.print(String(pmSp2_5));
    Serial.print(String(pmSp10_0));
    Serial.print(String(pmAe1_0));
    Serial.print(String(pmAe2_5));
    Serial.print(String(pmAe10_0));
  }

  delay(60000);
}
