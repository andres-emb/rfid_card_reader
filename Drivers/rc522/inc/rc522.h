#ifndef __RC522_H
#define __RC522_H
/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include "rc522_port.h"

/* Exported definitions ------------------------------------------------------*/
#define MAX_TRANSCEIVE_BUFFER_SIZE 16

/* Exported functions prototypes ---------------------------------------------*/
bool_t card_reader_initialize(spi_device_t * spi_dev, reset_device_t * rst);
bool_t is_there_a_new_card();
void card_reader_poll(void);

#endif
