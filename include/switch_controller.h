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

enum SPIFlashReadAddress {
    SERIAL_NO = 0x6000,
    DEVICE_TYPE = 0x6012,
    COLOR_INFO_EXISTS = 0x601B,
    FACTORY_IMU_CALIBRATION = 0x6020,
    FACTORY_STICK_CALIBRATION = 0x603D,
    COLOR_DATA = 0x6050,
    USER_STICK_CALIBRATION = 0x8010,
    USER_IMU_CALIBRATION = 0x8026
};

enum SwitchControllerType: uint8_t {
    JOY_CON_L = 1,
    JOY_CON_R = 2,
    PRO_CONTROLLER = 3
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
    uint8_t buf[361];

    SwitchDeviceInfo info;
    SwitchControllerColors colors;

    JoyConReport report;

    uint8_t packet_num = 0;

public:
    enum Stick
    {
        LEFT,
        RIGHT
    };
    void set_input_report_mode(InputReportMode mode);
    void set_imu_enabled(bool enabled);
    void set_mcu_enabled(bool enabled);
    void configure_mcu(uint8_t command, uint8_t subcommand, uint8_t mode);
    uint16_t get_external_device_id();
    void set_external_format_config(uint8_t *data);
    void enable_external_polling(uint8_t *data);

    void enable_ringcon();

    double get_ringcon_flex();

    void request_device_info();
    void request_stick_calibration();
    void request_color_data();
    void poll();

    void rumble(const HDRumbleConfig &p_config);

    enum PlayerLight {
        OFF,
        ON = 0x01,
        FLASH = 0x10
    };

    void set_player_lights(PlayerLight p1, PlayerLight p2, PlayerLight p3, PlayerLight p4);

    void write_to_hid(SPIFlashReadSubcommand cmd);

    Vector2 get_stick(Stick stick) const;

    SwitchController(hid_device *p_handle);
    ~SwitchController();

private:
    void handle_request_device_info(uint8_t *data);
    void handle_spi_flash_read(uint8_t *reply);
    void update_stick_calibration(uint8_t *stick_cal, uint8_t size);
    void update_color_data(uint8_t *data, uint8_t size);
};

#endif // SWITCH_CONTROLLER_H
