# BSP 组件注册：将 LED 和 EXTI 驱动源文件及头文件路径注册到 IDF 构建系统
idf_component_register(
    SRCS "LED/led.c"           # 源文件：LED 驱动
         "EXTI/exti.c"         # 源文件：外部中断（KEY）驱动
         "PWM/pwm.c"
    INCLUDE_DIRS "LED"         # 头文件路径：LED 驱动
                 "EXTI"        # 头文件路径：外部中断（KEY）驱动
                 "PWM"         # PWM 驱动源文件及头文件路径注册到
    REQUIRES driver freertos   # 依赖组件：GPIO 驱动(driver)、FreeRTOS
)