#ifndef API_H
#define API_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>

#include "helpers.h"

//extern AsyncWebServer server;

esp_err_t setupApi();


#endif //API_H