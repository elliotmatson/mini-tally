#include <Arduino.h>
#include <AsyncTCP.h>
#include <regex>

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

  delay(5000);
  // setup Telnet connection to Streaming Bridge
  ESP_LOGI(__func__, "Connecting to Streaming Bridge...");
  AsyncClient *client = new AsyncClient();
  IPAddress remoteIP = IPAddress(192, 168, 150, 155);
  client->onConnect([](void *arg, AsyncClient *client)
                    {
    ESP_LOGI(__func__, "Connected to Streaming Bridge");
    client->onError([](void *arg, AsyncClient *client, err_t error)
                    { ESP_LOGI(__func__, "Error: %s fromStreaming Bridge", client->errorToString(error)); },
                    NULL);
    client->onDisconnect([](void *arg, AsyncClient *client)
                         { ESP_LOGI(__func__, "Disconnected from Streaming Bridge"); },
                         NULL);
    client->onData([](void *arg, AsyncClient *client, void *data, size_t len)
                   {
      ESP_LOGI(__func__, "Received %d bytes from Streaming Bridge", len);
      std::string str((char*)data);
      std::regex re("STREAM STATE:\nStatus: (.*?)\n");
      std::smatch match;
      if (std::regex_search(str, match, re)) {
        ESP_LOGI(__func__, "Streaming Bridge Status updated: %s", match[1].str().c_str());
        if (match[1].str() == "Idle")
        {
          setLedColor(0x000044);
        }
        else if (match[1].str() == "Connecting")
        {
          setLedColor(0xFFFF00);
        } 
        else if (match[1].str() == "Streaming")
        {
          setLedColor(0xFF0000);
        }
        else if (match[1].str() == "Interrupted")
        {
          setLedColor(0xFF6600);
        }
      } },
                   NULL); },
                    NULL);
  client->connect(remoteIP, 9977);

  // setup Telnet connection to Hyperdeck
  ESP_LOGI(__func__, "Connecting to Hyperdeck...");
  AsyncClient *client2 = new AsyncClient();
  IPAddress remoteIP2 = IPAddress(192, 168, 150, 11);
  client2->onConnect([](void *arg, AsyncClient *client)
                    {
    ESP_LOGI(__func__, "Connected to Hyperdeck");
    client->onError([](void *arg, AsyncClient *client, err_t error)
                    { ESP_LOGI(__func__, "Error: %s from Hyperdeck", client->errorToString(error)); },
                    NULL);
    client->onDisconnect([](void *arg, AsyncClient *client)
                         { ESP_LOGI(__func__, "Disconnected from Hyperdeck"); },
                         NULL);
    client->onData([](void *arg, AsyncClient *client, void *data, size_t len)
                   {
      ESP_LOGI(__func__, "Received %d bytes from Hyperdeck", len);
      ESP_LOGI(__func__, "Data: \n%s", (char*)data);
      std::string str((char*)data);
      std::regex re("508 transport info:\r\nstatus: (.*?)\r\n");
      std::smatch match;
      if (std::regex_search(str, match, re)) {
        ESP_LOGI(__func__, "Hyperdeck Status updated: %s", match[1].str().c_str());
        if (match[1].str() == "stopped")
        {
          setLedColor(0x000044);
        }
        else if (match[1].str() == "play")
        {
          setLedColor(0x00FF00);
        } 
        else if (match[1].str() == "record")
        {
          setLedColor(0xFF0000);
        }
        else if (match[1].str() == "preview")
        {
          setLedColor(0x004400);
        }
      } },
                   NULL); 
    client->write("notify: transport: true\n");},
                    NULL);
  client2->connect(remoteIP2, 9993);

  delay(1000000);
}


/**
 * @brief Arduino loop function, deleted immediately.
 */
void loop() {
  vTaskDelete(NULL);
}