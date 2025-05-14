#ifndef SUBCOMMAND_H
#define SUBCOMMAND_H

#include <cstdint>
#include <cstring>
#include <stdio.h>

namespace SwitchKit {
// From mod.rs in joy, which in turn appears to be from jc_toolkit
const uint8_t MCU_CRC8_TABLE[] = {
    0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
    0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65, 0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
    0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5, 0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
    0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
    0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2, 0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
    0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
    0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
    0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42, 0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
    0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
    0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
    0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, 0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
    0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
    0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
    0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, 0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
    0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
    0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3,
};

uint8_t calc_crc8(uint8_t *data, uint8_t size);



enum SubcommandType
{
	SCMD_GET_ONLY_CONTROLLER_STATE = 0x00,
	SCMD_BT_MANUAL_PAIRING = 0x01,
	SCMD_REQUEST_DEVICE_INFO = 0x02,
	SCMD_SET_INPUT_REPORT_MODE = 0x03,
	SCMD_TRIGGER_BUTTONS_ELAPSED_TIME = 0x04,
	SCMD_GET_PAGE_LIST_STATE = 0x05,
	SCMD_SET_HCI_STATE = 0x06,
	SCMD_RESET_PAIRING_INFO = 0x07,
	SCMD_SET_SHIPMENT_LOW_POWER_STATE = 0x08,
	SCMD_SPI_FLASH_READ = 0x10,
	SCMD_SPI_FLASH_WRITE = 0x11,
	SCMD_SPI_SECTOR_ERASE = 0x12,
	SCMD_RESET_NFC_IR_MCU = 0x20,
	SCMD_SET_NFC_IR_MCU_CONFIG = 0x21,
	SCMD_SET_NFC_IR_MCU_STATE = 0x22,
	SCMD_SET_UNKNOWN_DATA = 0x24,
	SCMD_RESET_0x24_UNKNOWN_DATA = 0x25,
	SCMD_SET_UNKNOWN_NFC_IR_MCU_DATA = 0x28,
	SCMD_GET_0x28_NFC_IR_MCU_DATA = 0x29,
	SCMD_SET_GPIO_PIN_OUTPUT_2_PORT_2 = 0x2A,
	SCMD_GET_0x29_NFC_IR_MCU_DATA = 0x2B,
	SCMD_SET_PLAYER_LIGHTS = 0x30,
	SCMD_GET_PLAYER_LIGHTS = 0x31,
	SCMD_SET_HOME_LIGHT = 0x38,
	SCMD_ENABLE_IMU = 0x40,
	SCMD_SET_IMU_SENSITIVITY = 0x41,
	SCMD_WRITE_IMU_REGISTERS = 0x42,
	SCMD_READ_IMU_REGISTERS = 0x43,
	SCMD_ENABLE_VIBRATION = 0x48,
	SCMD_GET_REGULATED_VOLTAGE = 0x50,
	SCMD_SET_GPIO_PIN_OUTPUT_7_15_PORT_1 = 0x51,
	SCMD_GET_GPIO_PIN_IO_VALUE = 0x52,
	SCMD_GET_EXTERNAL_DEVICE_INFO = 0x59,
	SCMD_ENABLE_EXTERNAL_POLLING = 0x5A,
	SCMD_SET_EXTERNAL_FORMAT_CONFIG = 0x5C
};

struct Subcommand {
protected:
	static SubcommandType const type = SCMD_GET_ONLY_CONTROLLER_STATE;

	void init_build(uint8_t *p_buf, uint8_t p_packet_num) const {
		bzero(p_buf, 0x40);
		p_buf[0] = 1;
		p_buf[1] = p_packet_num;
		p_buf[10] = type;
	}

public:
	void build(uint8_t *buf, uint8_t packet_num) const;
};

struct SetInputModeSubcommand: Subcommand {
	static SubcommandType const type = SCMD_SET_INPUT_REPORT_MODE;

	uint8_t mode;

	void build(uint8_t *p_buf, uint8_t p_packet_num) const {
		bzero(p_buf, 0x40);
		p_buf[0] = 1;
		p_buf[1] = p_packet_num;
		p_buf[10] = type;
		p_buf[11] = mode;
	}
};

struct SPIFlashReadSubcommand: Subcommand {
    static SubcommandType const type = SCMD_SPI_FLASH_READ;

    uint32_t address;
    uint8_t size;

   	void build(uint8_t *p_buf, uint8_t p_packet_num) const {
		bzero(p_buf, 0x40);
		p_buf[0] = 1;
		p_buf[1] = p_packet_num;
		p_buf[10] = type;
        memcpy(p_buf + 11, &address, sizeof(uint32_t));
        memcpy(p_buf + 11 + sizeof(uint32_t), &size, sizeof(uint8_t));
    }
};

struct RequestDeviceInfoSubcommand: Subcommand {
	static SubcommandType const type = SCMD_REQUEST_DEVICE_INFO;

	void build(uint8_t *p_buf, uint8_t p_packet_num) const {
		bzero(p_buf, 0x40);
		p_buf[0] = 1;
		p_buf[1] = p_packet_num;
		p_buf[10] = type;
	}
};

struct SetIMUEnabledSubcommand: Subcommand {
	static SubcommandType const type = SCMD_ENABLE_IMU;

	bool enabled = false;

	void build(uint8_t *p_buf, uint8_t p_packet_num) const {
		bzero(p_buf, 0x40);
		p_buf[0] = 1;
		p_buf[1] = p_packet_num;
		p_buf[10] = type;
		p_buf[11] = enabled ? 0x01 : 0x00;
	}
};

struct SetMCUEnabledSubcommand: Subcommand {
	static SubcommandType const type = SCMD_SET_NFC_IR_MCU_STATE;

	bool enabled = false;

	void build(uint8_t *p_buf, uint8_t p_packet_num) const {
		bzero(p_buf, 0x40);
		p_buf[0] = 1;
		p_buf[1] = p_packet_num;
		p_buf[10] = type;
		p_buf[11] = enabled ? 0x01 : 0x00;
	}
};

struct ConfigureMCUSubcommand: Subcommand {
	static SubcommandType const type = SCMD_SET_NFC_IR_MCU_CONFIG;

	uint8_t command;
	uint8_t subcommand;
	uint8_t mode;

	void build(uint8_t *p_buf, uint8_t p_packet_num) const {
		bzero(p_buf, 0x40);
		p_buf[0] = 1;
		p_buf[1] = p_packet_num;
		p_buf[10] = type;

		uint8_t subcommand_data[38];
		bzero(subcommand_data, 38);
		subcommand_data[0] = command;
		subcommand_data[1] = subcommand;
		subcommand_data[2] = mode;

		// Calculate CRC
		uint8_t crc = 0;
		uint8_t *d = subcommand_data + 1;
		for (int i = 0; i < 36; i++) {
			crc = SwitchKit::MCU_CRC8_TABLE[(uint8_t)(crc ^ d[i])];
		}

		subcommand_data[37] = crc;
		memcpy(p_buf + 11, subcommand_data, sizeof(subcommand_data));
	}
};

struct GetExternalDeviceIDSubcommand: Subcommand {
	static SubcommandType const type = SCMD_GET_EXTERNAL_DEVICE_INFO;

	void build(uint8_t *p_buf, uint8_t p_packet_num) const {
		bzero(p_buf, 0x40);
		p_buf[0] = 1;
		p_buf[1] = p_packet_num;
		p_buf[10] = type;
	}
};

struct SetExternalFormatConfigSubcommand: Subcommand {
	static SubcommandType const type = SCMD_SET_EXTERNAL_FORMAT_CONFIG;

	uint8_t *data;
	uint8_t size;
	
	void build(uint8_t *p_buf, uint8_t p_packet_num) const {
		bzero(p_buf, 0x40);
		p_buf[0] = 1;
		p_buf[1] = p_packet_num;
		p_buf[10] = type;
		memcpy(p_buf + 11, data, size);
	}
};

struct EnableExternalPollingSubcommand: Subcommand {
	static SubcommandType const type = SCMD_ENABLE_EXTERNAL_POLLING;

	uint8_t *data;
	uint8_t size;

	void build(uint8_t *p_buf, uint8_t p_packet_num) const {
		bzero(p_buf, 0x40);
		p_buf[0] = 1;
		p_buf[1] = p_packet_num;
		p_buf[10] = type;
		memcpy(p_buf + 11, data, size);
	}
};


}

#endif // SUBCOMMAND_H
