#define LVGL_ENABLE 0
#if LVGL_ENABLE
#include "DEVICE.h"
#include "lvgl.h"
#include "src/display/lv_display.h"
#include "src/misc/lv_types.h"
#include "src/tick/lv_tick.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_conf.h"
#include <cstdint>
class UI
{
    public:
    friend class DEVICE;

    UI()
    {
        lv_display_set_user_data(display,nullptr); // clear user data on construction
    }

    protected:

    using UI_StatusType = DEVICE::DEVICE_StatusType;
    using UI_BufferType = uint8_t; 

    lv_display_t *display{nullptr};  // LVGL display object, bonded to the class (and the object)

    struct DISPLAY_INFO
    {
        int32_t hor_res{};
        int32_t ver_res{};
        int32_t byte_per_pixel{};
        /* there's no way I can define a buffer without dynamic allocation here*/
    }info; // need to be updated

    UI_StatusType Register_DispInfo(int32_t hor_res, int32_t ver_res, int32_t byte_per_pixel)
    {
        info.hor_res = hor_res;
        info.ver_res = ver_res;
        info.byte_per_pixel = byte_per_pixel;
        return UI_StatusType::DEVICE_SUCCESS;
    }

    /*
     
     LVGL callback asks for a static function, which requires a this pointer to access member functions.
     the this pointer is stored in lv_display_t's user_data field during initialization.
     when defining the flush callback, use lv_display_get_user_data to retrieve the instance pointer.
     and finally the flush callback should be passed to LVGL during UI_Init.

     static void UI_Flush_Callback(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
     {
        <class name> *self = static_cast<<class name>*>(lv_display_get_user_data(disp));
        if(!self)
        {
            lv_display_flush_ready(disp);
            return;
        }
        *logic that uses member contents* 
        
     }
    */

    UI_StatusType LVGL_Init(lv_display_flush_cb_t flush_cb,          // defined in the sub classes
                            UI_BufferType        *buf1,              // also defined in the sub classes
                            UI_BufferType        *buf2 = nullptr)    
    {
        assert_param(info.hor_res > 0  && info.ver_res > 0 && info.byte_per_pixel > 0);

        /* 1.SET TICK INTERFACE */
        lv_tick_set_cb(HAL_GetTick);

        /* 2.CREATE LVGL DISPLAY OBJECT */
        if((display = lv_display_create(info.hor_res, info.ver_res)) == nullptr)
            return UI_StatusType::DEVICE_FAILED;
        
        /* 3.SET FLUSH CALLBACK */

        lv_display_set_user_data(display,this); // bind instance pointer to userdata
                                                // 'this' will be used in the static flush callback 
        
        lv_display_set_flush_cb(display, flush_cb);
        
        /* 4.SET LVGL BUFFER */    
        if(buf1 != nullptr && buf2 != nullptr)                         // dual buffer
        {
            /*todo*/
        }
        else if (buf1 != nullptr && buf2 == nullptr)                   // single buffer, 10 rows
        {
            lv_display_set_buffers(display,
                                   buf1,
                                   nullptr,
                                   (info.hor_res * 10 * info.byte_per_pixel), // i.e. sizeof(buf1)
                                   LV_DISPLAY_RENDER_MODE_PARTIAL);
        }
        else return UI_StatusType::DEVICE_FAILED;                      // no available buffer
        
        return UI_StatusType::DEVICE_SUCCESS;
    }

    union PIXEL_RGB565_T
    {
        struct
        {
            /* separate RGB channel */
            uint16_t red:   5;
            uint16_t green: 6;
            uint16_t blue:  5;
        }channel;
        uint16_t full;
    }PIXEL_RGB565;
};
#endif