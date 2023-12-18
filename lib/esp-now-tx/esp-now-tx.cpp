#include "esp-now-tx.h"

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

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
    led_state_t *state = (led_state_t *)data;
    setLed(*state);
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
 * @brief Sets up the ESP-NOW communication.
 *
 * This function initializes the ESP-NOW module and prepares it for communication.
 *
 * @return esp_err_t Returns ESP_OK if the setup is successful, otherwise an error code.
 */
esp_err_t setupEspNow() {
    ESP_LOGI(__func__, "Setting up ESP-NOW...");      
    if (esp_now_init() != ESP_OK) {
        ESP_LOGE(__func__, "Error initializing ESP-NOW");
        return ESP_FAIL;
    }
    ESP_LOGI(__func__, "Registering callbacks...");
    esp_now_register_recv_cb(OnDataRecv);
    esp_now_register_send_cb(OnDataSent);
    
    ESP_LOGI(__func__, "Adding peer...");
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return ESP_FAIL;
    }

    //register update callback
    registerLedUpdateCallback(sendLedPacket);
    
    ESP_LOGI(__func__, "ESP-NOW setup complete");
    return ESP_OK;
}

void sendLedPacket(led_state_t state)
{
    esp_err_t err = esp_now_send(broadcastAddress, (uint8_t *)&state, sizeof(state));
    ESP_LOGI(__func__, "Sending LED packet: %s", esp_err_to_name(err));
}