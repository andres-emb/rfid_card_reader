# RFID Card Reader with STM32F429ZI - RC522 (SPI) & LCD (I2C)

This project demonstrates how to use a **Nucleo STM32 F429ZI** board to interface with an **RC522 RFID module** via **SPI**, read the serial number (UID) of **Mifare Classic** cards, and display the Serial Number (UID) on an **I2C LCD** (16x2  character LCD with PCF8574 I2C backpack).

## Hardware Overview

| Component             | Model / Spec                                | Interface |
| --------------------- | ------------------------------------------- | --------- |
| Microcontroller board | STM32F429ZI (Nucleo-144)                    | -         |
| RFID reader           | RC522 (MFRC522)                             | SPI       |
| LCD                   | 16x2 character LCD + I2C backpack (PCF8574) | I2C       |

## Block Diagram
<img width="747" height="419" alt="image" src="https://github.com/user-attachments/assets/8f30d18c-08a1-4c52-a018-e78f31bbd3c6" />

## Finite State Machine (FSM) in the RC522 Driver

The RC522 driver implements a **Finite State Machine** to handle card detection UID reading in a non‑blocking  approach. This FSM is particularly useful when integrating the RFID task into a superloop.
### States

| State                     | Description                                                                                                     |
| ------------------------- | --------------------------------------------------------------------------------------------------------------- |
| `INITIALIZE`              | The STM32 sends the proper commands to perform a soft reset and get ready the module to interact with the card. |
| `SENSE_NEW_CARD`          | Send `REQA` (0x26) command to detect any card in the field.                                                     |
| `WAITING_SENSE_RESPONSE`  | Check if the RC522 has received any communication from a card                                                   |
| `SELECT_NEW_CARD`         | Send the select command the card to get its UID.                                                                |
| `WAITING_SELECT_RESPONSE` | Check if the RC522 has received any response to the select command.                                             |
| `REPORT_SERIAL_NUMBER`    | Send the UID to be displayed in the LCD.                                                                        |
| `ERROR`                   | Resets the RC522 module                                                                                         |
### State Transitions

<img width="1015" height="675" alt="image" src="https://github.com/user-attachments/assets/cb1bf0fd-adcf-461e-8ce0-ade4c19b6aab" />

## References

* RC522 
	* Arduino library: https://github.com/miguelbalboa/rfid
	* Datasheet: https://www.nxp.com/docs/en/data-sheet/MFRC522.pdf
* LCD I2C code from the class Comunication Protocols in Embedded Systems
## Author

Andres Urian
