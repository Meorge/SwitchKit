#include "switch_controller.h"

#include "extra_helpers.h"

SwitchController::SwitchController(hid_device *p_handle) {
    handle = p_handle;
    if (!handle) {
        printf("ERROR: Handle is 0, so apparently we didn't connect\n");
    }
}

SwitchController::~SwitchController() {
}

void SwitchController::poll() {
    hid_read(handle, buf, 361);
    report = JoyConReport(buf);

    if (report.subcommand_reply_to != 0) {
        printf("Report subcommand reply is %d\n", report.subcommand_reply_to);
    }

    // Subcommand support
    switch (report.subcommand_reply_to)
    {
    case REQUEST_DEVICE_INFO:
        handle_request_device_info(report.subcommand_reply);
        break;
    case SPI_FLASH_READ:
        handle_spi_flash_read(report.subcommand_reply);
        break;
    }
}

void SwitchController::request_device_info() {
    uint8_t buf[0x40];
    bzero(buf, 0x40);
    buf[0] = 1;
    buf[1] = packet_num++;
    buf[10] = REQUEST_DEVICE_INFO;
    hid_write(handle, buf, 0x40);
}

void SwitchController::set_input_report_mode(InputReportMode mode) {
    uint8_t buf[0x40];
    bzero(buf, 0x40);
    buf[0] = 1;
    buf[1] = packet_num++;
    buf[10] = SET_INPUT_REPORT_MODE;
    buf[11] = 0x31; // NFC/IR mode
    hid_write(handle, buf, 0x40);
}

void SwitchController::request_stick_calibration() {
	SPIFlashReadSubcommand cmd;
	cmd.address = FACTORY_STICK_CALIBRATION;
	cmd.size = 0x12;
    write_to_hid(cmd);
}

void SwitchController::request_color_data() {
    SPIFlashReadSubcommand cmd;
    cmd.address = COLOR_DATA;
    cmd.size = 0x605B - COLOR_DATA;
    write_to_hid(cmd);
}

void SwitchController::write_to_hid(SPIFlashReadSubcommand cmd) {
    uint8_t buf[0x40];
    cmd.to_buf(buf, packet_num++);
    hid_write(handle, buf, 0x40);
}

void SwitchController::handle_request_device_info(uint8_t *data) {
    memcpy(&info, (SwitchDeviceInfo*)data, sizeof(SwitchDeviceInfo));
}

void SwitchController::handle_spi_flash_read(uint8_t *reply) {
	uint32_t addr;
	uint8_t size;

	memcpy(&addr, reply, sizeof(uint32_t));
	memcpy(&size, reply + sizeof(uint32_t), sizeof(uint8_t));

	uint8_t data[size];
	memcpy(data, reply + sizeof(uint32_t) + sizeof(uint8_t), size);

    printf("SPI flash read! Address is 0x%X\n", addr);

    switch (addr) {
        case 0x5000: // Shipment data
            break;
        case SERIAL_NO:
            break;
        case FACTORY_IMU_CALIBRATION:
            break;
        case FACTORY_STICK_CALIBRATION:
            break;
        case COLOR_DATA:
            update_color_data(data, size);
            break;
        case 0x6086: // Stick device parameters 1
            break;
        case 0x6098: // Stick device parameters 2
            break;
        case USER_IMU_CALIBRATION:
            break;
        case USER_STICK_CALIBRATION:
            update_stick_calibration(data, size);
            break;
        default:
            break;
        
    }
}

void SwitchController::update_stick_calibration(uint8_t *stick_cal, uint8_t size) {
	// https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/spi_flash_notes.md#analog-stick-factory-and-user-calibration
	printf("Stick calibration time!\n");

    if (size != 0x12) {
        printf("Size of data for stick calibration is %d but it should be %d\n", size, 0x12);
        return;
    }

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

Vector2 SwitchController::get_stick(Stick stick) const {
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

    double x = (INV_LERP(x_min, x_max, x_raw) * 2.0) - 1.0;
    double y = (INV_LERP(y_min, y_max, y_raw) * 2.0) - 1.0;

    return Vector2(x, y);
}

void SwitchController::update_color_data(uint8_t *data, uint8_t size) {
    if (size != 0x0B) {
        printf("Color data size was 0x%X but should have been 0x0B\n", size);
        return;
    }

    memcpy(&colors, (SwitchControllerColors*)data, sizeof(SwitchControllerColors));

    printf("Body color: r=%d g=%d b=%d\n", colors.body_color.r, colors.body_color.g, colors.body_color.b);
    printf("Button color: r=%d g=%d b=%d\n", colors.button_color.r, colors.button_color.g, colors.button_color.b);
}