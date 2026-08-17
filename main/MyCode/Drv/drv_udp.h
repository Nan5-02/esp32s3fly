#ifndef __DRV_UDP_H__
#define __DRV_UDP_H__

#include <stdint.h>
#include <stdbool.h>

/* ==================== UDP 遥控协议 ====================
 * 自研遥控器 APP 通过 UDP 与 ESP32 通信。
 *
 * - ESP32 作为 UDP 服务端，监听 0.0.0.0:DRV_UDP_PORT
 * - APP 作为 UDP 客户端，向 ESP32 的 IP:DRV_UDP_PORT 发送控制包
 *
 * 数据包格式（小端，packed，共 16 字节）：
 *   偏移  大小  字段       说明
 *   0     1     magic      固定 0xAA
 *   1     1     version    协议版本，当前 0x01
 *   2     2     seq        序列号（APP 自增，用于回放/调试）
 *   4     2     roll       -100..100
 *   6     2     pitch      -100..100
 *   8     2     yaw        -100..100
 *   10    2     throttle   0..100
 *   12    1     armed      0=解锁 1=布防（上锁）
 *   13    1     reserved   保留，填 0
 *   14    2     checksum   前 14 字节累加和（uint16）
 * ====================================================== */

#define DRV_UDP_PORT           9000
#define DRV_UDP_CTRL_MAGIC     0xAA
#define DRV_UDP_CTRL_VERSION   0x01

typedef struct __attribute__((packed)) {
    uint8_t  magic;       /* 帧头 0xAA */
    uint8_t  version;     /* 协议版本 */
    uint16_t seq;         /* 序列号 */
    int16_t  roll;        /* -100..100 */
    int16_t  pitch;       /* -100..100 */
    int16_t  yaw;         /* -100..100 */
    int16_t  throttle;    /* 0..100 */
    uint8_t  armed;       /* 0/1 */
    uint8_t  reserved;    /* 保留 */
    uint16_t checksum;    /* 前 14 字节累加和 */
} udp_ctrl_packet_t;

typedef struct {
    int roll;
    int pitch;
    int yaw;
    int throttle;
    int armed;
} udp_joystick_t;

/**
 * @brief 初始化 UDP 服务端（创建 socket + 接收任务），失败时内部记录错误
 */
void Drv_Udp_Init(void);

/**
 * @brief 周期调用：检测控制超时并触发 failsafe 急停
 */
void Drv_Udp_Tick(void);

/**
 * @brief 获取最近一次摇杆控制值
 * @param out 输出结构体（不能为 NULL）
 * @return true=成功
 */
bool Drv_Udp_Get_Joystick(udp_joystick_t *out);

/**
 * @brief 链路是否存活（曾收到有效控制包且未超时）
 */
bool Drv_Udp_Is_Link_Alive(void);

#endif // __DRV_UDP_H__
