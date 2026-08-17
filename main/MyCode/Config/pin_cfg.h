#ifndef __PIN_CFG_H__
#define __PIN_CFG_H__

#include "peripheral_cfg.h"

// UART0 配置
#define UART0_PORT              (UART_NUM_0)
#define UART0_TX_PIN            (GPIO_NUM_11)
#define UART0_RX_PIN            (GPIO_NUM_12)
#define UART0_BAUD_RATE         (460800U)
#define UART0_TX_BUF_SIZE       (2048)  // 扩大以支持OTA固件传输
#define UART0_RX_BUF_SIZE       (1024)
#define UART0_EVENT_QUEUE_SIZE  (20)

// UART1 配置 (BS21已移至STM32管理，保留定义供后续复用)
#if PERIPHERAL_UART1_ENABLE
#define UART1_PORT              (UART_NUM_1)
#define UART1_TX_PIN            (GPIO_NUM_7)
#define UART1_RX_PIN            (GPIO_NUM_8)
#define UART1_BAUD_RATE         (460800U)
#define UART1_TX_BUF_SIZE       (1024)
#define UART1_RX_BUF_SIZE       (1024)
#define UART1_EVENT_QUEUE_SIZE  (20)
#endif

// UART2 配置 (LP_UART, BS21已移至STM32管理，保留定义供后续复用)
#if PERIPHERAL_UART2_ENABLE
#define UART2_PORT              (LP_UART_NUM_0)
#define UART2_TX_PIN            (GPIO_NUM_5)
#define UART2_RX_PIN            (GPIO_NUM_4)
#define UART2_BAUD_RATE         (460800U)
#define UART2_TX_BUF_SIZE       (1024)
#define UART2_RX_BUF_SIZE       (1024)
#define UART2_EVENT_QUEUE_SIZE  (20)
#endif

// UART结构体访问宏
#define GET_UART_RX(num)            Uart_##num.uart_rx
#define GET_UART_TX(num)            Uart_##num.uart_tx

//MCU_UART0
#if defined(PERIPHERAL_UART0_ENABLE) && (PERIPHERAL_UART0_ENABLE == 1)
#define PERIPHERAL_UART0            0
#define MCU_RX                      GET_UART_RX(0)
#define MCU_TX                      GET_UART_TX(0)
#endif
//MCU_UART0


/* ES8388 音频芯片引脚定义 */
#define AUDIO_I2C_NUM           (0)
#define AUDIO_I2C_SCL_PIN       (GPIO_NUM_24)  // ES8388 CCLK
#define AUDIO_I2C_SDA_PIN       (GPIO_NUM_23)  // ES8388 CDATA

#define AUDIO_I2S_NUM           (0)
#define AUDIO_I2S_MCLK_PIN      (GPIO_NUM_25)  // ES8388 MCLK
#define AUDIO_I2S_BCLK_PIN      (GPIO_NUM_28)  // ES8388 SCLK
#define AUDIO_I2S_LRCLK_PIN     (GPIO_NUM_26)  // ES8388 LRCK
#define AUDIO_I2S_DOUT_PIN      (GPIO_NUM_27)  // DSDIN: ESP32 -> ES8388 (播放)
#define AUDIO_I2S_DIN_PIN       (GPIO_NUM_9)   // ASDOUT: ES8388 -> ESP32 (录音)

/* 音频功放使能引脚 */
#define AUDIO_PA_EN_PIN         (GPIO_NUM_10)   // 功放使能，高电平有效

#endif /* __PIN_CFG_H__ */
