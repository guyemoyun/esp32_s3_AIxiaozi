#include "driver/gpio.h"       // GPIO 驱动头文件，提供 gpio 的函数接口
#include "freertos/FreeRTOS.h" // FreeRTOS 内核头文件，提供任务、延时等接口
#include "freertos/queue.h"    // FreeRTOS 消息队列头文件，提供 xQueueSend 等消息队列函数
#include "freertos/task.h"     // FreeRTOS 任务头文件，提供 vTaskDelay 等任务函数
#include "exti.h"
#include <stdio.h>             // 标准输入输出头文件，提供 printf 等函数

// 全局标志位：LED 呼吸灯使能标志（供 pwm.c 读取）
// true = 呼吸灯运行，false = LED 熄灭；默认关闭，由 KEY 按键开启
bool g_led_breathing_enabled = false;

// 全局队列句柄：用于按键中断与 LED 任务之间的消息传递
static QueueHandle_t s_queue_key_handle = NULL;
// 全局变量：LED 任务句柄，可用于任务挂起/恢复/删除等管理操作
static TaskHandle_t s_task_led_handle = NULL;

// 函数声明：task_led 任务函数（消费队列消息，翻转呼吸灯开关标志位）
static void task_led(void* pvParameters);

/**
 * @brief KEY 中断服务函数：处理 KEY 中断事件
 * @param arg 中断参数，用于标识是 KEY1 还是 KEY2 中断
 * @note 该函数位于 IRAM_ATTR 内存区域，是中断服务函数（ISR）；
 *       中断服务函数必须放在 IRAM_ATTR，否则会导致段错误复位；
 *       若在 ISR 中添加 printf 打印，同样会触发段错误复位
 */
static void IRAM_ATTR key_irq_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    uint32_t key_state = 0;

    if (gpio_num == KEY1_PIN)
    {
        key_state |= 0x01;
    }
    else if (gpio_num == KEY2_PIN)
    {
        key_state |= 0x02;
    }

    // 发送消息队列：将 KEY 中断事件发送到消息队列中
    xQueueSendFromISR(s_queue_key_handle, &key_state, NULL);
}

/**
 * @brief KEY GPIO 初始化：将 KEY1/KEY2 配置为输入模式，使能内部上拉
 */
static void key_init(void)
{
    // 配置 KEY1 和 KEY2 为 GPIO 输入，启用上拉和下降沿中断
    gpio_config_t gpio = {
        .pin_bit_mask = (1ULL << KEY1_PIN) | (1ULL << KEY2_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,     // 下降沿触发中断
    };
    gpio_config(&gpio);
}

/**
 * @brief KEY 中断初始化：为 KEY1/KEY2 安装 GPIO 中断服务函数
 */
static void key_irq_init(void)
{
    // 安装 KEY1 中断服务函数
    gpio_install_isr_service(0);            // 参数 0 表示使用默认中断分配标志
    gpio_isr_handler_add(KEY1_PIN, key_irq_handler, (void*)KEY1_PIN);
    // 安装 KEY2 中断服务函数
    gpio_isr_handler_add(KEY2_PIN, key_irq_handler, (void*)KEY2_PIN);
}

/**
 * @brief KEY 外部中断初始化函数（封装入口）
 * @note 调用此函数即可完成 KEY 中断的整套初始化：
 *       创建消息队列 → 初始化 KEY GPIO → 安装中断 ISR → 创建按键任务
 *       按键任务负责翻转 g_led_breathing_enabled 全局标志位，
 *       由 pwm.c 的 pwm_led_test() 读取该标志位来控制呼吸灯开关
 */
void exti_key_init(void)
{
    // 创建消息队列：用于按键中断与 LED 任务之间的消息传递
    s_queue_key_handle = xQueueCreate(5, sizeof(uint32_t));

    // KEY GPIO 初始化
    key_init();

    // KEY 中断初始化
    key_irq_init();

    // 创建 LED 任务：接收队列消息并根据按键值翻转呼吸灯开关
    xTaskCreate(task_led, "task_led", 2048, NULL, 5, &s_task_led_handle);

    printf("exti_key_init done\r\n");
}

/**
 * @brief 按键任务（消费者）：阻塞等待队列消息，翻转呼吸灯开关标志位
 * @param pvParameters 任务参数指针（本任务未使用）
 * @note 任意按键按下后翻转 g_led_breathing_enabled：
 *       true  → false：关闭呼吸灯（pwm.c 中调用 ledc_stop 强制高电平灭灯）
 *       false → true ：开启呼吸灯（pwm.c 中恢复 PWM 步进输出）
 */
static void task_led(void* pvParameters)
{
    while (1)
    {
        uint32_t key_state = 0;
        // 从队列接收消息：阻塞等待，直到有消息到达队列才返回
        BaseType_t xStatus = xQueueReceive(s_queue_key_handle, &key_state, portMAX_DELAY);

        if (xStatus == pdTRUE)
        {
            printf("[task_led]key_state = %ld\r\n", key_state);

            // KEY1 或 KEY2 任意按键按下：翻转呼吸灯使能标志
            if (key_state & 0x03)
            {
                g_led_breathing_enabled = !g_led_breathing_enabled;
                if (g_led_breathing_enabled)
                {
                    printf("[task_led]呼吸灯：开启\r\n");
                }
                else
                {
                    printf("[task_led]呼吸灯：熄灭\r\n");
                }
            }
        }
    }
}