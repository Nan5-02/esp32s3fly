#ifndef __RTOS_TASK_H__
#define __RTOS_TASK_H__

#include "proj_cfg.h"
#include "peripheral_cfg.h"

// 任务栈大小
#define WIFI_STACK_SIZE         (4096)
#define WIFI_TASK_PRIO          (6)

void Rtos_Task_Create(void);

#endif /* __RTOS_TASK_H__ */
