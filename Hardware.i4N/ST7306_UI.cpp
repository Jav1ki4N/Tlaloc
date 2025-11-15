#define LVGL_ENABLE 0
#if LVGL_ENABLE
#include "ST7306.h"
#include "lvgl.h"
#include "src/display/lv_display.h"
#include "src/tick/lv_tick.h"
#include "stm32f4xx_hal.h"

ST7306::DEVICE_StatusType ST7306::UI_Init()
{
    Init_Sequence();
    
}

// ST7306::DEVICE_StatusType ST7306::Flush_Callback(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
// {
   
// }
#endif