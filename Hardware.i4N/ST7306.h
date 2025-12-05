#pragma once
#include "DEVICE.h"
#include "UI.h"
#include "spi.h"
#include "src/hal/lv_hal_disp.h"
#include "usart.h"
#include <cstdint>

class ST7306 : public DEVICE, public UI
{
    public:
        explicit ST7306(I2C_HandleType   *hi2cx   = nullptr , // no I2C support
                        SPI_HandleType   *hspix   = &hspi1  ,
                        UART_HandleType  *huartx  = nullptr,
                        DEBUG_HandleType *hdebugx = &huart1)
        :DEVICE(hi2cx,hspix,huartx,hdebugx)
        ,UI()
        {
            isDebug = false;
        }
        
        enum class COLOR : byte
        {
            /* RGB ORDER, 0 vaild */
            BLACK   = 0b111, 
            RED     = 0b011,   
            GREEN   = 0b101, 
            YELLOW  = 0b001, 
            BLUE    = 0b110,   
            MAGENTA = 0b010,
            CYAN    = 0b100,  
            WHITE   = 0b000,
        };

        typedef union
        {
            /* minimal operate object */    //* --> RGB1 - row1
            using bit = byte;               //* --> RGB2 - row2
            struct
            {
                bit B1    :1;
                bit G1    :1; // 1st pixel --> left(top)
                bit R1    :1;
                //
                bit B2    :1;
                bit G2    :1; // 2nd pixel --> right(bottom)
                bit R2    :1;
                //
                bit dummy1:1; // discarded
                bit dummy2:1; // discarded
            };
            byte full;
        }pixel_byte_t;

        pixel_byte_t FULL_SCREEN_BUFFER[240][53][4]; // for index,
                                                     // min is [0][0][0]
                                                     // max is [239][52][3]
                                                     // for address, 0-239 is fine
                                                     // while 0-52 would be 4-56
        // 4 - 4 bytes in a ram unit (offset)
        // 53 - 53 53 ram units in a row
        // 240 - 240 columns

        /* APIs */
        DEVICE_StatusType Init_Sequence    ();
        DEVICE_StatusType Clear_FullScreen ();           
        DEVICE_StatusType Update_FullScreen();
        DEVICE_StatusType Update(byte x, byte y);          
        DEVICE_StatusType Fill_FullScreen  (COLOR color, byte  color_detailed = 0);  
        DEVICE_StatusType Draw_Pixel       (res_t x,     res_t y, COLOR color); 
        
        //DEVICE_StatusType Run_Refresh_Test();
        

        // * this function draws a minimum ram unit (4*2 pixels) at (x,y)
        // * the (x,y) is not pixel coordinate, but ram unit coordinate
        // ! for test only
        DEVICE_StatusType Draw_Min_Ram_Unit(byte x, byte y, byte color = 0xff);

        DEVICE_StatusType Draw_Min_Ram_Unit_free(byte x, byte y,
                                                 byte color1, byte color2,byte color3, byte color4);
        DEVICE_StatusType Draw_Min_Square_block(byte x, byte y, byte color);
        DEVICE_StatusType Draw_Subpixel_bonded(byte x,byte y,byte color = 0xff);
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

        struct INFO
        {
            /* one ram unit = 12*2 subpx = 4px in hor, 2px in ver                */
            static constexpr uint16_t WIDTH       {210};          // Source Line Drived
            static constexpr uint16_t HEIGHT      {480};          // Gate Line Drived
            static constexpr uint16_t BUFFER_SIZE {HEIGHT/2 - 1}; // half-screen (239)

            /* min ram unit is 12*2 subpx, or 4*2 px */

            static constexpr uint8_t  XS          {0x04},         // address window, default full screen 
                                      XE          {0x38},         // 0x38 - 0x04 = 53,53*4 = 212 pixels
                                      YS          {0x00},
                                      YE          {BUFFER_SIZE};  // 0xEF = 0x00 = 240,240*2 = 480 pixels

            static constexpr uint8_t  BYTES_PER_PIXEL {2};        // 8 color mode, 1 pixel = 3bits and 2 pixels packed in 1 byte
            static constexpr uint32_t FULL_SCREEN_BYTE_SIZE {((WIDTH+2)*HEIGHT)/BYTES_PER_PIXEL}; // 2 extra bytes for padding
                                                                                                  // value confirmed by experiment
        };

        /* BELOW ARE INIT CORE FUNCS    */
        /* BETTER DON'T MODIFY FOR GOOD */
        
        void Init_SetFrameRate()
        {
            using enum SPI_DataType;
            SPI_Send(CMD::OSC_SETTING,     COMMAND); // enable 51hz
            SPI_Send(0x80,                 DATA);
            SPI_Send(0xE9,                 DATA); 
            SPI_Send(CMD::FRAMERATE_CTRL,  COMMAND); // 51hz is unlocked by settingg osc
            SPI_Send(0x12,                 DATA);    // set highest frame rate ~51 hz, low power mode using ~1hz
        }

    //     enum class DIRECTION : byte{/* FPC cable side: West */
    //                                 NW_TO_SE = (0x8<<4)|(0x0<<4),
    //                                 SW_TO_NE = (0x0<<4)|(0x0<<4),
    //                                 NE_TO_SW = (0x8<<4)|(0x4<<4),
    //                                 SE_TO_NW = (0x0<<4)|(0x4<<4)
    //    };

    //     enum class ROTATION : byte{/* When FPC cable pointing west, Rotation = 0 */
    //                                 NO_ROTATION      = (0x2)<<4 | static_cast<byte>(DIRECTION::NW_TO_SE), // ↘
    //                                 FLIP_X           = (0x2)<<4 | static_cast<byte>(DIRECTION::SW_TO_NE), // ↗
    //                                 FLIP_Y           = (0x2)<<4 | static_cast<byte>(DIRECTION::NE_TO_SW), // ↙
    //                                 CENTRAL_SYMMETRY = (0x2)<<4 | static_cast<byte>(DIRECTION::SE_TO_NW), // ↖
    //                                 /**/
    //                                 VERTICAL         = 0x48
    //     };  

        enum class GRAYSCALE : byte{MONO       = 0x20,  FOUR_GS  = 0x00};
        enum class COLORDEPTH: byte{THREE_BITS = 0x2<<4};
        enum class DUMMYBYTE : byte{TWO_DUMMY  = 0x0,   NO_DUMMY = 0x1};
        /**/
        enum class INVERSION : byte{COLUMN     = 0x0<<4,ONE_DOT  = 0x2<<4,TWO_DOT = 0x4<<4};
        enum class INTERVAL  : byte{TWO_LINE   = 0x0<<2,ONE_LINE = 0x1<<2,FRAME   = 0x2<<2};
        enum class INTERLACE : byte{TWO_LINE   = 0x0,   ONE_LINE = 0x1,   NON     = 0x2   };

        void Init_SetPixelAndRGB(COLORDEPTH depth     = COLORDEPTH::THREE_BITS,
                                 DUMMYBYTE  dummy     = DUMMYBYTE ::TWO_DUMMY,
                                 GRAYSCALE  graysacle = GRAYSCALE ::MONO     ,
                                 INVERSION  inversion = INVERSION ::ONE_DOT   ,
                                 INTERVAL   interval  = INTERVAL  ::FRAME ,
                                 INTERLACE  interlace = INTERLACE ::ONE_LINE       )
        {
            /* better don't modify this */
            using enum SPI_DataType;
            SPI_Send(CMD::MAD_CTRL,        COMMAND);
            SPI_Send(0x00,                 DATA); // affects color dispalyed for the same data
            SPI_Send(CMD::DATAFMT_SEL,     COMMAND);
            SPI_Send((static_cast<byte>(depth)) | (0x1<<4) | 0x2 | (static_cast<byte>(dummy)),DATA);
            //SPI_Send(0x32,DATA); // 0b00110010
            SPI_Send(CMD::GAMMAMODE_SET,   COMMAND);
            SPI_Send(static_cast<uint8_t>(graysacle),DATA); 
            SPI_Send(CMD::PANEL_SET,       COMMAND);
            SPI_Send(static_cast<uint8_t>(inversion)|
                     static_cast<uint8_t>(interval) |
                     static_cast<uint8_t>(interlace),DATA);
        }


        /***************************************************************************************************************************/

         /*$   /$$ /$$$$$$
        | $$  | $$|_  $$_/
        | $$  | $$  | $$  
        | $$  | $$  | $$  
        | $$  | $$  | $$  
        | $$  | $$  | $$  
        |  $$$$$$/ /$$$$$$
        \______/  |_____*/

        #if defined (LVGL_USE_V8)

        lv_color_t BUFFER_A[INFO::WIDTH*10],
                   BUFFER_B[INFO::WIDTH*10];

        byte Convert_ColorDepth_16to3(lv_color_t color_16);

        void        Flush_Core    (lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
        static void Flush_CallBack(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);

        public:
        DEVICE_StatusType UI_Init()
        {
            Init_Sequence();                                      // Hardware Init
            LVGL_Init(UI::REG_TABLE::ST7306_210X480,  // Device ID
                      INFO::HEIGHT, 
                      INFO::WIDTH,                  // UI Init
                      Flush_CallBack, 
                      BUFFER_A, 
                      BUFFER_B);
                      
            #define TEMP_TEST_2 0
            #if     TEMP_TEST_2
                HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
                Delay_ms(1000);
                HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
                Delay_ms(200);
                HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
                Delay_ms(50);
                HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
            #endif
            return DEVICE_StatusType::DEVICE_SUCCESS;
        }

        
        #endif
        
}; 