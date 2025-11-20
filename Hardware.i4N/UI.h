
#include "DEVICE.h"
#include "src/hal/lv_hal_disp.h"
#include <cstddef>


#define LVGL_USE_V8 1
#if     LVGL_USE_V8
#include "lvgl.h"
    


#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_conf.h"
#include <cstdint>
#pragma once
class UI
{
    public:
    friend class DEVICE;
    using enum   DEVICE::DEVICE_StatusType;
    using byte = DEVICE::byte;
    using flag = DEVICE::flag;
    using init_t        = DEVICE::DEVICE_StatusType(*)();
    using flush_cb_t    = void(*)(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
    using buffer_t = lv_color_t;
    using info = uint8_t;
   

    UI() 
    {}

    protected:

    /* REGISTER DISPLAY V8.4 */

    lv_disp_drv_t       disp_drv;
    lv_disp_draw_buf_t  buf_mono,      
                        buf_dual;      

    template<size_t N>
    DEVICE::DEVICE_StatusType Init(uint16_t      res_h,
                                   uint16_t      res_v,
                                   init_t        base_init,          // hardware init 
                                   flush_cb_t    flush_cb,           // flush callback, defined by user
                                   buffer_t      (&buf1)[N],         // avoid using dynamic memory allocation
                                   buffer_t      *buf2 = nullptr)    // the size of buffer should be defined by user
    {
        if(base_init) base_init();                     // hardware init, i.e. disp_init
        lv_disp_drv_init(&disp_drv);
        flag isDual = (buf2);
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
        
        return DEVICE_SUCCESS;
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