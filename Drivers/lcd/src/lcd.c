/* Includes ------------------------------------------------------------------*/
#include "lcd.h"
#include "string.h"
#include "API_delay.h"
#include "common.h"

/* Private defines -----------------------------------------------------------*/

/* Defines the max values to store in the queue to send data to the lcd */
#define MAX_QUEUE_SIZE 			256

/* Initialize commands */
#define CMD_INIT1 				0x30
#define CMD_INIT2 				0x20

/* Configuration commands */
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
#define SET_HIGH_LINE_ORIGIN	0x80
#define SET_LOW_LINE_ORIGIN		0xC0
#define ENABLE					(0x01 << 2)
#define BACK_LIGHT				(0x01 << 3)

/* Useful mask definitions */
#define DATA_MASK				0x01
#define CONTROL_MASK			0x00
#define HIGH_NIBBLE_MASK		0xF0
#define LOW_NIBBLE_MASK			0x0F
#define LOW_NIBBLE_SHIFT		4

/* Delay definitions */
#define DATA_TRANSMIT_DELAY_MS	1
#define CMD_TRANSMIT_DELAY_MS	10

/* HEX to ASCII translations */
#define HIGH_DIGIT_HEX_OFFSET	0x37
#define LOW_DIGIT_HEX_OFFSET	0x30

/* Private typedef -----------------------------------------------------------*/

/* Circular queue to store the data the user want to sent to the lcd */
typedef struct {
	uint8_t buffer[MAX_QUEUE_SIZE];
	uint8_t head;
	uint8_t tail;
	uint8_t size;
} lcd_transmit_queue_t;

/* FSM to handle the circular queue and sent data to the lcd */
typedef enum {
	CHECK_QUEUE_SIZE,
	SEND_DATA,
	WAIT,
} lcd_transmit_state_t;

/* Private variables ---------------------------------------------------------*/

/* Track the state of the FSM */
static lcd_transmit_state_t transmit_state;

/* Handle the circular queue */
static lcd_transmit_queue_t transmit_queue;

/* Handle delay between data transmission to the lcd */
static delay_t delay;

/* Private functions prototypes ----------------------------------------------*/
static void send_half_byte_control(const uint8_t value);
static void send_byte_control(const uint8_t value);

static void transmit_queue_push_byte(const uint8_t data);
static uint8_t transmit_queue_pop_byte(void);

static void lcd_push_byte_data_array(const uint8_t * buffer, const uint8_t size);
static void lcd_push_byte_data(const uint8_t data);
static void lcd_push_byte_control(const uint8_t data);

static void validate_buffer(const uint8_t * buffer);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Validate if the buffer pointer is not NULL
  * @param  None
  * @retval None
  */
static void validate_buffer(const uint8_t * buffer)
{
	if (buffer == NULL) error_handler();
}

/**
  * @brief  Send 4 control bits to the lcd
  * @param  value: data to be transmitted to the lcd
  * @retval None
  */
static void send_half_byte_control(const uint8_t value)
{
	/* Code taken from the LCD example of the Communication Protocols in Embedded
	 * Systems class */
	lcd_send_byte(value | CONTROL_MASK | ENABLE | BACK_LIGHT);
	HAL_Delay(1);
	lcd_send_byte(value | CONTROL_MASK | BACK_LIGHT);
	HAL_Delay(1);
}

/**
  * @brief  Send a control byte to the lcd, it is required to separate the byte
  * 		in nibbles in order to communicate with the lcd
  * @param  value: data to be transmitted to the lcd
  * @retval None
  */
static void send_byte_control(const uint8_t value)
{
	/* Code taken from the LCD example of the Communication Protocols in Embedded
	 * Systems class */
	send_half_byte_control(value & HIGH_NIBBLE_MASK);
	send_half_byte_control((value & LOW_NIBBLE_MASK) << LOW_NIBBLE_SHIFT);
}


/**
  * @brief  Get a byte from the transmission queue
  * @param  None
  * @retval data: byte from the transmission queue
  */
static uint8_t transmit_queue_pop_byte(void) {
	uint8_t data;
	if (transmit_queue.size > 0) {
		data = transmit_queue.buffer[transmit_queue.head];
		transmit_queue.head++;
		transmit_queue.size--;

		if (transmit_queue.head % MAX_QUEUE_SIZE == 0) {
			transmit_queue.head = 0;
		}

	} else {
		error_handler();
	}

	return data;
}

/**
  * @brief  Push a byte into the transmission queue
  * @param  data: byte to be added
  * @retval None
  */
static void transmit_queue_push_byte(const uint8_t data) {
	if (transmit_queue.size < MAX_QUEUE_SIZE - 1) {
		transmit_queue.buffer[transmit_queue.tail] = data;
		transmit_queue.tail++;
		transmit_queue.size++;

		if (transmit_queue.tail % MAX_QUEUE_SIZE == 0) {
			transmit_queue.tail = 0;
		}
	} else {
		error_handler();
	}
}

/**
  * @brief  Push a byte into the transmit queue.
  * @param  data: byte to be sent
  * 		type_mask: defines if the byte is a CONTROL or DATA byte
  * @retval None
  */
static void lcd_push_byte(const uint8_t data, const uint8_t type_mask) {
	uint8_t high_nibble = (data & HIGH_NIBBLE_MASK);
	transmit_queue_push_byte(high_nibble | type_mask  | ENABLE | BACK_LIGHT);
	transmit_queue_push_byte(high_nibble | type_mask | BACK_LIGHT);

	uint8_t low_nibble = (data & LOW_NIBBLE_MASK) << LOW_NIBBLE_SHIFT;
	transmit_queue_push_byte(low_nibble | type_mask  | ENABLE | BACK_LIGHT);
	transmit_queue_push_byte(low_nibble | type_mask | BACK_LIGHT);
}

/**
  * @brief  Push a data byte array into the transmit queue.
  * @param  buffer: contains the array to be pushed
  * 		size: size of the buffer
  * @retval None
  */
static void lcd_push_byte_data_array(const uint8_t * buffer, const uint8_t size)
{
	validate_buffer(buffer);
	for (int i = 0; i < size; i++) {
		lcd_push_byte_data(buffer[i]);
	}
}

/**
  * @brief  Push a data byte into the transmit queue.
  * @param  data: data byte to be pushed
  * @retval None
  */
static void lcd_push_byte_data(const uint8_t data) {
	lcd_push_byte(data, DATA_MASK);
}

/**
  * @brief  Push a control byte into the transmit queue.
  * @param  data: control byte to be pushed
  * @retval None
  */
static void lcd_push_byte_control(const uint8_t data) {
	lcd_push_byte(data, CONTROL_MASK);
}

/**
  * @brief  Push the set high row position 0 to the transmit queue.
  * @param  None
  * @retval None
  */
static void lcd_reset_high_row_position(void)
{
	lcd_push_byte_control(SET_HIGH_LINE_ORIGIN);
}

/**
  * @brief  Push the set low row position 0 to the transmit queue.
  * @param  None
  * @retval None
  */
static void lcd_reset_low_row_position(void)
{
	lcd_push_byte_control(SET_LOW_LINE_ORIGIN);
}

/**
  * @brief  Push an array of hexadecimal data to the transmit queue.
  * @param  buffer: contains the hexadecimal data
  * 		size: size of the buffer
  * @retval None
  */
static void lcd_push_byte_hex_data_array(const uint8_t * buffer, const uint8_t size) {
	validate_buffer(buffer);
	for (int i = 0; i < size; i++) {
		uint8_t data = buffer[i];
		uint8_t high_data = (data & HIGH_NIBBLE_MASK) >> LOW_NIBBLE_SHIFT;
		uint8_t low_data = (data & LOW_NIBBLE_MASK);

		/* In order to represent the hexadecimal digits (A to F)  it is
		 * required to apply different offset than the decimal bits, otherwise
		 * the higher digits (A to F) would be translated to a wrong ASCII representation
		 */

		uint8_t ascii_offset = (high_data >= 0x0A && high_data <= 0x0F) ? HIGH_DIGIT_HEX_OFFSET : LOW_DIGIT_HEX_OFFSET;
		lcd_push_byte_data(high_data + ascii_offset);

		ascii_offset = (low_data >= 0x0A && low_data <= 0x0F) ? HIGH_DIGIT_HEX_OFFSET : LOW_DIGIT_HEX_OFFSET;
		lcd_push_byte_data(low_data + ascii_offset);
	}
}

/* Public functions ----------------------------------------------------------*/

/**
  * @brief  Send the proper control bytes to the lcd to be ready to receive data.
  * 		This function operates in blocking mode.
  * @param  i2c_handler: pointer to the I2C handler in order to sent the data
  * @retval None
  */
void lcd_initialize(I2C_HandleTypeDef * i2c_handler)
{
	if (i2c_handler == NULL) error_handler();

	capture_i2c_handlers(i2c_handler);

	/* Code taken from the LCD example of the Communication Protocols in Embedded
	 * Systems class */
	send_half_byte_control(CMD_INIT1);
	HAL_Delay(CMD_TRANSMIT_DELAY_MS);
	send_half_byte_control(CMD_INIT1);
	HAL_Delay(DATA_TRANSMIT_DELAY_MS);
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

	delayInit(&delay, DATA_TRANSMIT_DELAY_MS);
}


/**
  * @brief  Push the clear screen command to the transmit queue.
  * @param  None
  * @retval None
  */
void lcd_clear_screen(void) {
	lcd_push_byte_control(CMD_CLR_LCD);
}

/**
  * @brief  Report the serial number of a Smart Card into the screen
  * @param  buffer: contains the serial number of the Smart Card
  * 		size: size of the buffer
  * @retval None
  */
void lcd_report_serial_number(const uint8_t * buffer, const uint8_t size) {
	validate_buffer(buffer);

	lcd_clear_screen();
	lcd_reset_high_row_position();
	uint8_t serial_number[] =  "Serial Number:";
	lcd_push_byte_data_array(serial_number, strlen((char *)serial_number));

	lcd_reset_low_row_position();
	lcd_push_byte_hex_data_array(buffer, size);

}

/**
  * @brief  Update the state of the lcd transmit queue
  * @param  None
  * @retval None
  */
void lcd_poll() {
	switch(transmit_state) {
	/* Initial state */
	case CHECK_QUEUE_SIZE:
		if (transmit_queue.size > 0) {
			/* Update the state to send the data in the transmit queue */
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
		/* Delay required between byte transmissions so the data can be
		 * correctly interpreted by the lcd*/
		if (delayRead(&delay)) {
			transmit_state = transmit_queue.size > 0 ? SEND_DATA : CHECK_QUEUE_SIZE;
		}
		break;
	default:
		transmit_state = CHECK_QUEUE_SIZE;
		break;
	}
}
