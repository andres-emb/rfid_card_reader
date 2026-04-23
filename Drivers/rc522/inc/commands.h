#ifndef __COMMANDS_H
#define __COMMANDS_H

/*
 * This file defines the possible commands to interact with the card reader
 */

/* Values used alongside the COMMAND register
 * */

#define CMD_IDLE			0x00 // Stops any previous command
#define CMD_TRANSCEIVE 		0x0C // Transceive data to the smart card
#define CMD_SOFT_RESET		0x0F // Perform a soft reset of the card reader

#define STANDARD_CMD_SIZE	1

#endif
