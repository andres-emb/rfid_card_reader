/* Includes ------------------------------------------------------------------*/
#include "rc522.h"
#include "rc522_port.h"
#include "API_delay.h"
#include "registers.h"
#include "commands.h"
#include "card_commands.h"
#include "lcd.h"

/* Private definitions ------------------------------------------------------*/
#define MAX_TRANSCEIVE_BUFFER_SIZE 		16
#define MAX_TRANSCEIVE_READ_ATTEMPTS 	32
#define IRQ_TRANSCEIVE_TIMEOUT 			0x01
#define IRQ_TRANSCEIVE_RX_RECEIVED 		0x30

#define RESET_REGISTER					0x00
#define RESET_MODULATION				0x26

#define AUTO_TIMEOUT					0x80
#define PRESCALER_FREQ_400_KHz			0xA9
#define RELOAD_TIMER_25_MS_H			0x03
#define RELOAD_TIMER_25_MS_L			0xE8
#define FORCE_100_ASK					0x40
#define CRC_MSB							0x3D
#define TX1_TX2_ENABLE					0x03
#define SET_ALL_INTERRUPTS				0x7F
#define FLUSH_FIFO_BUFFER				0x80
#define START_TRANSMISSION				0x80
#define CLEAR_MASK						0x80

#define VALID_BITS						7
#define RESET_DELAY_MS					100
#define REPORT_DELAY_MS					10000
#define CARD_SERIAL_NUMBER_BYTES		4

/* Private typedef -----------------------------------------------------------*/

/* Store the card_reader state */
typedef enum {
	INITIALIZE,
	SENSE_NEW_CARD,
	WAITING_SENSE_RESPONSE,
	SELECT_NEW_CARD,
	WAITING_SELECT_RESPONSE,
	REPORT_SERIAL_NUMBER,
	CARD_ERROR
} card_reader_state_t;

/* Store the result of the transceiver operation*/
typedef enum {
	TRANSCEIVE_OK,
	TRANSCEIVE_WAITING,
	TRANSCEIVE_ERROR,
	TRANSCEIVE_TIMEOUT
} transceive_status_t;

/* Store the response of the transceiver operation */
typedef struct {
	uint8_t buffer[MAX_TRANSCEIVE_BUFFER_SIZE];
	uint8_t size;
} transceive_request_t;

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Store the response of the transceiver operation */
static transceive_request_t select_response;
/* Store the card reader state */
static card_reader_state_t card_reader_state;
/* Store the delay used in transceive responses */
static delay_t transaction_delay;
/* Store the delay used in report operation */
static delay_t report_delay;

/* Private function prototypes -----------------------------------------------*/
static void write_register(const uint8_t reg, const uint8_t value);
static void write_register_multiple(
		const uint8_t reg,
		const uint8_t * buffer,
		const uint8_t size
);
static void clear_register(const uint8_t reg, const uint8_t mask);
static void card_reader_init(void);
static void card_reader_reset(void);
static void read_register_multiple(
		const uint8_t reg,
		uint8_t * buffer,
		const uint8_t size
);
static void look_for_new_card(void);

static uint8_t read_register(const uint8_t reg);

static transceive_status_t listen_to_new_card(void);
static transceive_status_t select_new_card(void);
static transceive_status_t listen_to_select_command(
		transceive_request_t * response
);
static transceive_status_t transceive_command(
		const transceive_request_t * request,
		const uint8_t valid_bits
);

static bool_t report_serial_number_to_lcd(transceive_request_t * request_response);

static void validate_transceive_request(const transceive_request_t * request);
static void validate_buffer(const uint8_t * buffer);

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Validate if the transceive_request_t pointer is not NULL
  * @param  None
  * @retval None
  */
static void validate_transceive_request(const transceive_request_t * request)
{
	if (request == NULL) error_handler();
}

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
  * @brief  Send a command to the card reader to communicate with the Smart Card
  * @param  request: struct with the buffer to send and the size of the buffer
  * @retval TRANSCEIVE_OK
  */
static transceive_status_t transceive_command(
		const transceive_request_t * request,
		const uint8_t valid_bits
)
{
	validate_transceive_request(request);

	uint8_t tx_last_bits = valid_bits;
	uint8_t bit_framming = tx_last_bits;

	write_register(COMMAND, CMD_IDLE);
	write_register(COM_IRQ, SET_ALL_INTERRUPTS);
	write_register(FIFO_LEVEL, FLUSH_FIFO_BUFFER);

	write_register_multiple(FIFO_DATA, request->buffer, request->size);

	write_register(BIT_FRAMMING, bit_framming);

	write_register(COMMAND, CMD_TRANSCEIVE);
	uint8_t current_framming = read_register(BIT_FRAMMING);

	write_register(BIT_FRAMMING, START_TRANSMISSION | current_framming);

	return TRANSCEIVE_OK;
}

/**
  * @brief  Clear the value of the desired register according with the mask
  * @param  reg: register to be modified
  * @param	mask: mask to be applied in the register value
  * @retval None
  */
static void clear_register(const uint8_t reg, const uint8_t mask)
{
	uint8_t masked_value = read_register(reg) & (~mask);
	write_register(reg, masked_value);
}

/**
  * @brief  Transceive a REQ_A command to activate Smart Cards in the near
  * 		field
  * @param  None
  * @retval None
  */
static void look_for_new_card(void)
{
	write_register(TX_MODE, RESET_REGISTER);
	write_register(RX_MODE, RESET_REGISTER);
	write_register(MOD_WIDTH, RESET_REGISTER);
	clear_register(COLLISION, CLEAR_MASK);

	transceive_request_t request;
	request.buffer[0] = SM_CMD_REQ_A;
	request.size = STANDARD_CMD_SIZE;

	transceive_command(&request, VALID_BITS);
}

/**
  * @brief  Wait for the response of the REQ_A command
  * 		field
  * @param  None
  * @retval TRANSCEIVE_OK: if the card reader receives a response from the Smart
  * 		Card
  *
  * 		TRANSCEIVE_ERROR: if the max attempts are reached, could indicate a
  * 		disconnection with the card reader
  *
  * 		TRANSCEIVE_TIMEOUT: if the interrupt bit of timeout has been
  * 		activated
  *
  * 		TRANSCEIVE_WAITING: if the card reader is waiting for the response
  * 		of the smart card
  */
static transceive_status_t listen_to_new_card(void)
{
	static uint8_t polling_attempts;

	if (polling_attempts == 0) {
		delayInit(&transaction_delay, 1);
		polling_attempts = 1;
	}

	if (polling_attempts > MAX_TRANSCEIVE_READ_ATTEMPTS) {
		polling_attempts = 0;
		return TRANSCEIVE_ERROR;
	}

	if (!delayRead(&transaction_delay)) {
		return TRANSCEIVE_WAITING;
	}

	polling_attempts++;

	uint8_t interruptions = read_register(COM_IRQ);
	if (interruptions & IRQ_TRANSCEIVE_RX_RECEIVED) {
		polling_attempts = 0;
		return TRANSCEIVE_OK;
	}

	if (interruptions & IRQ_TRANSCEIVE_TIMEOUT) {
		return TRANSCEIVE_TIMEOUT;
	}

	return TRANSCEIVE_WAITING;
}

/**
  * @brief  Initialize the card reader to be ready to send commands to the
  * 		smart card
  * @param  None
  * @retval None
  */
static void card_reader_init(void)
{
	card_reader_reset();

	write_register(TX_MODE, RESET_REGISTER);
	write_register(RX_MODE, RESET_REGISTER);
	write_register(MOD_WIDTH, RESET_MODULATION);
	write_register(T_MODE, AUTO_TIMEOUT);
	write_register(T_PRESCALER, PRESCALER_FREQ_400_KHz);
	write_register(T_RELOAD_H, RELOAD_TIMER_25_MS_H);
	write_register(T_RELOAD_L, RELOAD_TIMER_25_MS_L);
	write_register(TX_ASK, FORCE_100_ASK);
	write_register(MODE, CRC_MSB);

	// Enable antenna TX1 and TX2 pins
	uint8_t current_tx_behavior = read_register(TX_CONTROL);
	if ((current_tx_behavior & TX1_TX2_ENABLE) != TX1_TX2_ENABLE) {
		write_register(TX_CONTROL, current_tx_behavior | TX1_TX2_ENABLE);
	}
	card_reader_state = INITIALIZE;
}

/**
  * @brief  Reset the card reader by toggling the reset pin and
  * 		executing the CMD_SOFT_RESET command
  * @param  None
  * @retval None
  */
static void card_reader_reset(void)
{
	reset_device();
	write_register(COMMAND, CMD_SOFT_RESET);
	HAL_Delay(RESET_DELAY_MS);
}

/**
  * @brief  Modify the desired register with one byte
  * @param  reg: register to be updated
  * 		value: value to be written
  * @retval None
  */
static void write_register(const uint8_t reg, const uint8_t value)
{
	card_reader_select_device();
	card_reader_write_byte(reg);
	card_reader_write_byte(value);
	card_reader_unselect_device();
}

/**
  * @brief  Modify the desired register with multiple bytes
  * @param  reg: register to be updated
  * 		buffer: array with the desired bytes to be written
  * 		size: size of the buffer
  * @retval None
  */
static void write_register_multiple(const uint8_t reg, const uint8_t * buffer, const uint8_t size)
{
	validate_buffer(buffer);

	card_reader_select_device();
	card_reader_write_byte(reg);

	for (int i = 0; i < size; i++) {
		card_reader_write_byte(buffer[i]);
	}

	card_reader_unselect_device();
}

/**
  * @brief  Read one byte of the desired register
  * @param  reg: register to be read
  * @retval value: the value of the register
  */
static uint8_t read_register(const uint8_t reg)
{
	card_reader_select_device();
	uint8_t value = card_reader_read_byte(reg);
	card_reader_unselect_device();
	return value;
}

/**
  * @brief  Read multiple bytes of the desired register
  * @param  reg: register to be read
  * 		buffer: buffer to store the readed bytes
  * 		size: number of bytes to be read
  * @retval None
  */
static void read_register_multiple(const uint8_t reg, uint8_t * buffer, const uint8_t size)
{
	validate_buffer(buffer);

	card_reader_select_device();
	card_reader_read_multiple_byte(reg, buffer, size);
	card_reader_select_device();
}

/**
  * @brief  Send the select command to read the Smart Card serial number
  * 		once it has discovered a new card
  * @param  None
  * @retval None
  */
static transceive_status_t select_new_card(void)
{
	clear_register(COLLISION, CLEAR_MASK);

	transceive_request_t request;

	request.buffer[0] = SM_CMD_SELECT_CL1;
	request.buffer[1] = 0x20;
	request.size = 2;

	uint8_t valid_bits = 0;

	write_register(BIT_FRAMMING, 0x00);

	transceive_command(&request, valid_bits);

	return TRANSCEIVE_OK;
}

/**
  * @brief  Constant poll of the card reader to wait for the select command
  * 		response
  *
  * @param  response: stores the response of the select command
  * @retval TRANSCEIVE_OK: if the card reader receives a response from the Smart
  * 		Card
  *
  * 		TRANSCEIVE_ERROR: if the max attempts are reached, could indicate a
  * 		disconnection with the card reader
  *
  * 		TRANSCEIVE_TIMEOUT: if the interrupt bit of timeout has been
  * 		activated
  *
  * 		TRANSCEIVE_WAITING: if the card reader is waiting for the response
  * 		of the smart card
  */
static transceive_status_t listen_to_select_command(transceive_request_t * response)
{
	validate_transceive_request(response);

	static uint8_t polling_attempts;

	if (polling_attempts == 0) {
		delayInit(&transaction_delay, 1);
		polling_attempts = 1;
	}

	if (polling_attempts > MAX_TRANSCEIVE_READ_ATTEMPTS) {
		polling_attempts = 0;
		return TRANSCEIVE_ERROR;
	}

	if (!delayRead(&transaction_delay)) {
		return TRANSCEIVE_WAITING;
	}

	polling_attempts++;

	uint8_t interruptions = read_register(COM_IRQ);
	if (interruptions & IRQ_TRANSCEIVE_RX_RECEIVED) {

		uint8_t received_bytes = read_register(FIFO_LEVEL);

		response->size = received_bytes;
		read_register_multiple(FIFO_DATA, response->buffer, response->size);

		polling_attempts = 0;
		return TRANSCEIVE_OK;
	}

	if (interruptions & IRQ_TRANSCEIVE_TIMEOUT) {
		return TRANSCEIVE_TIMEOUT;
	}

	return TRANSCEIVE_WAITING;
}

/**
  * @brief  Once it was received the response of the select command, send the
  * 		serial number to be displayed on the LCD for 10 seconds
  * @param  select_response: struct with the response of the select_command
  * 		includes the serial number of the Smart Card
  * 		rst: GPIO pin to control the reset of the card reader
  * @retval True once the 10 seconds has been reached
  */
static bool_t report_serial_number_to_lcd(transceive_request_t * select_response)
{
	validate_transceive_request(select_response);

	static bool_t reported = false;

	if (!reported) {
		delayInit(&report_delay, REPORT_DELAY_MS);
		lcd_report_serial_number(select_response->buffer, CARD_SERIAL_NUMBER_BYTES);
		reported = true;
	}

	if (delayRead(&report_delay)) {
		reported = false;
		lcd_clear_screen();
		return true;
	} else {
		return false;
	}
}

/* Public functions ---------------------------------------------------------*/

/**
  * @brief  Reset the card reader state and capture the handlers of the
  * 		SPI communication
  * @param  spi_dev: structure to the SPI handler
  * 		rst: GPIO pin to control the reset of the card reader
  * @retval True
  */
bool_t card_reader_initialize(spi_device_t * spi_dev, reset_device_t * rst)
{
	if (spi_dev == NULL || rst == NULL) error_handler();
	capture_handlers(spi_dev, rst);
	card_reader_state = INITIALIZE;
	return true;
}

/**
  * @brief  Based on the card reader state update the FSM
  * @param  None
  * @retval None
  */
void card_reader_poll()
{
	switch(card_reader_state) {
	case INITIALIZE:
		card_reader_init();
		card_reader_state = SENSE_NEW_CARD;
		break;
	case SENSE_NEW_CARD:
		look_for_new_card();
		card_reader_state = WAITING_SENSE_RESPONSE;
		break;
	case WAITING_SENSE_RESPONSE:
		transceive_status_t listen_result = listen_to_new_card();
		switch(listen_result) {
		case TRANSCEIVE_OK:
			card_reader_state = SELECT_NEW_CARD;
			break;
		case TRANSCEIVE_TIMEOUT:
			card_reader_state = SENSE_NEW_CARD;
			break;
		case TRANSCEIVE_WAITING:
			card_reader_state = WAITING_SENSE_RESPONSE;
			break;
		case TRANSCEIVE_ERROR:
		default:
			card_reader_state = CARD_ERROR;
			break;
		}
		break;
	case SELECT_NEW_CARD:
		select_new_card();
		card_reader_state = WAITING_SELECT_RESPONSE;
		break;
	case WAITING_SELECT_RESPONSE:
		select_response.size = 5;
		transceive_status_t select_result = listen_to_select_command(&select_response);
		switch(select_result) {
		case TRANSCEIVE_OK:
			card_reader_state = REPORT_SERIAL_NUMBER;
			break;
		case TRANSCEIVE_TIMEOUT:
			card_reader_state = SENSE_NEW_CARD;
			break;
		case TRANSCEIVE_WAITING:
			card_reader_state = WAITING_SELECT_RESPONSE;
			break;
		case TRANSCEIVE_ERROR:
		default:
			card_reader_state = CARD_ERROR;
			break;
		}
		break;
	case REPORT_SERIAL_NUMBER:
		if (report_serial_number_to_lcd(&select_response)) {
			card_reader_state = SENSE_NEW_CARD;
		} else {
			card_reader_state = REPORT_SERIAL_NUMBER;
		}
		break;
	case CARD_ERROR:
	default:
		card_reader_init();
		break;
	}
}
