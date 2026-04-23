#ifndef __LDC_H
#define __LCD_H
/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include "lcd_port.h"

/* Exported functions prototypes ---------------------------------------------*/
void lcd_initialize(I2C_HandleTypeDef * i2c_handler);
void lcd_clear_screen(void);
void lcd_report_serial_number(const uint8_t * buffer, const uint8_t size);
void lcd_poll(void);

#endif
