#include "api.h"

/**
 * @brief Sets up the API.
 *
 * This function initializes the API and performs any necessary setup tasks.
 *
 * @return esp_err_t Returns ESP_OK if the setup is successful, otherwise an error code.
 */
esp_err_t setupApi() {
    ESP_LOGI(__func__, "Setting up API...");
    // GET LED state
    server.on("/api/led", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        DynamicJsonDocument jsonBuffer(1024);
        JsonObject json = jsonBuffer.to<JsonObject>();
        json["color"] = colorToString(getLedColor());
        json["brightness"] = getLedBrightness();
        serializeJson(json, *response);
        request->send(response); 
        });

    // POST LED state
    server.on("/api/led", HTTP_POST, [](AsyncWebServerRequest *request)
              {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        DynamicJsonDocument jsonBuffer(1024);
        JsonObject json = jsonBuffer.to<JsonObject>();
        ESP_LOGI(__func__, "POST %s", request->url().c_str());
        if(request->hasParam("color") || request->hasParam("brightness"))
        {
            led_state_t state;
            if (request->hasParam("color"))
            {
                state.color = stringToColor(request->getParam("color")->value());
                json["color"] = request->getParam("color")->value();
            } else {
                state.color = getLedColor();
            }
            if (request->hasParam("brightness"))
            {
                state.brightness = request->getParam("brightness")->value().toInt();
                json["brightness"] = request->getParam("brightness")->value();
            } else {
                state.brightness = getLedBrightness();
            }
            setLed(state);
            serializeJson(json, *response);
            request->send(response);
        }
        else
        {
            json["error"] = "missing parameters";
            serializeJson(json, *response);
            request->send(response);
        }});
    ESP_LOGI(__func__, "API setup complete.");
    return ESP_OK;
}