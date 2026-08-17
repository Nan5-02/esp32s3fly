#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "drv_wifi.h"
#include "drv_websoket.h"
#include "app_wifi.h"

static const char *TAG = "APP_WIFI";

void App_Wifi_Init(void)
{
    ESP_LOGI(TAG, "Initializing WiFi...");
    Drv_Wifi_Init();
}

void App_Wifi_Tick(void)
{
    static uint32_t last_broadcast_time = 0;
    uint32_t current_time = pdTICKS_TO_MS(xTaskGetTickCount());
    if (current_time - last_broadcast_time >= 100) {
        last_broadcast_time = current_time;
        Drv_Websocket_Broadcast_Telemetry(get_webserver_handle());
    }
}