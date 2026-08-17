#ifndef __PROJ_CFG_H__
#define __PROJ_CFG_H__

#include <stdint.h>
#include <string.h>
#include <stdio.h>

// ESP-IDF 头文件
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

// 配置文件
#include "version_cfg.h"
#include "peripheral_cfg.h"
#include "pin_cfg.h"
#include "print_cfg.h"

#define PROJ_NAME           "LMT_ESP32C5"
#define FW_SERIAL_NUMBER    "ESP32C5-YY2601002"

// 默认WiFi配置（NVS无配置时使用）
#define DEFAULT_WIFI_SSID       "SY_Guest"
#define DEFAULT_WIFI_PASSWORD   "chenyi@123"

// 系统时间获取函数（ms）
// Systime_Get_MS(0) - 获取当前时间
// Systime_Get_MS(start) - 获取从start到现在的差值
static inline uint32_t Systime_Get_MS(uint32_t ms)
{
    uint32_t tick = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
    if (ms == 0) {
        return tick;
    }
    if (tick >= ms) {
        return tick - ms;
    } else {
        return (0xFFFFFFFFU - ms) + tick + 1;
    }
}

#endif /* __PROJ_CFG_H__ */
