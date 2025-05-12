#include "switch_controller.h"

#include "extra_helpers.h"

namespace SwitchKit {
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
    report = SwitchControllerReport(buf);

    switch (report.report_type) {
        case SwitchControllerReport::REPORT_NONE:
            break;
        case SwitchControllerReport::REPORT_BASIC:
            break;
        case SwitchControllerReport::REPORT_SUBCOMMAND_REPLY:
            break;
        case SwitchControllerReport::REPORT_NFC_IR_MCU_FR_UPDATE_INPUT_REPORT:
            break;
        case SwitchControllerReport::REPORT_STANDARD:
            break;
        case SwitchControllerReport::REPORT_STANDARD_WITH_NFC_IR_MCU:
            break;
        default:
            break;
    }
}

void SwitchController::request_device_info() {
    uint8_t buf[0x40];
    RequestDeviceInfoSubcommand cmd;
    cmd.build(buf, packet_num++);
    hid_write(handle, buf, 0x40);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = SwitchControllerReport(in_buf);
        if (report.report_type == SwitchControllerReport::InputReportType::REPORT_SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == REQUEST_DEVICE_INFO) {
            handle_request_device_info(report.subcommand_reply.data);
            break;
        }
    }
}

void SwitchController::set_input_report_mode(InputReportMode mode) {
    uint8_t buf[0x40];
    SetInputModeSubcommand cmd;
    cmd.mode = mode;
    cmd.build(buf, packet_num++);
    hid_write(handle, buf, 0x40); // TODO: Make write_to_hid work for different commands

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = SwitchControllerReport(in_buf);
        if (report.report_type == SwitchControllerReport::InputReportType::REPORT_SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == SET_INPUT_REPORT_MODE) {
            break;
        }
    }
}

void SwitchController::set_imu_enabled(bool enabled) {
    uint8_t buf[0x40];
    SetIMUEnabledSubcommand cmd;
    cmd.enabled = enabled;
    cmd.build(buf, packet_num++);
    hid_write(handle, buf, 0x40);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = SwitchControllerReport(in_buf);
        if (report.report_type == SwitchControllerReport::InputReportType::REPORT_SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == ENABLE_IMU) {
            break;
        }
    }
}

void SwitchController::set_mcu_enabled(bool enabled) {
    uint8_t buf[0x40];
    SetMCUEnabledSubcommand cmd;
    cmd.enabled = enabled;
    cmd.build(buf, packet_num++);
    hid_write(handle, buf, 0x40);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = SwitchControllerReport(in_buf);
        if (report.report_type == SwitchControllerReport::InputReportType::REPORT_SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == SET_NFC_IR_MCU_STATE) {
            break;
        }
    }
}

void SwitchController::rumble(const HDRumbleConfig &p_config) {
    uint8_t buf[0x40];
    bzero(buf, 0x40);
    buf[0] = 0x10; // Rumble only
    buf[1] = packet_num++;

    HDRumbleDataInternal left = HDRumbleDataInternal::from_side_config(p_config.left);
    HDRumbleDataInternal right = HDRumbleDataInternal::from_side_config(p_config.right);
    memcpy(buf + 2, &left, sizeof(HDRumbleDataInternal));
    memcpy(buf + 2 + 4, &right, sizeof(HDRumbleDataInternal));
    hid_write(handle, buf, 0x40);
}

void SwitchController::configure_mcu(uint8_t command, uint8_t subcommand, uint8_t mode) {
    uint8_t buf[0x40];
    ConfigureMCUSubcommand cmd;
    cmd.command = command;
    cmd.subcommand = subcommand;
    cmd.mode = mode;
    cmd.build(buf, packet_num++);
    hid_write(handle, buf, 0x40);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = SwitchControllerReport(in_buf);
        if (report.report_type == SwitchControllerReport::InputReportType::REPORT_SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == SET_NFC_IR_MCU_CONFIG) {
            break;
        }
    }
}

uint16_t SwitchController::get_external_device_id() {
    uint8_t buf[0x40];
    GetExternalDeviceIDSubcommand cmd;
    cmd.build(buf, packet_num++);
    hid_write(handle, buf, 0x40);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = SwitchControllerReport(in_buf);
        if (report.report_type == SwitchControllerReport::InputReportType::REPORT_SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == 0x59) {
            return *((uint16_t*)report.subcommand_reply.data);
        }
    }
}

void SwitchController::set_external_format_config(uint8_t *data, uint8_t size) {
    uint8_t buf[0x40];
    SetExternalFormatConfigSubcommand cmd;
    cmd.data = data;
    cmd.size = size;
    cmd.build(buf, packet_num++);
    hid_write(handle, buf, 0x40);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = SwitchControllerReport(in_buf);
        if (report.report_type == SwitchControllerReport::InputReportType::REPORT_SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == 0x5C) {
            break;
        }
    }
}

void SwitchController::enable_external_polling(uint8_t *data, uint8_t size) {
    uint8_t buf[0x40];
    EnableExternalPollingSubcommand cmd;
    cmd.data = data;
    cmd.size = size;
    cmd.build(buf, packet_num++);
    hid_write(handle, buf, 0x40);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = SwitchControllerReport(in_buf);
        if (report.report_type == SwitchControllerReport::InputReportType::REPORT_SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == 0x5A) {
            break;
        }
    }
}

void SwitchController::enable_ringcon() {
    // Enable IMU
    set_imu_enabled(true);

    // Enable MCU
    set_mcu_enabled(true);

    // Configure MCU
    configure_mcu(33, 1, 1); // CONGIFURE_MCU, SET_DEVICE_MODE, STANDBY

    // Check if ring is connected
    uint16_t external_id = get_external_device_id();

    // Send SET_EXTERNAL_FORMAT_CONFIG with data
    uint8_t ext_format_config[] = { 0x06, 0x03, 0x25, 0x06, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x16, 0xED, 0x34, 0x36,
        0x00, 0x00, 0x00, 0x0A, 0x64, 0x0B, 0xE6, 0xA9, 0x22, 0x00, 0x00, 0x04, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0xA8, 0xE1, 0x34, 0x36 };
    set_external_format_config(ext_format_config, 37);

    // Set ENABLE_EXTERNAL_POLLING with data
    uint8_t enable_external_polling_data[] = { 0x04, 0x01, 0x01, 0x02 };
    enable_external_polling(enable_external_polling_data, 4);
}

double SwitchController::get_ringcon_flex() {
    double low_bound = 2280.0 - 2500.0;
    double high_bound = 2280.0 + 2500.0;
    double current = (double)report.imu_packets[2].accel_y;
    return INV_LERP(2280.0, high_bound, current);
}

void SwitchController::request_stick_calibration() {
	SPIFlashReadSubcommand cmd;
	cmd.address = FACTORY_STICK_CALIBRATION;
	cmd.size = 0x12;
    write_to_hid(cmd);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = SwitchControllerReport(in_buf);
        if (report.report_type == SwitchControllerReport::InputReportType::REPORT_SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == SPI_FLASH_READ) {
            handle_spi_flash_read(report.subcommand_reply.data);
            break;
        }
    }
}

void SwitchController::request_imu_calibration() {
    SPIFlashReadSubcommand cmd;
    cmd.address = FACTORY_IMU_CALIBRATION;
    cmd.size = 24;
    write_to_hid(cmd);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = SwitchControllerReport(in_buf);
        if (report.report_type == SwitchControllerReport::InputReportType::REPORT_SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == SPI_FLASH_READ) {
            handle_spi_flash_read(report.subcommand_reply.data);
            break;
        }
    }
}

void SwitchController::request_color_data() {
    SPIFlashReadSubcommand cmd;
    cmd.address = COLOR_DATA;
    cmd.size = 12;
    write_to_hid(cmd);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = SwitchControllerReport(in_buf);
        if (report.report_type == SwitchControllerReport::InputReportType::REPORT_SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == SPI_FLASH_READ) {
            handle_spi_flash_read(report.subcommand_reply.data);
            break;
        }
    }
}

void SwitchController::write_to_hid(SPIFlashReadSubcommand cmd) {
    uint8_t buf[0x40];
    cmd.build(buf, packet_num++);
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
    
    switch (addr) {
        case 0x5000: // Shipment data
            break;
        case SERIAL_NO:
            break;
        case FACTORY_IMU_CALIBRATION:
            update_imu_calibration(data, size);
            break;
        case FACTORY_STICK_CALIBRATION:
            update_stick_calibration(data, size);
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



void SwitchController::update_imu_calibration(uint8_t *data, uint8_t size) {
    memcpy(&imu_calib, data, size);
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
            x_max = static_cast<double>(rs_calib.x_max);
            y_min = static_cast<double>(rs_calib.y_min);
            y_max = static_cast<double>(rs_calib.y_max);
            break;
    }

    double x = (INV_LERP(x_min, x_max, x_raw) * 2.0) - 1.0;
    double y = (INV_LERP(y_min, y_max, y_raw) * 2.0) - 1.0;

    return Vector2(x, y);
}

Vector3 SwitchController::get_accel() const {
    // NOTE: When the Ring-Con is connected, we will only want to read from
    // the first two acceleration packets (it uses the third one to pass flex data).
    double x = 0, y = 0, z = 0;
    for (int i = 0; i < 3; i++) {
        Vector3 acc = report.imu_packets[i].get_accel();
        x += acc.x;
        y += acc.y;
        z += acc.z;
    }
    x /= 3.0;
    y /= 3.0;
    z /= 3.0;
    return Vector3(x, y, z);
}

Vector3 SwitchController::get_gyro() const {
    Vector3 val = report.imu_packets[0].get_gyro(imu_calib);
    double delta_time = 0.005; // 5ms
    val.x *= delta_time;
    val.y *= delta_time;
    val.z *= delta_time;
    return val;
}

void SwitchController::update_color_data(uint8_t *data, uint8_t size) {
    if (size != 0x0C) {
        printf("Color data size was 0x%X but should have been 0x0B\n", size);
        return;
    }
    memcpy(&colors, (SwitchControllerColors*)data, sizeof(SwitchControllerColors));
}

Color24 SwitchController::get_color(ColorRole role) const {
    switch (role) {
        case COLOR_BODY:
            return colors.body_color;
        case COLOR_BUTTON:
            return colors.button_color;
        case COLOR_LEFT_GRIP:
            return colors.left_grip_color;
        case COLOR_RIGHT_GRIP:
            return colors.right_grip_color;
    }
}

void SwitchController::set_player_lights(PlayerLight p1, PlayerLight p2, PlayerLight p3, PlayerLight p4) {
    uint8_t buf[0x40];
    bzero(buf, 0x40);
    buf[0] = 1;
    buf[1] = packet_num++;
    buf[10] = SET_PLAYER_LIGHTS;

    buf[11] = (p1) | (p2 << 1) | (p3 << 2) | (p4 << 3);
    hid_write(handle, buf, 0x40);
}
}
