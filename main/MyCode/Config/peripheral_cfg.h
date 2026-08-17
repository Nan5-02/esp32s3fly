#ifndef __PERIPHERAL_CFG_H__
#define __PERIPHERAL_CFG_H__

// 外设使能开关
#define PERIPHERAL_UART0_ENABLE     (1)
/* BS21测距模块已移至STM32管理，ESP32C5不再需要UART1/UART2直连BS21 */
#define PERIPHERAL_UART1_ENABLE     (0)
#define PERIPHERAL_UART2_ENABLE     (0)

// WiFi使能开关
#define PERIPHERAL_WIFI_ENABLE      (1)

// BLE使能开关
#define PERIPHERAL_BLE_ENABLE       (1)

// SPIFFS使能开关
#define PERIPHERAL_SPIFFS_ENABLE    (1)

// 音频使能开关 (ES8388)
#define PERIPHERAL_AUDIO_ENABLE     (1)

// OTA使能开关
#define PERIPHERAL_OTA_ENABLE       (1)

// 超声波使能开关
#define PERIPHERAL_ULTRASONIC_ENABLE (0)

#endif /* __PERIPHERAL_CFG_H__ */
