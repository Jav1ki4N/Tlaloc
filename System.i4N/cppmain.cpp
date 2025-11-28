
/* Header Files */
#include "cppmain.h"
#include "ST7306.h"
#include "i2c.h"
#include "src/misc/lv_color.h"
#include "stm32f411xe.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_spi.h"
#include "stm32f4xx_hal_tim.h"
#include "tim.h"
#include "uart_app.h"
#include <cstdint>


ST7306* p_display = nullptr;

/* main function in C++ */
void cppmain()
{
    
    /* Pre-Init Stuffs */
    HAL_TIM_Base_Start_IT(&htim3);

    /* *Initializations Here */
    UART_App_Init();
    
    /* Static instance to prevent stack overflow */
    static ST7306 display(&hi2c1,&hspi1,&huart1,&huart1);
    p_display = &display; // for interrupt use

    display.Set_SPI_GPIO(GPIO_PIN_2, GPIO_PIN_3, GPIO_PIN_4);
    display.Init_Sequence();
    //display.UI_Init();

    /* --- LVGL UI Test Code --- */
    // lv_obj_t* scr = lv_scr_act();
    // lv_obj_set_style_bg_color(scr, lv_color_black(), 0); // Set White Background
    // lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    // lv_obj_t* lbl = lv_label_create(scr);
    // lv_label_set_text(lbl, "FUCK THE WORLD");
    // lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
    // lv_obj_center(lbl);
    /* ------------------------- */
    
    /* Infinite Loop */
    for (;;)
    {
        /* Always run handler, remove #if guard to be sure */
        //lv_timer_handler();
        //HAL_Delay(5);
        display.Fill_FullScreen(ST7306::COLOR::GREEN);
        display.Update_FullScreen();
    }
}

extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    // 1ms
    if(htim->Instance == TIM3)
    {  
        //display.foo();
        //lv_tick_inc(1);
    }
}

extern "C" void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if(hspi->Instance == SPI1)
    {
        if (p_display == nullptr)return;
        p_display->SPI_On_DMAOver();
    }
}
