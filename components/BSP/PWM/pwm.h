#ifndef __PWM_H__
#define __PWM_H__

#include "esp_err.h"

// PWM 呼吸灯初始化：配置 LEDC 定时器与 GPIO10 通道，初始灭灯状态
esp_err_t pwm_led_init(void);
// PWM 呼吸灯步进函数：每 50ms 调用一次，根据 g_led_breathing_enabled 控制呼吸/熄灭
void pwm_led_test(void);

#endif