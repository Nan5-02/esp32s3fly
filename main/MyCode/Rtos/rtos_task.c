#include "rtos_task.h"
#include "rtos_msg.h"
#include "esp_log.h"
#include "sta_wifi.h"

static const char *TAG = "RTOS_TASK";

// ============ 任务创建入口 ============
void Rtos_Task_Create(void)
{
    uint8_t ret = 0;
    // 创建消息队列
    ret = Rtos_msg_create();
    if (ret != 0) {
        ESP_LOGE(TAG, "Failed to create message queues ret=%d", ret);
        return;
    }
    ESP_LOGI(TAG, "Message queues created ret=%d", ret);

    // 创建WIFI任务
    xTaskCreate(wifi_task, "WIFITask", WIFI_STACK_SIZE, NULL, WIFI_TASK_PRIO, NULL);
    ESP_LOGI(TAG, "Task created: wifi_task");

}
