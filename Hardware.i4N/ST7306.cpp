#include "ST7306.h"
#include "stm32f4xx_hal_gpio.h"

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
    SPI_Send(CMD::FRAMERATE_CTRL,  COMMAND);
    SPI_Send(0x12,                 DATA); // sets frame rates: High Power Mode ~32Hz, Low Power Mode ~1Hz (affects update speed & power)
    SPI_Send(CMD::OSC_SETTING,     COMMAND);
    SPI_Send(0x80,                 DATA); // oscillator tuning: increases internal clock (affects max refresh rate)
    SPI_Send(0xE9,                 DATA); // oscillator fine-tune parameter (stabilizes chosen frequency)
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
    SPI_Send(CMD::MAD_CTRL,        COMMAND);
    SPI_Send(0x48,                 DATA); // MADCTL: affects row/column order and data direction; effect: controls display rotation/mirroring
    SPI_Send(CMD::DATAFMT_SEL,     COMMAND);
    SPI_Send(0x32,                 DATA); // DATAFMT: selects pixel packing/bit-depth; effect: here configures 8-color / 24-bit packing used by driver
    
    SPI_Send(CMD::GAMMAMODE_SET,    COMMAND);
    SPI_Send(0x00,                  DATA); 

    SPI_Send(CMD::PANEL_SET,       COMMAND);
    SPI_Send(0x0A,                 DATA); // panel setting: configures panel parameters (polarity, inversion, interlace)
    
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

    return DEVICE_StatusType::DEVICE_SUCCESS;
}


/* Quick test funcs     */
ST7306::DEVICE_StatusType ST7306::Quick_Set_Window()
{
    using enum SPI_DataType;

    SPI_Send(CMD::COL_ADDR,COMMAND);
    SPI_Send(0x04,       DATA); // XS
    SPI_Send(0x38,       DATA); // XE
    SPI_Send(CMD::ROW_ADDR,COMMAND);
    SPI_Send(0x00,       DATA); // YS
    SPI_Send(0xEF,       DATA); // YE
    SPI_Send(CMD::MEM_WRITE,COMMAND);
    return DEVICE_StatusType::DEVICE_SUCCESS;
} 

ST7306::DEVICE_StatusType ST7306::Quick_Test()
{
    static uint8_t flip = 0;
    flip = !flip;
    using enum SPI_DataType;
    //Quick_Set_Window();
    for (unsigned int i = 0; i < (480*106); i++)
    {
        SPI_Send((flip ? 0xFF : 0x00), DATA);
    }
    return DEVICE_StatusType::DEVICE_SUCCESS;
}

