#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "drv_wifi.h"
#include "drv_udp.h"
#include "app_wifi.h"

static const char *TAG = "APP_WIFI";

void App_Wifi_Init(void)
{
    ESP_LOGI(TAG, "Initializing WiFi...");
    Drv_Wifi_Init();
    Drv_Udp_Init();
}

void App_Wifi_Tick(void)
{
    static uint32_t last_tick_ms = 0;
    uint32_t current_time = pdTICKS_TO_MS(xTaskGetTickCount());
    if (current_time - last_tick_ms >= 20) {
        last_tick_ms = current_time;
        Drv_Udp_Tick();
    }
}