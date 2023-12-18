#ifndef UI_H
#define UI_H

#include <Arduino.h>

#include "helpers.h"

#include <ESPAsyncWebServer.h>
#include <ESPDashPro.h>


extern AsyncWebServer server;

void setupDashboard();
void updateDashboard(led_state_t state);

#endif //UI_H