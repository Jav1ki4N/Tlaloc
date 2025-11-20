//
// Created by i4N on 2025/10/13.
//

/* Header Files */
#include "cppmain.h"
#include "ST7306.h"
#include "i2c.h"
#include "stm32f411xe.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "uart_app.h"
#include <cstdint>

 uint8_t isLit{0};
 #define EXTI_PORT GPIOB
 #define EXTI_KEY1 GPIO_PIN_14
 #define EXTI_KEY2 GPIO_PIN_15

/* main function in C++ */
void cppmain()
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    /* *Initializations Here */
    UART_App_Init();
    ST7306 display(&hi2c1,&hspi1,&huart1,&huart1);
    display.Set_SPI_GPIO(GPIO_PIN_3, GPIO_PIN_4, GPIO_PIN_2);
    display.Init_Sequence();
    /* Infinite Loop */
    for (;;)
    {
        //  using enum ST7306::COLOR;
        // for(int i = 0;i<210;i++)
        // {
        //     for(int j = 0;j<480;j++)
        //     {
        //         display.Draw_Pixel(j,i,WHITE);
        //     }
        // }

        // for(int i = 51;i<100; i++)
        // {
        //     for(int j = 51;j<100;j++)
        //     {
        //         display.Draw_Pixel(j,i,CYAN);
        //     }
        // }

        // for(int i = 101;i<150; i++)
        // {
        //     for(int j = 101;j<150;j++)
        //     {
        //         display.Draw_Pixel(j,i,MAGENTA);
        //     }
        // }

        // display.Update_FullScreen();
        
        //HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14, GPIO_PIN_RESET);
        //HAL_GPIO_WritePin(GPIOB,GPIO_PIN_15, GPIO_PIN_RESET);

        /*****************************************/

        // if(isLit)
        // {
        //     HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_13);
        //     // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
        //     HAL_Delay(1);
        // }
        // Simple debouncing: sample and require stable value for debounce_ms
        static GPIO_PinState last_sample1 = GPIO_PIN_RESET;
        static GPIO_PinState last_sample2 = GPIO_PIN_RESET;
        static GPIO_PinState stable1 = GPIO_PIN_RESET;
        static GPIO_PinState stable2 = GPIO_PIN_RESET;
        static uint32_t last_time1 = 0;
        static uint32_t last_time2 = 0;
        const uint32_t debounce_ms = 50;

        GPIO_PinState sample1 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15); // KEY RIGHT
        GPIO_PinState sample2 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14); // KEY LEFT
        uint32_t now = HAL_GetTick();

        // key1 debouncing
        if (sample1 != last_sample1) {
            last_sample1 = sample1;
            last_time1 = now;
        } else if ((now - last_time1) >= debounce_ms && sample1 != stable1) {
            // stable state changed
            stable1 = sample1;
            if (stable1 == GPIO_PIN_SET) {
                HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_12);
            }
        }

        // key2 debouncing
        if (sample2 != last_sample2) {
            last_sample2 = sample2;
            last_time2 = now;
        } else if ((now - last_time2) >= debounce_ms && sample2 != stable2) {
            stable2 = sample2;
            if (stable2 == GPIO_PIN_SET) {
                HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
            }
        }

        // small delay to avoid busy-looping (keeps sampling at reasonable rate)
        HAL_Delay(10);
       // HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,(s1 == GPIO_PIN_SET)?GPIO_PIN_SET:GPIO_PIN_RESET);
        //HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,(s2 == GPIO_PIN_SET)?GPIO_PIN_SET:GPIO_PIN_RESET);

        
    }
}

// void EXTI15_10_IRQHandler(void)
// {
//     HAL_GPIO_EXTI_IRQHandler(EXTI_KEY1);
//     HAL_GPIO_EXTI_IRQHandler(EXTI_KEY2);
// }

// void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
// {
//     HAL_Delay(10);
//     if(GPIO_Pin == EXTI_KEY1) // Toggle LED
//     {
//         if(HAL_GPIO_ReadPin(EXTI_PORT, EXTI_KEY1) == GPIO_PIN_RESET){ // PRESSED
//         HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_12);}
//     }
//     else if (GPIO_Pin == EXTI_KEY2) // Toggle isLit
//     {
//         GPIO_PinState status = HAL_GPIO_ReadPin(EXTI_PORT, EXTI_KEY2);
//         isLit = (status == GPIO_PIN_RESET)?1:0;
//     }
// }