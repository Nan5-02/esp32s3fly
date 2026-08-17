#include "sta_wifi.h"
#include "app_wifi.h"
#include "rtos_msg.h"
#include "proj_cfg.h"

static const char *TAG = "STA_WIFI";

void Sta_Wifi_Init(void)
{
    ESP_LOGI(TAG, "Initializing STA WiFi...");
    App_Wifi_Init();
}

void wifi_task(void *pvParameters)
{
    IO_MSG_t msg;
    uint32_t last_tick_ms = Systime_Get_MS(0);

    for (;;) {
        if(pdTRUE == Rtos_msg_receive(WIFI_TASK, &msg, pdMS_TO_TICKS(2))){
            switch(msg.type) {
            }
        }
        if (Systime_Get_MS(last_tick_ms) >= 3) {
            App_Wifi_Tick();
            last_tick_ms = Systime_Get_MS(0);
        }
    }
}