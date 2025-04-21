#ifndef SUBCOMMAND_H
#define SUBCOMMAND_H

#include <cstdint>
#include <cstring>

enum SubcommandType
{
	GET_ONLY_CONTROLLER_STATE = 0x00,
	BT_MANUAL_PAIRING = 0x01,
	REQUEST_DEVICE_INFO = 0x02,
	SET_INPUT_REPORT_MODE = 0x03,
	TRIGGER_BUTTONS_ELAPSED_TIME = 0x04,
	GET_PAGE_LIST_STATE = 0x05,
	SET_HCI_STATE = 0x06,
	RESET_PAIRING_INFO = 0x07,
	SET_SHIPMENT_LOW_POWER_STATE = 0x08,
	SPI_FLASH_READ = 0x10,
	SPI_FLASH_WRITE = 0x11,
	SPI_SECTOR_ERASE = 0x12,
	RESET_NFC_IR_MCU = 0x20,
	SET_NFC_IR_MCU_CONFIG = 0x21,
	SET_NFC_IR_MCU_STATE = 0x22,
	SET_UNKNOWN_DATA = 0x24,
	RESET_0x24_UNKNOWN_DATA = 0x25,
	SET_UNKNOWN_NFC_IR_MCU_DATA = 0x28,
	GET_0x28_NFC_IR_MCU_DATA = 0x29,
	SET_GPIO_PIN_OUTPUT_2_PORT_2 = 0x2A,
	GET_0x29_NFC_IR_MCU_DATA = 0x2B,
	SET_PLAYER_LIGHTS = 0x30,
	GET_PLAYER_LIGHTS = 0x31,
	SET_HOME_LIGHT = 0x38,
	ENABLE_IMU = 0x40,
	SET_IMU_SENSITIVITY = 0x41,
	WRITE_IMU_REGISTERS = 0x42,
	READ_IMU_REGISTERS = 0x43,
	ENABLE_VIBRATION = 0x48,
	GET_REGULATED_VOLTAGE = 0x50,
	SET_GPIO_PIN_OUTPUT_7_15_PORT_1 = 0x51,
	GET_GPIO_PIN_IO_VALUE = 0x52
};

struct Subcommand {
    static SubcommandType const type;
};

struct SetInputModeSubcommand: Subcommand {
	static SubcommandType const type = SET_INPUT_REPORT_MODE;

	uint8_t mode;

	void to_buf(uint8_t *buf, uint8_t packet_num) const {
		bzero(buf, 0x40);
		buf[0] = 1;
		buf[1] = packet_num;

		buf[10] = type;
		buf[11] = mode;
	}
};

struct SPIFlashReadSubcommand: Subcommand {
    static SubcommandType const type = SPI_FLASH_READ;

    uint32_t address;
    uint8_t size;

   	void to_buf(uint8_t *buf, uint8_t packet_num) const {
        bzero(buf, 0x40);
        buf[0] = 1;
        buf[1] = packet_num;

        buf[10] = type;
        memcpy(buf + 11, &address, sizeof(uint32_t));
        memcpy(buf + 11 + sizeof(uint32_t), &size, sizeof(uint8_t));
    }
};

#endif // SUBCOMMAND_H