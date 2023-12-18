#ifndef ESP_NOW_TX_H
#define ESP_NOW_TX_H

#include <esp_now.h>
#include <esp_wifi.h>

#include "helpers.h"


void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len);
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void sendLedPacket(led_state_t state);

esp_err_t setupEspNow();

#endif //ESP_NOW_TX_H