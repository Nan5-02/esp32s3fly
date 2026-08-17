#include "esp_log.h"
#include "sta_wifi.h"

static const char *TAG = "STA_MAIN";

void Sta_Main_Init(void)
{
    ESP_LOGI(TAG, "Initializing STA Main...");
    Sta_Wifi_Init();
}

