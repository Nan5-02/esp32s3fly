/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "rtos_task.h"
#include "sta_main.h"

void app_main(void)
{
    //创建RTOS任务
    Rtos_Task_Create();
    //初始化状态机
    Sta_Main_Init();
}
