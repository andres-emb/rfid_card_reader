/* Includes ------------------------------------------------------------------*/
#include "lcd.h"
#include "string.h"
#include "API_delay.h"

#define MAX_QUEUE_SIZE 256

typedef struct {
	uint8_t buffer[MAX_QUEUE_SIZE];
	uint8_t head;
	uint8_t tail;
	uint8_t size;
} lcd_transmit_queue_t;

typedef enum {
	CHECK_QUEUE_SIZE,
	SEND_DATA,
	WAIT,
} lcd_transmit_state_t;

static lcd_transmit_state_t transmit_state;

static lcd_transmit_queue_t transmit_queue;

static delay_t delay;

/* Private functions prototypes ----------------------------------------------*/
static void send_half_byte_control(const uint8_t value);
static void send_byte_control(const uint8_t value);

static void send_half_byte_data(const uint8_t value);
static void send_byte_data(const uint8_t value);

static void transmit_queue_push_byte(const uint8_t data);
static uint8_t transmit_queue_pop_byte(void);

/* Private functions ---------------------------------------------------------*/
static void send_half_byte_control(const uint8_t value)
{
	lcd_send_byte(value | CONTROL_MASK | ENABLE | BACK_LIGHT);
	HAL_Delay(1);
	lcd_send_byte(value | CONTROL_MASK | BACK_LIGHT);
	HAL_Delay(1);
}

static void send_byte_control(const uint8_t value)
{
	send_half_byte_control(value & HIGH_NIBBLE_MASK);
	send_half_byte_control((value & LOW_NIBBLE_MASK) << LOW_NIBBLE_SHIFT);
}

static void send_half_byte_data(const uint8_t value)
{
	lcd_send_byte(value | DATA_MASK | ENABLE | BACK_LIGHT);
	HAL_Delay(1);
	lcd_send_byte(value | DATA_MASK | BACK_LIGHT);
	HAL_Delay(1);
}

static void send_byte_data(const uint8_t value)
{
	send_half_byte_data(value & HIGH_NIBBLE_MASK);
	send_half_byte_data((value & LOW_NIBBLE_MASK) << LOW_NIBBLE_SHIFT);
}

/* Public functions ----------------------------------------------------------*/
void lcd_initialize(I2C_HandleTypeDef * i2c_handler)
{
	capture_i2c_handlers(i2c_handler);
	send_half_byte_control(CMD_INIT1);
	HAL_Delay(10);
	send_half_byte_control(CMD_INIT1);
	HAL_Delay(1);
	send_half_byte_control(CMD_INIT1);
	send_half_byte_control(CMD_INIT2);

	send_byte_control(CMD_4_BIT_MODE);
	send_byte_control(CMD_DISPLAY_CONTROL);
	send_byte_control(CMD_RETURN_HOME);
	send_byte_control(CMD_ENTRY_MODE + CMD_AUTOINCREMENT);
	send_byte_control(CMD_DISPLAY_CONTROL + CMD_DISPLAY_ON);
	send_byte_control(CMD_CLR_LCD);

	send_byte_control(CMD_DISPLAY_CONTROL | CMD_CURSOR_ON | CMD_DISPLAY_ON | CMD_CURSOR_BLINK);

	transmit_state = CHECK_QUEUE_SIZE;
	transmit_queue.head = 0;
	transmit_queue.tail = 0;
	transmit_queue.size = 0;

	delayInit(&delay, 1);
}

void lcd_send_data(uint8_t data)
{
	send_byte_data(data);
}

void set_high_line_position(uint8_t position)
{

	send_byte_control(position | HIGH_LINE);

	/*
	uint8_t string[] = {(uint8_t)"H",(uint8_t)"O",(uint8_t)"L",(uint8_t)"A"};
	for (int i = 0; i < 4; i++) {
		send_byte_control(string[i] | DATA);
	}
	*/
}

static uint8_t transmit_queue_pop_byte(void) {
	uint8_t data = 0x00;
	if (transmit_queue.size > 0) {
		data = transmit_queue.buffer[transmit_queue.head];
		transmit_queue.head++;
		transmit_queue.size--;

		if (transmit_queue.head % MAX_QUEUE_SIZE == 0) {
			transmit_queue.head = 0;
		}

	} else {
		// HANDLE ERROR
	}

	return data;
}

static void transmit_queue_push_byte(const uint8_t data) {
	if (transmit_queue.size < MAX_QUEUE_SIZE - 1) {
		transmit_queue.buffer[transmit_queue.tail] = data;
		transmit_queue.tail++;
		transmit_queue.size++;


		if (transmit_queue.tail % MAX_QUEUE_SIZE == 0) {
			transmit_queue.tail = 0;
		}
	} else {
		// HANDLE ERROR
	}
}

void lcd_push_byte(const uint8_t data, const uint8_t type_mask) {
	uint8_t high_nibble = (data & HIGH_NIBBLE_MASK);
	transmit_queue_push_byte(high_nibble | type_mask  | ENABLE | BACK_LIGHT);
	transmit_queue_push_byte(high_nibble | type_mask | BACK_LIGHT);

	uint8_t low_nibble = (data & LOW_NIBBLE_MASK) << LOW_NIBBLE_SHIFT;
	transmit_queue_push_byte(low_nibble | type_mask  | ENABLE | BACK_LIGHT);
	transmit_queue_push_byte(low_nibble | type_mask | BACK_LIGHT);
}

void lcd_push_byte_data_array(const uint8_t * buffer, const uint8_t size)
{
	for (int i = 0; i < size; i++) {
		lcd_push_byte_data(buffer[i]);
	}
}

void lcd_push_byte_data(const uint8_t data) {
	lcd_push_byte(data, DATA_MASK);
}

void lcd_push_byte_control(const uint8_t data) {
	lcd_push_byte(data, CONTROL_MASK);
}

void lcd_clear_screen(void) {
	lcd_push_byte_control(CMD_CLR_LCD);
}

void lcd_reset_high_row_position(void)
{
	lcd_push_byte_control(HIGH_LINE);
}

void lcd_reset_low_row_position(void)
{
	lcd_push_byte_control(LOW_LINE);
}

void lcd_push_byte_hex_data_array(const uint8_t * buffer, const uint8_t size) {
	for (int i = 0; i < size; i++) {
		uint8_t data = buffer[i];
		uint8_t high_data = (data & HIGH_NIBBLE_MASK) >> LOW_NIBBLE_SHIFT;
		uint8_t low_data = (data & LOW_NIBBLE_MASK);

		uint8_t ascii_offset = (high_data >= 0x0A && high_data <= 0x0F) ? 0x37 : 0x30;
		lcd_push_byte_data(high_data + ascii_offset);

		ascii_offset = (low_data >= 0x0A && low_data <= 0x0F) ? 0x37 : 0x30;
		lcd_push_byte_data(low_data + ascii_offset);
	}
}

void lcd_report_serial_number(const uint8_t * buffer, const uint8_t size) {

	lcd_clear_screen();
	lcd_reset_high_row_position();
	uint8_t serial_number[] =  "Serial Number:";
	lcd_push_byte_data_array(serial_number, strlen((char *)serial_number));

	lcd_reset_low_row_position();
	lcd_push_byte_hex_data_array(buffer, size);

}

void lcd_poll() {
	switch(transmit_state) {
	case CHECK_QUEUE_SIZE:
		if (transmit_queue.size > 0) {
			transmit_state = SEND_DATA;
		}
		break;
	case SEND_DATA:
	{
		uint8_t data = transmit_queue_pop_byte();
		lcd_send_byte(data);
		transmit_state = WAIT;
		break;
	}
	case WAIT:
		if (delayRead(&delay)) {
			transmit_state = transmit_queue.size > 0 ? SEND_DATA : CHECK_QUEUE_SIZE;
		}
		break;
	default:
		transmit_state = CHECK_QUEUE_SIZE;
		break;
	}
}
