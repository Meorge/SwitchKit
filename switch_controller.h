#ifndef SWITCH_CONTROLLER_H
#define SWITCH_CONTROLLER_H

#include <stdio.h>
#include <cstdint>
#include <cstring>
#include <hidapi/hidapi.h>

#include "joycon_report.h"
#include "stick_calibration_data.h"
#include "subcommand.h"
#include "vector2.h"

#define SPI_ADDR_STICK_CALIB 0x603D
#define SPI_LEN_STICK_CALIB 0x12

class SwitchProController
{
    hid_device *handle;
    StickCalibrationData ls_calib;
    StickCalibrationData rs_calib;
    uint8_t buf[361];

    JoyConReport report;

    uint8_t packet_num = 0;

public:
    enum Stick
    {
        LEFT,
        RIGHT
    };
    void request_stick_calibration();
    void poll();

    Vector2 get_stick(Stick stick) const;

    SwitchProController();
    ~SwitchProController();

private:
    void handle_spi_flash_read(uint8_t *reply);
    void update_stick_calibration(uint8_t *stick_cal);
};

#endif // SWITCH_CONTROLLER_H