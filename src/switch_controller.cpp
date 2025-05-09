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

    switch (report.report_type) {
        case JoyConReport::InputReportType::SUBCOMMAND_REPLY:
        {
            if (report.subcommand_reply.reply_to != 0) {
                printf("Report subcommand reply is 0x%02X\n", report.subcommand_reply.reply_to);
            }
        }
        case JoyConReport::InputReportType::STANDARD:
        {
        }
    }
}

void SwitchController::request_device_info() {
    uint8_t buf[0x40];
    bzero(buf, 0x40);
    buf[0] = 1;
    buf[1] = packet_num++;
    buf[10] = REQUEST_DEVICE_INFO;
    hid_write(handle, buf, 0x40);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = JoyConReport(in_buf);
        if (report.report_type == JoyConReport::InputReportType::SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == REQUEST_DEVICE_INFO) {
            printf("Device info received!\n");
            handle_request_device_info(report.subcommand_reply.data);
            break;
        }
    }
}

void SwitchController::set_input_report_mode(InputReportMode mode) {
    uint8_t buf[0x40];
    SetInputModeSubcommand cmd;
    cmd.mode = mode;
    cmd.to_buf(buf, packet_num++);
    hid_write(handle, buf, 0x40); // TODO: Make write_to_hid work for different commands
    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = JoyConReport(in_buf);
        if (report.report_type == JoyConReport::InputReportType::SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == SET_INPUT_REPORT_MODE) {
            printf("Set input report mode received!\n");
            break;
        }
    }
}

void SwitchController::set_imu_enabled(bool enabled) {
    uint8_t buf[0x40];
    bzero(buf, 0x40);
    buf[0] = 1;
    buf[1] = packet_num++;

    buf[10] = ENABLE_IMU;
    buf[11] = enabled ? 0x01 : 0x00;
    hid_write(handle, buf, 0x40);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = JoyConReport(in_buf);
        if (report.report_type == JoyConReport::InputReportType::SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == ENABLE_IMU) {
            printf("Enable IMU received!\n");
            break;
        }
    }
}

void SwitchController::set_mcu_enabled(bool enabled) {
    printf("Enabling MCU...\n");
    bzero(buf, 0x40);
    buf[0] = 1;
    buf[1] = packet_num++;
    buf[10] = SET_NFC_IR_MCU_STATE;
    buf[11] = enabled ? 0x01 : 0x00; // Resume/standby mode
    hid_write(handle, buf, 0x40);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = JoyConReport(in_buf);
        if (report.report_type == JoyConReport::InputReportType::SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == SET_NFC_IR_MCU_STATE) {
            printf("Set MCU state received!\n");
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

uint8_t calc_crc8(uint8_t *data, uint8_t size) {
    uint8_t crc = 0;
    for (int i = 0; i < size; i++) {
        crc = MCU_CRC8_TABLE[(uint8_t)(crc ^ data[i])];
    }
    return crc;
}

void SwitchController::configure_mcu(uint8_t command, uint8_t subcommand, uint8_t mode) {
    uint8_t buf[0x40];
    bzero(buf, 0x40);
    buf[0] = 1;
    buf[1] = packet_num++;
    buf[10] = SET_NFC_IR_MCU_CONFIG;

    uint8_t subcommand_data[38];
    bzero(subcommand_data, 38);
    subcommand_data[0] = command;
    subcommand_data[1] = subcommand;
    subcommand_data[2] = mode;
    subcommand_data[37] = calc_crc8(subcommand_data + 1, 36);
    memcpy(buf + 11, subcommand_data, sizeof(subcommand_data));
    hid_write(handle, buf, 0x40);

    printf("Attempting to configure MCU...\n");
    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = JoyConReport(in_buf);
        if (report.report_type == JoyConReport::InputReportType::SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == SET_NFC_IR_MCU_CONFIG) {
            printf("Configure MCU response received!\n");
            for (int i = 0; i < 35; i++) {
                printf("%02X ", report.subcommand_reply.data[i]);
            }
            printf("\n");
            break;
        }
    }
}

uint16_t SwitchController::get_external_device_id() {
    printf("Checking if Ring-Con is connected...\n");
    bzero(buf, 0x40);
    buf[0] = 1;
    buf[1] = packet_num++;
    buf[10] = 0x59; // GET_EXTERNAL_DEVICE_INFO
    hid_write(handle, buf, 0x40);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = JoyConReport(in_buf);
        if (report.report_type == JoyConReport::InputReportType::SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == 0x59) {
            printf("Response on external device info received!\n");
            return *((uint16_t*)report.subcommand_reply.data);
        }
    }
}

void SwitchController::set_external_format_config(uint8_t *data) {
    uint8_t buf[0x40];
    bzero(buf, 0x40);
    buf[0] = 1;
    buf[1] = packet_num++;
    buf[10] = 0x5C; // SET_EXTERNAL_FORMAT_CONFIG
    memcpy(buf + 11, data, sizeof(data));
    hid_write(handle, buf, 0x40);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = JoyConReport(in_buf);
        if (report.report_type == JoyConReport::InputReportType::SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == 0x5C) {
            break;
        }
    }
}

void SwitchController::enable_external_polling(uint8_t *data) {
    uint8_t buf[0x40];
    bzero(buf, 0x40);
    buf[0] = 1;
    buf[1] = packet_num++;
    buf[10] = 0x5A; // ENABLE_EXTERNAL_POLLING
    memcpy(buf + 11, data, sizeof(data));
    hid_write(handle, buf, 0x40);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = JoyConReport(in_buf);
        if (report.report_type == JoyConReport::InputReportType::SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == 0x5A) {
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
    printf("External device connected: 0x%04X\n", external_id);

    // Send SET_EXTERNAL_FORMAT_CONFIG with data
    uint8_t ext_format_config[] = { 0x06, 0x03, 0x25, 0x06, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x16, 0xED, 0x34, 0x36,
        0x00, 0x00, 0x00, 0x0A, 0x64, 0x0B, 0xE6, 0xA9, 0x22, 0x00, 0x00, 0x04, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0xA8, 0xE1, 0x34, 0x36 };
    set_external_format_config(ext_format_config);

    // Set ENABLE_EXTERNAL_POLLING with data
    uint8_t enable_external_polling_data[] = { 0x04, 0x01, 0x01, 0x02 };
    enable_external_polling(enable_external_polling_data);
}

double SwitchController::get_ringcon_flex() {
    double low_bound = 2280.0 - 2500.0;
    double high_bound = 2280.0 + 2500.0;
    double current = (double)report.imu_packets[2].accel_y;
    return INV_LERP(low_bound, high_bound, current);
}

void SwitchController::request_stick_calibration() {
	SPIFlashReadSubcommand cmd;
	cmd.address = FACTORY_STICK_CALIBRATION;
	cmd.size = 0x12;
    write_to_hid(cmd);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = JoyConReport(in_buf);
        if (report.report_type == JoyConReport::InputReportType::SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == SPI_FLASH_READ) {
            printf("Stick calibration data(?) received!\n");
            handle_spi_flash_read(report.subcommand_reply.data);
            break;
        }
    }
}

void SwitchController::request_color_data() {
    SPIFlashReadSubcommand cmd;
    cmd.address = COLOR_DATA;
    cmd.size = 0x605B - COLOR_DATA;
    write_to_hid(cmd);

    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = JoyConReport(in_buf);
        if (report.report_type == JoyConReport::InputReportType::SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == SPI_FLASH_READ) {
            printf("Color data(?) received!\n");
            handle_spi_flash_read(report.subcommand_reply.data);
            break;
        }
    }
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

void SwitchController::set_player_lights(PlayerLight p1, PlayerLight p2, PlayerLight p3, PlayerLight p4) {
    uint8_t buf[0x40];
    bzero(buf, 0x40);
    buf[0] = 1;
    buf[1] = packet_num++;
    buf[10] = SET_PLAYER_LIGHTS;

    buf[11] = (p1) | (p2 << 1) | (p3 << 2) | (p4 << 3);
    hid_write(handle, buf, 0x40);
}