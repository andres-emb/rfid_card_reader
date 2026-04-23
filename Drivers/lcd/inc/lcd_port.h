#ifndef __LCD_PORT_H
#define __LCD_PORT_H
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Exported functions prototypes ---------------------------------------------*/
void capture_i2c_handlers(I2C_HandleTypeDef * i2c_dev);
void lcd_send_byte(uint8_t value);

#endif
