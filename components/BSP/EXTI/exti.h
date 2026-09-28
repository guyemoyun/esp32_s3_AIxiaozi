#ifndef __EXTI_H__
#define __EXTI_H__

#include "driver/gpio.h"
#include <stdbool.h>

// 宏定义：KEY1 和 KEY2 所连接的 GPIO 引脚号（根据实际硬件修改）
#define KEY1_PIN GPIO_NUM_0
#define KEY2_PIN GPIO_NUM_1

// 全局标志位：LED 呼吸灯使能标志（供 pwm.c 读取，由 exti.c 按键任务翻转）
// true = 呼吸灯运行，false = LED 熄灭（ledc_stop 强制高电平）
extern bool g_led_breathing_enabled;

void exti_key_init(void);

#endif // __EXTI_H__