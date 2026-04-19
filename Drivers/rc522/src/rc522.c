#include "rc522.h"
#include "rc522_port.h"
#include "API_delay.h"
#include "registers.h"
#include "commands.h"
#include "card_commands.h"

#define MAX_TRANSCEIVE_BUFFER_SIZE 64

typedef enum {
	INITIALIZE,
	SENSE_NEW_CARD,
	WAITING_SENSE_RESPONSE,
	RESPONSE_TIMED_OUT,
	CARD_ERROR
} card_reader_state_t;

typedef enum {
	TRANSCEIVE_OK,
	TRANSCEIVE_WAITING,
	TRANSCEIVE_ERROR,
	TRANSCEIVE_TIMEOUT
} transceive_status_t;

typedef struct {
	uint8_t buffer[MAX_TRANSCEIVE_BUFFER_SIZE];
	uint8_t size;
} transceive_response_t;

static card_reader_state_t card_reader_state;
static void write_register(const uint8_t reg, const uint8_t value);
static void write_register_multiple(const uint8_t reg, const uint8_t count, const uint8_t * data);
static void clear_register(const uint8_t reg, const uint8_t mask);
static void card_reader_init(void);
static void card_reader_reset(void);
static uint8_t read_register(const uint8_t reg);
static void read_register_multiple(const uint8_t reg, const uint8_t count, uint8_t *values, const uint8_t byte_aling);

static void look_for_new_card(void);
static transceive_status_t listen_to_new_card(void);

static void turn_on_antenna(void);

static transceive_status_t transceive_command(
		const uint8_t smart_card_command,
		const uint8_t valid_bits
);


static transceive_status_t transceive_command(
		const uint8_t smart_card_command,
		const uint8_t valid_bits
)
{
	uint8_t tx_last_bits = valid_bits;
	uint8_t bit_framming = (0x00 << 4) + tx_last_bits;

	write_register(COMMAND, CMD_IDLE);
	write_register(COM_IRQ, 0x7F);
	write_register(FIFO_LEVEL, 0x80);

	write_register(FIFO_DATA, smart_card_command);

	write_register(BIT_FRAMMING, bit_framming);

	write_register(COMMAND, CMD_TRANSCEIVE);
	uint8_t tmp = read_register(BIT_FRAMMING_REG);

	write_register(BIT_FRAMMING_REG, 0x80 | tmp);

	return TRANSCEIVE_OK;

}

static void clear_register(const uint8_t reg, const uint8_t mask)
{
	uint8_t masked_value = read_register(reg) & (~mask);
	write_register(reg, masked_value);
}

static void look_for_new_card(void)
{
	write_register(TX_MODE, 0x00);
	write_register(RX_MODE, 0x00);
	write_register(MOD_WIDTH, 0x00);
	clear_register(COLLISION, 0x80);
	uint8_t valid_bits = 7;

	transceive_command(SM_CMD_REQ_A, valid_bits);
}

static transceive_status_t listen_to_new_card(void)
{
	static uint8_t polling_attempts = 0;
	static delay_t delay;

	if (polling_attempts == 0) {
		delayInit(&delay, 10);
	}

	if (delayRead(&delay)) {
		polling_attempts++;

		if (polling_attempts > 32) {
			polling_attempts = 0;
			return TRANSCEIVE_ERROR;
		}

		uint8_t n = read_register(COM_IRQ);
		if (n & 0x30) {
			polling_attempts = 0;
			return TRANSCEIVE_OK;
		}

		if (n & 0x01) {
			return TRANSCEIVE_TIMEOUT;
		}

	}

	return TRANSCEIVE_WAITING;
}

bool_t card_reader_initialize(spi_device_t * spi_dev, reset_device_t * rst)
{
	capture_handlers(spi_dev, rst);
	card_reader_state = INITIALIZE;

	return true;
}


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
			break;
		case TRANSCEIVE_ERROR:
		default:
			card_reader_state = CARD_ERROR;
			break;
		}
		break;
	case RESPONSE_TIMED_OUT:
		break;
	case CARD_ERROR:
	default:
		card_reader_init();
		break;
	}
}

static void card_reader_init(void)
{
	card_reader_reset();

	write_register(TX_MODE, 0x00);
	write_register(RX_MODE, 0x00);
	write_register(MOD_WIDTH, 0x26);
	write_register(T_MODE, 0x80);
	write_register(T_PRESCALER, 0xA9);
	write_register(T_RELOAD_H, 0x03);
	write_register(T_RELOAD_L, 0xE8);
	write_register(TX_ASK, 0x40);
	write_register(MODE, 0x3D);

	turn_on_antenna();
}

static void card_reader_reset(void)
{
	reset_device();
	write_register(COMMAND, CMD_SOFT_RESET);
	HAL_Delay(100);
}


static void write_register(const uint8_t reg, const uint8_t value)
{
	card_reader_select_device();
	card_reader_write_byte(reg);
	card_reader_write_byte(value);
	card_reader_unselect_device();
}

static void write_register_multiple(const uint8_t reg, const uint8_t count, const uint8_t * data)
{
	card_reader_select_device();
	card_reader_write_byte(reg);

	for (int i = 0; i < count; i++) {
		card_reader_write_byte(data[i]);
	}

	card_reader_unselect_device();
}

static uint8_t read_register(const uint8_t reg)
{
	card_reader_select_device();
	uint8_t value = card_reader_read_byte(reg);
	card_reader_unselect_device();
	return value;
}

static void read_register_multiple(const uint8_t reg, const uint8_t count, uint8_t *values, const uint8_t byte_aling)
{

	card_reader_select_device();
	card_reader_read_multiple_byte(reg, count, values);
	card_reader_select_device();
}

static void turn_on_antenna()
{
	uint8_t value = read_register(TX_CONTROL);

	if ((value & 0x03) != 0x03) {
		write_register(TX_CONTROL, value | 0x03);
	}
}

bool_t self_test()
{
	write_register(COMMAND, 0x0F);
	HAL_Delay(1000);

	uint8_t zeros[25] = {0x00};
	write_register(FIFO_LEVEL, 0x80);
	write_register_multiple(FIFO_DATA, 25, zeros);
	write_register(COMMAND, 0x01);


	write_register(AUTO_TEST, 0x09);
	write_register(FIFO_DATA, 0x00);

	write_register(COMMAND, 0x03);

	uint8_t n;

	for (uint8_t i = 0; i < 0xFF; i++) {
		n = read_register(FIFO_LEVEL);
		if (n >= 64) {
			break;
		}
	}

	write_register(COMMAND, 0x00);


	uint8_t result[64] = {0x00};
	write_register_multiple(FIFO_DATA, 64, result);
	write_register(AUTO_TEST, 0x00);

	uint8_t version = read_register(VERSION);

	return true;

}
