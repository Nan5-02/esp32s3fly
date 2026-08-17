#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"

#include "drv_udp.h"

static const char *TAG = "DRV_UDP";

#define UDP_RX_BUF_SIZE    256
#define CONTROL_TIMEOUT_MS 1500
#define UDP_RX_TASK_STACK  4096
#define UDP_RX_TASK_PRIO   5

static int g_sock = -1;

static udp_joystick_t g_joystick = {0, 0, 0, 0, 0};
static uint32_t g_last_ctrl_ms = 0;
static bool g_failsafe_active = true;
static bool g_link_alive = false;

/**
 * @brief 将整数限制在指定范围内
 * @param v 待限制的整数
 * @param lo 最小值
 * @param hi 最大值
 * @return 限制后的整数
 */
static int clamp_int(int v, int lo, int hi)
{
    if (v < lo) {
        return lo;
    }
    if (v > hi) {
        return hi;
    }
    return v;
}

/**
 * @brief 计算校验和
 * @param buf 数据缓冲区
 * @param len 缓冲区长度
 * @return 校验和
 */
static uint16_t calc_checksum(const uint8_t *buf, size_t len)
{
    uint16_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += buf[i];
    }
    return sum;
}

/**
 * @brief 应用控制指令
 * @param pkt 控制数据包
 */
static void apply_control(const udp_ctrl_packet_t *pkt)
{
    g_joystick.roll     = clamp_int(pkt->roll, -100, 100);
    g_joystick.pitch    = clamp_int(pkt->pitch, -100, 100);
    g_joystick.yaw      = clamp_int(pkt->yaw, -100, 100);
    g_joystick.throttle = clamp_int(pkt->throttle, 0, 100);
    g_joystick.armed    = pkt->armed ? 1 : 0;

    g_last_ctrl_ms = pdTICKS_TO_MS(xTaskGetTickCount());
    g_failsafe_active = false;
    g_link_alive = true;

    /* 日志节流：最多 1 秒一条，避免刷屏 */
    static uint32_t last_log_ms = 0;
    uint32_t now_ms = pdTICKS_TO_MS(xTaskGetTickCount());
    if (now_ms - last_log_ms >= 1000) {
        last_log_ms = now_ms;
        ESP_LOGI(TAG, "RX JOY seq=%u roll=%d pitch=%d yaw=%d throttle=%d armed=%d",
                 (unsigned)pkt->seq, g_joystick.roll, g_joystick.pitch,
                 g_joystick.yaw, g_joystick.throttle, g_joystick.armed);
    }
}

/**
 * @brief UDP接收任务：循环接收UDP数据包并处理
 * @param arg 任务参数（未使用）
 */
static void udp_rx_task(void *arg)
{
    uint8_t buf[UDP_RX_BUF_SIZE];
    struct sockaddr_in from;
    socklen_t from_len = sizeof(from);

    while (1) {
        from_len = sizeof(from);
        int len = recvfrom(g_sock, buf, sizeof(buf), 0,
                           (struct sockaddr *)&from, &from_len);
        if (len < 0) {
            continue; /* 非致命错误，继续收包 */
        }

        if (len != (int)sizeof(udp_ctrl_packet_t)) {
            ESP_LOGW(TAG, "Drop: bad packet size %d", len);
            continue;
        }

        ESP_LOGI(TAG, "Received UDP packet from %s:%u, len=%d",
                 inet_ntoa(from.sin_addr), ntohs(from.sin_port), len);

        ESP_LOGI(TAG, "Received Data:");
        for (int i = 0; i < len; i++) {
            ESP_LOGI(TAG, "0x%02X ", buf[i]);
        }

        const udp_ctrl_packet_t *pkt = (const udp_ctrl_packet_t *)buf;
        if (pkt->magic != DRV_UDP_CTRL_MAGIC || pkt->version != DRV_UDP_CTRL_VERSION) {
            ESP_LOGW(TAG, "Drop: bad magic/version 0x%02X v%u", pkt->magic, pkt->version);
            continue;
        }

        if (calc_checksum(buf, sizeof(udp_ctrl_packet_t) - sizeof(uint16_t)) != pkt->checksum) {
            ESP_LOGW(TAG, "Drop: checksum mismatch");
            continue;
        }

        apply_control(pkt);
    }
}

/**
 * @brief 初始化UDP服务
 * @note 此函数应在系统启动时调用
 */
void Drv_Udp_Init(void)
{
    if (g_sock >= 0) {
        return; /* 已初始化 */
    }

    /*1. 创建UDP套接字 */
    g_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (g_sock < 0) {
        ESP_LOGE(TAG, "socket create failed");
        return;
    }

    /*2. 设置套接字选项：允许地址重用 */
    int opt = 1;
    setsockopt(g_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DRV_UDP_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /*3. 绑定套接字到指定端口 */
    if (bind(g_sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        ESP_LOGE(TAG, "bind port %u failed", DRV_UDP_PORT);
        close(g_sock);
        g_sock = -1;
        return;
    }

    /*4. 创建接收任务 */
    xTaskCreate(udp_rx_task, "udp_rx", UDP_RX_TASK_STACK, NULL, UDP_RX_TASK_PRIO, NULL);

    g_last_ctrl_ms = pdTICKS_TO_MS(xTaskGetTickCount());
    g_failsafe_active = true;

    ESP_LOGI(TAG, "UDP server started, listen 0.0.0.0:%u", DRV_UDP_PORT);
}

/**
 * @brief 周期调用：检测控制超时并触发 failsafe 急停
 */
void Drv_Udp_Tick(void)
{
    if (g_sock < 0) {
        return;
    }

    uint32_t now = pdTICKS_TO_MS(xTaskGetTickCount());
    if ((now - g_last_ctrl_ms) > CONTROL_TIMEOUT_MS && !g_failsafe_active) {
        g_joystick.roll = 0;
        g_joystick.pitch = 0;
        g_joystick.yaw = 0;
        g_joystick.throttle = 0;
        g_joystick.armed = 0;
        g_failsafe_active = true;
        g_link_alive = false;
        ESP_LOGW(TAG, "FAILSAFE triggered: control timeout, motors disarmed");
    }
}

/**
 * @brief 获取摇杆数据
 * @param out 输出参数，指向udp_joystick_t结构体
 * @return true表示成功，false表示失败
 */
bool Drv_Udp_Get_Joystick(udp_joystick_t *out)
{
    if (out == NULL) {
        return false;
    }
    *out = g_joystick;
    return true;
}

/**
 * @brief 检查UDP链路是否存活（曾收到有效控制包且未超时）
 * @return true表示链路存活，false表示链路不活跃
 */
bool Drv_Udp_Is_Link_Alive(void)
{
    return g_link_alive;
}
