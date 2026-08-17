#ifndef __DRV_WIFI_H__
#define __DRV_WIFI_H__

#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

// ==================== WiFi 配置 ====================
#define WIFI_AP_SSID          "ESP32_Drone"      // 热点名称
#define WIFI_AP_PASS          "12345678"         // 密码（至少8位）
#define WIFI_AP_CHANNEL       6                  // WiFi信道 (1-13)
#define WIFI_AP_MAX_CONNECT   4                  // 最大连接数

void Drv_Wifi_Init(void);
void Drv_Wifi_WS_Broadcast_Telemetry(httpd_handle_t server);
httpd_handle_t get_webserver_handle(void);

#endif // __DRV_WIFI_H__