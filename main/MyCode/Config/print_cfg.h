#ifndef __PRINT_CFG_H
#define __PRINT_CFG_H

/*============================================================
 * 模块日志等级配置
 * 
 * ESP_LOG_NONE    (0) - 关闭所有日志
 * ESP_LOG_INFO   (1) - 仅错误
 * ESP_LOG_WARN    (2) - 错误 + 警告
 * ESP_LOG_INFO    (3) - 错误 + 警告 + 信息
 * ESP_LOG_DEBUG   (4) - 错误 + 警告 + 信息 + 调试
 * ESP_LOG_VERBOSE (5) - 全部日志
 *============================================================*/

#define LOG_MODULE_MAIN      ESP_LOG_INFO
#define LOG_MODULE_BOARD     ESP_LOG_INFO
#define LOG_MODULE_UART      ESP_LOG_DEBUG
#define LOG_MODULE_RTOS      ESP_LOG_INFO
#define LOG_MODULE_WIFI      ESP_LOG_INFO
#define LOG_MODULE_BLE       ESP_LOG_INFO
#define LOG_MODULE_STORAGE   ESP_LOG_INFO
#define LOG_MODULE_BS21      ESP_LOG_INFO
#define LOG_MODULE_AUDIO     ESP_LOG_DEBUG
#define LOG_MODULE_MFG       ESP_LOG_INFO
#define LOG_MODULE_OTA       ESP_LOG_INFO
#define LOG_MODULE_MCU       ESP_LOG_INFO


#endif /* __PRINT_CFG_H */
