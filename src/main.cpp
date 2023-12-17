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
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t brightness;
} led_packet_t;

void sendLedPacket(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness);
void updateDashboard(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _brightness);

/**
 * @brief Sets the color and brightness of an LED.
 * 
 * This function takes in the values for red (r), green (g), blue (b), and brightness (brightness)
 * and sets the color and brightness of an LED accordingly.
 * 
 * @param r The value for the red component of the LED color (0-255).
 * @param g The value for the green component of the LED color (0-255).
 * @param b The value for the blue component of the LED color (0-255).
 * @param brightness The brightness level of the LED (0-255).
 */
void setLedColor(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness)
{
  led.setPixelColor(0, r, g, b);
  led.show();
  sendLedPacket(r, g, b, brightness);
  updateDashboard(r, g, b, brightness);
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
  led_packet_t *packet = (led_packet_t *)data;
  led.setPixelColor(0, packet->r, packet->g, packet->b);
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
  Serial.printf("Data sent to %02x:%02x:%02x:%02x:%02x:%02x\nStatus: %s\nr=%i, g=%i, b=%i, br=%i", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5], esp_err_to_name(status), led.getPixelColor(0) >> 0, led.getPixelColor(0) >> 8, led.getPixelColor(0) >> 16, led.getBrightness());
}

/**
 * @brief Sends the current LED state to all devices.
 * 
 * This function sends the current LED state to all devices using ESP-NOW.
 */
void sendLedPacket(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness)
{
  led_packet_t packet;
  packet.r = r;
  packet.g = g;
  packet.b = b;
  packet.brightness = brightness;
  esp_err_t err = esp_now_send(broadcastAddress, (uint8_t *)&packet, sizeof(packet));
  Serial.printf("Sending LED packet: %s\n", esp_err_to_name(err));

}

/**
 * @brief Sets up ESP-Dash
 * 
 * This function initializes and configures the dashboard for the application.
 * It should be called once during the setup phase of the program.
 */
void setupDashboard()
{
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
    setLedColor((led.getPixelColor(0) >> 0) & 0xFF, (led.getPixelColor(0) >> 8) & 0xFF, (led.getPixelColor(0) >> 16) & 0xFF, value);
    brightness->update(value);
    dashboard.sendUpdates(); });

  red->attachCallback([&](int value)
                      {
    setLedColor(value, (led.getPixelColor(0) >> 8) & 0xFF, (led.getPixelColor(0) >> 16) & 0xFF, led.getBrightness());
    red->update(value);
    dashboard.sendUpdates(); });

  green->attachCallback([&](int value)
                        {
    setLedColor((led.getPixelColor(0) >> 0) & 0xFF, value, (led.getPixelColor(0) >> 16) & 0xFF, led.getBrightness());
    green->update(value);
    dashboard.sendUpdates(); });

  blue->attachCallback([&](int value)
                       {
    setLedColor((led.getPixelColor(0) >> 0) & 0xFF, (led.getPixelColor(0) >> 8) & 0xFF, value, led.getBrightness());
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
  red->update(_r);
  green->update(_g);
  blue->update(_b);
  brightness->update(_brightness);
  dashboard.sendUpdates();
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
    json["r"] = (led.getPixelColor(0) >> 0) & 0xFF;
    json["g"] = (led.getPixelColor(0) >> 8) & 0xFF;
    json["b"] = (led.getPixelColor(0) >> 16) & 0xFF;
    json["brightness"] = led.getBrightness();
    serializeJson(json, *response);
    request->send(response); });

  // POST LED color
  server.on("/api/led/color", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument jsonBuffer(1024);
    JsonObject json = jsonBuffer.to<JsonObject>();
    Serial.printf("POST %s", request->url().c_str());
    Serial.printf("PARAMS %d", request->params());
    for (int i = 0; i < request->params(); i++)
    {
      Serial.printf("--PARAM %s\n", request->getParam(i)->name().c_str());
      Serial.printf("  VALUE %s\n", request->getParam(i)->value().c_str());
    }
    if (request->hasParam("r") && request->hasParam("g") && request->hasParam("b"))
    {
      int r = request->getParam("r")->value().toInt();
      int g = request->getParam("g")->value().toInt();
      int b = request->getParam("b")->value().toInt();
      setLedColor(r, g, b, led.getBrightness());
      json["r"] = (led.getPixelColor(0) >> 0) & 0xFF;
      json["g"] = (led.getPixelColor(0) >> 8) & 0xFF;
      json["b"] = (led.getPixelColor(0) >> 16) & 0xFF;
      json["brightness"] = led.getBrightness();
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
      setLedColor(r, g, b, brightness);
      json["r"] = r;
      json["g"] = g;
      json["b"] = b;
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
  delay(5000);
  led.begin();
  led.setPixelColor(0, 50, 0, 0);
  led.show();

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi Failed!\n");
    
  }
  else
  {
    Serial.printf("WiFi Connected, channel %i\n", WiFi.channel());
    Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
  }

  esp_err_t err = esp_now_init();
  Serial.printf("ESP-NOW init: %s\n", esp_err_to_name(err));
  err = esp_now_register_recv_cb(OnDataRecv);
  Serial.printf("ESP-NOW register recv: %s\n", esp_err_to_name(err));
  err = esp_now_register_send_cb(OnDataSent);
  Serial.printf("ESP-NOW register send: %s\n", esp_err_to_name(err));
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  err = esp_now_add_peer(&peerInfo);
  Serial.printf("ESP-NOW add peer: %s\n", esp_err_to_name(err));

  server.begin();
  led.setPixelColor(0, 0, 50, 0);
  led.show();
  delay(1000);
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