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

void SwitchController::enable_ringcon() {
    uint8_t buf[0x40];
    bzero(buf, 0x40);
    buf[0] = 1;
    
    // Step 1 - Set MCU state to standby, wait for its response
    printf("Setting MCU state to standby...\n");
    bzero(buf, 0x40);
    buf[0] = 1;
    buf[1] = packet_num++;
    buf[10] = SET_NFC_IR_MCU_STATE;
    buf[11] = 0x01; // Resume/standby mode
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


    // Step 2 - Set MCU mode to MaybeRingcon, wait to get its response and see that state report state to be MaybeRingcon
    while (true) {
        bzero(buf, 0x40);
        buf[0] = 1;
        buf[1] = packet_num++;
        buf[10] = SET_NFC_IR_MCU_CONFIG; // Set NFC/IR MCU configuration
        buf[11] = 0x00; // Set MCU mode
        buf[12] = 0x03; // MaybeRingcon
        hid_write(handle, buf, 0x40);

        // TODO: I think the next thing to try is that damn CRC table thing.
        // It seems like I'm close, but I'm just not getting it quite right yet,
        // and the CRC table seems to be the last big discrepancy.

        bool should_break = false;
        while (true) {
            uint8_t in_buf[361];
            hid_read(handle, in_buf, 361);
            report = JoyConReport(in_buf);
            if (report.report_type == JoyConReport::InputReportType::SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == SET_NFC_IR_MCU_CONFIG) {
                printf("Set MCU config to MaybeRingcon received!\n");

                // See if it's a state report - waiting for 0x01
                printf("MCU report ID is 0x%02X\n", report.subcommand_reply.data[0]);
                if (report.subcommand_reply.data[0] == 0x01) {
                    printf("Got the state report\n");

                    // TODO: check if it's mayberingcon
                    should_break = true;
                    break;
                }
            }
        }

        if (should_break) break;
    }

    // NOTE: not doing that CRC stuff yet, that's scary

    // Step 3 - Configure MCU IR data, wait for its response
    bzero(buf, 0x40);
    buf[0] = 1;
    buf[1] = packet_num++;
    buf[10] = SET_NFC_IR_MCU_CONFIG; // Set NFC/IR MCU configuration
    buf[11] = 0x01; // Set IR mode
    buf[12] = 0x01; // Mode is sensor sleep
    buf[13] = 0; // 0 packets to output per buffer
    // MCU FW version is 0 - we already zeroed out the buffer so this should be done??

    hid_write(handle, buf, 0x40);
    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = JoyConReport(in_buf);
        if (report.report_type == JoyConReport::InputReportType::SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == SET_NFC_IR_MCU_CONFIG) {
            printf("Configure MCU IR command received!\n");
            break;
        }
    }

    // Step 4 - Call subcommand 0x59, wait for its response
    bzero(buf, 0x40);
    buf[0] = 1;
    buf[1] = packet_num++;
    buf[10] = 0x59; // Command but we don't know what it is... but it works maybe!?
    hid_write(handle, buf, 0x40);
    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = JoyConReport(in_buf);
        if (report.report_type == JoyConReport::InputReportType::SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == 0x59) {
            printf("Mysterious 0x59 command received!\n");
            break;
        }
    }

    // Step 5 - Set IMU mode to MaybeRingcon, wait for its response
    bzero(buf, 0x40);
    buf[0] = 1;
    buf[1] = packet_num++;
    buf[10] = ENABLE_IMU; // Enable IMU command
    buf[11] = 0x03; // Maybe Ring-Con
    hid_write(handle, buf, 0x40);
    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = JoyConReport(in_buf);
        if (report.report_type == JoyConReport::InputReportType::SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == ENABLE_IMU) {
            printf("Enable IMU command received!\n");
            break;
        }
    }

    // Step 6 - Call subcommand 0x5c_6, wait for its response
    bzero(buf, 0x40);
    buf[0] = 1;
    buf[1] = packet_num++;
    buf[10] = 0x5C;

    uint8_t stuffs[] = { 6, 3, 37, 6, 0, 0, 0, 0, 236, 153, 172, 227, 28, 0, 0, 0, 105, 155, 22, 246, 93, 86, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 144, 40, 161, 227, 28, 0 };
    memcpy(buf + 11, stuffs, sizeof(stuffs));

    hid_write(handle, buf, 0x40);
    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = JoyConReport(in_buf);
        if (report.report_type == JoyConReport::InputReportType::SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == 0x5C) {
            printf("Mysterious 0x5C command received!\n");
            break;
        }
    }

    // Step 7 - Call subcommand 0x5a, wait for its response
    bzero(buf, 0x40);
    buf[0] = 1;
    buf[1] = packet_num++;
    buf[10] = 0x5A;

    uint8_t stuffs2[] = { 4, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
    memcpy(buf + 11, stuffs2, sizeof(stuffs2));

    hid_write(handle, buf, 0x40);
    while (true) {
        uint8_t in_buf[361];
        hid_read(handle, in_buf, 361);
        report = JoyConReport(in_buf);
        if (report.report_type == JoyConReport::InputReportType::SUBCOMMAND_REPLY && report.subcommand_reply.reply_to == 0x5A) {
            printf("Mysterious 0x5A command received!\n");
            break;
        }
    }
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