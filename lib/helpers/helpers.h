#ifndef HELPERS_H
#define HELPERS_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <vector.h>

#include "config.h"
#include "secrets.h"

// Patterns
enum led_pattern_t
{
    PATTERN_SOLID,
    PATTERN_BLINK,
    PATTERN_BREATHE
};

// LED State struct, packed sice it's sent over ESP-NOW
typedef struct __attribute__((packed))
{
    uint32_t color;
    uint8_t brightness;
    led_pattern_t pattern;
} led_state_t;

// LED Update Callback struct
typedef struct
{
    void (*callback)(led_state_t state);
} led_update_callback_t;

extern AsyncWebServer server;

void setupSerial();
void setupLeds();
esp_err_t setupWifi();
void setLedState(led_state_t state);
led_state_t getLedState();
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