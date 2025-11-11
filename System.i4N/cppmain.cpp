//
// Created by i4N on 2025/10/13.
//

/* Header Files */
#include "cppmain.h"
#include "ST7306.h"
#include "i2c.h"
#include "stm32f4xx_hal_gpio.h"
#include "uart_app.h"

/* main function in C++ */
void cppmain()
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    /* *Initializations Here */
    UART_App_Init();
    ST7306 display(&hi2c1,&hspi1,&huart1,&huart1);
    display.Set_SPI_GPIO(GPIO_PIN_3, GPIO_PIN_4, GPIO_PIN_2);
    display.Init_Sequence();
    display.Quick_Set_Window();
    /* Infinite Loop */
    for (;;)
    {
        display.Quick_Test();
    }
}
