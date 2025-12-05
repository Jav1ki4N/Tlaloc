#pragma once

/* C&C++ Libs */
#include <cstdint>
#include <cstdio>
#include <cstring>

/* HAL_Libs*/
#include "main.h"
#include "stm32f411xe.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_spi.h"

class DEVICE
{
public:
	using I2C_HandleType   = I2C_HandleTypeDef ;
	using SPI_HandleType   = SPI_HandleTypeDef ;
	using UART_HandleType  = UART_HandleTypeDef;
	using DEBUG_HandleType = UART_HandleType   ;

	DEVICE(I2C_HandleType   *hi2cx,
		   SPI_HandleType   *hspix,
		   UART_HandleType  *huartx,
		   DEBUG_HandleType *hdebugx)

		  :HI2CX   (hi2cx),
		   HSPIX   (hspix),
	       HDEBUGX (hdebugx)
	{
		Get_HandlerInfo(); // update which_XXX
	}
	
	friend class UI;

    

protected:

	void Get_HandlerInfo()
	{
		which_i2c = (HI2CX == nullptr       ) ? 0:
			        (HI2CX->Instance == I2C1) ? 1:
					(HI2CX->Instance == I2C2) ? 2:
					#ifdef I2C3
					(HI2CX->Instance == I2C3) ? 3:
					#endif
					0;

		which_spi = (HSPIX == nullptr       ) ? 0:
					(HSPIX->Instance == SPI1) ? 1:
					(HSPIX->Instance == SPI2) ? 2:
					#ifdef SPI3
					(HSPIX->Instance == SPI3) ? 3:
					#endif
					#ifdef SPI4
					(HSPIX->Instance == SPI4) ? 4:
					#endif
					#ifdef SPI5
					(HSPIX->Instance == SPI5) ? 5:
					#endif
					0;
	}
	/*********************************************************************/
	/* General Purpose */
	/* Optional fixed constants, used if needed */
	static constexpr uint16_t GENERAL_BUFFER_LENGTH{256};
	static constexpr uint16_t GENERAL_TIMEOUT      {1000}; // in ms

	// Moved to public
	// static inline GPIO_TypeDef* const TEST_LED_PORT{GPIOC};
	// static constexpr uint16_t         TEST_LED_PIN {GPIO_PIN_13};

	enum class DEVICE_StatusType : uint8_t
	{
		DEVICE_FAILED  = 0,
		DEVICE_SUCCESS = 1
		/* RTOS */
		/* ... */
	};

	using byte = uint8_t       ;
	using word = uint16_t      ;
	using flag = bool          ;
	using pin  = uint16_t      ;
	using port = GPIO_TypeDef *;

	enum class PINSTATE:bool
	{
		LOW  = false,
		HIGH = true
	};

	void Set_PinState(port port,pin pin, PINSTATE state)
	{
		port->BSRR = (state == PINSTATE::HIGH) ? pin : static_cast<uint32_t>(pin) << 16; // Reset bit
	}

	/*********************************************************************/

	/*********************************************************************/
	/* I2C */
	uint8_t            which_i2c = 0;        // for debugging info
	I2C_HandleTypeDef *HI2CX;     // current I2C handler of the device
	uint16_t            I2C_ADDR;  // current I2C address of the device

	/*func DEVICE_I2C_Set_Timeout                                         */
	/*? Set I2C timeout manually                                          */
	/*! timeout value will be changed whether the status is true or false */
	uint16_t I2C_timeout{};
	bool     I2C_timeout_status{false};
	DEVICE_StatusType I2C_Set_Timeout(uint16_t timeout,bool status = true)
	{
		I2C_timeout        = timeout;
		I2C_timeout_status = status;
		Debug_Print("I2C Timeout is set to %d ms\r\n",I2C_timeout);
		return DEVICE_StatusType::DEVICE_SUCCESS;
	}

	/*func I2C_DirectTx */
	/*? Directly transmit data to the device instead of registers */

	template<typename DATA_T>
	DEVICE_StatusType I2C_DirectTX(DATA_T *pData, const uint16_t amount)
	{
		uint16_t          byte_size = sizeof(DATA_T)*amount;
		Debug_Print("I2C%d: Sending %d bytes to device [0x%x]\r\n",which_i2c,byte_size,I2C_ADDR);
		HAL_StatusTypeDef status    = HAL_I2C_Master_Transmit(HI2CX,
			                                                  I2C_ADDR,
			                                                  (uint8_t*)pData,
			                                                  byte_size,
			                                                  I2C_timeout_status?I2C_timeout:GENERAL_TIMEOUT);
		if (status != HAL_OK)return Debug_Print("I2C%d: I2C Transmission Failed\r\n",which_i2c), DEVICE_StatusType::DEVICE_FAILED;
		return Debug_Print("I2C%d: I2C Transmission Success\r\n",which_i2c), DEVICE_StatusType::DEVICE_SUCCESS;
	}

    /*func I2C_RegTX */
	/*? Transmit data to the device's registers*/

	template<typename REG_T,typename DATA_T>
	DEVICE_StatusType I2C_RegTX(REG_T reg,DATA_T *pData,const uint16_t amount)
	{
		static_assert(sizeof(REG_T)==1 || sizeof(REG_T)==2,"register byte is too large");
		uint16_t          byte_size = sizeof(DATA_T)*amount;
		Debug_Print("I2C%d: Sending %d bytes to device [0x%x]\r\n",which_i2c,byte_size,I2C_ADDR);
		Debug_Print("I2C%d: Starting from register 0x%x\r\n",which_i2c,reg);
		HAL_StatusTypeDef status    = HAL_I2C_Mem_Write(HI2CX,
														I2C_ADDR,
														static_cast<uint16_t>(reg),
														sizeof(REG_T),
														(uint8_t*)pData,
														byte_size,
														I2C_timeout_status?I2C_timeout:GENERAL_TIMEOUT);
		if (status != HAL_OK)return Debug_Print("I2C%d: I2C Transmission Failed\r\n",which_i2c), DEVICE_StatusType::DEVICE_FAILED;
		return Debug_Print("I2C%d: I2C Transmission Success\r\n",which_i2c), DEVICE_StatusType::DEVICE_SUCCESS;

	}

	template<typename REG_T,typename DATA_T>
	DEVICE_StatusType I2C_RegRX(REG_T reg,DATA_T *pData,const uint16_t amount)
	{
		//static_assert(sizeof(REG_T)==1 || sizeof(REG_T)==2,"register byte is too large");
		uint16_t          byte_size = sizeof(DATA_T)*amount;
		Debug_Print("I2C%d: Reading %d bytes from device [0x%x]\r\n",which_i2c,byte_size,I2C_ADDR>>1);
		Debug_Print("I2C%d: Starting from register 0x%x\r\n",which_i2c,reg);
		HAL_StatusTypeDef status    = HAL_I2C_Mem_Read(HI2CX,
		                                               I2C_ADDR,
		                                               static_cast<uint16_t>(reg),
		                                               1,
		                                               (uint8_t*)pData,
		                                               byte_size,
		                                               I2C_timeout_status?I2C_timeout:GENERAL_TIMEOUT);
		if (status != HAL_OK)return Debug_Print("I2C%d: I2C Transmission Failed\r\n",which_i2c), DEVICE_StatusType::DEVICE_FAILED;
		return Debug_Print("I2C%d: I2C Transmission Success\r\n",which_i2c), DEVICE_StatusType::DEVICE_SUCCESS;

	}

	/****************************************************************************************/
	/* Debugging */
	DEBUG_HandleType *HDEBUGX;

	/*func DEVICE_Debug_Set_Status                */
	/*? Manually enable or disable UART debugging */
	/*! Debugging is enabled by Default           */

	bool              isDebug{true};
	DEVICE_StatusType DEVICE_Debug_Set_Status(const bool status = true)
	{
		isDebug = status;
		return DEVICE_StatusType::DEVICE_SUCCESS;
	}

	char Debug_buffer[GENERAL_BUFFER_LENGTH]{0};

	public:

	template<typename... Args>
	DEVICE_StatusType Debug_Print(const char* format, Args... args)
	{
		if (!isDebug)return DEVICE_StatusType::DEVICE_SUCCESS;
		const int length = std::snprintf(Debug_buffer, GENERAL_BUFFER_LENGTH, format, +args...);
		if (length<0) return DEVICE_StatusType::DEVICE_FAILED;

		auto length_to_send = (length >= GENERAL_BUFFER_LENGTH) ?GENERAL_BUFFER_LENGTH - 1
																        :static_cast<size_t>(length);
		HAL_UART_Transmit(HDEBUGX,
						  reinterpret_cast<uint8_t*>(Debug_buffer),
						  static_cast<uint16_t>     (length_to_send),
						  GENERAL_TIMEOUT);
		//HAL_Delay(750);
		return DEVICE_StatusType::DEVICE_SUCCESS;
	}


	/*********************************************************************************************/
	/* SPI */
	/*********************************************************************************************/

	protected:

	uint8_t which_spi{0};
	SPI_HandleType *HSPIX;

	// GPIO mapping for SPI devices. PIN_CS is chip-select, PIN_RST is device reset,
	// PIN_DC is used on displays to distinguish command vs data (only relevant in 4-line mode).
	struct SPI_GPIO_T
	{
		port PORT   ;
		pin  PIN_CS ;
		pin  PIN_RST;
		pin  PIN_DC ; 
	}SPI_GPIO{};

	// Configure the GPIO pins used by the SPI peripheral for this device.
	// 'port' defaults to GPIOA when omitted.
	public:
	DEVICE_StatusType Set_SPI_GPIO(pin cs,pin dc,pin rst,port port = GPIOA)
	{
		SPI_GPIO.PORT	= port;
		SPI_GPIO.PIN_CS	= cs  ;
		SPI_GPIO.PIN_RST= rst ;
		SPI_GPIO.PIN_DC	= dc  ;
		return DEVICE_StatusType::DEVICE_SUCCESS;
	}
	protected:
	
	enum class SPI_Polarity:bool
	{
		ACTIVE_AT_LOW  = true, // Typical case
		ACTIVE_AT_HIGH = false 
	};

	enum class SPI_DataType:bool
	{
		COMMAND = false,
		DATA    = true
	};

	enum class SPI_Mode:uint8_t
	{
		FOUR_LINE  = 4, // RST,CS,DC
		THREE_LINE = 3  // RST,CS    
	};

	DEVICE_StatusType SPI_SendCore(byte   *data, 
					   SPI_DataType type,
					   uint16_t     length,                
				       SPI_Polarity polarity = SPI_Polarity::ACTIVE_AT_LOW,
					   SPI_Mode     mode     = SPI_Mode::    FOUR_LINE    )
	{
		if(mode == SPI_Mode::FOUR_LINE)
		{
			// Use DC pin to tell the device whether the following bytes are commands or data.
			Set_PinState(SPI_GPIO.PORT,SPI_GPIO.PIN_DC,
			             (type == SPI_DataType::DATA)?PINSTATE::HIGH
					                         :PINSTATE::LOW);
		}
		//Debug_Print("SPI%d: Sending %d bytes in %d line mode\r\n",which_spi,length,static_cast<uint8_t>(mode));

		// Assert chip-select using the configured active polarity.
		Set_PinState(SPI_GPIO.PORT,SPI_GPIO.PIN_CS,
				     (polarity == SPI_Polarity::ACTIVE_AT_LOW)?PINSTATE::LOW
					                                          :PINSTATE::HIGH);
					  
		auto status = HAL_SPI_Transmit(HSPIX,data,length,GENERAL_TIMEOUT);
		
		// Deassert chip-select (invert what we asserted earlier).
		Set_PinState(SPI_GPIO.PORT,SPI_GPIO.PIN_CS,
				     (polarity == SPI_Polarity::ACTIVE_AT_LOW)?PINSTATE::HIGH
					                                          :PINSTATE::LOW);
		
		 return DEVICE_StatusType::DEVICE_SUCCESS;

		//if (status != HAL_OK)return Debug_Print("SPI%d: SPI Transmission Failed\r\n",which_spi), DEVICE_StatusType::DEVICE_FAILED;
		//return Debug_Print("SPI%d: SPI Transmission Success\r\n",which_spi), DEVICE_StatusType::DEVICE_SUCCESS;
	}


	template<size_t N>
	DEVICE_StatusType SPI_Send(byte        (&data)[N],
							   SPI_DataType type,
							   uint16_t     length   = N,                 
						       SPI_Polarity polarity = SPI_Polarity::ACTIVE_AT_LOW,
							   SPI_Mode     mode     = SPI_Mode::    FOUR_LINE    )
	{
		return SPI_SendCore(data,type,length,polarity,mode);
	}

	DEVICE_StatusType SPI_Send(byte         data,
							   SPI_DataType type,
							   SPI_Polarity polarity = SPI_Polarity::ACTIVE_AT_LOW,
							   SPI_Mode     mode     = SPI_Mode::    FOUR_LINE    )
	{
		return SPI_SendCore(&data,type,1,polarity,mode);
	}

	/** If DMA is enabled **/

	inline static volatile bool isBusy{false};

	DEVICE_StatusType SPI_SendCore_DMA(byte        *data, 
					   				   SPI_DataType type,
					   				   uint16_t     length,                
				       			       SPI_Polarity polarity = SPI_Polarity::ACTIVE_AT_LOW,
					   			       SPI_Mode     mode     = SPI_Mode::    FOUR_LINE    )
	{
		using enum SPI_Polarity; 
		using enum PINSTATE; 
		using enum SPI_DataType;
		using enum SPI_Mode;
		Set_PinState(SPI_GPIO.PORT,SPI_GPIO.PIN_DC,(mode == FOUR_LINE)?(type == DATA)?HIGH:LOW:LOW);
		Set_PinState(SPI_GPIO.PORT,SPI_GPIO.PIN_CS,(polarity == ACTIVE_AT_LOW)?LOW:HIGH);
		isBusy = true;
		//while(HAL_SPI_GetState(HSPIX)==HAL_SPI_STATE_BUSY_TX){};
		HAL_SPI_Transmit_DMA(HSPIX,data,length);
		return DEVICE_StatusType::DEVICE_SUCCESS;
	}

	public:
	void SPI_On_DMAOver(SPI_Polarity polarity = SPI_Polarity::ACTIVE_AT_LOW)
	{
		using enum SPI_Polarity;
		using enum PINSTATE;
		Set_PinState(SPI_GPIO.PORT,SPI_GPIO.PIN_CS,
				     (polarity == ACTIVE_AT_LOW)?HIGH:LOW);
	}
	/****************************************************************************************/
	/* Delay */
	DEVICE_StatusType Delay_ms(uint32_t ms)
	{
		HAL_Delay(ms);
		return DEVICE_StatusType::DEVICE_SUCCESS;
	}
	/*****************************************************************************************/

	void foo()
	{
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_SET);
	}
};

/* END OF FILE */
