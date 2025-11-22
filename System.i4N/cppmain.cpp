
/* Header Files */
#include "cppmain.h"
#include "ST7306.h"
#include "i2c.h"
#include "stm32f411xe.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_tim.h"
#include "tim.h"
#include "uart_app.h"
#include <cstdint>

/* Global Tick Counter for Debugging */
volatile uint32_t lvgl_tick_count = 0;

/* main function in C++ */
void cppmain()
{
    /* Pre-Init Stuffs */
    HAL_TIM_Base_Start_IT(&htim3);

    /* *Initializations Here */
    UART_App_Init();
    
    /* Static instance to prevent stack overflow */
    static ST7306 display(&hi2c1,&hspi1,&huart1,&huart1);
    
    display.Set_SPI_GPIO(GPIO_PIN_3, GPIO_PIN_4, GPIO_PIN_2);
    display.UI_Init();

    /* --- LVGL UI Test Code --- */
    lv_obj_t* scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_white(), 0); // Set White Background
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    lv_obj_t* lbl = lv_label_create(scr);
    lv_label_set_text(lbl, "LVGL Running...");
    lv_obj_set_style_text_color(lbl, lv_color_black(), 0);
    lv_obj_center(lbl);
    /* ------------------------- */
    
    uint32_t last_debug_time = 0;

    /* Infinite Loop */
    for (;;)
    {
        /* Always run handler, remove #if guard to be sure */
        lv_timer_handler();
        HAL_Delay(5);

        /* Heartbeat Logic */
        if(lvgl_tick_count - last_debug_time > 1000)
        {
            last_debug_time = lvgl_tick_count;
            /* Toggle LED every second to show Main Loop + ISR are alive */
            HAL_GPIO_TogglePin(DEVICE::TEST_LED_PORT, DEVICE::TEST_LED_PIN);
        }
    }
}

extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM3)
    {  
        lv_tick_inc(1); // 1ms
        lvgl_tick_count++;
    }
}

