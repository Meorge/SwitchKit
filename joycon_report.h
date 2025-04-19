#ifndef JOYCON_REPORT_H
#define JOYCON_REPORT_H

#include <stdio.h>
#include <cstdint>
#include <cstring>

#include "subcommand.h"
#include "battery_level.h"

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

    uint8_t subcommand_reply_to = 0;
    uint8_t subcommand_reply[35];

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
            auto data_type = buf[13] & 0x7F;

            subcommand_reply_to = buf[14];
            memcpy(subcommand_reply, buf + 15, 35);
        }

        // printf("In report, ls_x is %d\n", ls_x);
    }
};

#endif // JOYCON_REPORT_H