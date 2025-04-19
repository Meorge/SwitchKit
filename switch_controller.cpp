#include "switch_controller.h"

#include "extra_helpers.h"

SwitchProController::SwitchProController() {
    handle = hid_open(0x057E, 0x2009, NULL);
    if (!handle) {
        printf("ERROR: Handle is 0, so apparently we didn't connect\n");
    }
}

SwitchProController::~SwitchProController() {
    hid_close(handle);
}

void SwitchProController::poll() {
    hid_read(handle, buf, 361);
    report = JoyConReport(buf);

    // Subcommand support
    switch (report.subcommand_reply_to)
    {
    case SPI_FLASH_READ:
        handle_spi_flash_read(report.subcommand_reply);
        break;
    }
}

void SwitchProController::request_stick_calibration() {
	SPIFlashReadSubcommand cmd;
	cmd.address = SPI_ADDR_STICK_CALIB;
	cmd.size = SPI_LEN_STICK_CALIB;
    printf("handle is %d\n", handle);
    hid_write(handle, cmd.to_buf(packet_num++), 0x40);
}

void SwitchProController::handle_spi_flash_read(uint8_t *reply) {
	uint32_t addr;
	uint8_t size;

	memcpy(&addr, reply, sizeof(uint32_t));
	memcpy(&size, reply + sizeof(uint32_t), sizeof(uint8_t));

	uint8_t data[size];
	memcpy(data, reply + sizeof(uint32_t) + sizeof(uint8_t), size);

	if (addr == SPI_ADDR_STICK_CALIB && size == SPI_LEN_STICK_CALIB) {
		update_stick_calibration(data);
	}
}

void SwitchProController::update_stick_calibration(uint8_t *stick_cal) {
	// https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/spi_flash_notes.md#analog-stick-factory-and-user-calibration
	printf("Stick calibration time!\n");

    uint8_t *ls_stick_cal = stick_cal;
    uint16_t ls_data[6];
    ls_data[0] = (ls_stick_cal[1] << 8) & 0xF00 | ls_stick_cal[0];    // X axis max above center
    ls_data[1] = (ls_stick_cal[2] << 4) | (ls_stick_cal[1] >> 4);     // Y axis max above center
    ls_data[2] = (ls_stick_cal[4] << 8) & 0xF00 | ls_stick_cal[3];    // X axis center
    ls_data[3] = (ls_stick_cal[5] << 4) | (ls_stick_cal[4] >> 4);     // Y axis center
    ls_data[4] = (ls_stick_cal[7] << 8) & 0xF00 | ls_stick_cal[6];    // X axis min below center
    ls_data[5] = (ls_stick_cal[8] << 4) | (ls_stick_cal[7] >> 4);     // Y axis min below center

    ls_calib.x_min = ls_data[2] - ls_data[4];
    ls_calib.x_center = ls_data[2];
    ls_calib.x_max = ls_data[2] + ls_data[0];

    // printf("LS X center at %hu\n", ls_data[2]);
    // printf("LS X center is at %hu, it can go as low as %hu and as high as %hu\n", ls_calib.x_center, ls_calib.x_min, ls_calib.x_max);

    ls_calib.y_min = ls_data[3] - ls_data[5];
    ls_calib.y_center = ls_data[3];
    ls_calib.y_max = ls_data[3] + ls_data[1];


    uint8_t *rs_stick_cal = stick_cal + 9;
    uint16_t rs_data[6];
    rs_data[0] = (rs_stick_cal[1] << 8) & 0xF00 | rs_stick_cal[0];    // X axis center
    rs_data[1] = (rs_stick_cal[2] << 4) | (rs_stick_cal[1] >> 4);     // Y axis center
    rs_data[2] = (rs_stick_cal[4] << 8) & 0xF00 | rs_stick_cal[3];    // X axis min below center
    rs_data[3] = (rs_stick_cal[5] << 4) | (rs_stick_cal[4] >> 4);     // Y axis min below center
    rs_data[4] = (rs_stick_cal[7] << 8) & 0xF00 | rs_stick_cal[6];    // X axis max above center
    rs_data[5] = (rs_stick_cal[8] << 4) | (rs_stick_cal[7] >> 4);     // Y axis max above center

    rs_calib.x_min = rs_data[0] - rs_data[2];
    rs_calib.x_center = rs_data[0];
    rs_calib.x_max = rs_data[0] + rs_data[4];

    rs_calib.y_min = rs_data[1] - rs_data[3];
    rs_calib.y_center = rs_data[1];
    rs_calib.y_max = rs_data[1] + rs_data[5];
}

Vector2 SwitchProController::get_stick(Stick stick) const {
    double x_raw, x_min, x_max;
    double y_raw, y_min, y_max;
    switch (stick) {
        case LEFT:
            x_raw = static_cast<double>(report.ls_x);
            y_raw = static_cast<double>(report.ls_y);
            x_min = static_cast<double>(ls_calib.x_min);
            x_max = static_cast<double>(ls_calib.x_max);
            y_min = static_cast<double>(ls_calib.y_min);
            y_max = static_cast<double>(ls_calib.y_max);
            break;
        case RIGHT:
            x_raw = static_cast<double>(report.rs_x);
            y_raw = static_cast<double>(report.rs_y);
            x_min = static_cast<double>(rs_calib.x_min);
            y_max = static_cast<double>(rs_calib.x_max);
            y_min = static_cast<double>(rs_calib.y_min);
            y_max = static_cast<double>(rs_calib.y_max);
            break;
    }

    // printf("x_raw is %.2lf, range is [%.2lf, %.2lf]\n", x_raw, x_min, x_max);

    double x = (INV_LERP(x_min, x_max, x_raw) * 2.0) - 1.0;
    double y = (INV_LERP(y_min, y_max, y_raw) * 2.0) - 1.0;

    return Vector2(x, y);
}