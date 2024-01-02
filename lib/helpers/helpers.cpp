#include "helpers.h"

Adafruit_NeoPixel led = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

std::vector<void (*)(led_state_t state)> ledUpdateCallbacks;

/**
 * @brief Sets up the serial communication.
 * 
 * This function initializes the serial communication for the device.
 * It also sends ESP-IDF debug messages to USB
 * 
 * @return void
 */
void setupSerial()
{
    ESP_LOGI(__func__, "Setting up serial...");
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    ESP_LOGI(__func__, "Serial setup complete.");
}

/**
 * @brief Sets up the LEDs for the application.
 * 
 * This function initializes the LEDs and prepares them for use in the application.
 * It should be called once during the setup phase of the program.
 */
void setupLeds()
{
    ESP_LOGI(__func__, "Setting up LEDs...");
    led.begin();
    led.setBrightness(255);
    setLedColor(0x00FF00);
    led.show();
    delay(200);
    setLedColor(0x000000);
    led.show();
    ESP_LOGI(__func__, "LED setup complete.");
}

/**
 * @brief Sets up the WiFi connection.
 * 
 * This function initializes the WiFi connection for the device.
 * It should be called once during the setup phase of the program.
 * 
 * @return esp_err_t Returns ESP_OK if the setup is successful, otherwise an error code.
 */
esp_err_t setupWifi()
{
    setLedColor(0x0000FF);
    ESP_LOGI(__func__, "Setting up WiFi...");
    WiFi.mode(WIFI_AP_STA);
    ESP_LOGI(__func__, "Connecting to %s...", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    ESP_LOGI(__func__, "Waiting for WiFi...");
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        ESP_LOGE(__func__, "WiFi Failed!");
    }
    else
    {
        ESP_LOGI(__func__, "WiFi Connected, channel %i", WiFi.channel());
        ESP_LOGI(__func__, "IP Address: %s", WiFi.localIP().toString().c_str());
    }
    server.begin();
    server.onNotFound([](AsyncWebServerRequest *request)
                      {
        request->send(404, "text/plain", "Not found");
        ESP_LOGI(__func__, "404: %s", request->url().c_str());
    });
    ESP_LOGI(__func__, "WiFi setup complete.");
    setLedColor(0x000000);
    return ESP_OK;
}

/**
 * @brief Sets the color and brightness of the LED.
 *
 * @param state The LED state to set.
 */
void setLedState(led_state_t state)
{
    ESP_LOGI(__func__, "Setting LED color: color=%i, br=%i", state.color, state.brightness);
    for (int i = 0; i < NUM_LEDS; i++)
    {
        led.setPixelColor(i, state.color);
    }
    led.setBrightness(state.brightness);
    led.show();
    updateCallbacks(state);
}

/**
 * @brief Retrieves the LED state.
 * 
 * @return The LED state as a led_state_t struct.
 */
led_state_t getLedState()
{
    led_state_t state;
    state.color = getLedColor();
    state.brightness = getLedBrightness();
    return state;
}

/**
 * @brief Sets the color and brightness of the LED.
 *
 * @param color The color value to set (in RGB format).
 * @param brightness The brightness level of the LED (0-255).
 */
void setLedColor(uint32_t color)
{
    ESP_LOGI(__func__, "Setting LED color: color=%i, br=%i", color);
    for (int i = 0; i < NUM_LEDS; i++)
    {
        led.setPixelColor(i, color);
    }
    led.show();
    led_state_t state;
    state.color = color;
    state.brightness = getLedBrightness();
    updateCallbacks(state);
}

/**
 * @brief Sets the color and brightness of an LED.
 *
 * This function takes in the values for red (r), green (g), blue (b), and brightness (brightness)
 * and sets the color and brightness of an LED accordingly.
 *
 * @param r The value for the red component of the LED color (0-255).
 * @param g The value for the green component of the LED color (0-255).
 * @param b The value for the blue component of the LED color (0-255).
 */
void setLedColor(uint8_t r, uint8_t g, uint8_t b)
{
    setLedColor(r << 16 | g << 8 | b);
}

/**
 * @brief Retrieves the LED color.
 * 
 * @return The LED color as a 32-bit unsigned integer.
 */
uint32_t getLedColor()
{
    return led.getPixelColor(0);
}

/**
 * @brief Sets the brightness of the LED.
 *
 * @param brightness The brightness value to set (0-255).
 */
void setLedBrightness(uint8_t brightness)
{
    led.setBrightness(brightness);
    led.show();
    led_state_t state;
    state.color = getLedColor();
    state.brightness = brightness;
    updateCallbacks(state);
}

/**
 * @brief Retrieves the LED brightness.
 * 
 * @return The LED brightness as an 8-bit unsigned integer.
 */
uint8_t getLedBrightness()
{
    return led.getBrightness();
}

/**
 * @brief Converts a string to a color value.
 *
 * @param color The string to convert to a color value.
 * @return uint32_t The color value.
 */
uint32_t stringToColor(String color)
{   
    color.toLowerCase();
    if (color == "red")
    {
        return 0xFF0000;
    }
    else if (color == "green")
    {
        return 0x00FF00;
    }
    else if (color == "blue")
    {
        return 0x0000FF;
    }
    else if (color == "yellow")
    {
        return 0xFFFF00;
    }
    else if (color == "cyan")
    {
        return 0x00FFFF;
    }
    else if (color == "magenta")
    {
        return 0xFF00FF;
    }
    else if (color == "white")
    {
        return 0xFFFFFF;
    }
    else if (color == "black")
    {
        return 0x000000;
    }
    else
    {
        return strtol(color.c_str(), NULL, 16);
    }
}

/**
 * @brief Converts a color value to a string.
 *
 * @param color The color value to convert to a string.
 * @return String The color value as a string.
 */
String colorToString(uint32_t color)
{
    char colorString[9];
    sprintf(colorString, "#%06X", color);
    return String(colorString);
}

/**
 * @brief Registers a callback function to be called when the LED state is updated.
 * 
 * @param callback The callback function to register.
 */
void registerLedUpdateCallback(void (*callback)(led_state_t state))
{
    ESP_LOGI(__func__, "Registering LED update callback...");
    ledUpdateCallbacks.push_back(callback);
}

/**
 * @brief Calls all registered callbacks with the specified LED state.
 * 
 * @param state The LED state to pass to the callbacks.
 */
void updateCallbacks(led_state_t state)
{
    ESP_LOGI(__func__, "Calling LED update callbacks...");
    for (int i = 0; i < ledUpdateCallbacks.size(); i++)
    {
        ledUpdateCallbacks[i](state);
    }
}