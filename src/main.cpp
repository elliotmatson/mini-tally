#include <Arduino.h>

#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ESPDashPro.h>

#include <Adafruit_NeoPixel.h>

#include "secrets.h"

#define LED_PIN 16

Adafruit_NeoPixel led = Adafruit_NeoPixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);

AsyncWebServer server(80);
ESPDash dashboard(&server);
Tab *ledTab = new Tab(&dashboard, "Manual Control");
Card *brightness = new Card(&dashboard, SLIDER_CARD, "Brightness", "%", 0, 255);
Card *red = new Card(&dashboard, SLIDER_CARD, "Red", "", 0, 255);
Card *green = new Card(&dashboard, SLIDER_CARD, "Green", "", 0, 255);
Card *blue = new Card(&dashboard, SLIDER_CARD, "Blue", "", 0, 255);

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ESP-NOW LED control packet structure
typedef struct __attribute__((packed)) {
  uint32_t color;
  uint8_t brightness;
} led_packet_t;

// Enum of colors and their hex values
enum colors
{
  RED = 0xFF0000,
  ORANGE = 0xFFA500,
  YELLOW = 0xFFFF00,
  GREEN = 0x00FF00,
  CYAN = 0x00FFFF,
  BLUE = 0x0000FF,
  PURPLE = 0x8000FF,
  MAGENTA = 0xFF00FF,
  WHITE = 0xFFFFFF,
  BLACK = 0x000000
};

//void sendLedPacket(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness);
void updateDashboard(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _brightness);

/**
 * @brief Sets the color and brightness of the LED.
 * 
 * @param color The color value to set (in RGB format).
 * @param brightness The brightness level of the LED (0-255).
 */
void setLedColor(uint32_t color)
{
  //
  ESP_LOGI(__func__, "Setting LED color: color=%i, br=%i", color, brightness);
  led.setPixelColor(0, color);
  led.show();
  //sendLedPacket((color >> 0) & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, brightness);
  updateDashboard((color >> 16) & 0xFF, (color >> 8) & 0xFF, (color >> 0) & 0xFF, led.getBrightness());
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
 * @brief Sets the brightness of the LED.
 * 
 * @param brightness The brightness value to set (0-255).
 */
void setLedBrightness(uint8_t brightness)
{
  led.setBrightness(brightness);
  led.show();
  //sendLedPacket((led.getPixelColor(0) >> 0) & 0xFF, (led.getPixelColor(0) >> 8) & 0xFF, (led.getPixelColor(0) >> 16) & 0xFF, brightness);
  updateDashboard((led.getPixelColor(0) >> 16) & 0xFF, (led.getPixelColor(0) >> 8) & 0xFF, (led.getPixelColor(0) >> 0) & 0xFF, brightness);
}

/**
 * @brief Callback function for receiving data.
 * 
 * This function is called when data is received.
 * 
 * @param mac_addr The MAC address of the device sending the data.
 * @param data The data received.
 * @param data_len The length of the data received.
 */
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
  ESP_LOGI(__func__, "Data received from %02x:%02x:%02x:%02x:%02x:%02x\nr=%i, g=%i, b=%i, br=%i", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5], (data[0] >> 0) & 0xFF, (data[0] >> 8) & 0xFF, (data[0] >> 16) & 0xFF, data[1]);
  led_packet_t *packet = (led_packet_t *)data;
  led.setPixelColor(0, packet->color);
  led.setBrightness(packet->brightness);
  led.show();
}

/**
 * @brief Callback function called when data is sent using ESP-NOW.
 * 
 * @param mac_addr Pointer to the MAC address of the recipient device.
 * @param status The status of the data send operation.
 */
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  ESP_LOGI(__func__, "Data sent to %02x:%02x:%02x:%02x:%02x:%02x\nStatus: %s", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5], esp_err_to_name(status));
}

/**
 * @brief Sends the current LED state to all devices.
 * 
 * This function sends the current LED state to all devices using ESP-NOW.
 */
void sendLedPacket(uint32_t color, uint8_t brightness)
{
  led_packet_t packet;
  packet.color = color;
  packet.brightness = brightness;
  esp_err_t err = esp_now_send(broadcastAddress, (uint8_t *)&packet, sizeof(packet));
  ESP_LOGI(__func__, "Sending LED packet: %s", esp_err_to_name(err));
}

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
  //Tab *restApiTab = new Tab(&dashboard, "REST API");
  //Card *restApi = new Card(&dashboard, BUTTON_CARD, "Enable REST API");
  //restApi->setTab(restApiTab);

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
    setLedBrightness(value);
    brightness->update(value);
    dashboard.sendUpdates(); });

  red->attachCallback([&](int value)
                      {
    setLedColor(value, (led.getPixelColor(0) >> 8) & 0xFF, (led.getPixelColor(0) >> 0) & 0xFF);
    red->update(value);
    dashboard.sendUpdates(); });

  green->attachCallback([&](int value)
                        {
    setLedColor((led.getPixelColor(0) >> 16) & 0xFF, value, (led.getPixelColor(0) >> 0) & 0xFF);
    green->update(value);
    dashboard.sendUpdates(); });

  blue->attachCallback([&](int value)
                       {
    setLedColor((led.getPixelColor(0) >> 16) & 0xFF, (led.getPixelColor(0) >> 8) & 0xFF, value);
    blue->update(value);
    dashboard.sendUpdates(); });

  dashboard.sendUpdates();
}

/**
 * @brief Updates the dashboard with the specified RGB color and brightness.
 * 
 * @param _r The red component of the RGB color.
 * @param _g The green component of the RGB color.
 * @param _b The blue component of the RGB color.
 * @param _brightness The brightness level of the color.
 */
void updateDashboard(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _brightness)
{
  ESP_LOGI(__func__, "Updating dashboard: r=%i, g=%i, b=%i, br=%i", _r, _g, _b, _brightness);
  red->update(_r);
  green->update(_g);
  blue->update(_b);
  brightness->update(_brightness);
  dashboard.sendUpdates();
}

uint32_t stringToColor(String color)
{
}

/**
 * @brief Initializes the REST API.
 * 
 * This function sets up the necessary configurations and resources for the API to function.
 */
void setupAPI()
{
  // GET LED state
  server.on("/api/led", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument jsonBuffer(1024);
    JsonObject json = jsonBuffer.to<JsonObject>();
    json["color"] = String(led.getPixelColor(0), HEX);
    json["brightness"] = led.getBrightness();
    serializeJson(json, *response);
    request->send(response); });

  // POST LED color
  server.on("/api/led/color", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument jsonBuffer(1024);
    JsonObject json = jsonBuffer.to<JsonObject>();
    ESP_LOGI(__func__, "POST %s", request->url().c_str());
    if (request->hasParam("color"))
    {
      long status = strtol(request->getParam("color")->value().c_str(), NULL, 16);
      uint32_t color = (uint32_t)status;
      setLedColor(color);
      json["color"] = String(color, HEX);
      serializeJson(json, *response);
      request->send(response);
    }
    else
    {
      json["error"] = "missing parameters";
      serializeJson(json, *response);
      request->send(response);
    } });

  // POST LED brightness
  server.on("/api/led/brightness", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument jsonBuffer(1024);
    JsonObject json = jsonBuffer.to<JsonObject>();
    if (request->hasParam("brightness"))
    {
      int brightness = request->getParam("brightness")->value().toInt();
      uint8_t r = (led.getPixelColor(0) >> 0) & 0xFF;
      uint8_t g = (led.getPixelColor(0) >> 8) & 0xFF;
      uint8_t b = (led.getPixelColor(0) >> 16) & 0xFF;
      setLedBrightness(brightness);
      json["brightness"] = brightness;
      serializeJson(json, *response);
      request->send(response);
    }
    else
    {
      json["error"] = "missing parameters";
      serializeJson(json, *response);
      request->send(response);
    } });
}


/**
 * @brief The setup function for the sketch.
 * 
 * This function is called once when the program starts.
 */
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  delay(2000);
  ESP_LOGI(__func__, "Mini Tally starting...");
  led.begin();
  led.setPixelColor(0, 50, 0, 0);
  led.show();

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    ESP_LOGE(__func__, "WiFi Failed!");
  }
  else
  {
    ESP_LOGI(__func__, "WiFi Connected, channel %i", WiFi.channel());
    ESP_LOGI(__func__, "IP Address: %s", WiFi.localIP().toString().c_str());
  }

  /*esp_err_t err = esp_now_init();
  ESP_LOGI(__func__, "ESP-NOW init: %s", esp_err_to_name(err));
  err = esp_now_register_recv_cb(OnDataRecv);
  ESP_LOGI(__func__, "ESP-NOW register recv: %s", esp_err_to_name(err));
  err = esp_now_register_send_cb(OnDataSent);
  ESP_LOGI(__func__, "ESP-NOW register send: %s", esp_err_to_name(err));
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  err = esp_now_add_peer(&peerInfo);
  ESP_LOGI(__func__, "ESP-NOW add peer: %s", esp_err_to_name(err));*/

  server.begin();
  led.setPixelColor(0, 0, 0, 0);
  led.show();
  
  setupDashboard();
  setupAPI();
}


/**
 * @brief Arduino loop function, deleted immediately.
 */
void loop() {
  vTaskDelete(NULL);
}