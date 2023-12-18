#ifndef HELPERS_H
#define HELPERS_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <vector.h>

#include "config.h"
#include "secrets.h"

// LED State struct
typedef struct __attribute__((packed))
{
    uint32_t color;
    uint8_t brightness;
} led_state_t;

extern AsyncWebServer server;

void setupSerial();
void setupLeds();
esp_err_t setupWifi();
void setLed(led_state_t state);
led_state_t getLed();
void setLedColor(uint32_t color);
void setLedColor(uint8_t r, uint8_t g, uint8_t b);
uint32_t getLedColor();
void setLedBrightness(uint8_t brightness);
uint8_t getLedBrightness();
uint32_t stringToColor(String color);
String colorToString(uint32_t color);
void registerLedUpdateCallback(void (*callback)(led_state_t state));
void unregisterLedUpdateCallback(void (*callback)(led_state_t state));
void updateCallbacks(led_state_t state);

#endif //HELPERS_H