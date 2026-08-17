#ifndef __DRV_WEBSOKET_H__
#define __DRV_WEBSOKET_H__

#include "esp_http_server.h"

/**
 * @brief 注册网页 / 遥测 / WebSocket 控制端点到指定 HTTP 服务器
 * @param server HTTP 服务器句柄
 */
void Drv_Websocket_Register_Handlers(httpd_handle_t server);

/**
 * @brief 周期广播遥测状态，并执行控制超时急停（failsafe）
 * @param server HTTP 服务器句柄
 */
void Drv_Websocket_Broadcast_Telemetry(httpd_handle_t server);

#endif // __DRV_WEBSOKET_H__
