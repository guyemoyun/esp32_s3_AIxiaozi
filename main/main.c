#include <stdio.h>              // 标准输入输出头文件，提供 printf 等函数
#include "freertos/FreeRTOS.h"  // FreeRTOS 内核头文件，提供任务、延时等接口
#include "freertos/task.h"      // FreeRTOS 任务头文件，提供 vTaskDelay 等任务函数
#include "exti.h"               // 外部中断头文件，提供 exti_key_init() 等函数
#include "pwm.h"                // PWM 头文件，提供 pwm_led_init() 等函数



void app_main(void)
{
    // 初始化 KEY 外部中断（按键翻转 g_led_breathing_enabled 控制呼吸灯开关）
    exti_key_init();

    // 初始化 LEDC PWM（GPIO10 输出 100Hz PWM，初始占空比 1023 = LED 灭）
    pwm_led_init();

    printf("This is key test\r\n");
    printf("This is pwm test\r\n");

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(50));// 每 50ms 执行一步呼吸灯效果
        pwm_led_test();// 步进一次：呼吸/熄灭由 KEY 按键翻转控制
    }
}


#if 0
void app_main(void)
{
    // 调用 exti_key_init() 即可完成 KEY 外部中断测试的全套初始化
    exti_key_init();

    printf("This is key test\r\n");

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
#endif





#if 0
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_psram.h"
#include "esp_flash.h"
/**
* @brief 程序入口
* @param 无
* @retval 无
*/
void app_main(void)
{
    
    esp_err_t ret;
     
    uint32_t flash_size;
     
    esp_chip_info_t chip_info; 
    /* 定义芯片信息结构体变量 */
     
    ret = nvs_flash_init(); 
    /* 初始化 NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES 
    || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
     
    {
     
    ESP_ERROR_CHECK(nvs_flash_erase());
     
    ret = nvs_flash_init();
     
    }
     
    esp_flash_get_size(NULL, &flash_size); 
    /* 获取 FLASH 大小 */
     
    esp_chip_info(&chip_info);
    printf("内核:CPU 数量%d\n",chip_info.cores); 
    /* 获取 CPU 内核数并显示 */
    /* 获取 FLASH 大小并显示 */
    printf("FLASH size:%ld MB flash\n",flash_size / (1024 * 1024));
    /* 获取 PARAM 大小并显示 */
     
    printf("PSRAM size: %d bytes\n", esp_psram_get_size()); 
 
    while(1)
    {
     
        printf("Hello-ESP32\r\n");
         
        vTaskDelay(1000);
         
    }
}
#endif