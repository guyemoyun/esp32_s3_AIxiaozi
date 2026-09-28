/**
 * ============================================================
 *  文件名称 : pwm.c —— LEDC（PWM）呼吸灯驱动模块
 *  功能描述 : 提供 LEDC 定时器与通道初始化接口 pwm_led_init()，
 *             将 GPIO10 配置为 PWM 输出；pwm_led_test() 实现单步呼吸灯，
 *             根据 exti.c 提供的 g_led_breathing_enabled 标志位控制
 *             呼吸灯运行（PWM 渐亮渐灭）或彻底熄灭（ledc_stop 强制高电平）。
 *
 *  【引脚与参数定义】
 *    LED_PIN     = GPIO_NUM_10
 *    PWM 频率    = 100Hz
 *    分辨率      = 10 位（占空比范围 0 ~ 1023）
 *    速度模式    = LEDC_LOW_SPEED_MODE
 *    定时器编号  = LEDC_TIMER_0
 *    通道编号    = LEDC_CHANNEL_0
 *
 *  【低电平点亮说明】
 *    占空比 0   = 始终低电平 = LED 最亮（100% 亮度）
 *    占空比 1023 = 始终高电平 = LED 熄灭（0% 亮度）
 *    占空比 512 = 50% 低/50% 高 = LED 半亮（50% 亮度）
 * ============================================================
 */

#include "driver/gpio.h"       // GPIO 驱动头文件，提供 gpio 的函数接口
#include "freertos/FreeRTOS.h" // FreeRTOS 内核头文件，提供任务、延时等接口
#include "freertos/task.h"     // FreeRTOS 任务头文件，提供 vTaskDelay 等任务函数
#include "exti.h"              // 外部中断头文件，提供 g_led_breathing_enabled 标志位
#include <stdio.h>             // 标准输入输出头文件，提供 printf 等函数

// 包含 ESP32 LEDC（LED Control）驱动库
// LEDC 是 ESP32 专用的 PWM 控制器，用于生成 PWM 信号
#include "driver/ledc.h"

/**
 * @brief LEDC PWM 初始化函数：配置 PWM 定时器与输出通道，使 GPIO10 输出 PWM 信号
 * @note 初始占空比设为 1023（100%高电平），确保上电时 LED 处于熄灭状态；
 *       后续由 pwm_led_test() 根据 g_led_breathing_enabled 标志位控制呼吸灯开关，
 *       关闭时调用 ledc_stop() 强制引脚高电平彻底灭灯，开启时恢复 PWM 呼吸效果。
 */
void pwm_led_init(void)
{
    // 定义 LEDC 定时器配置结构体，PWM 输出频率 100Hz
    ledc_timer_config_t ledc_timer = {
        // 设置定时器速度模式：低速模式
        .speed_mode = LEDC_LOW_SPEED_MODE,

        // 设置定时器编号：定时器0
        .timer_num = LEDC_TIMER_0,

        // 设置PWM分辨率：10位（0-1023）
        // 分辨率越高，占空比调节越精细
        .duty_resolution = LEDC_TIMER_10_BIT,

        // 设置PWM频率：100Hz
        // 频率越高，PWM周期越短
        .freq_hz = 100,

        // 设置定时器分频器：自动计算
        // ESP32会根据频率和分辨率自动计算分频器值
        .clk_cfg = LEDC_AUTO_CLK,
    };
    // 将定时器配置应用到 LEDC 硬件，使定时器生效
    ledc_timer_config(&ledc_timer);

    // 定义 LEDC 通道配置结构体
    ledc_channel_config_t ledc_channel = {
        // 设置通道速度模式：低速模式
        .speed_mode = LEDC_LOW_SPEED_MODE,

        // 设置通道编号：通道0
        .channel = LEDC_CHANNEL_0,

        // 设置定时器编号：定时器0
        // 通道必须关联到一个定时器
        .timer_sel = LEDC_TIMER_0,

        // 设置GPIO引脚：GPIO10
        // PWM信号将从该引脚输出
        .gpio_num = GPIO_NUM_10,

        // 设置初始占空比：1023（100%占空比 = 高电平 = LED 熄灭）
        // 占空比范围：0-1023（基于10位分辨率，对应0-100%占空比）
        .duty = 1023,

        // 设置初始输出电平：低电平
        .hpoint = 0,
    };

    // 将通道配置应用到 LEDC 硬件，PWM 信号从 GPIO10 输出
    ledc_channel_config(&ledc_channel);
}

// 呼吸灯静态变量：当前占空比和变化方向
// 占空比范围 0~1023：0 = LED最亮（100%亮度），1023 = LED熄灭（0%亮度）
// 初始状态：占空比 1023（0%亮度），方向 -1（向 100% 亮度渐变）
static int32_t s_breath_duty = 1023;
static int8_t  s_breath_dir   = -1;  // -1 = 占空比递减（渐亮，0%→100%），+1 = 递增（渐灭，100%→0%）

/**
 * @brief PWM 呼吸灯步进函数：每次调用执行一步呼吸灯效果
 * @note 由 main.c 主循环每 50ms 调用一次，根据 g_led_breathing_enabled 标志位：
 *       - true： 执行呼吸灯渐亮/渐灭步进（占空比 1023→0→1023 循环，步长 10）
 *       - false：调用 ledc_stop() 停止 PWM，引脚强制高电平，LED 彻底熄灭无余亮
 *       每步占空比变化 10，配合 main.c 中 50ms 延时 ≈ 约 10 秒一个完整呼吸周期
 */
void pwm_led_test(void)
{
    // 判断呼吸灯是否使能（标志位由 exti.c 按键中断控制）
    if (!g_led_breathing_enabled)
    {
        // 呼吸灯关闭：停止 PWM 输出，引脚强制高电平（LED 彻底熄灭，无余亮）
        ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 1);  // idle_level=1 = 高电平
        s_breath_duty = 1023;
        return;
    }

    // 呼吸灯开启：更新占空比到当前值
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, s_breath_duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    // 步进占空比：每次变化 10
    s_breath_duty += s_breath_dir * 10;

    // 到达边界时反转方向
    if (s_breath_duty >= 1023)
    {
        s_breath_duty = 1023;
        s_breath_dir = -1;      // 到达 0% 亮度，反转：开始渐亮（0%→100%）
    }
    else if (s_breath_duty <= 0)
    {
        s_breath_duty = 0;
        s_breath_dir = 1;       // 到达 100% 亮度，反转：开始渐灭（100%→0%）
    }
}