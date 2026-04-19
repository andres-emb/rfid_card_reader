#include "rc522_port.h"

#define SPI_TRANSMIT_TIMEOUT_MS 10

static spi_device_t * spi_device;
static reset_device_t * reset;

void capture_handlers(spi_device_t * spi_dev, reset_device_t * rst)
{
	spi_device = spi_dev;
	reset = rst;
}

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

void card_reader_select_device(void)
{
	HAL_GPIO_WritePin(
		spi_device->cs_type,
		spi_device->cs_pin,
		GPIO_PIN_RESET
	);
}

void card_reader_write_byte(const uint8_t value)
{
	HAL_SPI_Transmit(
		&spi_device->spi_handler,
		&value,
		1,
		SPI_TRANSMIT_TIMEOUT_MS
	);
}

void card_reader_unselect_device(void)
{
	HAL_GPIO_WritePin(
		spi_device->cs_type,
		spi_device->cs_pin,
		GPIO_PIN_SET
	);
}

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

void card_reader_read_multiple_byte(const uint8_t reg, const uint8_t count, uint8_t * buffer)
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

	int i = 0;

	while (i < count) {
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
