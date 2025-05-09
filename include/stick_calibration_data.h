#ifndef STICK_CALIBRATION_DATA_H
#define STICK_CALIBRATION_DATA_H

#include <cstdint>

namespace SwitchKit {
struct StickCalibrationData {
    uint16_t x_min;
    uint16_t x_center;
    uint16_t x_max;

    uint16_t y_min;
    uint16_t y_center;
    uint16_t y_max;
};
}

#endif // STICK_CALIBRATION_DATA_H
