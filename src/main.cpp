#include <Arduino.h>

#include "helpers.h"
#include "api.h"
#include "ui.h"
#include "esp-now-tx.h"

#include "secrets.h"


void setup() {
  // put your setup code here, to run once:
  setupSerial();
  delay(2000);
  ESP_LOGI(__func__, "Mini Tally starting...");
  setupLeds();
  setupWifi();

  //setupEspNow();
  setupApi();
  setupDashboard();
  updateCallbacks(getLedState());
  ESP_LOGI(__func__, "Mini Tally setup complete.");
}


/**
 * @brief Arduino loop function, deleted immediately.
 */
void loop() {
  vTaskDelete(NULL);
}