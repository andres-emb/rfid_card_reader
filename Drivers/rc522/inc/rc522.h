#ifndef __RC522_H
#define __RC522_H

#include "common.h"
#include "rc522_port.h"

bool_t card_reader_initialize(spi_device_t * spi_dev, reset_device_t * rst);
bool_t is_there_a_new_card();
bool_t self_test();
void card_reader_poll(void);

#endif
