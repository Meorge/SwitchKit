#ifndef JOYCON_REPORT_H
#define JOYCON_REPORT_H

#include "subcommand.h"
#include "battery_level.h"
#include "imu.h"

namespace SwitchKit {
enum InputReportMode {
    MODE_ACTIVE_NFC_IR_POLLING = 0x00, // Used with command 0x11; 0x31 data format must be set first
    MODE_ACTIVE_NFC_IR_POLLING_CONFIG = 0x01,
    MODE_ACTIVE_NFC_IR_POLLING_DATA_CONFIG = 0x02,
    MODE_ACTIVE_IR_CAMERA_POLLING = 0x03,
    MODE_MCU_UPDATE_STATE_REPORT_UNK = 0x23,
    MODE_STANDARD = 0x30,
    MODE_NFC_IR = 0x31,
    BASIC = 0x3F
};

struct SwitchControllerReport
{
    enum InputReportType
    {
        REPORT_NONE = 0x00,
        REPORT_BASIC = 0x3F,
        REPORT_SUBCOMMAND_REPLY = 0x21,
        REPORT_NFC_IR_MCU_FR_UPDATE_INPUT_REPORT = 0x23,
        REPORT_STANDARD = 0x30,
        REPORT_STANDARD_WITH_NFC_IR_MCU = 0x31,
    };

    InputReportType report_type = InputReportType::REPORT_NONE;
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

    struct NFCIRReport {
        uint8_t data[313];
    };

    // TODO: put this in a union for efficiency
    SubcommandReply subcommand_reply; // x21
    MCUReport mcu_report; // x23
    IMUPacket imu_packets[3]; // x30, x31, x32, x33
    NFCIRReport nfc_ir_report; // x31
    
    SwitchControllerReport() {}
    SwitchControllerReport(unsigned char *buf);

public:
    enum Button {
        BTN_A,
        BTN_B,
        BTN_X,
        BTN_Y,

        BTN_SR,
        BTN_SL,
        BTN_R,
        BTN_ZR,

        BTN_MINUS,
        BTN_PLUS,
        BTN_STICK_R,
        BTN_STICK_L,
        BTN_HOME,
        BTN_CAPTURE,
        BTN_CHARGING_GRIP,

        BTN_DOWN,
        BTN_UP,
        BTN_RIGHT,
        BTN_LEFT,
        BTN_L,
        BTN_ZL
    };
    bool get_button(Button button);
};
}

#endif // JOYCON_REPORT_H
