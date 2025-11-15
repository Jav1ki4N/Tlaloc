//
// Created by i4N on 2025/10/13.
//

/* Header Files */
#include "cppmain.h"
#include "ST7306.h"
#include "i2c.h"
#include "stm32f4xx_hal.h"
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
    /* Infinite Loop */
    for (;;)
    {
         using enum ST7306::COLOR;
        for(int i = 1;i<211;i++)
        {
            for(int j = 1;j<481;j++)
            {
                display.Draw_Pixel(j,i,BLUE);
            }
        }

        for(int i = 25;i<50; i++)
        {
            for(int j = 25;j<50;j++)
            {
                display.Draw_Pixel(j,i,RED);
            }
        }

        for(int i = 51;i<100; i++)
        {
            for(int j = 51;j<100;j++)
            {
                display.Draw_Pixel(j,i,MAGENTA);
            }
        }

        for(int i = 101;i<150; i++)
        {
            for(int j = 101;j<150;j++)
            {
                display.Draw_Pixel(j,i,YELLOW);
            }
        }

        display.Update_FullScreen();
        

    }
}
