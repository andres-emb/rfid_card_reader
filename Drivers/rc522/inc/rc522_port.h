#ifndef __RC522_PORT_H
#define __RC522_PORT_H
#include "stm32f4xx_hal.h"

typedef struct {
	SPI_HandleTypeDef spi_handler;
	GPIO_TypeDef * cs_type;
	uint16_t cs_pin;
} spi_device_t;

typedef struct {
	GPIO_TypeDef * reset_type;
	uint16_t reset_pin;
} reset_device_t;

void capture_handlers(spi_device_t * spi_dev, reset_device_t * reset_dev);
void card_reader_select_device(void);
void card_reader_write_byte(const uint8_t value);
uint8_t card_reader_read_byte(const uint8_t reg);
void card_reader_unselect_device(void);

void card_reader_read_multiple_byte(const uint8_t reg, const uint8_t count, uint8_t * buffer);


void reset_device(void);

#endif
