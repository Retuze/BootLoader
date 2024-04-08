#include "soft_spi.h"
/* CPOL = 0, CPHA = 0, MSB first */

#define SPI_MOSI(val) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, (GPIO_PinState)val)
#define SPI_CLK(val) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, (GPIO_PinState)val)
#define SPI_MISO() (uint8_t) HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6)

uint8_t spi_transfer(uint8_t txData)
{
	uint8_t rxData = 0;
	uint8_t i;

	for (i = 0; i < 8; i++)
	{
		SPI_CLK(0);
		delayMicroseconds(1);
		// 数据发送
		if (txData & 0x80)
		{
			SPI_MOSI(1);
		}
		else
		{
			SPI_MOSI(0);
		}
		txData <<= 1;
		delayMicroseconds(1);

		SPI_CLK(1);
		delayMicroseconds(1);
		// 数据接收
		rxData <<= 1;
		if (SPI_MISO())
		{
			rxData |= 0x01;
		}
		delayMicroseconds(1);
	}
	SPI_CLK(0);

	return rxData;
}