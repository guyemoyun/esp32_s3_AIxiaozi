
#include "driver/gpio.h"       // GPIO 驱动头文件，提供 gpio 的函数接口
#include "freertos/FreeRTOS.h" // FreeRTOS 内核头文件，提供任务、延时等接口
#include "freertos/task.h"     // FreeRTOS 任务头文件，提供 vTaskDelay 等任务函数
#include <stdio.h>             // 标准输入输出头文件，提供 printf 等函数


#define LED_PIN GPIO_NUM_10 // 宏定义：LED 所连接的 GPIO 引脚号（GPIO10）

/**
 * @brief LED 初始化函数：将 GPIO 配置为推挽输出模式，并设置初始状态为熄灭
 */
void led_init(void)
{
    // 定义 GPIO 配置结构体，设置各配置项
    gpio_config_t gpio = {
        .pin_bit_mask = 1 << LED_PIN,          // 位掩码：选中 GPIO10 引脚（bit10 置 1）
        .mode = GPIO_MODE_OUTPUT,              // 模式：推挽输出模式
        .pull_up_en = GPIO_PULLUP_DISABLE,     // 上拉：禁用内部上拉电阻
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // 下拉：禁用内部下拉电阻
        .intr_type = GPIO_INTR_DISABLE,        // 中断：不使用中断（禁用 GPIO 中断）
    };
    // gpio的配置：将上述配置写入硬件寄存器，完成 GPIO 初始化
    gpio_config(&gpio);
    // 灯为熄灭状态：输出高电平（本工程中低电平点亮、高电平熄灭）
    gpio_set_level(LED_PIN, 1);
}

/**
 * @brief 应用主函数：程序入口，负责初始化外设并执行主循环逻辑
 */
void app_main(void)
{
    // led初始化：调用 LED 初始化函数
    led_init();

    // 串口打印测试信息（通过 UART0 输出到串口监视器）
    printf("This is led test\r\n");

    // 主循环：无限循环执行 LED 闪烁逻辑
    while (1)
    {
        // 亮灯：输出低电平，LED 点亮
        gpio_set_level(LED_PIN, 0);

        // 延时一会：阻塞延时 1000ms（亮灯持续时间）
        vTaskDelay(pdMS_TO_TICKS(1000));

        // 灭灯：输出高电平，LED 熄灭
        gpio_set_level(LED_PIN, 1);
        
        // 延时一会：阻塞延时 1000ms（灭灯持续时间）
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
/**
 * @brief 应用主函数：程序入口，负责初始化外设并执行主循环逻辑
 */
void app_main(void)
{
    int32_t duty = 0;
    
    // 初始化 LEDC PWM 定时器与输出通道（GPIO10 输出 100Hz PWM 信号）
    pwm_led_init();

    // 通过 UART0 输出调试信息，提示 LED 测试开始
    printf("This is pwm led test\n");

    // 设置目标占空比：100%，灯为熄灭状态
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 1023);

    // 更新占空比：使 ledc_set_duty 设置的值立即生效
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    // 主循环：无限循环维持程序运行，LED 保持点亮状态
    while (1)
    {
        //帮忙实现呼吸灯效果，占空比从0%~100%循环变化
        for (duty = 0; duty <= 1023; duty+=10)
        {
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        for (duty = 1023; duty >= 0; duty-=10)
        {
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        // 阻塞延时 50ms：避免空转占用 CPU
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}