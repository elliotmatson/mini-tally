#include "ui.h"

AsyncWebServer server(80);

ESPDash dashboard(&server);
Tab *ledTab = new Tab(&dashboard, "Manual Control");
Card *brightness = new Card(&dashboard, SLIDER_CARD, "Brightness", "%", 0, 255);
Card *red = new Card(&dashboard, SLIDER_CARD, "Red", "", 0, 255);
Card *green = new Card(&dashboard, SLIDER_CARD, "Green", "", 0, 255);
Card *blue = new Card(&dashboard, SLIDER_CARD, "Blue", "", 0, 255);

/**
 * @brief Sets up ESP-Dash
 *
 * This function initializes and configures the dashboard for the application.
 * It should be called once during the setup phase of the program.
 */
void setupDashboard()
{
    ESP_LOGI(__func__, "Setting up dashboard...");
    dashboard.setTitle("Mini Tally");
    // Tab *restApiTab = new Tab(&dashboard, "REST API");
    // Card *restApi = new Card(&dashboard, BUTTON_CARD, "Enable REST API");
    // restApi->setTab(restApiTab);

    brightness->setTab(ledTab);
    red->setTab(ledTab);
    green->setTab(ledTab);
    blue->setTab(ledTab);

    brightness->update(255);
    red->update(0);
    green->update(0);
    blue->update(0);
    brightness->attachCallback([&](int value)
                               {
    setLedBrightness(value); });

    red->attachCallback([&](int value)
                        {
    setLedColor(value, (getLedColor() >> 8) & 0xFF, (getLedColor() >> 0) & 0xFF);});

    green->attachCallback([&](int value)
                          {
    setLedColor((getLedColor() >> 16) & 0xFF, value, (getLedColor() >> 0) & 0xFF);});

    blue->attachCallback([&](int value)
                         {
    setLedColor((getLedColor() >> 16) & 0xFF, (getLedColor() >> 8) & 0xFF, value);});
    registerLedUpdateCallback(updateDashboard);
    ESP_LOGI(__func__, "Dashboard setup complete.");
}

/**
 * @brief Updates the dashboard with the specified RGB color and brightness.
 *
 * @param state The LED state to update the dashboard with.
 */
void updateDashboard(led_state_t state)
{
    int _r = (state.color >> 16) & 0xFF;
    int _g = (state.color >> 8) & 0xFF;
    int _b = (state.color >> 0) & 0xFF;
    ESP_LOGI(__func__, "Updating dashboard: r=%i, g=%i, b=%i, br=%i", _r, _g, _b, state.brightness);
    red->update(_r);
    green->update(_g);
    blue->update(_b);
    brightness->update(state.brightness);
    dashboard.sendUpdates();
}