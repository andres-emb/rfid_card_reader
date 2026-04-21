#ifndef __SMART_CARD_COMMANDS_H
#define __SMART_CARD_COMMANDS_H

/* Values used to interact with Smart Cards according to ISO 14443-3, Type A
 * These values will be sent to the FIFO to be transmitted to the Smart Card
 * */

#define SM_CMD_REQ_A			0x26
#define SM_CMD_SELECT_CL1		0x93
#define SM_CMD_CASCADE_TAG		0x88

#endif
