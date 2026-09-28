#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"


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
void led_test(void)
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