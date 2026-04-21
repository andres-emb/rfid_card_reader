#include "rc522_port.h"

/* Private define ------------------------------------------------------------*/
/* Set the global timeout of the transmit operation */
#define SPI_TRANSMIT_TIMEOUT_MS 10

/* Private variables ---------------------------------------------------------*/
/* Store the SPI handler */
static spi_device_t * spi_device;
/* Store the reset GPIO handler */
static reset_device_t * reset;

/* Public functions ---------------------------------------------------------*/

/**
  * @brief  Initialize the SPI and reset handlers
  * @param  spi_dev: SPI handler
  * 		rst: reset GPIO handler
  * @retval None
  */
void capture_handlers(spi_device_t * spi_dev, reset_device_t * rst)
{
	spi_device = spi_dev;
	reset = rst;
}

/**
  * @brief  Reset the card reader
  * @param  None
  * @retval None
  */
void reset_device(void)
{
	HAL_GPIO_WritePin(
		reset->reset_type,
		reset->reset_pin,
		GPIO_PIN_RESET
	);

	HAL_GPIO_WritePin(
		reset->reset_type,
		reset->reset_pin,
		GPIO_PIN_SET
	);
}

/**
  * @brief  Select the card reader (CS) to begin a SPI transaction
  * @param  None
  * @retval None
  */
void card_reader_select_device(void)
{
	HAL_GPIO_WritePin(
		spi_device->cs_type,
		spi_device->cs_pin,
		GPIO_PIN_RESET
	);
}

/**
  * @brief  Write a register using SPI
  * @param  value: value to be transfered
  * @retval None
  */
void card_reader_write_byte(const uint8_t value)
{
	HAL_SPI_Transmit(
		&spi_device->spi_handler,
		&value,
		1,
		SPI_TRANSMIT_TIMEOUT_MS
	);
}

/**
  * @brief  Un-select the card reader (CS) to end a SPI transaction
  * @param  None
  * @retval None
  */
void card_reader_unselect_device(void)
{
	HAL_GPIO_WritePin(
		spi_device->cs_type,
		spi_device->cs_pin,
		GPIO_PIN_SET
	);
}

/**
  * @brief  Read a register value using SPI
  * @param  reg: register to be transfered
  * @retval value: value of the register
  */
uint8_t card_reader_read_byte(const uint8_t reg)
{
	uint8_t value;
	card_reader_write_byte(0x80 | reg);
	uint8_t stop_value = 0x00;

	HAL_SPI_TransmitReceive(
		&spi_device->spi_handler,
		&stop_value,
		&value,
		1,
		SPI_TRANSMIT_TIMEOUT_MS
	);

	return value;
}

/**
  * @brief  Read multiple bytes from a register using SPI
  * @param  reg: register to be transfered
  * 		buffer: stores the read information}
  * 		size: bytes to be read
  * @retval None
  */
void card_reader_read_multiple_byte(const uint8_t reg, uint8_t * buffer, const uint8_t size)
{
	uint8_t reg_value = 0x80 | reg;
	uint8_t stop_value = 0x00;
	uint8_t read_value;

	HAL_SPI_Transmit(
		&spi_device->spi_handler,
		&reg_value,
		1,
		SPI_TRANSMIT_TIMEOUT_MS
	);

	uint8_t i = 0;

	while (i < size) {
		HAL_SPI_TransmitReceive(
			&spi_device->spi_handler,
			&reg_value,
			&read_value,
			1,
			SPI_TRANSMIT_TIMEOUT_MS
		);
		buffer[i] = read_value;
		i++;
	}

	HAL_SPI_TransmitReceive(
		&spi_device->spi_handler,
		&stop_value,
		&read_value,
		1,
		SPI_TRANSMIT_TIMEOUT_MS
	);

	buffer[i] = read_value;
}
