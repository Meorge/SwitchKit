#include "switch_controller.h"
#include <math.h>
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
    prev_report = SwitchControllerReport(report);
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

    const IMUCalibrationData *calib = has_user_imu_calib ? &user_imu_calib : &imu_calib;
    Vector3 val = report.imu_packets[0].get_gyro(calib);
    double delta_time = 0.005; // 5ms
    val *= delta_time;
    val *= (M_PI / 180.0); // Convert from degrees to radians.
    accumulated_gyro += val; // Add to accumulated value.

    if (get_ringcon_enabled()) {
        double flex = get_ringcon_flex();
        report.btn_ringcon_flex = flex > btn_ringcon_flex_threshold;
    } else {
        report.btn_ringcon_flex = false;
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
        if (report.report_type == SwitchControllerReport::InputReportType::REPORT_SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == SCMD_REQUEST_DEVICE_INFO) {
            handle_request_device_info(report.subcommand_reply.data);
            break;
        }
    }
}

void SwitchController::set_input_report_mode(InputReportMode p_mode) {
    uint8_t buf[0x40];
    SetInputModeSubcommand cmd;
    cmd.mode = p_mode;
    cmd.build(buf, packet_num++);
    hid_write(handle, buf, 0x40); // TODO: Make write_to_hid work for different commands

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = SwitchControllerReport(in_buf);
        if (report.report_type == SwitchControllerReport::InputReportType::REPORT_SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == SCMD_SET_INPUT_REPORT_MODE) {
            break;
        }
    }
}

void SwitchController::set_hci_state(HCIState p_state) {
    uint8_t buf[0x40];
    SetHCIStateSubcommand cmd;
    cmd.mode = p_state;
    cmd.build(buf, packet_num++);
    hid_write(handle, buf, 0x40);
}

void SwitchController::set_imu_enabled(bool p_enabled) {
    uint8_t buf[0x40];
    SetIMUEnabledSubcommand cmd;
    cmd.enabled = p_enabled;
    cmd.build(buf, packet_num++);
    hid_write(handle, buf, 0x40);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = SwitchControllerReport(in_buf);
        if (report.report_type == SwitchControllerReport::InputReportType::REPORT_SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == SCMD_ENABLE_IMU) {
            break;
        }
    }
}

void SwitchController::set_mcu_enabled(bool p_enabled) {
    uint8_t buf[0x40];
    SetMCUEnabledSubcommand cmd;
    cmd.enabled = p_enabled;
    cmd.build(buf, packet_num++);
    hid_write(handle, buf, 0x40);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = SwitchControllerReport(in_buf);
        if (report.report_type == SwitchControllerReport::InputReportType::REPORT_SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == SCMD_SET_NFC_IR_MCU_STATE) {
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

void SwitchController::configure_mcu(uint8_t p_command, uint8_t p_subcommand, uint8_t p_mode) {
    uint8_t buf[0x40];
    ConfigureMCUSubcommand cmd;
    cmd.command = p_command;
    cmd.subcommand = p_subcommand;
    cmd.mode = p_mode;
    cmd.build(buf, packet_num++);
    hid_write(handle, buf, 0x40);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = SwitchControllerReport(in_buf);
        if (report.report_type == SwitchControllerReport::InputReportType::REPORT_SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == SCMD_SET_NFC_IR_MCU_CONFIG) {
            break;
        }
    }
}

SwitchController::ExternalDevice SwitchController::get_external_device_id() {
    uint8_t buf[0x40];
    GetExternalDeviceIDSubcommand cmd;
    cmd.build(buf, packet_num++);
    hid_write(handle, buf, 0x40);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = SwitchControllerReport(in_buf);
        if (report.report_type == SwitchControllerReport::InputReportType::REPORT_SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == SCMD_GET_EXTERNAL_DEVICE_INFO) {
            return (SwitchController::ExternalDevice)(*((uint16_t*)report.subcommand_reply.data));
        }
    }
}

void SwitchController::set_external_format_config(uint8_t *p_data, uint8_t p_size) {
    uint8_t buf[0x40];
    SetExternalFormatConfigSubcommand cmd;
    cmd.data = p_data;
    cmd.size = p_size;
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

void SwitchController::enable_external_polling(uint8_t *p_data, uint8_t p_size) {
    uint8_t buf[0x40];
    EnableExternalPollingSubcommand cmd;
    cmd.data = p_data;
    cmd.size = p_size;
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

    ringcon_enabled = true;
}

void SwitchController::disable_ringcon() {
    set_mcu_enabled(false);
    ringcon_enabled = false;
}

bool SwitchController::get_ringcon_connected() {
    return get_external_device_id() == ExternalDevice::EXT_RINGCON;
}

double SwitchController::get_ringcon_flex() {
    if (!ringcon_enabled) return 0.0;

    double high_bound = 2280.0 + 2500.0;
    double current = (double)report.imu_packets[2].accel_y;
    return INV_LERP(2280.0, high_bound, current);
}

void SwitchController::request_stick_calibration() {
    // Factory calibration
	SPIFlashReadSubcommand cmd;
	cmd.address = SPI_FACTORY_STICK_CALIBRATION;
	cmd.size = 18;
    write_to_hid(cmd);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = SwitchControllerReport(in_buf);
        if (report.report_type == SwitchControllerReport::InputReportType::REPORT_SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == SCMD_SPI_FLASH_READ) {
            handle_spi_flash_read(report.subcommand_reply.data);
            break;
        }
    }

    // User calibration
    cmd.address = 0x8010; // Start of stick calibration
    cmd.size = 22;
    write_to_hid(cmd); 

    while (true) {
        uint8_t in_buf[361];
        bzero(in_buf, 361);
        hid_read(handle, in_buf, 361);
        report = SwitchControllerReport(in_buf);
        if (report.report_type == SwitchControllerReport::InputReportType::REPORT_SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == SCMD_SPI_FLASH_READ) {
            uint32_t addr;
            uint8_t size;

            memcpy(&addr, report.subcommand_reply.data, sizeof(uint32_t));
            memcpy(&size, report.subcommand_reply.data + sizeof(uint32_t), sizeof(uint8_t));

            uint8_t data[size];
            memcpy(data, report.subcommand_reply.data + sizeof(uint32_t) + sizeof(uint8_t), size);

            uint16_t ls_magic = *((uint16_t*)data);
            uint16_t rs_magic = *((uint16_t*)(data + sizeof(uint16_t) + 9));

            if (ls_magic == 0xA1B2) {
                // Read left stick calibration, save
                has_user_ls_calib = true;
                parse_stick_calibration(data + sizeof(uint16_t), &user_ls_calib, STICK_LEFT);
            }

            if (rs_magic == 0xA2B2) {
                // Read right stick calibration, save
                has_user_rs_calib = true;
                parse_stick_calibration(data + sizeof(uint16_t) + 9 + sizeof(uint16_t), &user_rs_calib, STICK_RIGHT);
            }
            break;
        }
    }
}

void SwitchController::request_imu_calibration() {
    // Factory calibration
    SPIFlashReadSubcommand cmd;
    cmd.address = SPI_FACTORY_IMU_CALIBRATION;
    cmd.size = 24;
    write_to_hid(cmd);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = SwitchControllerReport(in_buf);
        if (report.report_type == SwitchControllerReport::InputReportType::REPORT_SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == SCMD_SPI_FLASH_READ) {
            handle_spi_flash_read(report.subcommand_reply.data);
            break;
        }
    }

    // User calibration
    cmd.address = 0x8026;
    cmd.size = 26;
    write_to_hid(cmd);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = SwitchControllerReport(in_buf);
        if (report.report_type == SwitchControllerReport::InputReportType::REPORT_SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == SCMD_SPI_FLASH_READ) {
            print_hex(report.subcommand_reply.data, 35);
            uint32_t addr;
            uint8_t size;

            memcpy(&addr, report.subcommand_reply.data, sizeof(uint32_t));
            memcpy(&size, report.subcommand_reply.data + sizeof(uint32_t), sizeof(uint8_t));

            uint8_t data[size];
            memcpy(data, report.subcommand_reply.data + sizeof(uint32_t) + sizeof(uint8_t), size);

            uint16_t magic = *((uint16_t*)data);

            if (magic == 0xA1B2) {
                has_user_imu_calib = true;
                parse_imu_calibration(data + sizeof(uint16_t), &user_imu_calib);
            }

            break;
        }
    }
}

void SwitchController::request_color_data() {
    SPIFlashReadSubcommand cmd;
    cmd.address = SPI_COLOR_DATA;
    cmd.size = 12;
    write_to_hid(cmd);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = SwitchControllerReport(in_buf);
        if (report.report_type == SwitchControllerReport::InputReportType::REPORT_SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == SCMD_SPI_FLASH_READ) {
            handle_spi_flash_read(report.subcommand_reply.data);
            break;
        }
    }
}

void SwitchController::write_to_hid(SPIFlashReadSubcommand p_cmd) {
    uint8_t buf[0x40];
    p_cmd.build(buf, packet_num++);
    hid_write(handle, buf, 0x40);
}

void SwitchController::handle_request_device_info(uint8_t *p_data) {
    memcpy(&info, (SwitchDeviceInfo*)p_data, sizeof(SwitchDeviceInfo));
}

void SwitchController::handle_spi_flash_read(uint8_t *p_reply) {
	uint32_t addr;
	uint8_t size;

	memcpy(&addr, p_reply, sizeof(uint32_t));
	memcpy(&size, p_reply + sizeof(uint32_t), sizeof(uint8_t));

	uint8_t data[size];
	memcpy(data, p_reply + sizeof(uint32_t) + sizeof(uint8_t), size);
    
    switch (addr) {
        case 0x5000: // Shipment data
            break;
        case SPI_SERIAL_NO:
            break;
        case SPI_FACTORY_IMU_CALIBRATION:
            parse_imu_calibration(data, &imu_calib);
            break;
        case SPI_FACTORY_STICK_CALIBRATION:
            update_factory_stick_calibration(data, size);
            break;
        case SPI_COLOR_DATA:
            update_color_data(data, size);
            break;
        case 0x6086: // Stick device parameters 1
            break;
        case 0x6098: // Stick device parameters 2
            break;
        case SPI_USER_IMU_CALIBRATION:
            break;
        case SPI_USER_STICK_CALIBRATION:
            break;
        default:
            break;
        
    }
}

void SwitchController::parse_stick_calibration(uint8_t *p_raw_data, StickCalibrationData *p_dest, Stick p_stick) {
    uint16_t parsed_data[6];
    switch (p_stick) {
        case STICK_LEFT: {
            parsed_data[0] = (p_raw_data[1] << 8) & 0xF00 | p_raw_data[0];    // X axis max above center
            parsed_data[1] = (p_raw_data[2] << 4) | (p_raw_data[1] >> 4);     // Y axis max above center
            parsed_data[2] = (p_raw_data[4] << 8) & 0xF00 | p_raw_data[3];    // X axis center
            parsed_data[3] = (p_raw_data[5] << 4) | (p_raw_data[4] >> 4);     // Y axis center
            parsed_data[4] = (p_raw_data[7] << 8) & 0xF00 | p_raw_data[6];    // X axis min below center
            parsed_data[5] = (p_raw_data[8] << 4) | (p_raw_data[7] >> 4);     // Y axis min below center

            p_dest->x_min = parsed_data[2] - parsed_data[4];
            p_dest->x_center = parsed_data[2];
            p_dest->x_max = parsed_data[2] + parsed_data[0];

            p_dest->y_min = parsed_data[3] - parsed_data[5];
            p_dest->y_center = parsed_data[3];
            p_dest->y_max = parsed_data[3] + parsed_data[1];
        }
        case STICK_RIGHT: {
            parsed_data[0] = (p_raw_data[1] << 8) & 0xF00 | p_raw_data[0];    // X axis center
            parsed_data[1] = (p_raw_data[2] << 4) | (p_raw_data[1] >> 4);     // Y axis center
            parsed_data[2] = (p_raw_data[4] << 8) & 0xF00 | p_raw_data[3];    // X axis min below center
            parsed_data[3] = (p_raw_data[5] << 4) | (p_raw_data[4] >> 4);     // Y axis min below center
            parsed_data[4] = (p_raw_data[7] << 8) & 0xF00 | p_raw_data[6];    // X axis max above center
            parsed_data[5] = (p_raw_data[8] << 4) | (p_raw_data[7] >> 4);     // Y axis max above center

            p_dest->x_min = parsed_data[0] - parsed_data[2];
            p_dest->x_center = parsed_data[0];
            p_dest->x_max = parsed_data[0] + parsed_data[4];

            p_dest->y_min = parsed_data[1] - parsed_data[3];
            p_dest->y_center = parsed_data[1];
            p_dest->y_max = parsed_data[1] + parsed_data[5];
        }
    }
}

void SwitchController::update_factory_stick_calibration(uint8_t *p_raw_data, uint8_t p_size) {
	// https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/spi_flash_notes.md#analog-stick-factory-and-user-calibration
    if (p_size != 0x12) {
        printf("Size of data for stick calibration is %d but it should be %d\n", p_size, 0x12);
        return;
    }

    parse_stick_calibration(p_raw_data, &ls_calib, STICK_LEFT);
    parse_stick_calibration(p_raw_data + 9, &rs_calib, STICK_RIGHT);
}

void SwitchController::parse_imu_calibration(uint8_t *p_raw_data, IMUCalibrationData *p_dest) {
    memcpy(p_dest, p_raw_data, sizeof(IMUCalibrationData));
}

Vector2 SwitchController::get_stick(Stick p_stick) const {
    double x_raw, x_min, x_max;
    double y_raw, y_min, y_max;

    const StickCalibrationData *calib;
    switch (p_stick) {
        case STICK_LEFT:
            calib = has_user_ls_calib ? &user_ls_calib : &ls_calib;
            x_raw = static_cast<double>(report.ls_x);
            y_raw = static_cast<double>(report.ls_y);
            x_min = static_cast<double>(calib->x_min);
            x_max = static_cast<double>(calib->x_max);
            y_min = static_cast<double>(calib->y_min);
            y_max = static_cast<double>(calib->y_max);
            break;
        case STICK_RIGHT:
            calib = has_user_rs_calib ? &user_rs_calib : &rs_calib;
            x_raw = static_cast<double>(report.rs_x);
            y_raw = static_cast<double>(report.rs_y);
            x_min = static_cast<double>(rs_calib.x_min);
            x_max = static_cast<double>(calib->x_max);
            y_min = static_cast<double>(calib->y_min);
            y_max = static_cast<double>(calib->y_max);
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
    return accumulated_gyro;
}

void SwitchController::update_color_data(uint8_t *p_data, uint8_t p_size) {
    if (p_size != 0x0C) {
        printf("Color data size was 0x%X but should have been 0x0B\n", p_size);
        return;
    }
    memcpy(&colors, (SwitchControllerColors*)p_data, sizeof(SwitchControllerColors));
}

Color24 SwitchController::get_color(ColorRole p_role) const {
    switch (p_role) {
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

void SwitchController::set_player_lights(PlayerLight p_p1, PlayerLight p_p2, PlayerLight p_p3, PlayerLight p_p4) {
    uint8_t buf[0x40];
    bzero(buf, 0x40);
    buf[0] = 1;
    buf[1] = packet_num++;
    buf[10] = SCMD_SET_PLAYER_LIGHTS;

    buf[11] = (p_p1) | (p_p2 << 1) | (p_p3 << 2) | (p_p4 << 3);
    hid_write(handle, buf, 0x40);
}
}
