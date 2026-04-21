#ifndef __RC522_PORT_H
#define __RC522_PORT_H
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Exported typedefs ---------------------------------------------------------*/

/* SPI device handler*/
typedef struct {
	SPI_HandleTypeDef spi_handler;
	GPIO_TypeDef * cs_type;
	uint16_t cs_pin;
} spi_device_t;

/* Reset device handler*/
typedef struct {
	GPIO_TypeDef * reset_type;
	uint16_t reset_pin;
} reset_device_t;

/* Exported functions prototypes ---------------------------------------------*/
void capture_handlers(spi_device_t * spi_dev, reset_device_t * reset_dev);
void card_reader_select_device(void);
void card_reader_unselect_device(void);
void card_reader_write_byte(const uint8_t value);
uint8_t card_reader_read_byte(const uint8_t reg);
void card_reader_read_multiple_byte(const uint8_t reg, uint8_t * buffer, const uint8_t size);
void reset_device(void);

#endif
