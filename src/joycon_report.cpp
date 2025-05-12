#include "joycon_report.h"

#include <stdio.h>
#include <cstdint>
#include <cstring>

namespace SwitchKit {
SwitchControllerReport::SwitchControllerReport(unsigned char *buf)
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

    if (report_type == REPORT_SUBCOMMAND_REPLY && buf[13] >> 7) {
        // Subcommand was acknowledged.
        // If data type is 0x00, it's a simple ACK.
        bool ack = buf[13] >> 7;
        auto data_type = buf[13] & 0x7F;

        subcommand_reply.reply_to = buf[14];
        memcpy(subcommand_reply.data, buf + 15, 35);
    }

    else if (report_type == REPORT_STANDARD) {
        // Unpack 6-axis data
        memcpy(imu_packets, buf + 13, sizeof(IMUPacket) * 3);
    }
}

bool SwitchControllerReport::get_button(Button button) {
    switch (button) {
        case BTN_A: return btn_a;
        case BTN_B: return btn_b;
        case BTN_X: return btn_x;
        case BTN_Y: return btn_y;
        case BTN_SR: return btn_sr;
        case BTN_SL: return btn_sl;
        case BTN_R: return btn_r;
        case BTN_ZR: return btn_zr;

        case BTN_MINUS: return btn_minus;
        case BTN_PLUS: return btn_plus;
        case BTN_STICK_R: return btn_rstick;
        case BTN_STICK_L: return btn_lstick;
        case BTN_HOME: return btn_home;
        case BTN_CAPTURE: return btn_capture;
        case BTN_CHARGING_GRIP: return btn_charginggrip;
        
        case BTN_DOWN: return btn_down;
        case BTN_UP: return btn_up;
        case BTN_RIGHT: return btn_right;
        case BTN_LEFT: return btn_left;
        case BTN_L: return btn_l;
        case BTN_ZL: return btn_zl;
    }
}
}