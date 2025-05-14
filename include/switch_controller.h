#ifndef SWITCH_CONTROLLER_H
#define SWITCH_CONTROLLER_H

#include <stdio.h>
#include <cstdint>
#include <cstring>
#include <hidapi/hidapi.h>

#include "colors.h"
#include "joycon_report.h"
#include "stick_calibration_data.h"
#include "subcommand.h"
#include "vectors.h"
#include "rumble.h"

namespace SwitchKit {
enum SPIFlashReadAddress {
    SPI_SERIAL_NO = 0x6000,
    SPI_DEVICE_TYPE = 0x6012,
    SPI_COLOR_INFO_EXISTS = 0x601B,
    SPI_FACTORY_IMU_CALIBRATION = 0x6020,
    SPI_FACTORY_STICK_CALIBRATION = 0x603D,
    SPI_COLOR_DATA = 0x6050,
    SPI_USER_STICK_CALIBRATION = 0x8010,
    SPI_USER_IMU_CALIBRATION = 0x8026
};

enum SwitchControllerType: uint8_t {
    CONTROLLER_L = 1,
    CONTROLLER_R = 2,
    CONTROLLER_PRO = 3
};

struct SwitchDeviceInfo {
    uint8_t firmware_major;
    uint8_t firmware_minor;

    SwitchControllerType type;
    uint8_t unknown_1;
    uint8_t mac_address[6];
    uint8_t unknown_2;
    bool custom_colors;  
};

class SwitchController
{
    hid_device *handle;
    StickCalibrationData ls_calib;
    StickCalibrationData rs_calib;

    bool has_user_ls_calib = false;
    StickCalibrationData user_ls_calib;
    bool has_user_rs_calib = false;
    StickCalibrationData user_rs_calib;

    IMUCalibrationData imu_calib;

    bool has_user_imu_calib = false;
    IMUCalibrationData user_imu_calib;

    Vector3 gyro_cal_coeff = Vector3(0,0,0);
    uint8_t buf[361];

    SwitchDeviceInfo info;
    SwitchControllerColors colors;

    SwitchControllerReport prev_report;
    SwitchControllerReport report;

    uint8_t packet_num = 0;

    bool ringcon_enabled = false;

public:
    BatteryLevel get_battery_level() const { return report.battery_level; }
    bool get_battery_charging() const { return report.is_charging; }
    SwitchControllerType get_controller_type() const { return info.type; }

    bool get_button_pressed_this_frame(SwitchControllerReport::Button p_button) {
        return report.get_button(p_button) && !prev_report.get_button(p_button);
    }
    bool get_button(SwitchControllerReport::Button p_button) { return report.get_button(p_button); }

    enum Stick
    {
        STICK_LEFT,
        STICK_RIGHT
    };
    Vector2 get_stick(Stick p_stick) const;

    Vector3 get_accel() const;
    Vector3 get_gyro() const;

    void set_input_report_mode(InputReportMode p_mode);

    enum HCIState: uint8_t {
        HCI_OFF = 0x00,
        HCI_RECONNECT = 0x01,
        HCI_PAIR = 0x02,
        HCI_RECONNECT_2 = 0x04
    };
    void set_hci_state(HCIState p_state);
    void set_imu_enabled(bool p_enabled);
    void set_mcu_enabled(bool p_enabled);
    void configure_mcu(uint8_t p_command, uint8_t p_subcommand, uint8_t p_mode);

    enum ExternalDevice {
        EXT_NONE = 0x0,
        EXT_RINGCON = 0x2000,
        EXT_STARLINK = 0x2800
    };
    ExternalDevice get_external_device_id();
    void set_external_format_config(uint8_t *p_data, uint8_t p_size);
    void enable_external_polling(uint8_t *p_data, uint8_t p_size);

    void enable_ringcon();
    void disable_ringcon();
    bool get_ringcon_connected();
    bool get_ringcon_enabled() const { return ringcon_enabled; }
    double get_ringcon_flex();

    void request_device_info();
    void request_stick_calibration();
    void request_imu_calibration();
    void request_color_data();
    void poll();

    void rumble(const HDRumbleConfig &p_config);

    enum PlayerLight {
        LIGHT_OFF,
        LIGHT_ON = 0x01,
        LIGHT_FLASH = 0x10
    };

    void set_player_lights(PlayerLight p_p1, PlayerLight p_p2, PlayerLight p_p3, PlayerLight p_p4);

    enum ColorRole {
        COLOR_BODY,
        COLOR_BUTTON,
        COLOR_LEFT_GRIP,
        COLOR_RIGHT_GRIP
    };
    Color24 get_color(ColorRole p_role) const;

    void write_to_hid(SPIFlashReadSubcommand p_cmd);

    

    SwitchController(hid_device *p_handle);
    ~SwitchController();

private:
    void handle_request_device_info(uint8_t *p_data);
    void handle_spi_flash_read(uint8_t *p_reply);
    void update_factory_stick_calibration(uint8_t *p_stick_cal, uint8_t p_size);
    void parse_stick_calibration(uint8_t *p_raw_data, StickCalibrationData *p_dest, Stick p_stick);
    void parse_imu_calibration(uint8_t *p_raw_data, IMUCalibrationData *p_dest);
    void update_color_data(uint8_t *p_data, uint8_t p_size);
};
}

#endif // SWITCH_CONTROLLER_H
