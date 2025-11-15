#include "ST7306.h"
#include "stm32f4xx_hal_conf.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_spi.h"
#include <cmath>
#include <cstdint>


ST7306::DEVICE_StatusType ST7306::Init_Sequence()
{
    Set_PinState(SPI_GPIO.PORT,SPI_GPIO.PIN_RST,PINSTATE::LOW);
    Delay_ms(50);
    Set_PinState(SPI_GPIO.PORT,SPI_GPIO.PIN_RST,PINSTATE::HIGH);

    using enum SPI_DataType;

    SPI_Send(0x01,                 COMMAND);
    Delay_ms(120);

    SPI_Send(CMD::NVMLOAD_CTRL,    COMMAND);
    SPI_Send(0x17,                 DATA); // enables internal voltage generator and ID read (turns on internal rails)
    SPI_Send(0x02,                 DATA); // select NVM load mode (affects when factory LUT/params are loaded)
    SPI_Send(CMD::BOOSTER_EN,      COMMAND);
    SPI_Send(0x01,                 DATA); // enable on-chip booster (powers up charge pumps used by VGH/VGL generation)
    SPI_Send(CMD::GV_CTRL,         COMMAND);
    SPI_Send(0x0E,                 DATA); // set VGH (inner positive rail). Effect: increases VGH (approx +12V on many panels)
    SPI_Send(0x0A,                 DATA); // set VGL (inner negative rail). Effect: drives VGL to a negative level (approx -6V)
    SPI_Send(CMD::VSHP_CTRL,       COMMAND);
    SPI_Send(0x41,                 DATA); // config for VSHP pump. Practical effect: sets VSHP output (~4.0V) for source driving strength
    SPI_Send(0x41,                 DATA);
    SPI_Send(0x41,                 DATA);
    SPI_Send(0x41,                 DATA);
    SPI_Send(CMD::VSLP_CTRL,       COMMAND);
    SPI_Send(0x32,                 DATA); // config for VSLP pump. Practical effect: sets VSLP output (~0.8V) used by source driver
    SPI_Send(0x32,                 DATA);
    SPI_Send(0x32,                 DATA);
    SPI_Send(0x32,                 DATA);
    SPI_Send(CMD::VSHN_CTRL,       COMMAND);
    SPI_Send(0x46,                 DATA); // VSHN_CTRL bytes: tune negative high-side level. Practical effect: VSHN ~ -3.3V (affects waveform symmetry)
    SPI_Send(0x46,                 DATA);
    SPI_Send(0x46,                 DATA);
    SPI_Send(0x46,                 DATA);
    SPI_Send(CMD::VSLN_CTRL,       COMMAND);
    SPI_Send(0x46,                 DATA); // VSLN_CTRL bytes: tune negative low-side level. Practical effect: VSLN near 0V / small negative (affects contrast)
    SPI_Send(0x46,                 DATA);
    SPI_Send(0x46,                 DATA);
    SPI_Send(0x46,                 DATA);

    Init_SetFrameRate();

    SPI_Send(CMD::UPGEQH_CTRL,     COMMAND);
    SPI_Send(0xE5,                 DATA); // EQ (high-power) — affects waveform shaping for stable pixels at higher refresh rates
    SPI_Send(0xF6,                 DATA);
    SPI_Send(0x05,                 DATA);
    SPI_Send(0x46,                 DATA);
    SPI_Send(0x77,                 DATA);
    SPI_Send(0x77,                 DATA);
    SPI_Send(0x77,                 DATA);
    SPI_Send(0x77,                 DATA);
    SPI_Send(0x76,                 DATA);
    SPI_Send(0x45,                 DATA);
    SPI_Send(CMD::UPGEQL_CTRL,     COMMAND);
    SPI_Send(0x05,                 DATA); // EQ (low-power) sequence — tunes waveform in LPM to reduce ghosting and power
    SPI_Send(0x46,                 DATA);
    SPI_Send(0x77,                 DATA);
    SPI_Send(0x77,                 DATA);
    SPI_Send(0x77,                 DATA);
    SPI_Send(0x77,                 DATA);
    SPI_Send(0x76,                 DATA);
    SPI_Send(0x45,                 DATA);
    SPI_Send(CMD::SOURCEEQ_EN,     COMMAND);
    SPI_Send(0x13,                 DATA); // enable source equalization; effect: improves source drive waveform and contrast
    SPI_Send(CMD::GATELINE_SET,    COMMAND);
    SPI_Send(0x78,                 DATA); // set number of gate lines / mapping. Practical effect: configures vertical resolution mapping (matches panel height)
    SPI_Send(CMD::SLEEP_OUT,       COMMAND); // Exit sleep mode (controller wakes; required before many voltage changes)
    Delay_ms(120);
    SPI_Send(CMD::VSHL_SEL,        COMMAND);
    SPI_Send(0x00,                 DATA);

    Init_SetPixelAndRGB();
    
    SPI_Send(CMD::TEAREFFECT_ON,   COMMAND);
    SPI_Send(0x00,                 DATA);

    // Auto power down
    SPI_Send(CMD::AUTOPWR_CTRL, COMMAND);
    SPI_Send(0xFF, DATA);
    // High power on and display on
    SPI_Send(CMD::HIGHPWR_ON, COMMAND);
    SPI_Send(0x20, COMMAND);
    SPI_Send(0xBB,                 COMMAND);
    SPI_Send(0x4F,                 DATA); // Enable Clear RAM to 0
    SPI_Send(CMD::DISPLAY_ON,      COMMAND);
    Clear_FullScreen();
    return DEVICE_StatusType::DEVICE_SUCCESS;
}

/*****************************************************************************************************************************/


ST7306::DEVICE_StatusType ST7306::Draw_Pixel(uint16_t x, uint16_t y,COLOR color) // DONE 
{
    assert_param(x>=0 && x<=INFO::HEIGHT && y>=0 && y<=INFO::WIDTH);

    byte bit_group   = (x%2)?2:1;                                         // x--> even (group1) | odd (group2) | dummy                                                   
    byte row_addr    = (x/2);                                             // which row of hardware
    byte byte_offset = (y%4);                                             // which byte in a ram unit                
    byte col_addr    = (y/4);                                             // which ram unit in a row
    byte pixel_blank = (bit_group == 1)?0b00011111:0b11100011;            // mask bit to clear the pixel 
    byte pixel       = static_cast<byte>(color) << ((bit_group==1)?5:2 ); // real color info
    
    FULL_SCREEN_BUFFER[row_addr][col_addr][byte_offset].full &= pixel_blank;
    FULL_SCREEN_BUFFER[row_addr][col_addr][byte_offset].full |= pixel;

    return DEVICE_StatusType::DEVICE_SUCCESS;
}

ST7306::DEVICE_StatusType ST7306::Clear_FullScreen()
{
    memset(FULL_SCREEN_BUFFER,0x00, sizeof(FULL_SCREEN_BUFFER));
    Update_FullScreen();
    return DEVICE_StatusType::DEVICE_SUCCESS;
}

ST7306::DEVICE_StatusType ST7306::Run_Refresh_Test()
{
    byte pattern[8] = {0b00000000,0b00100100,
                       0b01001000,0b01101100,
                       0b10010000,0b10110100,
                       0b11011000,0b11111100};
    static byte index = 0;
    if(index == 8)index = 0;
    memset(FULL_SCREEN_BUFFER,pattern[index++],sizeof(FULL_SCREEN_BUFFER));
    Update_FullScreen();
    return DEVICE_StatusType::DEVICE_SUCCESS;
}

ST7306::DEVICE_StatusType ST7306::Update_FullScreen()
{
    using enum SPI_DataType;
    SPI_Send(CMD::COL_ADDR,COMMAND);
    SPI_Send(INFO::XS,       DATA); // XS
    SPI_Send(INFO::XE,       DATA); // XE
    SPI_Send(CMD::ROW_ADDR,COMMAND);
    SPI_Send(INFO::YS,       DATA); // YS
    SPI_Send(INFO::YE,       DATA); // YE
    SPI_Send(CMD::MEM_WRITE,COMMAND);
    SPI_SendCore(reinterpret_cast<byte*>(FULL_SCREEN_BUFFER),DATA,sizeof(FULL_SCREEN_BUFFER)); // limited due to multi dimension
    return DEVICE_StatusType::DEVICE_SUCCESS;
}

ST7306::DEVICE_StatusType ST7306::Fill_Screen(byte color)
{
    memset(FULL_SCREEN_BUFFER,color,sizeof(FULL_SCREEN_BUFFER));
    Update_FullScreen();
    return DEVICE_StatusType::DEVICE_SUCCESS;
}

/* FUNCTIONS BELOW ARE ONLY FOR TESTMENT USAGES */

// ST7306::DEVICE_StatusType ST7306::Quick_Test(byte xs, byte xe, byte ys, byte ye, byte color)
// {
//     using enum SPI_DataType;

//     /* address windows is always 8bit byte */
//     assert_param(xs>0x00);
//     assert_param(xe>xs && xe<=INFO::XE);
//     assert_param(ys>=0x00);
//     assert_param(ye>ys && ye<=INFO::YE);

//     SPI_Send(CMD::COL_ADDR,COMMAND);
//     SPI_Send(xs,       DATA); // XS
//     SPI_Send(xe,       DATA); // XE
//     SPI_Send(CMD::ROW_ADDR,COMMAND);
//     SPI_Send(ys,       DATA); // YS
//     SPI_Send(ye,       DATA); // YE
//     SPI_Send(CMD::MEM_WRITE,COMMAND);

//     uint16_t byte_horizontal = (xe-xs+1)*12/3/2;
//     // xe-xs+1: horizontal ram units
//     // horizontal sub pixels = ram units * 12
//     // honrizontal pixels = sub pixels / 3
//     // honrizontal bytes = horizontal pixels / 2
//     // if xs=xe, val = 2
    
//     uint16_t byte_vertical   = (ye-ys+1)*2;
//     // (ye-ys+1): vertical ram units
//     // vertical bytes = vertical ram units * 2
//     // honrizontal bytes = vertical pixels
//     // if ys=ye, val = 2

//     uint32_t area = (byte_horizontal*byte_vertical);
//     // if xs=xe and ys=ye, area = 4
//     uint8_t buf[INFO::FULL_SCREEN_BYTE_SIZE];
//     memset(buf,color,area);
//     SPI_Send(buf,DATA,4);
    
//     return DEVICE_StatusType::DEVICE_SUCCESS;
// }

// ST7306::DEVICE_StatusType ST7306::Draw_Min_Ram_Unit(byte x, byte y, byte color)
// {
//     assert_param(x>INFO::XS&&x<INFO::XE);
//     assert_param(y>INFO::YS&&y<INFO::YE);
//     using enum SPI_DataType;
//     SPI_Send(CMD::COL_ADDR,COMMAND);
//     SPI_Send(x,       DATA); // XS
//     SPI_Send(x,       DATA); // XE
//     SPI_Send(CMD::ROW_ADDR,COMMAND);
//     SPI_Send(y,       DATA); // YS
//     SPI_Send(y,       DATA); // YE
//     SPI_Send(CMD::MEM_WRITE,COMMAND);
//     uint8_t buf[4] = {color,color,color,color}; // 4 byte in min
//     SPI_Send(buf,DATA,4);  
//     return DEVICE_StatusType::DEVICE_SUCCESS;
// }

// ST7306::DEVICE_StatusType ST7306::Draw_Min_Ram_Unit_free(byte x, byte y,
//                                                          byte color1, 
//                                                          byte color2,
//                                                          byte color3, 
//                                                          byte color4)
// {
    
//     using enum SPI_DataType;
//     SPI_Send(CMD::COL_ADDR,COMMAND);
//     SPI_Send(x,       DATA); // XS
//     SPI_Send(x,       DATA); // XE
//     SPI_Send(CMD::ROW_ADDR,COMMAND);
//     SPI_Send(y,       DATA); // YS
//     SPI_Send(y,       DATA); // YE
//     SPI_Send(CMD::MEM_WRITE,COMMAND);
//     uint8_t buf[4] = {color1, color2, color3, color4};
//     SPI_Send(buf,DATA,4);
//     return DEVICE_StatusType::DEVICE_SUCCESS;
// }

// ST7306::DEVICE_StatusType ST7306::Draw_Min_Square_block(byte x, byte y, byte color)
// {
//     /* Draw a 4*4px block */
//     /* This block is composed of 2 ram units,one in the upper and one in the lower */
//     /* i.e the horizontal remains the same while the vertical doubles              */
//     /* this block does not affects any other blocks or pixels */
//     /* since it contains 2 full ram units */
//     /* however by doing so the resolution (if used the block as pixels) will be downsized to 52*120 */
//     /* which is 4 times smaller than 210*480 */

//     if(x>52)x=52;
//     if(y>120)y=120;
//     byte x_address = x+4; // start from 0x04
//     byte y_address = (y==0)?0:y*2;
//     assert_param(x_address>INFO::XS&&x_address<INFO::XE);
//     assert_param(y_address>INFO::YS&&y_address<INFO::YE);
//     using enum SPI_DataType;
//     SPI_Send(CMD::COL_ADDR,COMMAND);
//     SPI_Send(x_address,       DATA); // XS
//     SPI_Send(x_address,       DATA); // XE
//     SPI_Send(CMD::ROW_ADDR,COMMAND);
//     SPI_Send(y_address,       DATA); // YS
//     SPI_Send(y_address+1,       DATA); // YE
//     SPI_Send(CMD::MEM_WRITE,COMMAND);
//     uint8_t buf[8] = {color,color,color,color,
//                       color,color,color,color}; // 8 byte in min square block
//     SPI_Send(buf,DATA,8);
//     return DEVICE_StatusType::DEVICE_SUCCESS;
// }

// ST7306::DEVICE_StatusType ST7306::Draw_Subpixel_bonded(byte x,byte y, byte color)
// {
//     assert_param(x>INFO::XS&&x<INFO::XE);
//     assert_param(y>INFO::YS&&y<INFO::YE);
//     using enum SPI_DataType;
//     SPI_Send(CMD::COL_ADDR,COMMAND);
//     SPI_Send(x,       DATA); // XS
//     SPI_Send(x,       DATA); // XE
//     SPI_Send(CMD::ROW_ADDR,COMMAND);
//     SPI_Send(y,       DATA); // YS
//     SPI_Send(y,       DATA); // YE
//     SPI_Send(CMD::MEM_WRITE,COMMAND);
//     color = color & 0xE0; // only high 3 bits valid
//     uint8_t buf[4] = {color,0x00,0x00,0x00}; // 4 byte in min
//     SPI_Send(buf,DATA,4);  
//     return DEVICE_StatusType::DEVICE_SUCCESS;
// }

/***********************************************************************/







