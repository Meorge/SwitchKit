#ifndef JOYCON_REPORT_H
#define JOYCON_REPORT_H

#include <stdio.h>
#include <cstdint>
#include <cstring>

#include "subcommand.h"
#include "battery_level.h"
#include "imu.h"

enum InputReportMode {
    ACTIVE_NFC_IR_POLLING = 0x00, // Used with command 0x11; 0x31 data format must be set first
    ACTIVE_NFC_IR_POLLING_CONFIG = 0x01,
    ACTIVE_NFC_IR_POLLING_DATA_CONFIG = 0x02,
    ACTIVE_IR_CAMERA_POLLING = 0x03,
    MCU_UPDATE_STATE_REPORT_UNK = 0x23,
    STANDARD = 0x30,
    NFC_IR = 0x31,
    UNKNOWN_1 = 0x33,
    UNKNOWN_2 = 0x35,
    BASIC = 0x3F
};

struct JoyConReport
{
    enum InputReportType
    {
        NONE = 0x00,
        BASIC = 0x3F,
        SUBCOMMAND_REPLY = 0x21,
        NFC_IR_MCU_FR_UPDATE_INPUT_REPORT = 0x23,
        STANDARD = 0x30,
        STANDARD_WITH_NFC_IR_MCU = 0x31,
        UNKNOWN_1 = 0x32,
        UNKNOWN_2 = 0x33
    };

    InputReportType report_type = InputReportType::NONE;
    char timer_value = 0;

    bool is_charging = false;
    BatteryLevel battery_level = BatteryLevel::EMPTY;

    bool btn_y = false;
    bool btn_x = false;
    bool btn_b = false;
    bool btn_a = false;

    bool btn_sr = false;
    bool btn_sl = false;
    bool btn_r = false;
    bool btn_zr = false;

    // Shared buttons between L and R
    bool btn_minus = false;
    bool btn_plus = false;
    bool btn_rstick = false;
    bool btn_lstick = false;
    bool btn_home = false;
    bool btn_capture = false;
    bool btn_charginggrip = false;

    // Buttons only on Joy-Con (L)
    bool btn_down = false;
    bool btn_up = false;
    bool btn_right = false;
    bool btn_left = false;

    bool btn_l = false;
    bool btn_zl = false;

    uint16_t ls_x = 0;
    uint16_t ls_y = 0;

    uint16_t rs_x = 0;
    uint16_t rs_y = 0;

    struct SubcommandReply {
        uint8_t reply_to = 0;
        uint8_t data[35];
    };

    struct MCUReport {
        uint8_t data[37];
    };

    // struct IMUReport {
    //     // "3 frames of 2 groups of 3 int16 LE each, acc followed by gyro"
    //     uint8_t data[36];
    // };

    struct NFCIRReport {
        uint8_t data[313];
    };

    // TODO: put this in a union for efficiency
    SubcommandReply subcommand_reply; // x21
    MCUReport mcu_report; // x23
    IMUPacket imu_packets[3]; // x30, x31, x32, x33
    NFCIRReport nfc_ir_report; // x31
    

    JoyConReport() {}

    JoyConReport(unsigned char *buf)
    {
        report_type = static_cast<InputReportType>(buf[0]);
        timer_value = buf[1];

        is_charging = ((buf[2] & 0x0f) & 0b0001) == 1;
        battery_level = static_cast<BatteryLevel>((buf[2] & 0x0F) & 0b1110);

        char r = buf[3];
        btn_y = (r & 0x01);
        btn_x = (r & 0x02);
        btn_b = (r & 0x04);
        btn_a = (r & 0x08);
        btn_sr = (r & 0x10);
        btn_sl = (r & 0x20);
        btn_r = (r & 0x40);
        btn_zr = (r & 0x80);

        char s = buf[4];
        btn_minus = (s & 0x01);
        btn_plus = (s & 0x02);
        btn_rstick = (s & 0x04);
        btn_lstick = (s & 0x08);
        btn_home = (s & 0x10);
        btn_capture = (s & 0x20);
        btn_charginggrip = (s & 0x80);

        char l = buf[5];
        btn_down = (l & 0x01);
        btn_up = (l & 0x02);
        btn_right = (l & 0x04);
        btn_left = (l & 0x08);
        btn_sr = (l & 0x10);
        btn_sl = (l & 0x20);
        btn_l = (l & 0x40);
        btn_zl = (l & 0x80);

        // Left stick
        unsigned char *left_stick_data = buf + 6;
        ls_x = left_stick_data[0] | ((left_stick_data[1] & 0xF) << 8);
        ls_y = (left_stick_data[1] >> 4) | (left_stick_data[2] << 4);

        // Right stick
        unsigned char *right_stick_data = buf + 9;
        rs_x = right_stick_data[0] | ((right_stick_data[1] & 0xF) << 8);
        rs_y = (right_stick_data[1] >> 4) | (right_stick_data[2] << 4);

        if (report_type == SUBCOMMAND_REPLY && buf[13] >> 7) {
            // Subcommand was acknowledged.
            // If data type is 0x00, it's a simple ACK.
            bool ack = buf[13] >> 7;
            auto data_type = buf[13] & 0x7F;

            subcommand_reply.reply_to = buf[14];
            memcpy(subcommand_reply.data, buf + 15, 35);
        }

        else if (report_type == STANDARD) {
            // Decode 6-axis data
            memcpy(imu_packets, buf + 13, sizeof(IMUPacket) * 3);

            // for (int i = 0; i < 3; i++) {
            //     Vector3 ac = get_accel(imu_packets[i]);
            //     Vector3 gy = get_rotation_deg(imu_packets[i]);
            //     printf("Packet %d\n", i);
            //     printf("Accel: (%.2lf, %.2lf, %.2lf)\n", ac.x, ac.y, ac.z);
            //     printf("Gyro: (%.2lf, %.2lf, %.2lf)\n", gy.x, gy.y, gy.z);
            //     printf("----\n");
            // }
            // printf("----\n");

            // for (int i = 0; i < 3; i++) {
            //     printf("Packet %d\n", i);
            //     printf("Accel: (%d, %d, %d)\n", imu_packets[i].accel_x, imu_packets[i].accel_y, imu_packets[i].accel_z);
            //     printf("Gyro: (%d, %d, %d)\n", imu_packets[i].gyro_1, imu_packets[i].gyro_2, imu_packets[i].gyro_3);
            //     printf("----\n");
            // }
            // printf("----\n");

            // for (int i = 0; i < sizeof(IMUPacket) * 3; i++) {
            //     if (i % sizeof(IMUPacket) == 0) {
            //         printf("| ");
            //     }
            //     printf("%02X ", *(buf + 13 + i));
            // }
            // printf("\n");


            IMUPacket packet3 = imu_packets[2];
            // Get this in u8's
            uint8_t *uint8s = (uint8_t*)&packet3;
            uint8_t datas[2] { uint8s[3], uint8s[2]};
            uint16_t *together = (uint16_t*)datas;
            // printf("Maybe Ring-Con data?? %d\n", *together);
            // printf("%x %x\n", uint8s[2], uint8s[3]);
            
        }
    }
};

#endif // JOYCON_REPORT_H
