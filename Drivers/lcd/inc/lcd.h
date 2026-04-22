#ifndef __LDC_H
#define __LCD_H
/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include "lcd_port.h"

/* Exported definitions ------------------------------------------------------*/
#define CMD_INIT1 				0x30
#define CMD_INIT2 				0x20

#define CMD_4_BIT_MODE 			0x28
#define CMD_DISPLAY_ON 			(0x01 << 2)
#define CMD_DISPLAY_CONTROL		(0x01 << 3)
#define CMD_RETURN_HOME			(0x01 << 1)
#define CMD_ENTRY_MODE			(0x01 << 2)
#define CMD_AUTO_INCREMENT		(0x01 << 1)
#define CMD_CURSOR_ON 			(0x01 << 1)
#define CMD_CURSOR_BLINK 		0x01
#define CMD_AUTOINCREMENT 		(0x01 << 1)
#define CMD_CLR_LCD				0x01

#define HIGH_NIBBLE_MASK		0xF0
#define LOW_NIBBLE_MASK			0x0F
#define LOW_NIBBLE_SHIFT		4

#define ENABLE					(0x01 << 2)
#define BACK_LIGHT				(0x01 << 3)

#define DATA_MASK				0x01
#define CONTROL_MASK			0x00

#define HIGH_LINE				0x80
#define LOW_LINE				0xC0

/* Exported functions prototypes ---------------------------------------------*/
void lcd_initialize(I2C_HandleTypeDef * i2c_handler);
void lcd_send_data(uint8_t data);
void set_high_line_position(uint8_t position);

void lcd_push_byte_data(const uint8_t data);
void lcd_push_byte_control(const uint8_t data);
void lcd_push_byte_data_array(const uint8_t * buffer, const uint8_t size);

void lcd_clear_screen(void);

void lcd_report_serial_number(const uint8_t * buffer, const uint8_t size);

void lcd_poll(void);


#endif
