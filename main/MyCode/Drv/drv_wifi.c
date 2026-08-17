#include <string.h>

#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "drv_wifi.h"
#include "drv_websoket.h"

static const char *TAG = "DRONE_WIFI";

static void wifi_init_softap(void)
{
    /*1.初始化WiFi软AP*/
    esp_err_t ret = esp_netif_init();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_ERROR_CHECK(ret);
    }

    /*2.创建默认事件循环*/
    ret = esp_event_loop_create_default();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_ERROR_CHECK(ret);
    }

    /*3.创建默认WiFi AP网络接口*/
    esp_netif_create_default_wifi_ap();

    /*4.初始化WiFi*/
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_AP_SSID,
            .ssid_len = strlen(WIFI_AP_SSID),
            .password = WIFI_AP_PASS,
            .channel = WIFI_AP_CHANNEL,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .max_connection = WIFI_AP_MAX_CONNECT,
        },
    };

    //5.设置WiFi模式为AP模式，并配置AP参数
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi SoftAP started. SSID:%s, Channel:%d",
             WIFI_AP_SSID, WIFI_AP_CHANNEL);
}

#if 0  // HTTP/WebSocket 服务已停用（改用 UDP），以下代码保留备用
static httpd_handle_t MyServer = NULL;

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    if (httpd_start(&server, &config) == ESP_OK) {
        Drv_Websocket_Register_Handlers(server);
        return server;
    }

    ESP_LOGE(TAG, "Failed to start HTTP server");
    return NULL;
}

httpd_handle_t get_webserver_handle(void)
{
    return MyServer;
}
#endif  // HTTP/WebSocket 服务已停用

void Drv_Wifi_Init(void)
{
    /* 初始化NVS */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* 初始化WiFi软AP */
    wifi_init_softap();

    ESP_LOGI(TAG, "=== Drone WiFi Ready ===");
    ESP_LOGI(TAG, "Connect to WiFi: %s", WIFI_AP_SSID);
}
