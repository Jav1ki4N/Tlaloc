#pragma once
#include "DEVICE.h"
#include <cstddef>
#include "src/core/lv_obj.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_conf.h"
#include <cstdint>

#define LVGL_USE_V8 1
#if     LVGL_USE_V8
#include "lvgl.h"
#include "src/hal/lv_hal_disp.h"
    
class UI
{
    public:
    friend class DEVICE;
    
    using init_t        = DEVICE::DEVICE_StatusType(*)();
    using flush_cb_t    = void(*)(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
    
    using buffer_t  = lv_color_t;
    /* resolution type */
    using res_t     = uint16_t;   
   

    UI() 
    {}

    protected:

    /* REGISTER DISPLAY V8.4 */

    lv_disp_drv_t       disp_drv; // lvgl display driver structure
    lv_disp_draw_buf_t  buf_mono, // lvgl buffer structure mono/dual     
                        buf_dual;      

    template<size_t N>
    DEVICE::DEVICE_StatusType LVGL_Init(res_t        res_h,
                                        res_t        res_v,
                                        flush_cb_t   flush_cb,           // flush callback, defined by user
                                        buffer_t   (&buf1)[N],         // avoid using dynamic memory allocation
                                        buffer_t    *buf2 = nullptr)    // the size of buffer should be defined by user
    {
        static bool isInited{false};
        if(!isInited)lv_init(),isInited = true;
        lv_disp_drv_init(&disp_drv);
        DEVICE::flag isDual = (buf2);
        lv_disp_draw_buf_t *active_buf = (isDual)?&buf_dual:&buf_mono;
        lv_disp_draw_buf_init(active_buf,buf1,buf2,N);
        /* set parameters */
        disp_drv.hor_res   = res_h;
        disp_drv.ver_res   = res_v;
        disp_drv.user_data = this;
        disp_drv.flush_cb  = flush_cb;   // the flush is defined by user and 
                                         // 'this' pointer must be retrieved by 'disp_drv->user_data'
        disp_drv.draw_buf  = active_buf;
        /* Register */
        lv_disp_drv_register(&disp_drv);
        
        return DEVICE::DEVICE_StatusType::DEVICE_SUCCESS;
    } 


    /* color structure: 16bit color depth*/
    typedef union
    {
        struct
        {
            uint16_t R:5;
            uint16_t G:6;
            uint16_t B:5;
        };
        uint16_t full;
    }RGB565_T; 
};
#endif

#define LVGL_USE_V9 0
#if     LVGL_USE_V9

/* todo */

#endif