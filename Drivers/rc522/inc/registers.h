#ifndef __REGISTERS_H
#define __REGISTERS_H

/*
 * This file defines the used registers of the card reader
 */

/* According with RC522 datasheet in the SPI address byte
 * the LSB should be 0 always
 */

#define COMMAND 		0x01 << 1 // h02
#define MODE 			0x11 << 1 // h22
#define TX_MODE 		0x12 << 1 // h24
#define RX_MODE 		0x13 << 1 // h26
#define TX_CONTROL 		0x14 << 1 // h28
#define TX_ASK 			0x15 << 1 // h2A
#define MOD_WIDTH 		0x24 << 1 // h24
#define T_MODE 			0x2A << 1 // h54
#define T_PRESCALER 	0x2B << 1 // h56
#define T_RELOAD_H 		0x2C << 1 // h58
#define T_RELOAD_L 		0x2D << 1 // h5A
#define COLLISION 		0x0E << 1 // h1C
#define COM_IRQ 		0x04 << 1 // h08
#define FIFO_LEVEL 		0x0A << 1 // h14
#define FIFO_DATA 		0x09 << 1 // h12
#define BIT_FRAMMING 	0x0D << 1 // h1A
#define ERROR_INFO 		0x06 << 1 // h0C
#define CONTROL 		0x0C << 1 // h18
#define AUTO_TEST 		0x36 << 1 // h6C
#define VERSION 		0x37 << 1 // h6E

#endif
