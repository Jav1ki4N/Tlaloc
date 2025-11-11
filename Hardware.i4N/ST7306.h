#pragma once
#include "DEVICE.h"
#include "UI.h"
#include "spi.h"
#include "usart.h"

class ST7306 : public DEVICE, public UI
{
    public:
        explicit ST7306(I2C_HandleType   *hi2cx   = nullptr ,
                        SPI_HandleType   *hspix   = &hspi1  ,
                        UART_HandleType  *huartx  = nullptr,
                        DEBUG_HandleType *hdebugx = &huart1)
        :DEVICE(hi2cx,hspix,huartx,hdebugx)
        {
            isDebug = false;
        }
        
        DEVICE_StatusType Init_Sequence();
        DEVICE_StatusType Quick_Test();
        DEVICE_StatusType Quick_Set_Window();
    private:
        using command  = byte;
        struct CMD
        {
            static constexpr command  DISPLAY_ON         {0x29};
            static constexpr command  NVMLOAD_CTRL       {0xD6};
            static constexpr command  BOOSTER_EN         {0xD1};

            /* Inner Voltage Settings */
            static constexpr command  GV_CTRL            {0xC0}; 
            static constexpr command  VSHP_CTRL          {0xC1};
            static constexpr command  VSLP_CTRL          {0xC2};
            static constexpr command  VSHN_CTRL          {0xC4};
            static constexpr command  VSLN_CTRL          {0xC5};

            /* Frame Rate settings */
            static constexpr command  FRAMERATE_CTRL     {0xB2};
            static constexpr command  OSC_SETTING        {0xD8};

            /* EQ Settings */
            /* Normally needn't to be modified */
            static constexpr command  UPGEQH_CTRL        {0xB3};
            static constexpr command  UPGEQL_CTRL        {0xB4};
            static constexpr command  SOURCEEQ_EN        {0xB7};

            static constexpr command  GATELINE_SET       {0xB0};
            static constexpr command  SLEEP_OUT	         {0x11};
            static constexpr command  VSHL_SEL	         {0xC9};
            static constexpr command  MAD_CTRL	         {0x36};
            static constexpr command  DATAFMT_SEL        {0x3A};

            static constexpr command  GAMMAMODE_SET      {0xB9};
            static constexpr command  PANEL_SET          {0xB8};
            static constexpr command  TEAREFFECT_ON      {0x35};
            static constexpr command  AUTOPWR_CTRL       {0xD0};
            static constexpr command  HIGHPWR_ON         {0x38};

            static constexpr command  COL_ADDR           {0x2A};
            static constexpr command  ROW_ADDR           {0x2B};
            static constexpr command  MEM_WRITE          {0x2C};  
        };

        /* UI */
        
}; 