/* Includes ------------------------------------------------------------------*/
#include "lcd_port.h"

/* Private define ------------------------------------------------------------*/
/* LCD I2C addres */
#define LCD_ADDRESS 0x27
/* Set the global timeout of the transmit operation */
#define I2C_TRANSMIT_TIMEOUT_MS 10

/* Private variables ------------------------------------------------------------*/
/* I2C handler */
static I2C_HandleTypeDef * i2c_handler;

/* Public functions ---------------------------------------------------------*/
/**
  * @brief  Capture the I2C handler reference to sent data
  * @param  i2c_dev: pointer to the I2C handler
  * @retval None
  */
void capture_i2c_handlers(I2C_HandleTypeDef * i2c_dev)
{
	i2c_handler = i2c_dev;
}

/**
  * @brief  Send a byte to the LCD using I2C
  * @param  i2c_dev: pointer to the I2C handler
  * @retval None
  */
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
