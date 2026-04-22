/* Includes ------------------------------------------------------------------*/
#include "lcd_port.h"

static I2C_HandleTypeDef * i2c_handler;

/* Private define ------------------------------------------------------------*/
/* Set the global timeout of the transmit operation */
#define I2C_TRANSMIT_TIMEOUT_MS 10

/* Public functions ---------------------------------------------------------*/
void capture_i2c_handlers(I2C_HandleTypeDef * i2c_dev)
{
	i2c_handler = i2c_dev;
}

void lcd_send_byte(uint8_t value)
{
	HAL_I2C_Master_Transmit(
		i2c_handler,
		LCD_ADDRESS << 1,
		&value,
		sizeof(value),
		I2C_TRANSMIT_TIMEOUT_MS
	);
}

